/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2017 Broadcom Limited.
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
 *     * Neither the name of Broadcom Corporation nor the names of its
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

#ifndef _PMD_BNXT_H_
#define _PMD_BNXT_H_

#include <rte_ethdev.h>

/**
 * Enable/Disable VF MAC anti spoof
 *
 * @param port
 *    The port identifier of the Ethernet device.
 * @param vf
 *   VF id.
 * @param on
 *    1 - Enable VF MAC anti spoof.
 *    0 - Disable VF MAC anti spoof.
 *
 * @return
 *   - (0) if successful.
 *   - (-ENODEV) if *port* invalid.
 *   - (-EINVAL) if bad parameter.
 */
int rte_pmd_bnxt_set_vf_mac_anti_spoof(uint8_t port, uint16_t vf, uint8_t on);

/**
 * Set the VF MAC address.
 *
 * @param port
 *   The port identifier of the Ethernet device.
 * @param vf
 *   VF id.
 * @param mac_addr
 *   VF MAC address.
 * @return
 *   - (0) if successful.
 *   - (-ENODEV) if *port* invalid.
 *   - (-EINVAL) if *vf* or *mac_addr* is invalid.
 */
int rte_pmd_bnxt_set_vf_mac_addr(uint8_t port, uint16_t vf,
		struct ether_addr *mac_addr);

/**
 * Enable/Disable vf vlan strip for all queues in a pool
 *
 * @param port
 *    The port identifier of the Ethernet device.
 * @param vf
 *    ID specifying VF.
 * @param on
 *    1 - Enable VF's vlan strip on RX queues.
 *    0 - Disable VF's vlan strip on RX queues.
 *
 * @return
 *   - (0) if successful.
 *   - (-ENOTSUP) if hardware doesn't support this feature.
 *   - (-ENODEV) if *port* invalid.
 *   - (-EINVAL) if bad parameter.
 */
int
rte_pmd_bnxt_set_vf_vlan_stripq(uint8_t port, uint16_t vf, uint8_t on);

/**
 * Enable/Disable hardware VF VLAN filtering by an Ethernet device of
 * received VLAN packets tagged with a given VLAN Tag Identifier.
 *
 * @param port
 *   The port identifier of the Ethernet device.
 * @param vlan
 *   The VLAN Tag Identifier whose filtering must be enabled or disabled.
 * @param vf_mask
 *    Bitmap listing which VFs participate in the VLAN filtering.
 * @param vlan_on
 *    1 - Enable VFs VLAN filtering.
 *    0 - Disable VFs VLAN filtering.
 * @return
 *   - (0) if successful.
 *   - (-ENOTSUP) if hardware doesn't support.
 *   - (-ENODEV) if *port_id* invalid.
 *   - (-EINVAL) if bad parameter.
 */
int rte_pmd_bnxt_set_vf_vlan_filter(uint8_t port, uint16_t vlan,
				    uint64_t vf_mask, uint8_t vlan_on);

/**
 * Enable/Disable tx loopback
 *
 * @param port
 *    The port identifier of the Ethernet device.
 * @param on
 *    1 - Enable tx loopback.
 *    0 - Disable tx loopback.
 *
 * @return
 *   - (0) if successful.
 *   - (-ENODEV) if *port* invalid.
 *   - (-EINVAL) if bad parameter.
 */
int rte_pmd_bnxt_set_tx_loopback(uint8_t port, uint8_t on);

/**
 * set all queues drop enable bit
 *
 * @param port
 *    The port identifier of the Ethernet device.
 * @param on
 *    1 - set the queue drop enable bit for all pools.
 *    0 - reset the queue drop enable bit for all pools.
 *
 * @return
 *   - (0) if successful.
 *   - (-ENODEV) if *port* invalid.
 *   - (-EINVAL) if bad parameter.
 */
int rte_pmd_bnxt_set_all_queues_drop_en(uint8_t port, uint8_t on);

/**
 * Set the VF rate limit.
 *
 * @param port
 *   The port identifier of the Ethernet device.
 * @param vf
 *   VF id.
 * @param tx_rate
 *   Tx rate for the VF
 * @param q_msk
 *   Mask of the Tx queue
 * @return
 *   - (0) if successful.
 *   - (-ENODEV) if *port* invalid.
 *   - (-EINVAL) if *vf* or *mac_addr* is invalid.
 */
int rte_pmd_bnxt_set_vf_rate_limit(uint8_t port, uint16_t vf,
				uint16_t tx_rate, uint64_t q_msk);

/**
 * Get VF's statistics
 *
 * @param port
 *    The port identifier of the Ethernet device.
 * @param vf_id
 *    VF on which to get.
 * @param stats
 *    A pointer to a structure of type *rte_eth_stats* to be filled with
 *    the values of device counters supported statistics:
 * @return
 *   - (0) if successful.
 *   - (-ENODEV) if *port* invalid.
 *   - (-EINVAL) if bad parameter.
 */

int rte_pmd_bnxt_get_vf_stats(uint8_t port,
			      uint16_t vf_id,
			      struct rte_eth_stats *stats);

/**
 * Clear VF's statistics
 *
 * @param port
 *    The port identifier of the Ethernet device.
 * @param vf_id
 *    VF on which to get.
 * @return
 *   - (0) if successful.
 *   - (-ENODEV) if *port* invalid.
 *   - (-EINVAL) if bad parameter.
 */
int rte_pmd_bnxt_reset_vf_stats(uint8_t port,
				uint16_t vf_id);

/**
 * Enable/Disable VF VLAN anti spoof
 *
 * @param port
 *    The port identifier of the Ethernet device.
 * @param vf
 *   VF id.
 * @param on
 *    1 - Enable VF VLAN anti spoof.
 *    0 - Disable VF VLAN anti spoof.
 *
 * @return
 *   - (0) if successful.
 *   - (-ENODEV) if *port* invalid.
 *   - (-EINVAL) if bad parameter.
 */
int rte_pmd_bnxt_set_vf_vlan_anti_spoof(uint8_t port, uint16_t vf, uint8_t on);

/**
 * Queries the TX drop counter for the function
 *
 * @param port
 *    The port identifier of the Ethernet device.
 * @param vf_id
 *    VF on which to get.
 * @param count
 *    Pointer to a uint64_t that will be populated with the counter value.
 * @return
 *   - Positive Non-zero value - Error code from HWRM
 *   - (-EINVAL) invalid vf_id specified.
 *   - (-ENOTSUP) Ethernet device is not a PF
 */
int rte_pmd_bnxt_get_vf_tx_drop_count(uint8_t port, uint16_t vf_id,
				      uint64_t *count);
#endif /* _PMD_BNXT_H_ */
