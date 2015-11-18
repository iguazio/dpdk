/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
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
 */

#ifndef _RTE_MEMZONE_H_
#define _RTE_MEMZONE_H_

/**
 * @file
 * RTE Memzone
 *
 * The goal of the memzone allocator is to reserve contiguous
 * portions of physical memory. These zones are identified by a name.
 *
 * The memzone descriptors are shared by all partitions and are
 * located in a known place of physical memory. This zone is accessed
 * using rte_eal_get_configuration(). The lookup (by name) of a
 * memory zone can be done in any partition and returns the same
 * physical address.
 *
 * A reserved memory zone cannot be unreserved. The reservation shall
 * be done at initialization time only.
 */

#include <stdio.h>
#include <rte_memory.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTE_MEMZONE_2MB            0x00000001   /**< Use 2MB pages. */
#define RTE_MEMZONE_1GB            0x00000002   /**< Use 1GB pages. */
#define RTE_MEMZONE_16MB           0x00000100   /**< Use 16MB pages. */
#define RTE_MEMZONE_16GB           0x00000200   /**< Use 16GB pages. */
#define RTE_MEMZONE_256KB          0x00010000   /**< Use 256KB pages. */
#define RTE_MEMZONE_256MB          0x00020000   /**< Use 256MB pages. */
#define RTE_MEMZONE_512MB          0x00040000   /**< Use 512MB pages. */
#define RTE_MEMZONE_4GB            0x00080000   /**< Use 4GB pages. */
#define RTE_MEMZONE_SIZE_HINT_ONLY 0x00000004   /**< Use available page size */

/**
 * A structure describing a memzone, which is a contiguous portion of
 * physical memory identified by a name.
 */
struct rte_memzone {

#define RTE_MEMZONE_NAMESIZE 32       /**< Maximum length of memory zone name.*/
	char name[RTE_MEMZONE_NAMESIZE];  /**< Name of the memory zone. */

	phys_addr_t phys_addr;            /**< Start physical address. */
	union {
		void *addr;                   /**< Start virtual address. */
		uint64_t addr_64;             /**< Makes sure addr is always 64-bits */
	};
#ifdef RTE_LIBRTE_IVSHMEM
	phys_addr_t ioremap_addr;         /**< Real physical address inside the VM */
#endif
	size_t len;                       /**< Length of the memzone. */

	uint64_t hugepage_sz;             /**< The page size of underlying memory */

	int32_t socket_id;                /**< NUMA socket ID. */

	uint32_t flags;                   /**< Characteristics of this memzone. */
	uint32_t memseg_id;               /**< Memseg it belongs. */
} __attribute__((__packed__));

/**
 * Reserve a portion of physical memory.
 *
 * This function reserves some memory and returns a pointer to a
 * correctly filled memzone descriptor. If the allocation cannot be
 * done, return NULL.
 *
 * @param name
 *   The name of the memzone. If it already exists, the function will
 *   fail and return NULL.
 * @param len
 *   The size of the memory to be reserved. If it
 *   is 0, the biggest contiguous zone will be reserved.
 * @param socket_id
 *   The socket identifier in the case of
 *   NUMA. The value can be SOCKET_ID_ANY if there is no NUMA
 *   constraint for the reserved zone.
 * @param flags
 *   The flags parameter is used to request memzones to be
 *   taken from specifically sized hugepages.
 *   - RTE_MEMZONE_2MB - Reserved from 2MB pages
 *   - RTE_MEMZONE_1GB - Reserved from 1GB pages
 *   - RTE_MEMZONE_16MB - Reserved from 16MB pages
 *   - RTE_MEMZONE_16GB - Reserved from 16GB pages
 *   - RTE_MEMZONE_256KB - Reserved from 256KB pages
 *   - RTE_MEMZONE_256MB - Reserved from 256MB pages
 *   - RTE_MEMZONE_512MB - Reserved from 512MB pages
 *   - RTE_MEMZONE_4GB - Reserved from 4GB pages
 *   - RTE_MEMZONE_SIZE_HINT_ONLY - Allow alternative page size to be used if
 *                                  the requested page size is unavailable.
 *                                  If this flag is not set, the function
 *                                  will return error on an unavailable size
 *                                  request.
 * @return
 *   A pointer to a correctly-filled read-only memzone descriptor, or NULL
 *   on error.
 *   On error case, rte_errno will be set appropriately:
 *    - E_RTE_NO_CONFIG - function could not get pointer to rte_config structure
 *    - E_RTE_SECONDARY - function was called from a secondary process instance
 *    - ENOSPC - the maximum number of memzones has already been allocated
 *    - EEXIST - a memzone with the same name already exists
 *    - ENOMEM - no appropriate memory area found in which to create memzone
 *    - EINVAL - invalid parameters
 */
const struct rte_memzone *rte_memzone_reserve(const char *name,
					      size_t len, int socket_id,
					      unsigned flags);

/**
 * Reserve a portion of physical memory with alignment on a specified
 * boundary.
 *
 * This function reserves some memory with alignment on a specified
 * boundary, and returns a pointer to a correctly filled memzone
 * descriptor. If the allocation cannot be done or if the alignment
 * is not a power of 2, returns NULL.
 *
 * @param name
 *   The name of the memzone. If it already exists, the function will
 *   fail and return NULL.
 * @param len
 *   The size of the memory to be reserved. If it
 *   is 0, the biggest contiguous zone will be reserved.
 * @param socket_id
 *   The socket identifier in the case of
 *   NUMA. The value can be SOCKET_ID_ANY if there is no NUMA
 *   constraint for the reserved zone.
 * @param flags
 *   The flags parameter is used to request memzones to be
 *   taken from specifically sized hugepages.
 *   - RTE_MEMZONE_2MB - Reserved from 2MB pages
 *   - RTE_MEMZONE_1GB - Reserved from 1GB pages
 *   - RTE_MEMZONE_16MB - Reserved from 16MB pages
 *   - RTE_MEMZONE_16GB - Reserved from 16GB pages
 *   - RTE_MEMZONE_256KB - Reserved from 256KB pages
 *   - RTE_MEMZONE_256MB - Reserved from 256MB pages
 *   - RTE_MEMZONE_512MB - Reserved from 512MB pages
 *   - RTE_MEMZONE_4GB - Reserved from 4GB pages
 *   - RTE_MEMZONE_SIZE_HINT_ONLY - Allow alternative page size to be used if
 *                                  the requested page size is unavailable.
 *                                  If this flag is not set, the function
 *                                  will return error on an unavailable size
 *                                  request.
 * @param align
 *   Alignment for resulting memzone. Must be a power of 2.
 * @return
 *   A pointer to a correctly-filled read-only memzone descriptor, or NULL
 *   on error.
 *   On error case, rte_errno will be set appropriately:
 *    - E_RTE_NO_CONFIG - function could not get pointer to rte_config structure
 *    - E_RTE_SECONDARY - function was called from a secondary process instance
 *    - ENOSPC - the maximum number of memzones has already been allocated
 *    - EEXIST - a memzone with the same name already exists
 *    - ENOMEM - no appropriate memory area found in which to create memzone
 *    - EINVAL - invalid parameters
 */
const struct rte_memzone *rte_memzone_reserve_aligned(const char *name,
			size_t len, int socket_id,
			unsigned flags, unsigned align);

/**
 * Reserve a portion of physical memory with specified alignment and
 * boundary.
 *
 * This function reserves some memory with specified alignment and
 * boundary, and returns a pointer to a correctly filled memzone
 * descriptor. If the allocation cannot be done or if the alignment
 * or boundary are not a power of 2, returns NULL.
 * Memory buffer is reserved in a way, that it wouldn't cross specified
 * boundary. That implies that requested length should be less or equal
 * then boundary.
 *
 * @param name
 *   The name of the memzone. If it already exists, the function will
 *   fail and return NULL.
 * @param len
 *   The size of the memory to be reserved. If it
 *   is 0, the biggest contiguous zone will be reserved.
 * @param socket_id
 *   The socket identifier in the case of
 *   NUMA. The value can be SOCKET_ID_ANY if there is no NUMA
 *   constraint for the reserved zone.
 * @param flags
 *   The flags parameter is used to request memzones to be
 *   taken from specifically sized hugepages.
 *   - RTE_MEMZONE_2MB - Reserved from 2MB pages
 *   - RTE_MEMZONE_1GB - Reserved from 1GB pages
 *   - RTE_MEMZONE_16MB - Reserved from 16MB pages
 *   - RTE_MEMZONE_16GB - Reserved from 16GB pages
 *   - RTE_MEMZONE_256KB - Reserved from 256KB pages
 *   - RTE_MEMZONE_256MB - Reserved from 256MB pages
 *   - RTE_MEMZONE_512MB - Reserved from 512MB pages
 *   - RTE_MEMZONE_4GB - Reserved from 4GB pages
 *   - RTE_MEMZONE_SIZE_HINT_ONLY - Allow alternative page size to be used if
 *                                  the requested page size is unavailable.
 *                                  If this flag is not set, the function
 *                                  will return error on an unavailable size
 *                                  request.
 * @param align
 *   Alignment for resulting memzone. Must be a power of 2.
 * @param bound
 *   Boundary for resulting memzone. Must be a power of 2 or zero.
 *   Zero value implies no boundary condition.
 * @return
 *   A pointer to a correctly-filled read-only memzone descriptor, or NULL
 *   on error.
 *   On error case, rte_errno will be set appropriately:
 *    - E_RTE_NO_CONFIG - function could not get pointer to rte_config structure
 *    - E_RTE_SECONDARY - function was called from a secondary process instance
 *    - ENOSPC - the maximum number of memzones has already been allocated
 *    - EEXIST - a memzone with the same name already exists
 *    - ENOMEM - no appropriate memory area found in which to create memzone
 *    - EINVAL - invalid parameters
 */
const struct rte_memzone *rte_memzone_reserve_bounded(const char *name,
			size_t len, int socket_id,
			unsigned flags, unsigned align, unsigned bound);

/**
 * Free a memzone.
 *
 * Note: an IVSHMEM zone cannot be freed.
 *
 * @param mz
 *   A pointer to the memzone
 * @return
 *  -EINVAL - invalid parameter, IVSHMEM memzone.
 *  0 - success
 */
int rte_memzone_free(const struct rte_memzone *mz);

/**
 * Lookup for a memzone.
 *
 * Get a pointer to a descriptor of an already reserved memory
 * zone identified by the name given as an argument.
 *
 * @param name
 *   The name of the memzone.
 * @return
 *   A pointer to a read-only memzone descriptor.
 */
const struct rte_memzone *rte_memzone_lookup(const char *name);

/**
 * Dump all reserved memzones to the console.
 *
 * @param f
 *   A pointer to a file for output
 */
void rte_memzone_dump(FILE *f);

/**
 * Walk list of all memzones
 *
 * @param func
 *   Iterator function
 * @param arg
 *   Argument passed to iterator
 */
void rte_memzone_walk(void (*func)(const struct rte_memzone *, void *arg),
		      void *arg);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_MEMZONE_H_ */
