/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2017 6WIND S.A.
 */

#ifndef _RTE_VFIO_H_
#define _RTE_VFIO_H_

/**
 * @file
 * RTE VFIO. This library provides various VFIO related utility functions.
 */

/*
 * determine if VFIO is present on the system
 */
#if !defined(VFIO_PRESENT) && defined(RTE_EAL_VFIO)
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
#define VFIO_PRESENT
#endif /* kernel version >= 3.6.0 */
#endif /* RTE_EAL_VFIO */

#ifdef VFIO_PRESENT

#include <linux/vfio.h>

#define VFIO_DIR "/dev/vfio"
#define VFIO_CONTAINER_PATH "/dev/vfio/vfio"
#define VFIO_GROUP_FMT "/dev/vfio/%u"
#define VFIO_NOIOMMU_GROUP_FMT "/dev/vfio/noiommu-%u"
#define VFIO_GET_REGION_ADDR(x) ((uint64_t) x << 40ULL)
#define VFIO_GET_REGION_IDX(x) (x >> 40)
#define VFIO_NOIOMMU_MODE      \
	"/sys/module/vfio/parameters/enable_unsafe_noiommu_mode"

#ifdef __cplusplus
extern "C" {
#endif

/* NOIOMMU is defined from kernel version 4.5 onwards */
#ifdef VFIO_NOIOMMU_IOMMU
#define RTE_VFIO_NOIOMMU VFIO_NOIOMMU_IOMMU
#else
#define RTE_VFIO_NOIOMMU 8
#endif

/**
 * Setup vfio_cfg for the device identified by its address.
 * It discovers the configured I/O MMU groups or sets a new one for the device.
 * If a new groups is assigned, the DMA mapping is performed.
 *
 * This function is only relevant to linux and will return
 * an error on BSD.
 *
 * @param sysfs_base
 *   sysfs path prefix.
 *
 * @param dev_addr
 *   device location.
 *
 * @param vfio_dev_fd
 *   VFIO fd.
 *
 * @param device_info
 *   Device information.
 *
 * @return
 *   0 on success.
 *   <0 on failure.
 *   >1 if the device cannot be managed this way.
 */
int rte_vfio_setup_device(const char *sysfs_base, const char *dev_addr,
		int *vfio_dev_fd, struct vfio_device_info *device_info);

/**
 * Release a device mapped to a VFIO-managed I/O MMU group.
 *
 * This function is only relevant to linux and will return
 * an error on BSD.
 *
 * @param sysfs_base
 *   sysfs path prefix.
 *
 * @param dev_addr
 *   device location.
 *
 * @param fd
 *   VFIO fd.
 *
 * @return
 *   0 on success.
 *   <0 on failure.
 */
int rte_vfio_release_device(const char *sysfs_base, const char *dev_addr, int fd);

/**
 * Enable a VFIO-related kmod.
 *
 * This function is only relevant to linux and will return
 * an error on BSD.
 *
 * @param modname
 *   kernel module name.
 *
 * @return
 *   0 on success.
 *   <0 on failure.
 */
int rte_vfio_enable(const char *modname);

/**
 * Check whether a VFIO-related kmod is enabled.
 *
 * This function is only relevant to linux and will return
 * an error on BSD.
 *
 * @param modname
 *   kernel module name.
 *
 * @return
 *   !0 if true.
 *   0 otherwise.
 */
int rte_vfio_is_enabled(const char *modname);

/**
 * Whether VFIO NOIOMMU mode is enabled.
 *
 * This function is only relevant to linux and will return
 * an error on BSD.
 *
 * @return
 *   !0 if true.
 *   0 otherwise.
 */
int rte_vfio_noiommu_is_enabled(void);

/**
 * Remove group fd from internal VFIO group fd array/
 *
 * This function is only relevant to linux and will return
 * an error on BSD.
 *
 * @param vfio_group_fd
 *   VFIO Grouup FD.
 *
 * @return
 *   0 on success.
 *   <0 on failure.
 */
int
rte_vfio_clear_group(int vfio_group_fd);

/**
 * Map memory region for use with VFIO.
 *
 * @note requires at least one device to be attached at the time of mapping.
 *
 * @param vaddr
 *   Starting virtual address of memory to be mapped.
 *
 * @param iova
 *   Starting IOVA address of memory to be mapped.
 *
 * @param len
 *   Length of memory segment being mapped.
 *
 * @return
 *   0 if success.
 *   -1 on error.
 */
int  __rte_experimental
rte_vfio_dma_map(uint64_t vaddr, uint64_t iova, uint64_t len);


/**
 * Unmap memory region from VFIO.
 *
 * @param vaddr
 *   Starting virtual address of memory to be unmapped.
 *
 * @param iova
 *   Starting IOVA address of memory to be unmapped.
 *
 * @param len
 *   Length of memory segment being unmapped.
 *
 * @return
 *   0 if success.
 *   -1 on error.
 */

int __rte_experimental
rte_vfio_dma_unmap(uint64_t vaddr, uint64_t iova, uint64_t len);
/**
 * Parse IOMMU group number for a device
 *
 * This function is only relevant to linux and will return
 * an error on BSD.
 *
 * @param sysfs_base
 *   sysfs path prefix.
 *
 * @param dev_addr
 *   device location.
 *
 * @param iommu_group_num
 *   iommu group number
 *
 * @return
 *  >0 on success
 *   0 for non-existent group or VFIO
 *  <0 for errors
 */
int __rte_experimental
rte_vfio_get_group_num(const char *sysfs_base,
		      const char *dev_addr, int *iommu_group_num);

/**
 * Open VFIO container fd or get an existing one
 *
 * This function is only relevant to linux and will return
 * an error on BSD.
 *
 * @return
 *  > 0 container fd
 *  < 0 for errors
 */
int __rte_experimental
rte_vfio_get_container_fd(void);

/**
 * Open VFIO group fd or get an existing one
 *
 * This function is only relevant to linux and will return
 * an error on BSD.
 *
 * @param iommu_group_num
 *   iommu group number
 *
 * @return
 *  > 0 group fd
 *  < 0 for errors
 */
int __rte_experimental
rte_vfio_get_group_fd(int iommu_group_num);

#ifdef __cplusplus
}
#endif

#endif /* VFIO_PRESENT */

#endif /* _RTE_VFIO_H_ */
