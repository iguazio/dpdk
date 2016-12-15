/*-
 * Copyright (c) 2016 Solarflare Communications Inc.
 * All rights reserved.
 *
 * This software was jointly developed between OKTET Labs (under contract
 * for Solarflare) and Solarflare Communications, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <rte_dev.h>
#include <rte_ethdev.h>
#include <rte_pci.h>

#include "efx.h"

#include "sfc.h"
#include "sfc_debug.h"
#include "sfc_log.h"
#include "sfc_kvargs.h"
#include "sfc_ev.h"
#include "sfc_rx.h"
#include "sfc_tx.h"


static void
sfc_dev_infos_get(struct rte_eth_dev *dev, struct rte_eth_dev_info *dev_info)
{
	struct sfc_adapter *sa = dev->data->dev_private;

	sfc_log_init(sa, "entry");

	dev_info->pci_dev = RTE_DEV_TO_PCI(dev->device);
	dev_info->max_rx_pktlen = EFX_MAC_PDU_MAX;

	dev_info->max_rx_queues = sa->rxq_max;
	dev_info->max_tx_queues = sa->txq_max;

	/* By default packets are dropped if no descriptors are available */
	dev_info->default_rxconf.rx_drop_en = 1;

	dev_info->tx_offload_capa =
		DEV_TX_OFFLOAD_IPV4_CKSUM |
		DEV_TX_OFFLOAD_UDP_CKSUM |
		DEV_TX_OFFLOAD_TCP_CKSUM;

	dev_info->default_txconf.txq_flags = ETH_TXQ_FLAGS_NOVLANOFFL |
					     ETH_TXQ_FLAGS_NOXSUMSCTP;

	dev_info->rx_desc_lim.nb_max = EFX_RXQ_MAXNDESCS;
	dev_info->rx_desc_lim.nb_min = EFX_RXQ_MINNDESCS;
	/* The RXQ hardware requires that the descriptor count is a power
	 * of 2, but rx_desc_lim cannot properly describe that constraint.
	 */
	dev_info->rx_desc_lim.nb_align = EFX_RXQ_MINNDESCS;

	dev_info->tx_desc_lim.nb_max = sa->txq_max_entries;
	dev_info->tx_desc_lim.nb_min = EFX_TXQ_MINNDESCS;
	/*
	 * The TXQ hardware requires that the descriptor count is a power
	 * of 2, but tx_desc_lim cannot properly describe that constraint
	 */
	dev_info->tx_desc_lim.nb_align = EFX_TXQ_MINNDESCS;
}

static int
sfc_dev_configure(struct rte_eth_dev *dev)
{
	struct rte_eth_dev_data *dev_data = dev->data;
	struct sfc_adapter *sa = dev_data->dev_private;
	int rc;

	sfc_log_init(sa, "entry n_rxq=%u n_txq=%u",
		     dev_data->nb_rx_queues, dev_data->nb_tx_queues);

	sfc_adapter_lock(sa);
	switch (sa->state) {
	case SFC_ADAPTER_CONFIGURED:
		sfc_close(sa);
		SFC_ASSERT(sa->state == SFC_ADAPTER_INITIALIZED);
		/* FALLTHROUGH */
	case SFC_ADAPTER_INITIALIZED:
		rc = sfc_configure(sa);
		break;
	default:
		sfc_err(sa, "unexpected adapter state %u to configure",
			sa->state);
		rc = EINVAL;
		break;
	}
	sfc_adapter_unlock(sa);

	sfc_log_init(sa, "done %d", rc);
	SFC_ASSERT(rc >= 0);
	return -rc;
}

static int
sfc_dev_start(struct rte_eth_dev *dev)
{
	struct sfc_adapter *sa = dev->data->dev_private;
	int rc;

	sfc_log_init(sa, "entry");

	sfc_adapter_lock(sa);
	rc = sfc_start(sa);
	sfc_adapter_unlock(sa);

	sfc_log_init(sa, "done %d", rc);
	SFC_ASSERT(rc >= 0);
	return -rc;
}

static int
sfc_dev_link_update(struct rte_eth_dev *dev, int wait_to_complete)
{
	struct sfc_adapter *sa = dev->data->dev_private;
	struct rte_eth_link *dev_link = &dev->data->dev_link;
	struct rte_eth_link old_link;
	struct rte_eth_link current_link;

	sfc_log_init(sa, "entry");

	if (sa->state != SFC_ADAPTER_STARTED)
		return 0;

retry:
	EFX_STATIC_ASSERT(sizeof(*dev_link) == sizeof(rte_atomic64_t));
	*(int64_t *)&old_link = rte_atomic64_read((rte_atomic64_t *)dev_link);

	if (wait_to_complete) {
		efx_link_mode_t link_mode;

		efx_port_poll(sa->nic, &link_mode);
		sfc_port_link_mode_to_info(link_mode, &current_link);

		if (!rte_atomic64_cmpset((volatile uint64_t *)dev_link,
					 *(uint64_t *)&old_link,
					 *(uint64_t *)&current_link))
			goto retry;
	} else {
		sfc_ev_mgmt_qpoll(sa);
		*(int64_t *)&current_link =
			rte_atomic64_read((rte_atomic64_t *)dev_link);
	}

	if (old_link.link_status != current_link.link_status)
		sfc_info(sa, "Link status is %s",
			 current_link.link_status ? "UP" : "DOWN");

	return old_link.link_status == current_link.link_status ? 0 : -1;
}

static void
sfc_dev_stop(struct rte_eth_dev *dev)
{
	struct sfc_adapter *sa = dev->data->dev_private;

	sfc_log_init(sa, "entry");

	sfc_adapter_lock(sa);
	sfc_stop(sa);
	sfc_adapter_unlock(sa);

	sfc_log_init(sa, "done");
}

static void
sfc_dev_close(struct rte_eth_dev *dev)
{
	struct sfc_adapter *sa = dev->data->dev_private;

	sfc_log_init(sa, "entry");

	sfc_adapter_lock(sa);
	switch (sa->state) {
	case SFC_ADAPTER_STARTED:
		sfc_stop(sa);
		SFC_ASSERT(sa->state == SFC_ADAPTER_CONFIGURED);
		/* FALLTHROUGH */
	case SFC_ADAPTER_CONFIGURED:
		sfc_close(sa);
		SFC_ASSERT(sa->state == SFC_ADAPTER_INITIALIZED);
		/* FALLTHROUGH */
	case SFC_ADAPTER_INITIALIZED:
		break;
	default:
		sfc_err(sa, "unexpected adapter state %u on close", sa->state);
		break;
	}
	sfc_adapter_unlock(sa);

	sfc_log_init(sa, "done");
}

static int
sfc_rx_queue_setup(struct rte_eth_dev *dev, uint16_t rx_queue_id,
		   uint16_t nb_rx_desc, unsigned int socket_id,
		   const struct rte_eth_rxconf *rx_conf,
		   struct rte_mempool *mb_pool)
{
	struct sfc_adapter *sa = dev->data->dev_private;
	int rc;

	sfc_log_init(sa, "RxQ=%u nb_rx_desc=%u socket_id=%u",
		     rx_queue_id, nb_rx_desc, socket_id);

	sfc_adapter_lock(sa);

	rc = sfc_rx_qinit(sa, rx_queue_id, nb_rx_desc, socket_id,
			  rx_conf, mb_pool);
	if (rc != 0)
		goto fail_rx_qinit;

	dev->data->rx_queues[rx_queue_id] = sa->rxq_info[rx_queue_id].rxq;

	sfc_adapter_unlock(sa);

	return 0;

fail_rx_qinit:
	sfc_adapter_unlock(sa);
	SFC_ASSERT(rc > 0);
	return -rc;
}

static void
sfc_rx_queue_release(void *queue)
{
	struct sfc_rxq *rxq = queue;
	struct sfc_adapter *sa;
	unsigned int sw_index;

	if (rxq == NULL)
		return;

	sa = rxq->evq->sa;
	sfc_adapter_lock(sa);

	sw_index = sfc_rxq_sw_index(rxq);

	sfc_log_init(sa, "RxQ=%u", sw_index);

	sa->eth_dev->data->rx_queues[sw_index] = NULL;

	sfc_rx_qfini(sa, sw_index);

	sfc_adapter_unlock(sa);
}

static int
sfc_tx_queue_setup(struct rte_eth_dev *dev, uint16_t tx_queue_id,
		   uint16_t nb_tx_desc, unsigned int socket_id,
		   const struct rte_eth_txconf *tx_conf)
{
	struct sfc_adapter *sa = dev->data->dev_private;
	int rc;

	sfc_log_init(sa, "TxQ = %u, nb_tx_desc = %u, socket_id = %u",
		     tx_queue_id, nb_tx_desc, socket_id);

	sfc_adapter_lock(sa);

	rc = sfc_tx_qinit(sa, tx_queue_id, nb_tx_desc, socket_id, tx_conf);
	if (rc != 0)
		goto fail_tx_qinit;

	dev->data->tx_queues[tx_queue_id] = sa->txq_info[tx_queue_id].txq;

	sfc_adapter_unlock(sa);
	return 0;

fail_tx_qinit:
	sfc_adapter_unlock(sa);
	SFC_ASSERT(rc > 0);
	return -rc;
}

static void
sfc_tx_queue_release(void *queue)
{
	struct sfc_txq *txq = queue;
	unsigned int sw_index;
	struct sfc_adapter *sa;

	if (txq == NULL)
		return;

	sw_index = sfc_txq_sw_index(txq);

	SFC_ASSERT(txq->evq != NULL);
	sa = txq->evq->sa;

	sfc_log_init(sa, "TxQ = %u", sw_index);

	sfc_adapter_lock(sa);

	SFC_ASSERT(sw_index < sa->eth_dev->data->nb_tx_queues);
	sa->eth_dev->data->tx_queues[sw_index] = NULL;

	sfc_tx_qfini(sa, sw_index);

	sfc_adapter_unlock(sa);
}

static void
sfc_stats_get(struct rte_eth_dev *dev, struct rte_eth_stats *stats)
{
	struct sfc_adapter *sa = dev->data->dev_private;
	struct sfc_port *port = &sa->port;
	uint64_t *mac_stats;

	rte_spinlock_lock(&port->mac_stats_lock);

	if (sfc_port_update_mac_stats(sa) != 0)
		goto unlock;

	mac_stats = port->mac_stats_buf;

	if (EFX_MAC_STAT_SUPPORTED(port->mac_stats_mask,
				   EFX_MAC_VADAPTER_RX_UNICAST_PACKETS)) {
		stats->ipackets =
			mac_stats[EFX_MAC_VADAPTER_RX_UNICAST_PACKETS] +
			mac_stats[EFX_MAC_VADAPTER_RX_MULTICAST_PACKETS] +
			mac_stats[EFX_MAC_VADAPTER_RX_BROADCAST_PACKETS];
		stats->opackets =
			mac_stats[EFX_MAC_VADAPTER_TX_UNICAST_PACKETS] +
			mac_stats[EFX_MAC_VADAPTER_TX_MULTICAST_PACKETS] +
			mac_stats[EFX_MAC_VADAPTER_TX_BROADCAST_PACKETS];
		stats->ibytes =
			mac_stats[EFX_MAC_VADAPTER_RX_UNICAST_BYTES] +
			mac_stats[EFX_MAC_VADAPTER_RX_MULTICAST_BYTES] +
			mac_stats[EFX_MAC_VADAPTER_RX_BROADCAST_BYTES];
		stats->obytes =
			mac_stats[EFX_MAC_VADAPTER_TX_UNICAST_BYTES] +
			mac_stats[EFX_MAC_VADAPTER_TX_MULTICAST_BYTES] +
			mac_stats[EFX_MAC_VADAPTER_TX_BROADCAST_BYTES];
		stats->imissed = mac_stats[EFX_MAC_VADAPTER_RX_OVERFLOW];
		stats->ierrors = mac_stats[EFX_MAC_VADAPTER_RX_BAD_PACKETS];
		stats->oerrors = mac_stats[EFX_MAC_VADAPTER_TX_BAD_PACKETS];
	} else {
		stats->ipackets = mac_stats[EFX_MAC_RX_PKTS];
		stats->opackets = mac_stats[EFX_MAC_TX_PKTS];
		stats->ibytes = mac_stats[EFX_MAC_RX_OCTETS];
		stats->obytes = mac_stats[EFX_MAC_TX_OCTETS];
		/*
		 * Take into account stats which are whenever supported
		 * on EF10. If some stat is not supported by current
		 * firmware variant or HW revision, it is guaranteed
		 * to be zero in mac_stats.
		 */
		stats->imissed =
			mac_stats[EFX_MAC_RX_NODESC_DROP_CNT] +
			mac_stats[EFX_MAC_PM_TRUNC_BB_OVERFLOW] +
			mac_stats[EFX_MAC_PM_DISCARD_BB_OVERFLOW] +
			mac_stats[EFX_MAC_PM_TRUNC_VFIFO_FULL] +
			mac_stats[EFX_MAC_PM_DISCARD_VFIFO_FULL] +
			mac_stats[EFX_MAC_PM_TRUNC_QBB] +
			mac_stats[EFX_MAC_PM_DISCARD_QBB] +
			mac_stats[EFX_MAC_PM_DISCARD_MAPPING] +
			mac_stats[EFX_MAC_RXDP_Q_DISABLED_PKTS] +
			mac_stats[EFX_MAC_RXDP_DI_DROPPED_PKTS];
		stats->ierrors =
			mac_stats[EFX_MAC_RX_FCS_ERRORS] +
			mac_stats[EFX_MAC_RX_ALIGN_ERRORS] +
			mac_stats[EFX_MAC_RX_JABBER_PKTS];
		/* no oerrors counters supported on EF10 */
	}

unlock:
	rte_spinlock_unlock(&port->mac_stats_lock);
}

static int
sfc_xstats_get(struct rte_eth_dev *dev, struct rte_eth_xstat *xstats,
	       unsigned int xstats_count)
{
	struct sfc_adapter *sa = dev->data->dev_private;
	struct sfc_port *port = &sa->port;
	uint64_t *mac_stats;
	int rc;
	unsigned int i;
	int nstats = 0;

	rte_spinlock_lock(&port->mac_stats_lock);

	rc = sfc_port_update_mac_stats(sa);
	if (rc != 0) {
		SFC_ASSERT(rc > 0);
		nstats = -rc;
		goto unlock;
	}

	mac_stats = port->mac_stats_buf;

	for (i = 0; i < EFX_MAC_NSTATS; ++i) {
		if (EFX_MAC_STAT_SUPPORTED(port->mac_stats_mask, i)) {
			if (xstats != NULL && nstats < (int)xstats_count) {
				xstats[nstats].id = nstats;
				xstats[nstats].value = mac_stats[i];
			}
			nstats++;
		}
	}

unlock:
	rte_spinlock_unlock(&port->mac_stats_lock);

	return nstats;
}

static int
sfc_xstats_get_names(struct rte_eth_dev *dev,
		     struct rte_eth_xstat_name *xstats_names,
		     unsigned int xstats_count)
{
	struct sfc_adapter *sa = dev->data->dev_private;
	struct sfc_port *port = &sa->port;
	unsigned int i;
	unsigned int nstats = 0;

	for (i = 0; i < EFX_MAC_NSTATS; ++i) {
		if (EFX_MAC_STAT_SUPPORTED(port->mac_stats_mask, i)) {
			if (xstats_names != NULL && nstats < xstats_count)
				strncpy(xstats_names[nstats].name,
					efx_mac_stat_name(sa->nic, i),
					sizeof(xstats_names[0].name));
			nstats++;
		}
	}

	return nstats;
}

static int
sfc_flow_ctrl_get(struct rte_eth_dev *dev, struct rte_eth_fc_conf *fc_conf)
{
	struct sfc_adapter *sa = dev->data->dev_private;
	unsigned int wanted_fc, link_fc;

	memset(fc_conf, 0, sizeof(*fc_conf));

	sfc_adapter_lock(sa);

	if (sa->state == SFC_ADAPTER_STARTED)
		efx_mac_fcntl_get(sa->nic, &wanted_fc, &link_fc);
	else
		link_fc = sa->port.flow_ctrl;

	switch (link_fc) {
	case 0:
		fc_conf->mode = RTE_FC_NONE;
		break;
	case EFX_FCNTL_RESPOND:
		fc_conf->mode = RTE_FC_RX_PAUSE;
		break;
	case EFX_FCNTL_GENERATE:
		fc_conf->mode = RTE_FC_TX_PAUSE;
		break;
	case (EFX_FCNTL_RESPOND | EFX_FCNTL_GENERATE):
		fc_conf->mode = RTE_FC_FULL;
		break;
	default:
		sfc_err(sa, "%s: unexpected flow control value %#x",
			__func__, link_fc);
	}

	fc_conf->autoneg = sa->port.flow_ctrl_autoneg;

	sfc_adapter_unlock(sa);

	return 0;
}

static int
sfc_flow_ctrl_set(struct rte_eth_dev *dev, struct rte_eth_fc_conf *fc_conf)
{
	struct sfc_adapter *sa = dev->data->dev_private;
	struct sfc_port *port = &sa->port;
	unsigned int fcntl;
	int rc;

	if (fc_conf->high_water != 0 || fc_conf->low_water != 0 ||
	    fc_conf->pause_time != 0 || fc_conf->send_xon != 0 ||
	    fc_conf->mac_ctrl_frame_fwd != 0) {
		sfc_err(sa, "unsupported flow control settings specified");
		rc = EINVAL;
		goto fail_inval;
	}

	switch (fc_conf->mode) {
	case RTE_FC_NONE:
		fcntl = 0;
		break;
	case RTE_FC_RX_PAUSE:
		fcntl = EFX_FCNTL_RESPOND;
		break;
	case RTE_FC_TX_PAUSE:
		fcntl = EFX_FCNTL_GENERATE;
		break;
	case RTE_FC_FULL:
		fcntl = EFX_FCNTL_RESPOND | EFX_FCNTL_GENERATE;
		break;
	default:
		rc = EINVAL;
		goto fail_inval;
	}

	sfc_adapter_lock(sa);

	if (sa->state == SFC_ADAPTER_STARTED) {
		rc = efx_mac_fcntl_set(sa->nic, fcntl, fc_conf->autoneg);
		if (rc != 0)
			goto fail_mac_fcntl_set;
	}

	port->flow_ctrl = fcntl;
	port->flow_ctrl_autoneg = fc_conf->autoneg;

	sfc_adapter_unlock(sa);

	return 0;

fail_mac_fcntl_set:
	sfc_adapter_unlock(sa);
fail_inval:
	SFC_ASSERT(rc > 0);
	return -rc;
}

static const struct eth_dev_ops sfc_eth_dev_ops = {
	.dev_configure			= sfc_dev_configure,
	.dev_start			= sfc_dev_start,
	.dev_stop			= sfc_dev_stop,
	.dev_close			= sfc_dev_close,
	.link_update			= sfc_dev_link_update,
	.stats_get			= sfc_stats_get,
	.xstats_get			= sfc_xstats_get,
	.xstats_get_names		= sfc_xstats_get_names,
	.dev_infos_get			= sfc_dev_infos_get,
	.rx_queue_setup			= sfc_rx_queue_setup,
	.rx_queue_release		= sfc_rx_queue_release,
	.tx_queue_setup			= sfc_tx_queue_setup,
	.tx_queue_release		= sfc_tx_queue_release,
	.flow_ctrl_get			= sfc_flow_ctrl_get,
	.flow_ctrl_set			= sfc_flow_ctrl_set,
};

static int
sfc_eth_dev_init(struct rte_eth_dev *dev)
{
	struct sfc_adapter *sa = dev->data->dev_private;
	struct rte_pci_device *pci_dev = SFC_DEV_TO_PCI(dev);
	int rc;
	const efx_nic_cfg_t *encp;
	const struct ether_addr *from;

	/* Required for logging */
	sa->eth_dev = dev;

	/* Copy PCI device info to the dev->data */
	rte_eth_copy_pci_info(dev, pci_dev);

	rc = sfc_kvargs_parse(sa);
	if (rc != 0)
		goto fail_kvargs_parse;

	rc = sfc_kvargs_process(sa, SFC_KVARG_DEBUG_INIT,
				sfc_kvarg_bool_handler, &sa->debug_init);
	if (rc != 0)
		goto fail_kvarg_debug_init;

	sfc_log_init(sa, "entry");

	dev->data->mac_addrs = rte_zmalloc("sfc", ETHER_ADDR_LEN, 0);
	if (dev->data->mac_addrs == NULL) {
		rc = ENOMEM;
		goto fail_mac_addrs;
	}

	sfc_adapter_lock_init(sa);
	sfc_adapter_lock(sa);

	sfc_log_init(sa, "attaching");
	rc = sfc_attach(sa);
	if (rc != 0)
		goto fail_attach;

	encp = efx_nic_cfg_get(sa->nic);

	/*
	 * The arguments are really reverse order in comparison to
	 * Linux kernel. Copy from NIC config to Ethernet device data.
	 */
	from = (const struct ether_addr *)(encp->enc_mac_addr);
	ether_addr_copy(from, &dev->data->mac_addrs[0]);

	dev->dev_ops = &sfc_eth_dev_ops;
	dev->rx_pkt_burst = &sfc_recv_pkts;
	dev->tx_pkt_burst = &sfc_xmit_pkts;

	sfc_adapter_unlock(sa);

	sfc_log_init(sa, "done");
	return 0;

fail_attach:
	sfc_adapter_unlock(sa);
	sfc_adapter_lock_fini(sa);
	rte_free(dev->data->mac_addrs);
	dev->data->mac_addrs = NULL;

fail_mac_addrs:
fail_kvarg_debug_init:
	sfc_kvargs_cleanup(sa);

fail_kvargs_parse:
	sfc_log_init(sa, "failed %d", rc);
	SFC_ASSERT(rc > 0);
	return -rc;
}

static int
sfc_eth_dev_uninit(struct rte_eth_dev *dev)
{
	struct sfc_adapter *sa = dev->data->dev_private;

	sfc_log_init(sa, "entry");

	sfc_adapter_lock(sa);

	sfc_detach(sa);

	rte_free(dev->data->mac_addrs);
	dev->data->mac_addrs = NULL;

	dev->dev_ops = NULL;
	dev->rx_pkt_burst = NULL;
	dev->tx_pkt_burst = NULL;

	sfc_kvargs_cleanup(sa);

	sfc_adapter_unlock(sa);
	sfc_adapter_lock_fini(sa);

	sfc_log_init(sa, "done");

	/* Required for logging, so cleanup last */
	sa->eth_dev = NULL;
	return 0;
}

static const struct rte_pci_id pci_id_sfc_efx_map[] = {
	{ RTE_PCI_DEVICE(EFX_PCI_VENID_SFC, EFX_PCI_DEVID_FARMINGDALE) },
	{ RTE_PCI_DEVICE(EFX_PCI_VENID_SFC, EFX_PCI_DEVID_GREENPORT) },
	{ RTE_PCI_DEVICE(EFX_PCI_VENID_SFC, EFX_PCI_DEVID_MEDFORD) },
	{ .vendor_id = 0 /* sentinel */ }
};

static struct eth_driver sfc_efx_pmd = {
	.pci_drv = {
		.id_table = pci_id_sfc_efx_map,
		.drv_flags =
			RTE_PCI_DRV_NEED_MAPPING,
		.probe = rte_eth_dev_pci_probe,
		.remove = rte_eth_dev_pci_remove,
	},
	.eth_dev_init = sfc_eth_dev_init,
	.eth_dev_uninit = sfc_eth_dev_uninit,
	.dev_private_size = sizeof(struct sfc_adapter),
};

RTE_PMD_REGISTER_PCI(net_sfc_efx, sfc_efx_pmd.pci_drv);
RTE_PMD_REGISTER_PCI_TABLE(net_sfc_efx, pci_id_sfc_efx_map);
RTE_PMD_REGISTER_PARAM_STRING(net_sfc_efx,
	SFC_KVARG_PERF_PROFILE "=" SFC_KVARG_VALUES_PERF_PROFILE " "
	SFC_KVARG_MCDI_LOGGING "=" SFC_KVARG_VALUES_BOOL " "
	SFC_KVARG_DEBUG_INIT "=" SFC_KVARG_VALUES_BOOL);
