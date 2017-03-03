/*
 *
 *   Copyright(c) 2016 Cavium networks. All rights reserved.
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
 *     * Neither the name of Cavium networks nor the names of its
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

#ifndef _RTE_EVENTDEV_PMD_H_
#define _RTE_EVENTDEV_PMD_H_

/** @file
 * RTE Event PMD APIs
 *
 * @note
 * These API are from event PMD only and user applications should not call
 * them directly.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include <rte_dev.h>
#include <rte_pci.h>
#include <rte_malloc.h>
#include <rte_log.h>
#include <rte_common.h>

#include "rte_eventdev.h"

#ifdef RTE_LIBRTE_EVENTDEV_DEBUG
#define RTE_PMD_DEBUG_TRACE(...) \
	rte_pmd_debug_trace(__func__, __VA_ARGS__)
#else
#define RTE_PMD_DEBUG_TRACE(...)
#endif

/* Logging Macros */
#define RTE_EDEV_LOG_ERR(fmt, args...) \
	RTE_LOG(ERR, EVENTDEV, "%s() line %u: " fmt "\n",  \
			__func__, __LINE__, ## args)

#ifdef RTE_LIBRTE_EVENTDEV_DEBUG
#define RTE_EDEV_LOG_DEBUG(fmt, args...) \
	RTE_LOG(DEBUG, EVENTDEV, "%s() line %u: " fmt "\n",  \
			__func__, __LINE__, ## args)
#else
#define RTE_EDEV_LOG_DEBUG(fmt, args...) (void)0
#endif

/* Macros to check for valid device */
#define RTE_EVENTDEV_VALID_DEVID_OR_ERR_RET(dev_id, retval) do { \
	if (!rte_event_pmd_is_valid_dev((dev_id))) { \
		RTE_EDEV_LOG_ERR("Invalid dev_id=%d\n", dev_id); \
		return retval; \
	} \
} while (0)

#define RTE_EVENTDEV_VALID_DEVID_OR_RET(dev_id) do { \
	if (!rte_event_pmd_is_valid_dev((dev_id))) { \
		RTE_EDEV_LOG_ERR("Invalid dev_id=%d\n", dev_id); \
		return; \
	} \
} while (0)

#define RTE_EVENTDEV_DETACHED  (0)
#define RTE_EVENTDEV_ATTACHED  (1)

/**
 * Initialisation function of a event driver invoked for each matching
 * event PCI device detected during the PCI probing phase.
 *
 * @param dev
 *   The dev pointer is the address of the *rte_eventdev* structure associated
 *   with the matching device and which has been [automatically] allocated in
 *   the *rte_event_devices* array.
 *
 * @return
 *   - 0: Success, the device is properly initialised by the driver.
 *        In particular, the driver MUST have set up the *dev_ops* pointer
 *        of the *dev* structure.
 *   - <0: Error code of the device initialisation failure.
 */
typedef int (*eventdev_init_t)(struct rte_eventdev *dev);

/**
 * Finalisation function of a driver invoked for each matching
 * PCI device detected during the PCI closing phase.
 *
 * @param dev
 *   The dev pointer is the address of the *rte_eventdev* structure associated
 *   with the matching device and which	has been [automatically] allocated in
 *   the *rte_event_devices* array.
 *
 * @return
 *   - 0: Success, the device is properly finalised by the driver.
 *        In particular, the driver MUST free the *dev_ops* pointer
 *        of the *dev* structure.
 *   - <0: Error code of the device initialisation failure.
 */
typedef int (*eventdev_uninit_t)(struct rte_eventdev *dev);

/**
 * The structure associated with a PMD driver.
 *
 * Each driver acts as a PCI driver and is represented by a generic
 * *event_driver* structure that holds:
 *
 * - An *rte_pci_driver* structure (which must be the first field).
 *
 * - The *eventdev_init* function invoked for each matching PCI device.
 *
 * - The size of the private data to allocate for each matching device.
 */
struct rte_eventdev_driver {
	struct rte_pci_driver pci_drv;	/**< The PMD is also a PCI driver. */
	unsigned int dev_private_size;	/**< Size of device private data. */

	eventdev_init_t eventdev_init;	/**< Device init function. */
	eventdev_uninit_t eventdev_uninit; /**< Device uninit function. */
};

/** Global structure used for maintaining state of allocated event devices */
struct rte_eventdev_global {
	uint8_t nb_devs;	/**< Number of devices found */
};

extern struct rte_eventdev_global *rte_eventdev_globals;
/** Pointer to global event devices data structure. */
extern struct rte_eventdev *rte_eventdevs;
/** The pool of rte_eventdev structures. */

/**
 * Get the rte_eventdev structure device pointer for the named device.
 *
 * @param name
 *   device name to select the device structure.
 *
 * @return
 *   - The rte_eventdev structure pointer for the given device ID.
 */
static inline struct rte_eventdev *
rte_event_pmd_get_named_dev(const char *name)
{
	struct rte_eventdev *dev;
	unsigned int i;

	if (name == NULL)
		return NULL;

	for (i = 0; i < RTE_EVENT_MAX_DEVS; i++) {
		dev = &rte_eventdevs[i];
		if ((dev->attached == RTE_EVENTDEV_ATTACHED) &&
				(strcmp(dev->data->name, name) == 0))
			return dev;
	}

	return NULL;
}

/**
 * Validate if the event device index is valid attached event device.
 *
 * @param dev_id
 *   Event device index.
 *
 * @return
 *   - If the device index is valid (1) or not (0).
 */
static inline unsigned
rte_event_pmd_is_valid_dev(uint8_t dev_id)
{
	struct rte_eventdev *dev;

	if (dev_id >= RTE_EVENT_MAX_DEVS)
		return 0;

	dev = &rte_eventdevs[dev_id];
	if (dev->attached != RTE_EVENTDEV_ATTACHED)
		return 0;
	else
		return 1;
}

/**
 * Definitions of all functions exported by a driver through the
 * the generic structure of type *event_dev_ops* supplied in the
 * *rte_eventdev* structure associated with a device.
 */

/**
 * Get device information of a device.
 *
 * @param dev
 *   Event device pointer
 * @param dev_info
 *   Event device information structure
 *
 * @return
 *   Returns 0 on success
 */
typedef void (*eventdev_info_get_t)(struct rte_eventdev *dev,
		struct rte_event_dev_info *dev_info);

/**
 * Configure a device.
 *
 * @param dev
 *   Event device pointer
 *
 * @return
 *   Returns 0 on success
 */
typedef int (*eventdev_configure_t)(const struct rte_eventdev *dev);

/**
 * Start a configured device.
 *
 * @param dev
 *   Event device pointer
 *
 * @return
 *   Returns 0 on success
 */
typedef int (*eventdev_start_t)(struct rte_eventdev *dev);

/**
 * Stop a configured device.
 *
 * @param dev
 *   Event device pointer
 */
typedef void (*eventdev_stop_t)(struct rte_eventdev *dev);

/**
 * Close a configured device.
 *
 * @param dev
 *   Event device pointer
 *
 * @return
 * - 0 on success
 * - (-EAGAIN) if can't close as device is busy
 */
typedef int (*eventdev_close_t)(struct rte_eventdev *dev);

/**
 * Retrieve the default event queue configuration.
 *
 * @param dev
 *   Event device pointer
 * @param queue_id
 *   Event queue index
 * @param[out] queue_conf
 *   Event queue configuration structure
 *
 */
typedef void (*eventdev_queue_default_conf_get_t)(struct rte_eventdev *dev,
		uint8_t queue_id, struct rte_event_queue_conf *queue_conf);

/**
 * Setup an event queue.
 *
 * @param dev
 *   Event device pointer
 * @param queue_id
 *   Event queue index
 * @param queue_conf
 *   Event queue configuration structure
 *
 * @return
 *   Returns 0 on success.
 */
typedef int (*eventdev_queue_setup_t)(struct rte_eventdev *dev,
		uint8_t queue_id,
		const struct rte_event_queue_conf *queue_conf);

/**
 * Release resources allocated by given event queue.
 *
 * @param dev
 *   Event device pointer
 * @param queue_id
 *   Event queue index
 *
 */
typedef void (*eventdev_queue_release_t)(struct rte_eventdev *dev,
		uint8_t queue_id);

/**
 * Retrieve the default event port configuration.
 *
 * @param dev
 *   Event device pointer
 * @param port_id
 *   Event port index
 * @param[out] port_conf
 *   Event port configuration structure
 *
 */
typedef void (*eventdev_port_default_conf_get_t)(struct rte_eventdev *dev,
		uint8_t port_id, struct rte_event_port_conf *port_conf);

/**
 * Setup an event port.
 *
 * @param dev
 *   Event device pointer
 * @param port_id
 *   Event port index
 * @param port_conf
 *   Event port configuration structure
 *
 * @return
 *   Returns 0 on success.
 */
typedef int (*eventdev_port_setup_t)(struct rte_eventdev *dev,
		uint8_t port_id,
		const struct rte_event_port_conf *port_conf);

/**
 * Release memory resources allocated by given event port.
 *
 * @param port
 *   Event port pointer
 *
 */
typedef void (*eventdev_port_release_t)(void *port);

/**
 * Link multiple source event queues to destination event port.
 *
 * @param dev
 *   Event device pointer
 * @param port
 *   Event port pointer
 * @param link
 *   Points to an array of *nb_links* event queues to be linked
 *   to the event port.
 * @param priorities
 *   Points to an array of *nb_links* service priorities associated with each
 *   event queue link to event port.
 * @param nb_links
 *   The number of links to establish
 *
 * @return
 *   Returns 0 on success.
 *
 */
typedef int (*eventdev_port_link_t)(struct rte_eventdev *dev, void *port,
		const uint8_t queues[], const uint8_t priorities[],
		uint16_t nb_links);

/**
 * Unlink multiple source event queues from destination event port.
 *
 * @param dev
 *   Event device pointer
 * @param port
 *   Event port pointer
 * @param queues
 *   An array of *nb_unlinks* event queues to be unlinked from the event port.
 * @param nb_unlinks
 *   The number of unlinks to establish
 *
 * @return
 *   Returns 0 on success.
 *
 */
typedef int (*eventdev_port_unlink_t)(struct rte_eventdev *dev, void *port,
		uint8_t queues[], uint16_t nb_unlinks);

/**
 * Converts nanoseconds to *timeout_ticks* value for rte_event_dequeue()
 *
 * @param dev
 *   Event device pointer
 * @param ns
 *   Wait time in nanosecond
 * @param[out] timeout_ticks
 *   Value for the *timeout_ticks* parameter in rte_event_dequeue() function
 *
 * @return
 *   Returns 0 on success.
 *
 */
typedef int (*eventdev_dequeue_timeout_ticks_t)(struct rte_eventdev *dev,
		uint64_t ns, uint64_t *timeout_ticks);

/**
 * Dump internal information
 *
 * @param dev
 *   Event device pointer
 * @param f
 *   A pointer to a file for output
 *
 */
typedef void (*eventdev_dump_t)(struct rte_eventdev *dev, FILE *f);

/** Event device operations function pointer table */
struct rte_eventdev_ops {
	eventdev_info_get_t dev_infos_get;	/**< Get device info. */
	eventdev_configure_t dev_configure;	/**< Configure device. */
	eventdev_start_t dev_start;		/**< Start device. */
	eventdev_stop_t dev_stop;		/**< Stop device. */
	eventdev_close_t dev_close;		/**< Close device. */

	eventdev_queue_default_conf_get_t queue_def_conf;
	/**< Get default queue configuration. */
	eventdev_queue_setup_t queue_setup;
	/**< Set up an event queue. */
	eventdev_queue_release_t queue_release;
	/**< Release an event queue. */

	eventdev_port_default_conf_get_t port_def_conf;
	/**< Get default port configuration. */
	eventdev_port_setup_t port_setup;
	/**< Set up an event port. */
	eventdev_port_release_t port_release;
	/**< Release an event port. */

	eventdev_port_link_t port_link;
	/**< Link event queues to an event port. */
	eventdev_port_unlink_t port_unlink;
	/**< Unlink event queues from an event port. */
	eventdev_dequeue_timeout_ticks_t timeout_ticks;
	/**< Converts ns to *timeout_ticks* value for rte_event_dequeue() */
	eventdev_dump_t dump;
	/* Dump internal information */
};

/**
 * Allocates a new eventdev slot for an event device and returns the pointer
 * to that slot for the driver to use.
 *
 * @param name
 *   Unique identifier name for each device
 * @param socket_id
 *   Socket to allocate resources on.
 * @return
 *   - Slot in the rte_dev_devices array for a new device;
 */
struct rte_eventdev *
rte_event_pmd_allocate(const char *name, int socket_id);

/**
 * Release the specified eventdev device.
 *
 * @param eventdev
 * The *eventdev* pointer is the address of the *rte_eventdev* structure.
 * @return
 *   - 0 on success, negative on error
 */
int
rte_event_pmd_release(struct rte_eventdev *eventdev);

/**
 * Creates a new virtual event device and returns the pointer to that device.
 *
 * @param name
 *   PMD type name
 * @param dev_private_size
 *   Size of event PMDs private data
 * @param socket_id
 *   Socket to allocate resources on.
 *
 * @return
 *   - Eventdev pointer if device is successfully created.
 *   - NULL if device cannot be created.
 */
struct rte_eventdev *
rte_event_pmd_vdev_init(const char *name, size_t dev_private_size,
		int socket_id);

/**
 * Destroy the given virtual event device
 *
 * @param name
 *   PMD type name
 * @return
 *   - 0 on success, negative on error
 */
int
rte_event_pmd_vdev_uninit(const char *name);

/**
 * Wrapper for use by pci drivers as a .probe function to attach to a event
 * interface.
 */
int rte_event_pmd_pci_probe(struct rte_pci_driver *pci_drv,
			    struct rte_pci_device *pci_dev);

/**
 * Wrapper for use by pci drivers as a .remove function to detach a event
 * interface.
 */
int rte_event_pmd_pci_remove(struct rte_pci_device *pci_dev);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_EVENTDEV_PMD_H_ */
