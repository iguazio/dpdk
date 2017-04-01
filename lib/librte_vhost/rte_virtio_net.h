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

#ifndef _VIRTIO_NET_H_
#define _VIRTIO_NET_H_

/**
 * @file
 * Interface to vhost net
 */

#include <stdint.h>
#include <linux/vhost.h>
#include <linux/virtio_ring.h>
#include <linux/virtio_net.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <linux/if.h>

#include <rte_memory.h>
#include <rte_mempool.h>
#include <rte_ether.h>

#define RTE_VHOST_USER_CLIENT		(1ULL << 0)
#define RTE_VHOST_USER_NO_RECONNECT	(1ULL << 1)
#define RTE_VHOST_USER_DEQUEUE_ZERO_COPY	(1ULL << 2)

/* Enum for virtqueue management. */
enum {VIRTIO_RXQ, VIRTIO_TXQ, VIRTIO_QNUM};

/**
 * Device and vring operations.
 */
struct virtio_net_device_ops {
	int (*new_device)(int vid);		/**< Add device. */
	void (*destroy_device)(int vid);	/**< Remove device. */

	int (*vring_state_changed)(int vid, uint16_t queue_id, int enable);	/**< triggered when a vring is enabled or disabled */

	void *reserved[5]; /**< Reserved for future extension */
};

int rte_vhost_enable_guest_notification(int vid, uint16_t queue_id, int enable);

/**
 * Register vhost driver. path could be different for multiple
 * instance support.
 */
int rte_vhost_driver_register(const char *path, uint64_t flags);

/* Unregister vhost driver. This is only meaningful to vhost user. */
int rte_vhost_driver_unregister(const char *path);

/**
 * Set the feature bits the vhost-user driver supports.
 *
 * @param path
 *  The vhost-user socket file path
 * @param features
 *  Supported features
 * @return
 *  0 on success, -1 on failure
 */
int rte_vhost_driver_set_features(const char *path, uint64_t features);

/**
 * Enable vhost-user driver features.
 *
 * Note that
 * - the param features should be a subset of the feature bits provided
 *   by rte_vhost_driver_set_features().
 * - it must be invoked before vhost-user negotiation starts.
 *
 * @param path
 *  The vhost-user socket file path
 * @param features
 *  Features to enable
 * @return
 *  0 on success, -1 on failure
 */
int rte_vhost_driver_enable_features(const char *path, uint64_t features);

/**
 * Disable vhost-user driver features.
 *
 * The two notes at rte_vhost_driver_enable_features() also apply here.
 *
 * @param path
 *  The vhost-user socket file path
 * @param features
 *  Features to disable
 * @return
 *  0 on success, -1 on failure
 */
int rte_vhost_driver_disable_features(const char *path, uint64_t features);

/**
 * Get the feature bits before feature negotiation.
 *
 * @param path
 *  The vhost-user socket file path
 * @param features
 *  A pointer to store the queried feature bits
 * @return
 *  0 on success, -1 on failure
 */
int rte_vhost_driver_get_features(const char *path, uint64_t *features);

/* Register callbacks. */
int rte_vhost_driver_callback_register(const char *path,
	struct virtio_net_device_ops const * const ops);
/* Start vhost driver session blocking loop. */
int rte_vhost_driver_session_start(void);

/**
 * Get the MTU value of the device if set in QEMU.
 *
 * @param vid
 *  virtio-net device ID
 * @param mtu
 *  The variable to store the MTU value
 *
 * @return
 *  0: success
 *  -EAGAIN: device not yet started
 *  -ENOTSUP: device does not support MTU feature
 */
int rte_vhost_get_mtu(int vid, uint16_t *mtu);

/**
 * Get the numa node from which the virtio net device's memory
 * is allocated.
 *
 * @param vid
 *  virtio-net device ID
 *
 * @return
 *  The numa node, -1 on failure
 */
int rte_vhost_get_numa_node(int vid);

/**
 * Get the number of queues the device supports.
 *
 * @param vid
 *  virtio-net device ID
 *
 * @return
 *  The number of queues, 0 on failure
 */
uint32_t rte_vhost_get_queue_num(int vid);

/**
 * Get the virtio net device's ifname, which is the vhost-user socket
 * file path.
 *
 * @param vid
 *  virtio-net device ID
 * @param buf
 *  The buffer to stored the queried ifname
 * @param len
 *  The length of buf
 *
 * @return
 *  0 on success, -1 on failure
 */
int rte_vhost_get_ifname(int vid, char *buf, size_t len);

/**
 * Get how many avail entries are left in the queue
 *
 * @param vid
 *  virtio-net device ID
 * @param queue_id
 *  virtio queue index
 *
 * @return
 *  num of avail entires left
 */
uint16_t rte_vhost_avail_entries(int vid, uint16_t queue_id);

/**
 * This function adds buffers to the virtio devices RX virtqueue. Buffers can
 * be received from the physical port or from another virtual device. A packet
 * count is returned to indicate the number of packets that were succesfully
 * added to the RX queue.
 * @param vid
 *  virtio-net device ID
 * @param queue_id
 *  virtio queue index in mq case
 * @param pkts
 *  array to contain packets to be enqueued
 * @param count
 *  packets num to be enqueued
 * @return
 *  num of packets enqueued
 */
uint16_t rte_vhost_enqueue_burst(int vid, uint16_t queue_id,
	struct rte_mbuf **pkts, uint16_t count);

/**
 * This function gets guest buffers from the virtio device TX virtqueue,
 * construct host mbufs, copies guest buffer content to host mbufs and
 * store them in pkts to be processed.
 * @param vid
 *  virtio-net device
 * @param queue_id
 *  virtio queue index in mq case
 * @param mbuf_pool
 *  mbuf_pool where host mbuf is allocated.
 * @param pkts
 *  array to contain packets to be dequeued
 * @param count
 *  packets num to be dequeued
 * @return
 *  num of packets dequeued
 */
uint16_t rte_vhost_dequeue_burst(int vid, uint16_t queue_id,
	struct rte_mempool *mbuf_pool, struct rte_mbuf **pkts, uint16_t count);

#endif /* _VIRTIO_NET_H_ */
