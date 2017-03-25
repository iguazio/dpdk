/*
 *   BSD LICENSE
 *
 *   Copyright(c) 2017 Cavium, Inc.. All rights reserved.
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
 *     * Neither the name of Cavium, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER(S) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_malloc.h>
#include <rte_alarm.h>

#include "lio_logs.h"
#include "lio_23xx_vf.h"
#include "lio_ethdev.h"
#include "lio_rxtx.h"

/* Default RSS key in use */
static uint8_t lio_rss_key[40] = {
	0x6D, 0x5A, 0x56, 0xDA, 0x25, 0x5B, 0x0E, 0xC2,
	0x41, 0x67, 0x25, 0x3D, 0x43, 0xA3, 0x8F, 0xB0,
	0xD0, 0xCA, 0x2B, 0xCB, 0xAE, 0x7B, 0x30, 0xB4,
	0x77, 0xCB, 0x2D, 0xA3, 0x80, 0x30, 0xF2, 0x0C,
	0x6A, 0x42, 0xB7, 0x3B, 0xBE, 0xAC, 0x01, 0xFA,
};

static const struct rte_eth_desc_lim lio_rx_desc_lim = {
	.nb_max		= CN23XX_MAX_OQ_DESCRIPTORS,
	.nb_min		= CN23XX_MIN_OQ_DESCRIPTORS,
	.nb_align	= 1,
};

static const struct rte_eth_desc_lim lio_tx_desc_lim = {
	.nb_max		= CN23XX_MAX_IQ_DESCRIPTORS,
	.nb_min		= CN23XX_MIN_IQ_DESCRIPTORS,
	.nb_align	= 1,
};

/* Wait for control command to reach nic. */
static uint16_t
lio_wait_for_ctrl_cmd(struct lio_device *lio_dev,
		      struct lio_dev_ctrl_cmd *ctrl_cmd)
{
	uint16_t timeout = LIO_MAX_CMD_TIMEOUT;

	while ((ctrl_cmd->cond == 0) && --timeout) {
		lio_flush_iq(lio_dev, lio_dev->instr_queue[0]);
		rte_delay_ms(1);
	}

	return !timeout;
}

/**
 * \brief Send Rx control command
 * @param eth_dev Pointer to the structure rte_eth_dev
 * @param start_stop whether to start or stop
 */
static int
lio_send_rx_ctrl_cmd(struct rte_eth_dev *eth_dev, int start_stop)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_dev_ctrl_cmd ctrl_cmd;
	struct lio_ctrl_pkt ctrl_pkt;

	/* flush added to prevent cmd failure
	 * incase the queue is full
	 */
	lio_flush_iq(lio_dev, lio_dev->instr_queue[0]);

	memset(&ctrl_pkt, 0, sizeof(struct lio_ctrl_pkt));
	memset(&ctrl_cmd, 0, sizeof(struct lio_dev_ctrl_cmd));

	ctrl_cmd.eth_dev = eth_dev;
	ctrl_cmd.cond = 0;

	ctrl_pkt.ncmd.s.cmd = LIO_CMD_RX_CTL;
	ctrl_pkt.ncmd.s.param1 = start_stop;
	ctrl_pkt.ctrl_cmd = &ctrl_cmd;

	if (lio_send_ctrl_pkt(lio_dev, &ctrl_pkt)) {
		lio_dev_err(lio_dev, "Failed to send RX Control message\n");
		return -1;
	}

	if (lio_wait_for_ctrl_cmd(lio_dev, &ctrl_cmd)) {
		lio_dev_err(lio_dev, "RX Control command timed out\n");
		return -1;
	}

	return 0;
}

/* Retrieve the device statistics (# packets in/out, # bytes in/out, etc */
static void
lio_dev_stats_get(struct rte_eth_dev *eth_dev,
		  struct rte_eth_stats *stats)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_droq_stats *oq_stats;
	struct lio_iq_stats *iq_stats;
	struct lio_instr_queue *txq;
	struct lio_droq *droq;
	int i, iq_no, oq_no;
	uint64_t bytes = 0;
	uint64_t pkts = 0;
	uint64_t drop = 0;

	for (i = 0; i < eth_dev->data->nb_tx_queues; i++) {
		iq_no = lio_dev->linfo.txpciq[i].s.q_no;
		txq = lio_dev->instr_queue[iq_no];
		if (txq != NULL) {
			iq_stats = &txq->stats;
			pkts += iq_stats->tx_done;
			drop += iq_stats->tx_dropped;
			bytes += iq_stats->tx_tot_bytes;
		}
	}

	stats->opackets = pkts;
	stats->obytes = bytes;
	stats->oerrors = drop;

	pkts = 0;
	drop = 0;
	bytes = 0;

	for (i = 0; i < eth_dev->data->nb_rx_queues; i++) {
		oq_no = lio_dev->linfo.rxpciq[i].s.q_no;
		droq = lio_dev->droq[oq_no];
		if (droq != NULL) {
			oq_stats = &droq->stats;
			pkts += oq_stats->rx_pkts_received;
			drop += (oq_stats->rx_dropped +
					oq_stats->dropped_toomany +
					oq_stats->dropped_nomem);
			bytes += oq_stats->rx_bytes_received;
		}
	}
	stats->ibytes = bytes;
	stats->ipackets = pkts;
	stats->ierrors = drop;
}

static void
lio_dev_stats_reset(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_droq_stats *oq_stats;
	struct lio_iq_stats *iq_stats;
	struct lio_instr_queue *txq;
	struct lio_droq *droq;
	int i, iq_no, oq_no;

	for (i = 0; i < eth_dev->data->nb_tx_queues; i++) {
		iq_no = lio_dev->linfo.txpciq[i].s.q_no;
		txq = lio_dev->instr_queue[iq_no];
		if (txq != NULL) {
			iq_stats = &txq->stats;
			memset(iq_stats, 0, sizeof(struct lio_iq_stats));
		}
	}

	for (i = 0; i < eth_dev->data->nb_rx_queues; i++) {
		oq_no = lio_dev->linfo.rxpciq[i].s.q_no;
		droq = lio_dev->droq[oq_no];
		if (droq != NULL) {
			oq_stats = &droq->stats;
			memset(oq_stats, 0, sizeof(struct lio_droq_stats));
		}
	}
}

static void
lio_dev_info_get(struct rte_eth_dev *eth_dev,
		 struct rte_eth_dev_info *devinfo)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);

	devinfo->max_rx_queues = lio_dev->max_rx_queues;
	devinfo->max_tx_queues = lio_dev->max_tx_queues;

	devinfo->min_rx_bufsize = LIO_MIN_RX_BUF_SIZE;
	devinfo->max_rx_pktlen = LIO_MAX_RX_PKTLEN;

	devinfo->max_mac_addrs = 1;

	devinfo->rx_offload_capa = (DEV_RX_OFFLOAD_IPV4_CKSUM		|
				    DEV_RX_OFFLOAD_UDP_CKSUM		|
				    DEV_RX_OFFLOAD_TCP_CKSUM);
	devinfo->tx_offload_capa = (DEV_TX_OFFLOAD_IPV4_CKSUM		|
				    DEV_TX_OFFLOAD_UDP_CKSUM		|
				    DEV_TX_OFFLOAD_TCP_CKSUM		|
				    DEV_TX_OFFLOAD_OUTER_IPV4_CKSUM);

	devinfo->rx_desc_lim = lio_rx_desc_lim;
	devinfo->tx_desc_lim = lio_tx_desc_lim;

	devinfo->reta_size = LIO_RSS_MAX_TABLE_SZ;
	devinfo->hash_key_size = LIO_RSS_MAX_KEY_SZ;
	devinfo->flow_type_rss_offloads = (ETH_RSS_IPV4			|
					   ETH_RSS_NONFRAG_IPV4_TCP	|
					   ETH_RSS_IPV6			|
					   ETH_RSS_NONFRAG_IPV6_TCP	|
					   ETH_RSS_IPV6_EX		|
					   ETH_RSS_IPV6_TCP_EX);
}

static int
lio_dev_validate_vf_mtu(struct rte_eth_dev *eth_dev, uint16_t new_mtu)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);

	PMD_INIT_FUNC_TRACE();

	if (!lio_dev->intf_open) {
		lio_dev_err(lio_dev, "Port %d down, can't check MTU\n",
			    lio_dev->port_id);
		return -EINVAL;
	}

	/* Limit the MTU to make sure the ethernet packets are between
	 * ETHER_MIN_MTU bytes and PF's MTU
	 */
	if ((new_mtu < ETHER_MIN_MTU) ||
			(new_mtu > lio_dev->linfo.link.s.mtu)) {
		lio_dev_err(lio_dev, "Invalid MTU: %d\n", new_mtu);
		lio_dev_err(lio_dev, "Valid range %d and %d\n",
			    ETHER_MIN_MTU, lio_dev->linfo.link.s.mtu);
		return -EINVAL;
	}

	return 0;
}

static int
lio_dev_rss_reta_update(struct rte_eth_dev *eth_dev,
			struct rte_eth_rss_reta_entry64 *reta_conf,
			uint16_t reta_size)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_rss_ctx *rss_state = &lio_dev->rss_state;
	struct lio_rss_set *rss_param;
	struct lio_dev_ctrl_cmd ctrl_cmd;
	struct lio_ctrl_pkt ctrl_pkt;
	int i, j, index;

	if (!lio_dev->intf_open) {
		lio_dev_err(lio_dev, "Port %d down, can't update reta\n",
			    lio_dev->port_id);
		return -EINVAL;
	}

	if (reta_size != LIO_RSS_MAX_TABLE_SZ) {
		lio_dev_err(lio_dev,
			    "The size of hash lookup table configured (%d) doesn't match the number hardware can supported (%d)\n",
			    reta_size, LIO_RSS_MAX_TABLE_SZ);
		return -EINVAL;
	}

	/* flush added to prevent cmd failure
	 * incase the queue is full
	 */
	lio_flush_iq(lio_dev, lio_dev->instr_queue[0]);

	memset(&ctrl_pkt, 0, sizeof(struct lio_ctrl_pkt));
	memset(&ctrl_cmd, 0, sizeof(struct lio_dev_ctrl_cmd));

	rss_param = (struct lio_rss_set *)&ctrl_pkt.udd[0];

	ctrl_cmd.eth_dev = eth_dev;
	ctrl_cmd.cond = 0;

	ctrl_pkt.ncmd.s.cmd = LIO_CMD_SET_RSS;
	ctrl_pkt.ncmd.s.more = sizeof(struct lio_rss_set) >> 3;
	ctrl_pkt.ctrl_cmd = &ctrl_cmd;

	rss_param->param.flags = 0xF;
	rss_param->param.flags &= ~LIO_RSS_PARAM_ITABLE_UNCHANGED;
	rss_param->param.itablesize = LIO_RSS_MAX_TABLE_SZ;

	for (i = 0; i < (reta_size / RTE_RETA_GROUP_SIZE); i++) {
		for (j = 0; j < RTE_RETA_GROUP_SIZE; j++) {
			if ((reta_conf[i].mask) & ((uint64_t)1 << j)) {
				index = (i * RTE_RETA_GROUP_SIZE) + j;
				rss_state->itable[index] = reta_conf[i].reta[j];
			}
		}
	}

	rss_state->itable_size = LIO_RSS_MAX_TABLE_SZ;
	memcpy(rss_param->itable, rss_state->itable, rss_state->itable_size);

	lio_swap_8B_data((uint64_t *)rss_param, LIO_RSS_PARAM_SIZE >> 3);

	if (lio_send_ctrl_pkt(lio_dev, &ctrl_pkt)) {
		lio_dev_err(lio_dev, "Failed to set rss hash\n");
		return -1;
	}

	if (lio_wait_for_ctrl_cmd(lio_dev, &ctrl_cmd)) {
		lio_dev_err(lio_dev, "Set rss hash timed out\n");
		return -1;
	}

	return 0;
}

static int
lio_dev_rss_reta_query(struct rte_eth_dev *eth_dev,
		       struct rte_eth_rss_reta_entry64 *reta_conf,
		       uint16_t reta_size)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_rss_ctx *rss_state = &lio_dev->rss_state;
	int i, num;

	if (reta_size != LIO_RSS_MAX_TABLE_SZ) {
		lio_dev_err(lio_dev,
			    "The size of hash lookup table configured (%d) doesn't match the number hardware can supported (%d)\n",
			    reta_size, LIO_RSS_MAX_TABLE_SZ);
		return -EINVAL;
	}

	num = reta_size / RTE_RETA_GROUP_SIZE;

	for (i = 0; i < num; i++) {
		memcpy(reta_conf->reta,
		       &rss_state->itable[i * RTE_RETA_GROUP_SIZE],
		       RTE_RETA_GROUP_SIZE);
		reta_conf++;
	}

	return 0;
}

static int
lio_dev_rss_hash_conf_get(struct rte_eth_dev *eth_dev,
			  struct rte_eth_rss_conf *rss_conf)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_rss_ctx *rss_state = &lio_dev->rss_state;
	uint8_t *hash_key = NULL;
	uint64_t rss_hf = 0;

	if (rss_state->hash_disable) {
		lio_dev_info(lio_dev, "RSS disabled in nic\n");
		rss_conf->rss_hf = 0;
		return 0;
	}

	/* Get key value */
	hash_key = rss_conf->rss_key;
	if (hash_key != NULL)
		memcpy(hash_key, rss_state->hash_key, rss_state->hash_key_size);

	if (rss_state->ip)
		rss_hf |= ETH_RSS_IPV4;
	if (rss_state->tcp_hash)
		rss_hf |= ETH_RSS_NONFRAG_IPV4_TCP;
	if (rss_state->ipv6)
		rss_hf |= ETH_RSS_IPV6;
	if (rss_state->ipv6_tcp_hash)
		rss_hf |= ETH_RSS_NONFRAG_IPV6_TCP;
	if (rss_state->ipv6_ex)
		rss_hf |= ETH_RSS_IPV6_EX;
	if (rss_state->ipv6_tcp_ex_hash)
		rss_hf |= ETH_RSS_IPV6_TCP_EX;

	rss_conf->rss_hf = rss_hf;

	return 0;
}

static int
lio_dev_rss_hash_update(struct rte_eth_dev *eth_dev,
			struct rte_eth_rss_conf *rss_conf)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_rss_ctx *rss_state = &lio_dev->rss_state;
	struct lio_rss_set *rss_param;
	struct lio_dev_ctrl_cmd ctrl_cmd;
	struct lio_ctrl_pkt ctrl_pkt;

	if (!lio_dev->intf_open) {
		lio_dev_err(lio_dev, "Port %d down, can't update hash\n",
			    lio_dev->port_id);
		return -EINVAL;
	}

	/* flush added to prevent cmd failure
	 * incase the queue is full
	 */
	lio_flush_iq(lio_dev, lio_dev->instr_queue[0]);

	memset(&ctrl_pkt, 0, sizeof(struct lio_ctrl_pkt));
	memset(&ctrl_cmd, 0, sizeof(struct lio_dev_ctrl_cmd));

	rss_param = (struct lio_rss_set *)&ctrl_pkt.udd[0];

	ctrl_cmd.eth_dev = eth_dev;
	ctrl_cmd.cond = 0;

	ctrl_pkt.ncmd.s.cmd = LIO_CMD_SET_RSS;
	ctrl_pkt.ncmd.s.more = sizeof(struct lio_rss_set) >> 3;
	ctrl_pkt.ctrl_cmd = &ctrl_cmd;

	rss_param->param.flags = 0xF;

	if (rss_conf->rss_key) {
		rss_param->param.flags &= ~LIO_RSS_PARAM_HASH_KEY_UNCHANGED;
		rss_state->hash_key_size = LIO_RSS_MAX_KEY_SZ;
		rss_param->param.hashkeysize = LIO_RSS_MAX_KEY_SZ;
		memcpy(rss_state->hash_key, rss_conf->rss_key,
		       rss_state->hash_key_size);
		memcpy(rss_param->key, rss_state->hash_key,
		       rss_state->hash_key_size);
	}

	if ((rss_conf->rss_hf & LIO_RSS_OFFLOAD_ALL) == 0) {
		/* Can't disable rss through hash flags,
		 * if it is enabled by default during init
		 */
		if (!rss_state->hash_disable)
			return -EINVAL;

		/* This is for --disable-rss during testpmd launch */
		rss_param->param.flags |= LIO_RSS_PARAM_DISABLE_RSS;
	} else {
		uint32_t hashinfo = 0;

		/* Can't enable rss if disabled by default during init */
		if (rss_state->hash_disable)
			return -EINVAL;

		if (rss_conf->rss_hf & ETH_RSS_IPV4) {
			hashinfo |= LIO_RSS_HASH_IPV4;
			rss_state->ip = 1;
		} else {
			rss_state->ip = 0;
		}

		if (rss_conf->rss_hf & ETH_RSS_NONFRAG_IPV4_TCP) {
			hashinfo |= LIO_RSS_HASH_TCP_IPV4;
			rss_state->tcp_hash = 1;
		} else {
			rss_state->tcp_hash = 0;
		}

		if (rss_conf->rss_hf & ETH_RSS_IPV6) {
			hashinfo |= LIO_RSS_HASH_IPV6;
			rss_state->ipv6 = 1;
		} else {
			rss_state->ipv6 = 0;
		}

		if (rss_conf->rss_hf & ETH_RSS_NONFRAG_IPV6_TCP) {
			hashinfo |= LIO_RSS_HASH_TCP_IPV6;
			rss_state->ipv6_tcp_hash = 1;
		} else {
			rss_state->ipv6_tcp_hash = 0;
		}

		if (rss_conf->rss_hf & ETH_RSS_IPV6_EX) {
			hashinfo |= LIO_RSS_HASH_IPV6_EX;
			rss_state->ipv6_ex = 1;
		} else {
			rss_state->ipv6_ex = 0;
		}

		if (rss_conf->rss_hf & ETH_RSS_IPV6_TCP_EX) {
			hashinfo |= LIO_RSS_HASH_TCP_IPV6_EX;
			rss_state->ipv6_tcp_ex_hash = 1;
		} else {
			rss_state->ipv6_tcp_ex_hash = 0;
		}

		rss_param->param.flags &= ~LIO_RSS_PARAM_HASH_INFO_UNCHANGED;
		rss_param->param.hashinfo = hashinfo;
	}

	lio_swap_8B_data((uint64_t *)rss_param, LIO_RSS_PARAM_SIZE >> 3);

	if (lio_send_ctrl_pkt(lio_dev, &ctrl_pkt)) {
		lio_dev_err(lio_dev, "Failed to set rss hash\n");
		return -1;
	}

	if (lio_wait_for_ctrl_cmd(lio_dev, &ctrl_cmd)) {
		lio_dev_err(lio_dev, "Set rss hash timed out\n");
		return -1;
	}

	return 0;
}

/**
 * Add vxlan dest udp port for an interface.
 *
 * @param eth_dev
 *  Pointer to the structure rte_eth_dev
 * @param udp_tnl
 *  udp tunnel conf
 *
 * @return
 *  On success return 0
 *  On failure return -1
 */
static int
lio_dev_udp_tunnel_add(struct rte_eth_dev *eth_dev,
		       struct rte_eth_udp_tunnel *udp_tnl)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_dev_ctrl_cmd ctrl_cmd;
	struct lio_ctrl_pkt ctrl_pkt;

	if (udp_tnl == NULL)
		return -EINVAL;

	if (udp_tnl->prot_type != RTE_TUNNEL_TYPE_VXLAN) {
		lio_dev_err(lio_dev, "Unsupported tunnel type\n");
		return -1;
	}

	/* flush added to prevent cmd failure
	 * incase the queue is full
	 */
	lio_flush_iq(lio_dev, lio_dev->instr_queue[0]);

	memset(&ctrl_pkt, 0, sizeof(struct lio_ctrl_pkt));
	memset(&ctrl_cmd, 0, sizeof(struct lio_dev_ctrl_cmd));

	ctrl_cmd.eth_dev = eth_dev;
	ctrl_cmd.cond = 0;

	ctrl_pkt.ncmd.s.cmd = LIO_CMD_VXLAN_PORT_CONFIG;
	ctrl_pkt.ncmd.s.param1 = udp_tnl->udp_port;
	ctrl_pkt.ncmd.s.more = LIO_CMD_VXLAN_PORT_ADD;
	ctrl_pkt.ctrl_cmd = &ctrl_cmd;

	if (lio_send_ctrl_pkt(lio_dev, &ctrl_pkt)) {
		lio_dev_err(lio_dev, "Failed to send VXLAN_PORT_ADD command\n");
		return -1;
	}

	if (lio_wait_for_ctrl_cmd(lio_dev, &ctrl_cmd)) {
		lio_dev_err(lio_dev, "VXLAN_PORT_ADD command timed out\n");
		return -1;
	}

	return 0;
}

/**
 * Remove vxlan dest udp port for an interface.
 *
 * @param eth_dev
 *  Pointer to the structure rte_eth_dev
 * @param udp_tnl
 *  udp tunnel conf
 *
 * @return
 *  On success return 0
 *  On failure return -1
 */
static int
lio_dev_udp_tunnel_del(struct rte_eth_dev *eth_dev,
		       struct rte_eth_udp_tunnel *udp_tnl)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_dev_ctrl_cmd ctrl_cmd;
	struct lio_ctrl_pkt ctrl_pkt;

	if (udp_tnl == NULL)
		return -EINVAL;

	if (udp_tnl->prot_type != RTE_TUNNEL_TYPE_VXLAN) {
		lio_dev_err(lio_dev, "Unsupported tunnel type\n");
		return -1;
	}

	/* flush added to prevent cmd failure
	 * incase the queue is full
	 */
	lio_flush_iq(lio_dev, lio_dev->instr_queue[0]);

	memset(&ctrl_pkt, 0, sizeof(struct lio_ctrl_pkt));
	memset(&ctrl_cmd, 0, sizeof(struct lio_dev_ctrl_cmd));

	ctrl_cmd.eth_dev = eth_dev;
	ctrl_cmd.cond = 0;

	ctrl_pkt.ncmd.s.cmd = LIO_CMD_VXLAN_PORT_CONFIG;
	ctrl_pkt.ncmd.s.param1 = udp_tnl->udp_port;
	ctrl_pkt.ncmd.s.more = LIO_CMD_VXLAN_PORT_DEL;
	ctrl_pkt.ctrl_cmd = &ctrl_cmd;

	if (lio_send_ctrl_pkt(lio_dev, &ctrl_pkt)) {
		lio_dev_err(lio_dev, "Failed to send VXLAN_PORT_DEL command\n");
		return -1;
	}

	if (lio_wait_for_ctrl_cmd(lio_dev, &ctrl_cmd)) {
		lio_dev_err(lio_dev, "VXLAN_PORT_DEL command timed out\n");
		return -1;
	}

	return 0;
}

/**
 * Atomically writes the link status information into global
 * structure rte_eth_dev.
 *
 * @param eth_dev
 *   - Pointer to the structure rte_eth_dev to read from.
 *   - Pointer to the buffer to be saved with the link status.
 *
 * @return
 *   - On success, zero.
 *   - On failure, negative value.
 */
static inline int
lio_dev_atomic_write_link_status(struct rte_eth_dev *eth_dev,
				 struct rte_eth_link *link)
{
	struct rte_eth_link *dst = &eth_dev->data->dev_link;
	struct rte_eth_link *src = link;

	if (rte_atomic64_cmpset((uint64_t *)dst, *(uint64_t *)dst,
				*(uint64_t *)src) == 0)
		return -1;

	return 0;
}

static uint64_t
lio_hweight64(uint64_t w)
{
	uint64_t res = w - ((w >> 1) & 0x5555555555555555ul);

	res =
	    (res & 0x3333333333333333ul) + ((res >> 2) & 0x3333333333333333ul);
	res = (res + (res >> 4)) & 0x0F0F0F0F0F0F0F0Ful;
	res = res + (res >> 8);
	res = res + (res >> 16);

	return (res + (res >> 32)) & 0x00000000000000FFul;
}

static int
lio_dev_link_update(struct rte_eth_dev *eth_dev,
		    int wait_to_complete __rte_unused)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct rte_eth_link link, old;

	/* Initialize */
	link.link_status = ETH_LINK_DOWN;
	link.link_speed = ETH_SPEED_NUM_NONE;
	link.link_duplex = ETH_LINK_HALF_DUPLEX;
	memset(&old, 0, sizeof(old));

	/* Return what we found */
	if (lio_dev->linfo.link.s.link_up == 0) {
		/* Interface is down */
		if (lio_dev_atomic_write_link_status(eth_dev, &link))
			return -1;
		if (link.link_status == old.link_status)
			return -1;
		return 0;
	}

	link.link_status = ETH_LINK_UP; /* Interface is up */
	link.link_duplex = ETH_LINK_FULL_DUPLEX;
	switch (lio_dev->linfo.link.s.speed) {
	case LIO_LINK_SPEED_10000:
		link.link_speed = ETH_SPEED_NUM_10G;
		break;
	default:
		link.link_speed = ETH_SPEED_NUM_NONE;
		link.link_duplex = ETH_LINK_HALF_DUPLEX;
	}

	if (lio_dev_atomic_write_link_status(eth_dev, &link))
		return -1;

	if (link.link_status == old.link_status)
		return -1;

	return 0;
}

/**
 * \brief Net device enable, disable allmulticast
 * @param eth_dev Pointer to the structure rte_eth_dev
 */
static void
lio_change_dev_flag(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_dev_ctrl_cmd ctrl_cmd;
	struct lio_ctrl_pkt ctrl_pkt;

	/* flush added to prevent cmd failure
	 * incase the queue is full
	 */
	lio_flush_iq(lio_dev, lio_dev->instr_queue[0]);

	memset(&ctrl_pkt, 0, sizeof(struct lio_ctrl_pkt));
	memset(&ctrl_cmd, 0, sizeof(struct lio_dev_ctrl_cmd));

	ctrl_cmd.eth_dev = eth_dev;
	ctrl_cmd.cond = 0;

	/* Create a ctrl pkt command to be sent to core app. */
	ctrl_pkt.ncmd.s.cmd = LIO_CMD_CHANGE_DEVFLAGS;
	ctrl_pkt.ncmd.s.param1 = lio_dev->ifflags;
	ctrl_pkt.ctrl_cmd = &ctrl_cmd;

	if (lio_send_ctrl_pkt(lio_dev, &ctrl_pkt)) {
		lio_dev_err(lio_dev, "Failed to send change flag message\n");
		return;
	}

	if (lio_wait_for_ctrl_cmd(lio_dev, &ctrl_cmd))
		lio_dev_err(lio_dev, "Change dev flag command timed out\n");
}

static void
lio_dev_allmulticast_enable(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);

	if (!lio_dev->intf_open) {
		lio_dev_err(lio_dev, "Port %d down, can't enable multicast\n",
			    lio_dev->port_id);
		return;
	}

	lio_dev->ifflags |= LIO_IFFLAG_ALLMULTI;
	lio_change_dev_flag(eth_dev);
}

static void
lio_dev_allmulticast_disable(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);

	if (!lio_dev->intf_open) {
		lio_dev_err(lio_dev, "Port %d down, can't disable multicast\n",
			    lio_dev->port_id);
		return;
	}

	lio_dev->ifflags &= ~LIO_IFFLAG_ALLMULTI;
	lio_change_dev_flag(eth_dev);
}

static void
lio_dev_rss_configure(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_rss_ctx *rss_state = &lio_dev->rss_state;
	struct rte_eth_rss_reta_entry64 reta_conf[8];
	struct rte_eth_rss_conf rss_conf;
	uint16_t i;

	/* Configure the RSS key and the RSS protocols used to compute
	 * the RSS hash of input packets.
	 */
	rss_conf = eth_dev->data->dev_conf.rx_adv_conf.rss_conf;
	if ((rss_conf.rss_hf & LIO_RSS_OFFLOAD_ALL) == 0) {
		rss_state->hash_disable = 1;
		lio_dev_rss_hash_update(eth_dev, &rss_conf);
		return;
	}

	if (rss_conf.rss_key == NULL)
		rss_conf.rss_key = lio_rss_key; /* Default hash key */

	lio_dev_rss_hash_update(eth_dev, &rss_conf);

	memset(reta_conf, 0, sizeof(reta_conf));
	for (i = 0; i < LIO_RSS_MAX_TABLE_SZ; i++) {
		uint8_t q_idx, conf_idx, reta_idx;

		q_idx = (uint8_t)((eth_dev->data->nb_rx_queues > 1) ?
				  i % eth_dev->data->nb_rx_queues : 0);
		conf_idx = i / RTE_RETA_GROUP_SIZE;
		reta_idx = i % RTE_RETA_GROUP_SIZE;
		reta_conf[conf_idx].reta[reta_idx] = q_idx;
		reta_conf[conf_idx].mask |= ((uint64_t)1 << reta_idx);
	}

	lio_dev_rss_reta_update(eth_dev, reta_conf, LIO_RSS_MAX_TABLE_SZ);
}

static void
lio_dev_mq_rx_configure(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_rss_ctx *rss_state = &lio_dev->rss_state;
	struct rte_eth_rss_conf rss_conf;

	switch (eth_dev->data->dev_conf.rxmode.mq_mode) {
	case ETH_MQ_RX_RSS:
		lio_dev_rss_configure(eth_dev);
		break;
	case ETH_MQ_RX_NONE:
	/* if mq_mode is none, disable rss mode. */
	default:
		memset(&rss_conf, 0, sizeof(rss_conf));
		rss_state->hash_disable = 1;
		lio_dev_rss_hash_update(eth_dev, &rss_conf);
	}
}

/**
 * Setup our receive queue/ringbuffer. This is the
 * queue the Octeon uses to send us packets and
 * responses. We are given a memory pool for our
 * packet buffers that are used to populate the receive
 * queue.
 *
 * @param eth_dev
 *    Pointer to the structure rte_eth_dev
 * @param q_no
 *    Queue number
 * @param num_rx_descs
 *    Number of entries in the queue
 * @param socket_id
 *    Where to allocate memory
 * @param rx_conf
 *    Pointer to the struction rte_eth_rxconf
 * @param mp
 *    Pointer to the packet pool
 *
 * @return
 *    - On success, return 0
 *    - On failure, return -1
 */
static int
lio_dev_rx_queue_setup(struct rte_eth_dev *eth_dev, uint16_t q_no,
		       uint16_t num_rx_descs, unsigned int socket_id,
		       const struct rte_eth_rxconf *rx_conf __rte_unused,
		       struct rte_mempool *mp)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct rte_pktmbuf_pool_private *mbp_priv;
	uint32_t fw_mapped_oq;
	uint16_t buf_size;

	if (q_no >= lio_dev->nb_rx_queues) {
		lio_dev_err(lio_dev, "Invalid rx queue number %u\n", q_no);
		return -EINVAL;
	}

	lio_dev_dbg(lio_dev, "setting up rx queue %u\n", q_no);

	fw_mapped_oq = lio_dev->linfo.rxpciq[q_no].s.q_no;

	if ((lio_dev->droq[fw_mapped_oq]) &&
	    (num_rx_descs != lio_dev->droq[fw_mapped_oq]->max_count)) {
		lio_dev_err(lio_dev,
			    "Reconfiguring Rx descs not supported. Configure descs to same value %u or restart application\n",
			    lio_dev->droq[fw_mapped_oq]->max_count);
		return -ENOTSUP;
	}

	mbp_priv = rte_mempool_get_priv(mp);
	buf_size = mbp_priv->mbuf_data_room_size - RTE_PKTMBUF_HEADROOM;

	if (lio_setup_droq(lio_dev, fw_mapped_oq, num_rx_descs, buf_size, mp,
			   socket_id)) {
		lio_dev_err(lio_dev, "droq allocation failed\n");
		return -1;
	}

	eth_dev->data->rx_queues[q_no] = lio_dev->droq[fw_mapped_oq];

	return 0;
}

/**
 * Release the receive queue/ringbuffer. Called by
 * the upper layers.
 *
 * @param rxq
 *    Opaque pointer to the receive queue to release
 *
 * @return
 *    - nothing
 */
static void
lio_dev_rx_queue_release(void *rxq)
{
	struct lio_droq *droq = rxq;
	struct lio_device *lio_dev = droq->lio_dev;
	int oq_no;

	/* Run time queue deletion not supported */
	if (lio_dev->port_configured)
		return;

	if (droq != NULL) {
		oq_no = droq->q_no;
		lio_delete_droq_queue(droq->lio_dev, oq_no);
	}
}

/**
 * Allocate and initialize SW ring. Initialize associated HW registers.
 *
 * @param eth_dev
 *   Pointer to structure rte_eth_dev
 *
 * @param q_no
 *   Queue number
 *
 * @param num_tx_descs
 *   Number of ringbuffer descriptors
 *
 * @param socket_id
 *   NUMA socket id, used for memory allocations
 *
 * @param tx_conf
 *   Pointer to the structure rte_eth_txconf
 *
 * @return
 *   - On success, return 0
 *   - On failure, return -errno value
 */
static int
lio_dev_tx_queue_setup(struct rte_eth_dev *eth_dev, uint16_t q_no,
		       uint16_t num_tx_descs, unsigned int socket_id,
		       const struct rte_eth_txconf *tx_conf __rte_unused)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	int fw_mapped_iq = lio_dev->linfo.txpciq[q_no].s.q_no;
	int retval;

	if (q_no >= lio_dev->nb_tx_queues) {
		lio_dev_err(lio_dev, "Invalid tx queue number %u\n", q_no);
		return -EINVAL;
	}

	lio_dev_dbg(lio_dev, "setting up tx queue %u\n", q_no);

	if ((lio_dev->instr_queue[fw_mapped_iq] != NULL) &&
	    (num_tx_descs != lio_dev->instr_queue[fw_mapped_iq]->max_count)) {
		lio_dev_err(lio_dev,
			    "Reconfiguring Tx descs not supported. Configure descs to same value %u or restart application\n",
			    lio_dev->instr_queue[fw_mapped_iq]->max_count);
		return -ENOTSUP;
	}

	retval = lio_setup_iq(lio_dev, q_no, lio_dev->linfo.txpciq[q_no],
			      num_tx_descs, lio_dev, socket_id);

	if (retval) {
		lio_dev_err(lio_dev, "Runtime IQ(TxQ) creation failed.\n");
		return retval;
	}

	retval = lio_setup_sglists(lio_dev, q_no, fw_mapped_iq,
				lio_dev->instr_queue[fw_mapped_iq]->max_count,
				socket_id);

	if (retval) {
		lio_delete_instruction_queue(lio_dev, fw_mapped_iq);
		return retval;
	}

	eth_dev->data->tx_queues[q_no] = lio_dev->instr_queue[fw_mapped_iq];

	return 0;
}

/**
 * Release the transmit queue/ringbuffer. Called by
 * the upper layers.
 *
 * @param txq
 *    Opaque pointer to the transmit queue to release
 *
 * @return
 *    - nothing
 */
static void
lio_dev_tx_queue_release(void *txq)
{
	struct lio_instr_queue *tq = txq;
	struct lio_device *lio_dev = tq->lio_dev;
	uint32_t fw_mapped_iq_no;

	/* Run time queue deletion not supported */
	if (lio_dev->port_configured)
		return;

	if (tq != NULL) {
		/* Free sg_list */
		lio_delete_sglist(tq);

		fw_mapped_iq_no = tq->txpciq.s.q_no;
		lio_delete_instruction_queue(tq->lio_dev, fw_mapped_iq_no);
	}
}

/**
 * Api to check link state.
 */
static void
lio_dev_get_link_status(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	uint16_t timeout = LIO_MAX_CMD_TIMEOUT;
	struct lio_link_status_resp *resp;
	union octeon_link_status *ls;
	struct lio_soft_command *sc;
	uint32_t resp_size;

	if (!lio_dev->intf_open)
		return;

	resp_size = sizeof(struct lio_link_status_resp);
	sc = lio_alloc_soft_command(lio_dev, 0, resp_size, 0);
	if (sc == NULL)
		return;

	resp = (struct lio_link_status_resp *)sc->virtrptr;
	lio_prepare_soft_command(lio_dev, sc, LIO_OPCODE,
				 LIO_OPCODE_INFO, 0, 0, 0);

	/* Setting wait time in seconds */
	sc->wait_time = LIO_MAX_CMD_TIMEOUT / 1000;

	if (lio_send_soft_command(lio_dev, sc) == LIO_IQ_SEND_FAILED)
		goto get_status_fail;

	while ((*sc->status_word == LIO_COMPLETION_WORD_INIT) && --timeout) {
		lio_flush_iq(lio_dev, lio_dev->instr_queue[sc->iq_no]);
		rte_delay_ms(1);
	}

	if (resp->status)
		goto get_status_fail;

	ls = &resp->link_info.link;

	lio_swap_8B_data((uint64_t *)ls, sizeof(union octeon_link_status) >> 3);

	if (lio_dev->linfo.link.link_status64 != ls->link_status64) {
		lio_dev->linfo.link.link_status64 = ls->link_status64;
		lio_dev_link_update(eth_dev, 0);
	}

	lio_free_soft_command(sc);

	return;

get_status_fail:
	lio_free_soft_command(sc);
}

/* This function will be invoked every LSC_TIMEOUT ns (100ms)
 * and will update link state if it changes.
 */
static void
lio_sync_link_state_check(void *eth_dev)
{
	struct lio_device *lio_dev =
		(((struct rte_eth_dev *)eth_dev)->data->dev_private);

	if (lio_dev->port_configured)
		lio_dev_get_link_status(eth_dev);

	/* Schedule periodic link status check.
	 * Stop check if interface is close and start again while opening.
	 */
	if (lio_dev->intf_open)
		rte_eal_alarm_set(LIO_LSC_TIMEOUT, lio_sync_link_state_check,
				  eth_dev);
}

static int
lio_dev_start(struct rte_eth_dev *eth_dev)
{
	uint16_t mtu = eth_dev->data->dev_conf.rxmode.max_rx_pkt_len;
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	uint16_t timeout = LIO_MAX_CMD_TIMEOUT;
	int ret = 0;

	lio_dev_info(lio_dev, "Starting port %d\n", eth_dev->data->port_id);

	if (lio_dev->fn_list.enable_io_queues(lio_dev))
		return -1;

	if (lio_send_rx_ctrl_cmd(eth_dev, 1))
		return -1;

	/* Ready for link status updates */
	lio_dev->intf_open = 1;
	rte_mb();

	/* Configure RSS if device configured with multiple RX queues. */
	lio_dev_mq_rx_configure(eth_dev);

	/* start polling for lsc */
	ret = rte_eal_alarm_set(LIO_LSC_TIMEOUT,
				lio_sync_link_state_check,
				eth_dev);
	if (ret) {
		lio_dev_err(lio_dev,
			    "link state check handler creation failed\n");
		goto dev_lsc_handle_error;
	}

	while ((lio_dev->linfo.link.link_status64 == 0) && (--timeout))
		rte_delay_ms(1);

	if (lio_dev->linfo.link.link_status64 == 0) {
		ret = -1;
		goto dev_mtu_check_error;
	}

	if (lio_dev->linfo.link.s.mtu != mtu) {
		ret = lio_dev_validate_vf_mtu(eth_dev, mtu);
		if (ret)
			goto dev_mtu_check_error;
	}

	return 0;

dev_mtu_check_error:
	rte_eal_alarm_cancel(lio_sync_link_state_check, eth_dev);

dev_lsc_handle_error:
	lio_dev->intf_open = 0;
	lio_send_rx_ctrl_cmd(eth_dev, 0);

	return ret;
}

static int
lio_dev_set_link_up(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);

	if (!lio_dev->intf_open) {
		lio_dev_info(lio_dev, "Port is stopped, Start the port first\n");
		return 0;
	}

	if (lio_dev->linfo.link.s.link_up) {
		lio_dev_info(lio_dev, "Link is already UP\n");
		return 0;
	}

	if (lio_send_rx_ctrl_cmd(eth_dev, 1)) {
		lio_dev_err(lio_dev, "Unable to set Link UP\n");
		return -1;
	}

	lio_dev->linfo.link.s.link_up = 1;
	eth_dev->data->dev_link.link_status = ETH_LINK_UP;

	return 0;
}

static int
lio_dev_set_link_down(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);

	if (!lio_dev->intf_open) {
		lio_dev_info(lio_dev, "Port is stopped, Start the port first\n");
		return 0;
	}

	if (!lio_dev->linfo.link.s.link_up) {
		lio_dev_info(lio_dev, "Link is already DOWN\n");
		return 0;
	}

	lio_dev->linfo.link.s.link_up = 0;
	eth_dev->data->dev_link.link_status = ETH_LINK_DOWN;

	if (lio_send_rx_ctrl_cmd(eth_dev, 0)) {
		lio_dev->linfo.link.s.link_up = 1;
		eth_dev->data->dev_link.link_status = ETH_LINK_UP;
		lio_dev_err(lio_dev, "Unable to set Link Down\n");
		return -1;
	}

	return 0;
}

/**
 * Enable tunnel rx checksum verification from firmware.
 */
static void
lio_enable_hw_tunnel_rx_checksum(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_dev_ctrl_cmd ctrl_cmd;
	struct lio_ctrl_pkt ctrl_pkt;

	/* flush added to prevent cmd failure
	 * incase the queue is full
	 */
	lio_flush_iq(lio_dev, lio_dev->instr_queue[0]);

	memset(&ctrl_pkt, 0, sizeof(struct lio_ctrl_pkt));
	memset(&ctrl_cmd, 0, sizeof(struct lio_dev_ctrl_cmd));

	ctrl_cmd.eth_dev = eth_dev;
	ctrl_cmd.cond = 0;

	ctrl_pkt.ncmd.s.cmd = LIO_CMD_TNL_RX_CSUM_CTL;
	ctrl_pkt.ncmd.s.param1 = LIO_CMD_RXCSUM_ENABLE;
	ctrl_pkt.ctrl_cmd = &ctrl_cmd;

	if (lio_send_ctrl_pkt(lio_dev, &ctrl_pkt)) {
		lio_dev_err(lio_dev, "Failed to send TNL_RX_CSUM command\n");
		return;
	}

	if (lio_wait_for_ctrl_cmd(lio_dev, &ctrl_cmd))
		lio_dev_err(lio_dev, "TNL_RX_CSUM command timed out\n");
}

/**
 * Enable checksum calculation for inner packet in a tunnel.
 */
static void
lio_enable_hw_tunnel_tx_checksum(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	struct lio_dev_ctrl_cmd ctrl_cmd;
	struct lio_ctrl_pkt ctrl_pkt;

	/* flush added to prevent cmd failure
	 * incase the queue is full
	 */
	lio_flush_iq(lio_dev, lio_dev->instr_queue[0]);

	memset(&ctrl_pkt, 0, sizeof(struct lio_ctrl_pkt));
	memset(&ctrl_cmd, 0, sizeof(struct lio_dev_ctrl_cmd));

	ctrl_cmd.eth_dev = eth_dev;
	ctrl_cmd.cond = 0;

	ctrl_pkt.ncmd.s.cmd = LIO_CMD_TNL_TX_CSUM_CTL;
	ctrl_pkt.ncmd.s.param1 = LIO_CMD_TXCSUM_ENABLE;
	ctrl_pkt.ctrl_cmd = &ctrl_cmd;

	if (lio_send_ctrl_pkt(lio_dev, &ctrl_pkt)) {
		lio_dev_err(lio_dev, "Failed to send TNL_TX_CSUM command\n");
		return;
	}

	if (lio_wait_for_ctrl_cmd(lio_dev, &ctrl_cmd))
		lio_dev_err(lio_dev, "TNL_TX_CSUM command timed out\n");
}

static int lio_dev_configure(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);
	uint16_t timeout = LIO_MAX_CMD_TIMEOUT;
	int retval, num_iqueues, num_oqueues;
	uint8_t mac[ETHER_ADDR_LEN], i;
	struct lio_if_cfg_resp *resp;
	struct lio_soft_command *sc;
	union lio_if_cfg if_cfg;
	uint32_t resp_size;

	PMD_INIT_FUNC_TRACE();

	/* Re-configuring firmware not supported.
	 * Can't change tx/rx queues per port from initial value.
	 */
	if (lio_dev->port_configured) {
		if ((lio_dev->nb_rx_queues != eth_dev->data->nb_rx_queues) ||
		    (lio_dev->nb_tx_queues != eth_dev->data->nb_tx_queues)) {
			lio_dev_err(lio_dev,
				    "rxq/txq re-conf not supported. Restart application with new value.\n");
			return -ENOTSUP;
		}
		return 0;
	}

	lio_dev->nb_rx_queues = eth_dev->data->nb_rx_queues;
	lio_dev->nb_tx_queues = eth_dev->data->nb_tx_queues;

	resp_size = sizeof(struct lio_if_cfg_resp);
	sc = lio_alloc_soft_command(lio_dev, 0, resp_size, 0);
	if (sc == NULL)
		return -ENOMEM;

	resp = (struct lio_if_cfg_resp *)sc->virtrptr;

	/* Firmware doesn't have capability to reconfigure the queues,
	 * Claim all queues, and use as many required
	 */
	if_cfg.if_cfg64 = 0;
	if_cfg.s.num_iqueues = lio_dev->nb_tx_queues;
	if_cfg.s.num_oqueues = lio_dev->nb_rx_queues;
	if_cfg.s.base_queue = 0;

	if_cfg.s.gmx_port_id = lio_dev->pf_num;

	lio_prepare_soft_command(lio_dev, sc, LIO_OPCODE,
				 LIO_OPCODE_IF_CFG, 0,
				 if_cfg.if_cfg64, 0);

	/* Setting wait time in seconds */
	sc->wait_time = LIO_MAX_CMD_TIMEOUT / 1000;

	retval = lio_send_soft_command(lio_dev, sc);
	if (retval == LIO_IQ_SEND_FAILED) {
		lio_dev_err(lio_dev, "iq/oq config failed status: %x\n",
			    retval);
		/* Soft instr is freed by driver in case of failure. */
		goto nic_config_fail;
	}

	/* Sleep on a wait queue till the cond flag indicates that the
	 * response arrived or timed-out.
	 */
	while ((*sc->status_word == LIO_COMPLETION_WORD_INIT) && --timeout) {
		lio_flush_iq(lio_dev, lio_dev->instr_queue[sc->iq_no]);
		lio_process_ordered_list(lio_dev);
		rte_delay_ms(1);
	}

	retval = resp->status;
	if (retval) {
		lio_dev_err(lio_dev, "iq/oq config failed\n");
		goto nic_config_fail;
	}

	lio_swap_8B_data((uint64_t *)(&resp->cfg_info),
			 sizeof(struct octeon_if_cfg_info) >> 3);

	num_iqueues = lio_hweight64(resp->cfg_info.iqmask);
	num_oqueues = lio_hweight64(resp->cfg_info.oqmask);

	if (!(num_iqueues) || !(num_oqueues)) {
		lio_dev_err(lio_dev,
			    "Got bad iqueues (%016lx) or oqueues (%016lx) from firmware.\n",
			    (unsigned long)resp->cfg_info.iqmask,
			    (unsigned long)resp->cfg_info.oqmask);
		goto nic_config_fail;
	}

	lio_dev_dbg(lio_dev,
		    "interface %d, iqmask %016lx, oqmask %016lx, numiqueues %d, numoqueues %d\n",
		    eth_dev->data->port_id,
		    (unsigned long)resp->cfg_info.iqmask,
		    (unsigned long)resp->cfg_info.oqmask,
		    num_iqueues, num_oqueues);

	lio_dev->linfo.num_rxpciq = num_oqueues;
	lio_dev->linfo.num_txpciq = num_iqueues;

	for (i = 0; i < num_oqueues; i++) {
		lio_dev->linfo.rxpciq[i].rxpciq64 =
		    resp->cfg_info.linfo.rxpciq[i].rxpciq64;
		lio_dev_dbg(lio_dev, "index %d OQ %d\n",
			    i, lio_dev->linfo.rxpciq[i].s.q_no);
	}

	for (i = 0; i < num_iqueues; i++) {
		lio_dev->linfo.txpciq[i].txpciq64 =
		    resp->cfg_info.linfo.txpciq[i].txpciq64;
		lio_dev_dbg(lio_dev, "index %d IQ %d\n",
			    i, lio_dev->linfo.txpciq[i].s.q_no);
	}

	lio_dev->linfo.hw_addr = resp->cfg_info.linfo.hw_addr;
	lio_dev->linfo.gmxport = resp->cfg_info.linfo.gmxport;
	lio_dev->linfo.link.link_status64 =
			resp->cfg_info.linfo.link.link_status64;

	/* 64-bit swap required on LE machines */
	lio_swap_8B_data(&lio_dev->linfo.hw_addr, 1);
	for (i = 0; i < ETHER_ADDR_LEN; i++)
		mac[i] = *((uint8_t *)(((uint8_t *)&lio_dev->linfo.hw_addr) +
				       2 + i));

	/* Copy the permanent MAC address */
	ether_addr_copy((struct ether_addr *)mac, &eth_dev->data->mac_addrs[0]);

	/* enable firmware checksum support for tunnel packets */
	lio_enable_hw_tunnel_rx_checksum(eth_dev);
	lio_enable_hw_tunnel_tx_checksum(eth_dev);

	lio_dev->glist_lock =
	    rte_zmalloc(NULL, sizeof(*lio_dev->glist_lock) * num_iqueues, 0);
	if (lio_dev->glist_lock == NULL)
		return -ENOMEM;

	lio_dev->glist_head =
		rte_zmalloc(NULL, sizeof(*lio_dev->glist_head) * num_iqueues,
			    0);
	if (lio_dev->glist_head == NULL) {
		rte_free(lio_dev->glist_lock);
		lio_dev->glist_lock = NULL;
		return -ENOMEM;
	}

	lio_dev_link_update(eth_dev, 0);

	lio_dev->port_configured = 1;

	lio_free_soft_command(sc);

	/* Disable iq_0 for reconf */
	lio_dev->fn_list.disable_io_queues(lio_dev);

	/* Reset ioq regs */
	lio_dev->fn_list.setup_device_regs(lio_dev);

	/* Free iq_0 used during init */
	lio_free_instr_queue0(lio_dev);

	return 0;

nic_config_fail:
	lio_dev_err(lio_dev, "Failed retval %d\n", retval);
	lio_free_soft_command(sc);
	lio_free_instr_queue0(lio_dev);

	return -ENODEV;
}

/* Define our ethernet definitions */
static const struct eth_dev_ops liovf_eth_dev_ops = {
	.dev_configure		= lio_dev_configure,
	.dev_start		= lio_dev_start,
	.dev_set_link_up	= lio_dev_set_link_up,
	.dev_set_link_down	= lio_dev_set_link_down,
	.allmulticast_enable	= lio_dev_allmulticast_enable,
	.allmulticast_disable	= lio_dev_allmulticast_disable,
	.link_update		= lio_dev_link_update,
	.stats_get		= lio_dev_stats_get,
	.stats_reset		= lio_dev_stats_reset,
	.dev_infos_get		= lio_dev_info_get,
	.rx_queue_setup		= lio_dev_rx_queue_setup,
	.rx_queue_release	= lio_dev_rx_queue_release,
	.tx_queue_setup		= lio_dev_tx_queue_setup,
	.tx_queue_release	= lio_dev_tx_queue_release,
	.reta_update		= lio_dev_rss_reta_update,
	.reta_query		= lio_dev_rss_reta_query,
	.rss_hash_conf_get	= lio_dev_rss_hash_conf_get,
	.rss_hash_update	= lio_dev_rss_hash_update,
	.udp_tunnel_port_add	= lio_dev_udp_tunnel_add,
	.udp_tunnel_port_del	= lio_dev_udp_tunnel_del,
};

static void
lio_check_pf_hs_response(void *lio_dev)
{
	struct lio_device *dev = lio_dev;

	/* check till response arrives */
	if (dev->pfvf_hsword.coproc_tics_per_us)
		return;

	cn23xx_vf_handle_mbox(dev);

	rte_eal_alarm_set(1, lio_check_pf_hs_response, lio_dev);
}

/**
 * \brief Identify the LIO device and to map the BAR address space
 * @param lio_dev lio device
 */
static int
lio_chip_specific_setup(struct lio_device *lio_dev)
{
	struct rte_pci_device *pdev = lio_dev->pci_dev;
	uint32_t dev_id = pdev->id.device_id;
	const char *s;
	int ret = 1;

	switch (dev_id) {
	case LIO_CN23XX_VF_VID:
		lio_dev->chip_id = LIO_CN23XX_VF_VID;
		ret = cn23xx_vf_setup_device(lio_dev);
		s = "CN23XX VF";
		break;
	default:
		s = "?";
		lio_dev_err(lio_dev, "Unsupported Chip\n");
	}

	if (!ret)
		lio_dev_info(lio_dev, "DEVICE : %s\n", s);

	return ret;
}

static int
lio_first_time_init(struct lio_device *lio_dev,
		    struct rte_pci_device *pdev)
{
	int dpdk_queues;

	PMD_INIT_FUNC_TRACE();

	/* set dpdk specific pci device pointer */
	lio_dev->pci_dev = pdev;

	/* Identify the LIO type and set device ops */
	if (lio_chip_specific_setup(lio_dev)) {
		lio_dev_err(lio_dev, "Chip specific setup failed\n");
		return -1;
	}

	/* Initialize soft command buffer pool */
	if (lio_setup_sc_buffer_pool(lio_dev)) {
		lio_dev_err(lio_dev, "sc buffer pool allocation failed\n");
		return -1;
	}

	/* Initialize lists to manage the requests of different types that
	 * arrive from applications for this lio device.
	 */
	lio_setup_response_list(lio_dev);

	if (lio_dev->fn_list.setup_mbox(lio_dev)) {
		lio_dev_err(lio_dev, "Mailbox setup failed\n");
		goto error;
	}

	/* Check PF response */
	lio_check_pf_hs_response((void *)lio_dev);

	/* Do handshake and exit if incompatible PF driver */
	if (cn23xx_pfvf_handshake(lio_dev))
		goto error;

	/* Initial reset */
	cn23xx_vf_ask_pf_to_do_flr(lio_dev);
	/* Wait for FLR for 100ms per SRIOV specification */
	rte_delay_ms(100);

	if (cn23xx_vf_set_io_queues_off(lio_dev)) {
		lio_dev_err(lio_dev, "Setting io queues off failed\n");
		goto error;
	}

	if (lio_dev->fn_list.setup_device_regs(lio_dev)) {
		lio_dev_err(lio_dev, "Failed to configure device registers\n");
		goto error;
	}

	if (lio_setup_instr_queue0(lio_dev)) {
		lio_dev_err(lio_dev, "Failed to setup instruction queue 0\n");
		goto error;
	}

	dpdk_queues = (int)lio_dev->sriov_info.rings_per_vf;

	lio_dev->max_tx_queues = dpdk_queues;
	lio_dev->max_rx_queues = dpdk_queues;

	/* Enable input and output queues for this device */
	if (lio_dev->fn_list.enable_io_queues(lio_dev))
		goto error;

	return 0;

error:
	lio_free_sc_buffer_pool(lio_dev);
	if (lio_dev->mbox[0])
		lio_dev->fn_list.free_mbox(lio_dev);
	if (lio_dev->instr_queue[0])
		lio_free_instr_queue0(lio_dev);

	return -1;
}

static int
lio_eth_dev_uninit(struct rte_eth_dev *eth_dev)
{
	struct lio_device *lio_dev = LIO_DEV(eth_dev);

	PMD_INIT_FUNC_TRACE();

	if (rte_eal_process_type() != RTE_PROC_PRIMARY)
		return -EPERM;

	/* lio_free_sc_buffer_pool */
	lio_free_sc_buffer_pool(lio_dev);

	rte_free(eth_dev->data->mac_addrs);
	eth_dev->data->mac_addrs = NULL;

	eth_dev->rx_pkt_burst = NULL;
	eth_dev->tx_pkt_burst = NULL;

	return 0;
}

static int
lio_eth_dev_init(struct rte_eth_dev *eth_dev)
{
	struct rte_pci_device *pdev = RTE_DEV_TO_PCI(eth_dev->device);
	struct lio_device *lio_dev = LIO_DEV(eth_dev);

	PMD_INIT_FUNC_TRACE();

	eth_dev->rx_pkt_burst = &lio_dev_recv_pkts;
	eth_dev->tx_pkt_burst = &lio_dev_xmit_pkts;

	/* Primary does the initialization. */
	if (rte_eal_process_type() != RTE_PROC_PRIMARY)
		return 0;

	rte_eth_copy_pci_info(eth_dev, pdev);
	eth_dev->data->dev_flags |= RTE_ETH_DEV_DETACHABLE;

	if (pdev->mem_resource[0].addr) {
		lio_dev->hw_addr = pdev->mem_resource[0].addr;
	} else {
		PMD_INIT_LOG(ERR, "ERROR: Failed to map BAR0\n");
		return -ENODEV;
	}

	lio_dev->eth_dev = eth_dev;
	/* set lio device print string */
	snprintf(lio_dev->dev_string, sizeof(lio_dev->dev_string),
		 "%s[%02x:%02x.%x]", pdev->driver->driver.name,
		 pdev->addr.bus, pdev->addr.devid, pdev->addr.function);

	lio_dev->port_id = eth_dev->data->port_id;

	if (lio_first_time_init(lio_dev, pdev)) {
		lio_dev_err(lio_dev, "Device init failed\n");
		return -EINVAL;
	}

	eth_dev->dev_ops = &liovf_eth_dev_ops;
	eth_dev->data->mac_addrs = rte_zmalloc("lio", ETHER_ADDR_LEN, 0);
	if (eth_dev->data->mac_addrs == NULL) {
		lio_dev_err(lio_dev,
			    "MAC addresses memory allocation failed\n");
		eth_dev->dev_ops = NULL;
		eth_dev->rx_pkt_burst = NULL;
		eth_dev->tx_pkt_burst = NULL;
		return -ENOMEM;
	}

	rte_atomic64_set(&lio_dev->status, LIO_DEV_RUNNING);
	rte_wmb();

	lio_dev->port_configured = 0;
	/* Always allow unicast packets */
	lio_dev->ifflags |= LIO_IFFLAG_UNICAST;

	return 0;
}

/* Set of PCI devices this driver supports */
static const struct rte_pci_id pci_id_liovf_map[] = {
	{ RTE_PCI_DEVICE(PCI_VENDOR_ID_CAVIUM, LIO_CN23XX_VF_VID) },
	{ .vendor_id = 0, /* sentinel */ }
};

static struct eth_driver rte_liovf_pmd = {
	.pci_drv = {
		.id_table	= pci_id_liovf_map,
		.drv_flags      = RTE_PCI_DRV_NEED_MAPPING,
		.probe		= rte_eth_dev_pci_probe,
		.remove		= rte_eth_dev_pci_remove,
	},
	.eth_dev_init		= lio_eth_dev_init,
	.eth_dev_uninit		= lio_eth_dev_uninit,
	.dev_private_size	= sizeof(struct lio_device),
};

RTE_PMD_REGISTER_PCI(net_liovf, rte_liovf_pmd.pci_drv);
RTE_PMD_REGISTER_PCI_TABLE(net_liovf, pci_id_liovf_map);
RTE_PMD_REGISTER_KMOD_DEP(net_liovf, "* igb_uio | vfio");
