/*-
 *   BSD LICENSE
 *
 *   Copyright (c) 2017 Intel Corporation. All rights reserved.
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

#ifndef _PMD_I40E_H_
#define _PMD_I40E_H_

/**
 * @file rte_pmd_i40e.h
 *
 * i40e PMD specific functions.
 *
 * @b EXPERIMENTAL: this API may change, or be removed, without prior notice
 *
 */

#include <rte_ethdev.h>

/**
 * Response sent back to i40e driver from user app after callback
 */
enum rte_pmd_i40e_mb_event_rsp {
	RTE_PMD_I40E_MB_EVENT_NOOP_ACK,  /**< skip mbox request and ACK */
	RTE_PMD_I40E_MB_EVENT_NOOP_NACK, /**< skip mbox request and NACK */
	RTE_PMD_I40E_MB_EVENT_PROCEED,  /**< proceed with mbox request  */
	RTE_PMD_I40E_MB_EVENT_MAX       /**< max value of this enum */
};

/**
 * Data sent to the user application when the callback is executed.
 */
struct rte_pmd_i40e_mb_event_param {
	uint16_t vfid;     /**< Virtual Function number */
	uint16_t msg_type; /**< VF to PF message type, see i40e_virtchnl_ops */
	uint16_t retval;   /**< return value */
	void *msg;         /**< pointer to message */
	uint16_t msglen;   /**< length of the message */
};

/**
 * Notify VF when PF link status changes.
 *
 * @param port
 *   The port identifier of the Ethernet device.
 * @param vf
 *   VF id.
 * @return
 *   - (0) if successful.
 *   - (-ENODEV) if *port* invalid.
 *   - (-EINVAL) if *vf* invalid.
 */
int rte_pmd_i40e_ping_vfs(uint8_t port, uint16_t vf);

#endif /* _PMD_I40E_H_ */
