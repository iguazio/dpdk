/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */
#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/queue.h>

#include <rte_memory.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_common.h>
#include <rte_spinlock.h>

#include "eal_memalloc.h"
#include "malloc_elem.h"
#include "malloc_heap.h"

#define MIN_DATA_SIZE (RTE_CACHE_LINE_SIZE)

/*
 * Initialize a general malloc_elem header structure
 */
void
malloc_elem_init(struct malloc_elem *elem, struct malloc_heap *heap,
		struct rte_memseg_list *msl, size_t size)
{
	elem->heap = heap;
	elem->msl = msl;
	elem->prev = NULL;
	elem->next = NULL;
	memset(&elem->free_list, 0, sizeof(elem->free_list));
	elem->state = ELEM_FREE;
	elem->size = size;
	elem->pad = 0;
	set_header(elem);
	set_trailer(elem);
}

void
malloc_elem_insert(struct malloc_elem *elem)
{
	struct malloc_elem *prev_elem, *next_elem;
	struct malloc_heap *heap = elem->heap;

	if (heap->first == NULL && heap->last == NULL) {
		/* if empty heap */
		heap->first = elem;
		heap->last = elem;
		prev_elem = NULL;
		next_elem = NULL;
	} else if (elem < heap->first) {
		/* if lower than start */
		prev_elem = NULL;
		next_elem = heap->first;
		heap->first = elem;
	} else if (elem > heap->last) {
		/* if higher than end */
		prev_elem = heap->last;
		next_elem = NULL;
		heap->last = elem;
	} else {
		/* the new memory is somewhere inbetween start and end */
		uint64_t dist_from_start, dist_from_end;

		dist_from_end = RTE_PTR_DIFF(heap->last, elem);
		dist_from_start = RTE_PTR_DIFF(elem, heap->first);

		/* check which is closer, and find closest list entries */
		if (dist_from_start < dist_from_end) {
			prev_elem = heap->first;
			while (prev_elem->next < elem)
				prev_elem = prev_elem->next;
			next_elem = prev_elem->next;
		} else {
			next_elem = heap->last;
			while (next_elem->prev > elem)
				next_elem = next_elem->prev;
			prev_elem = next_elem->prev;
		}
	}

	/* insert new element */
	elem->prev = prev_elem;
	elem->next = next_elem;
	if (prev_elem)
		prev_elem->next = elem;
	if (next_elem)
		next_elem->prev = elem;
}

/*
 * Attempt to find enough physically contiguous memory in this block to store
 * our data. Assume that element has at least enough space to fit in the data,
 * so we just check the page addresses.
 */
static bool
elem_check_phys_contig(const struct rte_memseg_list *msl,
		void *start, size_t size)
{
	return eal_memalloc_is_contig(msl, start, size);
}

/*
 * calculate the starting point of where data of the requested size
 * and alignment would fit in the current element. If the data doesn't
 * fit, return NULL.
 */
static void *
elem_start_pt(struct malloc_elem *elem, size_t size, unsigned align,
		size_t bound, bool contig)
{
	size_t elem_size = elem->size;

	/*
	 * we're allocating from the end, so adjust the size of element by
	 * alignment size.
	 */
	while (elem_size >= size) {
		const size_t bmask = ~(bound - 1);
		uintptr_t end_pt = (uintptr_t)elem +
				elem_size - MALLOC_ELEM_TRAILER_LEN;
		uintptr_t new_data_start = RTE_ALIGN_FLOOR((end_pt - size),
				align);
		uintptr_t new_elem_start;

		/* check boundary */
		if ((new_data_start & bmask) != ((end_pt - 1) & bmask)) {
			end_pt = RTE_ALIGN_FLOOR(end_pt, bound);
			new_data_start = RTE_ALIGN_FLOOR((end_pt - size),
					align);
			end_pt = new_data_start + size;

			if (((end_pt - 1) & bmask) != (new_data_start & bmask))
				return NULL;
		}

		new_elem_start = new_data_start - MALLOC_ELEM_HEADER_LEN;

		/* if the new start point is before the exist start,
		 * it won't fit
		 */
		if (new_elem_start < (uintptr_t)elem)
			return NULL;

		if (contig) {
			size_t new_data_size = end_pt - new_data_start;

			/*
			 * if physical contiguousness was requested and we
			 * couldn't fit all data into one physically contiguous
			 * block, try again with lower addresses.
			 */
			if (!elem_check_phys_contig(elem->msl,
					(void *)new_data_start,
					new_data_size)) {
				elem_size -= align;
				continue;
			}
		}
		return (void *)new_elem_start;
	}
	return NULL;
}

/*
 * use elem_start_pt to determine if we get meet the size and
 * alignment request from the current element
 */
int
malloc_elem_can_hold(struct malloc_elem *elem, size_t size,	unsigned align,
		size_t bound, bool contig)
{
	return elem_start_pt(elem, size, align, bound, contig) != NULL;
}

/*
 * split an existing element into two smaller elements at the given
 * split_pt parameter.
 */
static void
split_elem(struct malloc_elem *elem, struct malloc_elem *split_pt)
{
	struct malloc_elem *next_elem = elem->next;
	const size_t old_elem_size = (uintptr_t)split_pt - (uintptr_t)elem;
	const size_t new_elem_size = elem->size - old_elem_size;

	malloc_elem_init(split_pt, elem->heap, elem->msl, new_elem_size);
	split_pt->prev = elem;
	split_pt->next = next_elem;
	if (next_elem)
		next_elem->prev = split_pt;
	else
		elem->heap->last = split_pt;
	elem->next = split_pt;
	elem->size = old_elem_size;
	set_trailer(elem);
}

/*
 * our malloc heap is a doubly linked list, so doubly remove our element.
 */
static void __rte_unused
remove_elem(struct malloc_elem *elem)
{
	struct malloc_elem *next, *prev;
	next = elem->next;
	prev = elem->prev;

	if (next)
		next->prev = prev;
	else
		elem->heap->last = prev;
	if (prev)
		prev->next = next;
	else
		elem->heap->first = next;

	elem->prev = NULL;
	elem->next = NULL;
}

static int
next_elem_is_adjacent(struct malloc_elem *elem)
{
	return elem->next == RTE_PTR_ADD(elem, elem->size);
}

static int
prev_elem_is_adjacent(struct malloc_elem *elem)
{
	return elem == RTE_PTR_ADD(elem->prev, elem->prev->size);
}

/*
 * Given an element size, compute its freelist index.
 * We free an element into the freelist containing similarly-sized elements.
 * We try to allocate elements starting with the freelist containing
 * similarly-sized elements, and if necessary, we search freelists
 * containing larger elements.
 *
 * Example element size ranges for a heap with five free lists:
 *   heap->free_head[0] - (0   , 2^8]
 *   heap->free_head[1] - (2^8 , 2^10]
 *   heap->free_head[2] - (2^10 ,2^12]
 *   heap->free_head[3] - (2^12, 2^14]
 *   heap->free_head[4] - (2^14, MAX_SIZE]
 */
size_t
malloc_elem_free_list_index(size_t size)
{
#define MALLOC_MINSIZE_LOG2   8
#define MALLOC_LOG2_INCREMENT 2

	size_t log2;
	size_t index;

	if (size <= (1UL << MALLOC_MINSIZE_LOG2))
		return 0;

	/* Find next power of 2 >= size. */
	log2 = sizeof(size) * 8 - __builtin_clzl(size-1);

	/* Compute freelist index, based on log2(size). */
	index = (log2 - MALLOC_MINSIZE_LOG2 + MALLOC_LOG2_INCREMENT - 1) /
	        MALLOC_LOG2_INCREMENT;

	return index <= RTE_HEAP_NUM_FREELISTS-1?
	        index: RTE_HEAP_NUM_FREELISTS-1;
}

/*
 * Add the specified element to its heap's free list.
 */
void
malloc_elem_free_list_insert(struct malloc_elem *elem)
{
	size_t idx;

	idx = malloc_elem_free_list_index(elem->size - MALLOC_ELEM_HEADER_LEN);
	elem->state = ELEM_FREE;
	LIST_INSERT_HEAD(&elem->heap->free_head[idx], elem, free_list);
}

/*
 * Remove the specified element from its heap's free list.
 */
void
malloc_elem_free_list_remove(struct malloc_elem *elem)
{
	LIST_REMOVE(elem, free_list);
}

/*
 * reserve a block of data in an existing malloc_elem. If the malloc_elem
 * is much larger than the data block requested, we split the element in two.
 * This function is only called from malloc_heap_alloc so parameter checking
 * is not done here, as it's done there previously.
 */
struct malloc_elem *
malloc_elem_alloc(struct malloc_elem *elem, size_t size, unsigned align,
		size_t bound, bool contig)
{
	struct malloc_elem *new_elem = elem_start_pt(elem, size, align, bound,
			contig);
	const size_t old_elem_size = (uintptr_t)new_elem - (uintptr_t)elem;
	const size_t trailer_size = elem->size - old_elem_size - size -
		MALLOC_ELEM_OVERHEAD;

	malloc_elem_free_list_remove(elem);

	if (trailer_size > MALLOC_ELEM_OVERHEAD + MIN_DATA_SIZE) {
		/* split it, too much free space after elem */
		struct malloc_elem *new_free_elem =
				RTE_PTR_ADD(new_elem, size + MALLOC_ELEM_OVERHEAD);

		split_elem(elem, new_free_elem);
		malloc_elem_free_list_insert(new_free_elem);

		if (elem == elem->heap->last)
			elem->heap->last = new_free_elem;
	}

	if (old_elem_size < MALLOC_ELEM_OVERHEAD + MIN_DATA_SIZE) {
		/* don't split it, pad the element instead */
		elem->state = ELEM_BUSY;
		elem->pad = old_elem_size;

		/* put a dummy header in padding, to point to real element header */
		if (elem->pad > 0) { /* pad will be at least 64-bytes, as everything
		                     * is cache-line aligned */
			new_elem->pad = elem->pad;
			new_elem->state = ELEM_PAD;
			new_elem->size = elem->size - elem->pad;
			set_header(new_elem);
		}

		return new_elem;
	}

	/* we are going to split the element in two. The original element
	 * remains free, and the new element is the one allocated.
	 * Re-insert original element, in case its new size makes it
	 * belong on a different list.
	 */
	split_elem(elem, new_elem);
	new_elem->state = ELEM_BUSY;
	malloc_elem_free_list_insert(elem);

	return new_elem;
}

/*
 * join two struct malloc_elem together. elem1 and elem2 must
 * be contiguous in memory.
 */
static inline void
join_elem(struct malloc_elem *elem1, struct malloc_elem *elem2)
{
	struct malloc_elem *next = elem2->next;
	elem1->size += elem2->size;
	if (next)
		next->prev = elem1;
	else
		elem1->heap->last = elem1;
	elem1->next = next;
}

struct malloc_elem *
malloc_elem_join_adjacent_free(struct malloc_elem *elem)
{
	/*
	 * check if next element exists, is adjacent and is free, if so join
	 * with it, need to remove from free list.
	 */
	if (elem->next != NULL && elem->next->state == ELEM_FREE &&
			next_elem_is_adjacent(elem)) {
		void *erase;

		/* we will want to erase the trailer and header */
		erase = RTE_PTR_SUB(elem->next, MALLOC_ELEM_TRAILER_LEN);

		/* remove from free list, join to this one */
		malloc_elem_free_list_remove(elem->next);
		join_elem(elem, elem->next);

		/* erase header and trailer */
		memset(erase, 0, MALLOC_ELEM_OVERHEAD);
	}

	/*
	 * check if prev element exists, is adjacent and is free, if so join
	 * with it, need to remove from free list.
	 */
	if (elem->prev != NULL && elem->prev->state == ELEM_FREE &&
			prev_elem_is_adjacent(elem)) {
		struct malloc_elem *new_elem;
		void *erase;

		/* we will want to erase trailer and header */
		erase = RTE_PTR_SUB(elem, MALLOC_ELEM_TRAILER_LEN);

		/* remove from free list, join to this one */
		malloc_elem_free_list_remove(elem->prev);

		new_elem = elem->prev;
		join_elem(new_elem, elem);

		/* erase header and trailer */
		memset(erase, 0, MALLOC_ELEM_OVERHEAD);

		elem = new_elem;
	}

	return elem;
}

/*
 * free a malloc_elem block by adding it to the free list. If the
 * blocks either immediately before or immediately after newly freed block
 * are also free, the blocks are merged together.
 */
struct malloc_elem *
malloc_elem_free(struct malloc_elem *elem)
{
	void *ptr;
	size_t data_len;

	ptr = RTE_PTR_ADD(elem, sizeof(*elem));
	data_len = elem->size - MALLOC_ELEM_OVERHEAD;

	elem = malloc_elem_join_adjacent_free(elem);

	malloc_elem_free_list_insert(elem);

	/* decrease heap's count of allocated elements */
	elem->heap->alloc_count--;

	memset(ptr, 0, data_len);

	return elem;
}

/* assume all checks were already done */
void
malloc_elem_hide_region(struct malloc_elem *elem, void *start, size_t len)
{
	struct malloc_elem *hide_start, *hide_end, *prev, *next;
	size_t len_before, len_after;

	hide_start = start;
	hide_end = RTE_PTR_ADD(start, len);

	prev = elem->prev;
	next = elem->next;

	/* we cannot do anything with non-adjacent elements */
	if (next && next_elem_is_adjacent(elem)) {
		len_after = RTE_PTR_DIFF(next, hide_end);
		if (len_after >= MALLOC_ELEM_OVERHEAD + MIN_DATA_SIZE) {
			/* split after */
			split_elem(elem, hide_end);

			malloc_elem_free_list_insert(hide_end);
		} else if (len_after >= MALLOC_ELEM_HEADER_LEN) {
			/* shrink current element */
			elem->size -= len_after;
			memset(hide_end, 0, sizeof(*hide_end));

			/* copy next element's data to our pad */
			memcpy(hide_end, next, sizeof(*hide_end));

			/* pad next element */
			next->state = ELEM_PAD;
			next->pad = len_after;
			next->size -= len_after;

			/* next element busy, would've been merged otherwise */
			hide_end->pad = len_after;
			hide_end->size += len_after;

			/* adjust pointers to point to our new pad */
			if (next->next)
				next->next->prev = hide_end;
			elem->next = hide_end;
		} else if (len_after > 0) {
			RTE_LOG(ERR, EAL, "Unaligned element, heap is probably corrupt\n");
			return;
		}
	}

	/* we cannot do anything with non-adjacent elements */
	if (prev && prev_elem_is_adjacent(elem)) {
		len_before = RTE_PTR_DIFF(hide_start, elem);
		if (len_before >= MALLOC_ELEM_OVERHEAD + MIN_DATA_SIZE) {
			/* split before */
			split_elem(elem, hide_start);

			prev = elem;
			elem = hide_start;

			malloc_elem_free_list_insert(prev);
		} else if (len_before > 0) {
			/*
			 * unlike with elements after current, here we don't
			 * need to pad elements, but rather just increase the
			 * size of previous element, copy the old header and set
			 * up trailer.
			 */
			void *trailer = RTE_PTR_ADD(prev,
					prev->size - MALLOC_ELEM_TRAILER_LEN);

			memcpy(hide_start, elem, sizeof(*elem));
			hide_start->size = len;

			prev->size += len_before;
			set_trailer(prev);

			/* update pointers */
			prev->next = hide_start;
			if (next)
				next->prev = hide_start;

			/* erase old trailer */
			memset(trailer, 0, MALLOC_ELEM_TRAILER_LEN);
			/* erase old header */
			memset(elem, 0, sizeof(*elem));

			elem = hide_start;
		}
	}

	remove_elem(elem);
}

/*
 * attempt to resize a malloc_elem by expanding into any free space
 * immediately after it in memory.
 */
int
malloc_elem_resize(struct malloc_elem *elem, size_t size)
{
	const size_t new_size = size + elem->pad + MALLOC_ELEM_OVERHEAD;

	/* if we request a smaller size, then always return ok */
	if (elem->size >= new_size)
		return 0;

	/* check if there is a next element, it's free and adjacent */
	if (!elem->next || elem->next->state != ELEM_FREE ||
			!next_elem_is_adjacent(elem))
		return -1;
	if (elem->size + elem->next->size < new_size)
		return -1;

	/* we now know the element fits, so remove from free list,
	 * join the two
	 */
	malloc_elem_free_list_remove(elem->next);
	join_elem(elem, elem->next);

	if (elem->size - new_size >= MIN_DATA_SIZE + MALLOC_ELEM_OVERHEAD) {
		/* now we have a big block together. Lets cut it down a bit, by splitting */
		struct malloc_elem *split_pt = RTE_PTR_ADD(elem, new_size);
		split_pt = RTE_PTR_ALIGN_CEIL(split_pt, RTE_CACHE_LINE_SIZE);
		split_elem(elem, split_pt);
		malloc_elem_free_list_insert(split_pt);
	}
	return 0;
}

static inline const char *
elem_state_to_str(enum elem_state state)
{
	switch (state) {
	case ELEM_PAD:
		return "PAD";
	case ELEM_BUSY:
		return "BUSY";
	case ELEM_FREE:
		return "FREE";
	}
	return "ERROR";
}

void
malloc_elem_dump(const struct malloc_elem *elem, FILE *f)
{
	fprintf(f, "Malloc element at %p (%s)\n", elem,
			elem_state_to_str(elem->state));
	fprintf(f, "  len: 0x%zx pad: 0x%" PRIx32 "\n", elem->size, elem->pad);
	fprintf(f, "  prev: %p next: %p\n", elem->prev, elem->next);
}
