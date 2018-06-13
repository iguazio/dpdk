/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */
#ifndef _QAT_QP_H_
#define _QAT_QP_H_

#include "qat_common.h"

typedef int (*build_request_t)(void *op,
		uint8_t *req, void *op_cookie,
		enum qat_device_gen qat_dev_gen);
/**< Build a request from an op. */

typedef int (*process_response_t)(void **ops,
		uint8_t *resp, void *op_cookie,
		enum qat_device_gen qat_dev_gen);
/**< Process a response descriptor and return the associated op. */

/**
 * Structure with data needed for creation of queue pair.
 */
struct qat_qp_config {
	uint8_t hw_bundle_num;
	uint8_t tx_ring_num;
	uint8_t rx_ring_num;
	uint16_t tx_msg_size;
	uint16_t rx_msg_size;
	uint32_t nb_descriptors;
	uint32_t cookie_size;
	int socket_id;
	build_request_t build_request;
	process_response_t process_response;
	const char *service_str;
};

/**
 * Structure associated with each queue.
 */
struct qat_queue {
	char		memz_name[RTE_MEMZONE_NAMESIZE];
	void		*base_addr;		/* Base address */
	rte_iova_t	base_phys_addr;		/* Queue physical address */
	uint32_t	head;			/* Shadow copy of the head */
	uint32_t	tail;			/* Shadow copy of the tail */
	uint32_t	modulo;
	uint32_t	msg_size;
	uint16_t	max_inflights;
	uint32_t	queue_size;
	uint8_t		hw_bundle_number;
	uint8_t		hw_queue_number;
	/* HW queue aka ring offset on bundle */
	uint32_t	csr_head;		/* last written head value */
	uint32_t	csr_tail;		/* last written tail value */
	uint16_t	nb_processed_responses;
	/* number of responses processed since last CSR head write */
	uint16_t	nb_pending_requests;
	/* number of requests pending since last CSR tail write */
};

struct qat_qp {
	void			*mmap_bar_addr;
	uint16_t		inflights16;
	struct	qat_queue	tx_q;
	struct	qat_queue	rx_q;
	struct	rte_cryptodev_stats stats;
	struct rte_mempool *op_cookie_pool;
	void **op_cookies;
	uint32_t nb_descriptors;
	enum qat_device_gen qat_dev_gen;
	build_request_t build_request;
	process_response_t process_response;
} __rte_cache_aligned;

uint16_t
qat_enqueue_op_burst(void *qp, void **ops, uint16_t nb_ops);

uint16_t
qat_dequeue_op_burst(void *qp, void **ops, uint16_t nb_ops);

#endif /* _QAT_QP_H_ */
