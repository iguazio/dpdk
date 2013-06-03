/*-
 *   BSD LICENSE
 * 
 *   Copyright(c) 2010-2013 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/queue.h>

#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_tailq.h>
#include <rte_eal.h>
#include <rte_eal_memconfig.h>
#include <rte_launch.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_common.h>
#include <rte_string_fns.h>
#include <rte_spinlock.h>
#include <rte_memcpy.h>
#include <rte_atomic.h>

#include "malloc_elem.h"
#include "malloc_heap.h"

/* since the memzone size starts with a digit, it will appear unquoted in
 * rte_config.h, so quote it so it can be passed to rte_str_to_size */
#define MALLOC_MEMZONE_SIZE RTE_STR(RTE_MALLOC_MEMZONE_SIZE)

/*
 * returns the configuration setting for the memzone size as a size_t value
 */
static inline size_t
get_malloc_memzone_size(void)
{
	return rte_str_to_size(MALLOC_MEMZONE_SIZE);
}

/*
 * reserve an extra memory zone and make it available for use by a particular
 * heap. This reserves the zone and sets a dummy malloc_elem header at the end
 * to prevent overflow. The rest of the zone is added to free list as a single
 * large free block
 */
static int
malloc_heap_add_memzone(struct malloc_heap *heap, size_t size, unsigned align)
{
	const unsigned mz_flags = 0;
	const size_t min_size = get_malloc_memzone_size();
	/* ensure the data we want to allocate will fit in the memzone */
	size_t mz_size = size + align + MALLOC_ELEM_OVERHEAD * 2;
	if (mz_size < min_size)
		mz_size = min_size;

	char mz_name[RTE_MEMZONE_NAMESIZE];
	rte_snprintf(mz_name, sizeof(mz_name), "MALLOC_S%u_HEAP_%u",
			heap->numa_socket, heap->mz_count++);
	const struct rte_memzone *mz = rte_memzone_reserve(mz_name, mz_size,
			heap->numa_socket, mz_flags);
	if (mz == NULL)
		return -1;

	/* allocate the memory block headers, one at end, one at start */
	struct malloc_elem *start_elem = (struct malloc_elem *)mz->addr;
	struct malloc_elem *end_elem = RTE_PTR_ADD(mz->addr,
			mz_size - MALLOC_ELEM_OVERHEAD);
	end_elem = RTE_PTR_ALIGN_FLOOR(end_elem, CACHE_LINE_SIZE);

	const unsigned elem_size = (uintptr_t)end_elem - (uintptr_t)start_elem;
	malloc_elem_init(start_elem, heap, elem_size);
	malloc_elem_mkend(end_elem, start_elem);

	start_elem->next_free = heap->free_head;
	heap->free_head = start_elem;
	/* increase heap total size by size of new memzone */
	heap->total_size+=mz_size - MALLOC_ELEM_OVERHEAD;
	return 0;
}

/*
 * initialise a malloc heap object. The heap is locked with a private
 * lock while being initialised. This function should only be called the
 * first time a thread calls malloc - if even then, as heaps are per-socket
 * not per-thread.
 */
static void
malloc_heap_init(struct malloc_heap *heap)
{
	struct rte_mem_config *mcfg = rte_eal_get_configuration()->mem_config;

	rte_eal_mcfg_wait_complete(mcfg);
	while (heap->initialised != INITIALISED) {
		if (rte_atomic32_cmpset(
				(volatile uint32_t*)&heap->initialised,
				NOT_INITIALISED, INITIALISING)) {

			heap->free_head = NULL;
			heap->mz_count = 0;
			heap->alloc_count = 0;
			heap->total_size = 0;
			/*
			 * Find NUMA socket of heap that is being initialised, so that
			 * malloc_heaps[n].numa_socket == n
			 */
			heap->numa_socket = heap - mcfg->malloc_heaps;
			rte_spinlock_init(&heap->lock);
			heap->initialised = INITIALISED;
		}
	}
}

/*
 * Iterates through the freelist for a heap to find a free element
 * which can store data of the required size and with the requested alignment.
 * Returns null on failure, or pointer to element on success, with the pointer
 * to the previous element in the list, if any, being returned in a parameter
 * (to make removing the element from the free list faster).
 */
static struct malloc_elem *
find_suitable_element(struct malloc_heap *heap, size_t size,
		unsigned align, struct malloc_elem **prev)
{
	struct malloc_elem *elem, *min_elem, *min_prev;
	size_t min_sz;

	elem = heap->free_head;
	min_elem = NULL;
	min_prev = NULL;
	min_sz = (size_t) SIZE_MAX;
	*prev = NULL;

	while(elem){
		if (malloc_elem_can_hold(elem, size, align)) {
			if (min_sz > elem->size) {
				min_elem = elem;
				*prev = min_prev;
				min_sz = elem->size;
			}
		}
		min_prev = elem;
		elem = elem->next_free;
	}
	return (min_elem);
}

/*
 * Main function called by malloc to allocate a block of memory from the
 * heap. It locks the free list, scans it, and adds a new memzone if the
 * scan fails. Once the new memzone is added, it re-scans and should return
 * the new element after releasing the lock.
 */
void *
malloc_heap_alloc(struct malloc_heap *heap,
		const char *type __attribute__((unused)), size_t size, unsigned align)
{
	if (!heap->initialised)
		malloc_heap_init(heap);

	size = CACHE_LINE_ROUNDUP(size);
	align = CACHE_LINE_ROUNDUP(align);
	rte_spinlock_lock(&heap->lock);
	struct malloc_elem *prev, *elem = find_suitable_element(heap,
			size, align, &prev);
	if (elem == NULL){
		if ((malloc_heap_add_memzone(heap, size, align)) == 0)
			elem = find_suitable_element(heap, size, align, &prev);
	}

	if (elem != NULL){
		elem = malloc_elem_alloc(elem, size, align, prev);
		/* increase heap's count of allocated elements */
		heap->alloc_count++;
	}
	rte_spinlock_unlock(&heap->lock);
	return elem == NULL ? NULL : (void *)(&elem[1]);

}

/*
 * Function to retrieve data for heap on given socket
 */
int
malloc_heap_get_stats(struct malloc_heap *heap,
		struct rte_malloc_socket_stats *socket_stats)
{
	if (!heap->initialised)
		return -1;

	struct malloc_elem *elem = heap->free_head;

	/* Initialise variables for heap */
	socket_stats->free_count = 0;
	socket_stats->heap_freesz_bytes = 0;
	socket_stats->greatest_free_size = 0;

	/* Iterate through free list */
	while(elem) {
		socket_stats->free_count++;
		socket_stats->heap_freesz_bytes += elem->size;
		if (elem->size > socket_stats->greatest_free_size)
			socket_stats->greatest_free_size = elem->size;

		elem = elem->next_free;
	}
	/* Get stats on overall heap and allocated memory on this heap */
	socket_stats->heap_totalsz_bytes = heap->total_size;
	socket_stats->heap_allocsz_bytes = (socket_stats->heap_totalsz_bytes -
			socket_stats->heap_freesz_bytes);
	socket_stats->alloc_count = heap->alloc_count;
	return 0;
}

