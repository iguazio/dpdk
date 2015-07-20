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

#ifndef _RTE_INTERRUPTS_H_
#error "don't include this file directly, please include generic <rte_interrupts.h>"
#endif

#ifndef _RTE_LINUXAPP_INTERRUPTS_H_
#define _RTE_LINUXAPP_INTERRUPTS_H_

#define RTE_MAX_RXTX_INTR_VEC_ID     32

enum rte_intr_handle_type {
	RTE_INTR_HANDLE_UNKNOWN = 0,
	RTE_INTR_HANDLE_UIO,          /**< uio device handle */
	RTE_INTR_HANDLE_UIO_INTX,     /**< uio generic handle */
	RTE_INTR_HANDLE_VFIO_LEGACY,  /**< vfio device handle (legacy) */
	RTE_INTR_HANDLE_VFIO_MSI,     /**< vfio device handle (MSI) */
	RTE_INTR_HANDLE_VFIO_MSIX,    /**< vfio device handle (MSIX) */
	RTE_INTR_HANDLE_ALARM,    /**< alarm handle */
	RTE_INTR_HANDLE_MAX
};

#define RTE_INTR_EVENT_ADD            1UL
#define RTE_INTR_EVENT_DEL            2UL

typedef void (*rte_intr_event_cb_t)(int fd, void *arg);

struct rte_epoll_data {
	uint32_t event;               /**< event type */
	void *data;                   /**< User data */
	rte_intr_event_cb_t cb_fun;   /**< IN: callback fun */
	void *cb_arg;	              /**< IN: callback arg */
};

enum {
	RTE_EPOLL_INVALID = 0,
	RTE_EPOLL_VALID,
	RTE_EPOLL_EXEC,
};

/** interrupt epoll event obj, taken by epoll_event.ptr */
struct rte_epoll_event {
	volatile uint32_t status;  /**< OUT: event status */
	int fd;                    /**< OUT: event fd */
	int epfd;       /**< OUT: epoll instance the ev associated with */
	struct rte_epoll_data epdata;
};

/** Handle for interrupts. */
struct rte_intr_handle {
	union {
		int vfio_dev_fd;  /**< VFIO device file descriptor */
		int uio_cfg_fd;  /**< UIO config file descriptor
					for uio_pci_generic */
	};
	int fd;	 /**< interrupt event file descriptor */
	enum rte_intr_handle_type type;  /**< handle type */
#ifdef RTE_NEXT_ABI
	uint32_t max_intr;             /**< max interrupt requested */
	uint32_t nb_efd;               /**< number of available efd(event fd) */
	int efds[RTE_MAX_RXTX_INTR_VEC_ID];  /**< intr vectors/efds mapping */
	struct rte_epoll_event elist[RTE_MAX_RXTX_INTR_VEC_ID];
				       /**< intr vector epoll event */
	int *intr_vec;                 /**< intr vector number array */
#endif
};

#define RTE_EPOLL_PER_THREAD        -1  /**< to hint using per thread epfd */

/**
 * It waits for events on the epoll instance.
 *
 * @param epfd
 *   Epoll instance fd on which the caller wait for events.
 * @param events
 *   Memory area contains the events that will be available for the caller.
 * @param maxevents
 *   Up to maxevents are returned, must greater than zero.
 * @param timeout
 *   Specifying a timeout of -1 causes a block indefinitely.
 *   Specifying a timeout equal to zero cause to return immediately.
 * @return
 *   - On success, returns the number of available event.
 *   - On failure, a negative value.
 */
int
rte_epoll_wait(int epfd, struct rte_epoll_event *events,
	       int maxevents, int timeout);

/**
 * It performs control operations on epoll instance referred by the epfd.
 * It requests that the operation op be performed for the target fd.
 *
 * @param epfd
 *   Epoll instance fd on which the caller perform control operations.
 * @param op
 *   The operation be performed for the target fd.
 * @param fd
 *   The target fd on which the control ops perform.
 * @param event
 *   Describes the object linked to the fd.
 *   Note: The caller must take care the object deletion after CTL_DEL.
 * @return
 *   - On success, zero.
 *   - On failure, a negative value.
 */
int
rte_epoll_ctl(int epfd, int op, int fd,
	      struct rte_epoll_event *event);

/**
 * The function returns the per thread epoll instance.
 *
 * @return
 *   epfd the epoll instance referred to.
 */
int
rte_intr_tls_epfd(void);

#endif /* _RTE_LINUXAPP_INTERRUPTS_H_ */
