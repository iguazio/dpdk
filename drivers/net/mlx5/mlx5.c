/*-
 *   BSD LICENSE
 *
 *   Copyright 2015 6WIND S.A.
 *   Copyright 2015 Mellanox.
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
 *     * Neither the name of 6WIND S.A. nor the names of its
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

#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <net/if.h>

/* Verbs header. */
/* ISO C doesn't support unnamed structs/unions, disabling -pedantic. */
#ifdef PEDANTIC
#pragma GCC diagnostic ignored "-pedantic"
#endif
#include <infiniband/verbs.h>
#ifdef PEDANTIC
#pragma GCC diagnostic error "-pedantic"
#endif

/* DPDK headers don't like -pedantic. */
#ifdef PEDANTIC
#pragma GCC diagnostic ignored "-pedantic"
#endif
#include <rte_malloc.h>
#include <rte_ethdev.h>
#include <rte_pci.h>
#include <rte_common.h>
#ifdef PEDANTIC
#pragma GCC diagnostic error "-pedantic"
#endif

#include "mlx5.h"
#include "mlx5_utils.h"
#include "mlx5_rxtx.h"
#include "mlx5_autoconf.h"
#include "mlx5_defs.h"

/**
 * DPDK callback to close the device.
 *
 * Destroy all queues and objects, free memory.
 *
 * @param dev
 *   Pointer to Ethernet device structure.
 */
static void
mlx5_dev_close(struct rte_eth_dev *dev)
{
	struct priv *priv = mlx5_get_priv(dev);
	void *tmp;
	unsigned int i;

	priv_lock(priv);
	DEBUG("%p: closing device \"%s\"",
	      (void *)dev,
	      ((priv->ctx != NULL) ? priv->ctx->device->name : ""));
	/* In case mlx5_dev_stop() has not been called. */
	priv_dev_interrupt_handler_uninstall(priv, dev);
	priv_special_flow_disable_all(priv);
	priv_mac_addrs_disable(priv);
	priv_destroy_hash_rxqs(priv);

	/* Remove flow director elements. */
	priv_fdir_disable(priv);
	priv_fdir_delete_filters_list(priv);

	/* Prevent crashes when queues are still in use. */
	dev->rx_pkt_burst = removed_rx_burst;
	dev->tx_pkt_burst = removed_tx_burst;
	if (priv->rxqs != NULL) {
		/* XXX race condition if mlx5_rx_burst() is still running. */
		usleep(1000);
		for (i = 0; (i != priv->rxqs_n); ++i) {
			tmp = (*priv->rxqs)[i];
			if (tmp == NULL)
				continue;
			(*priv->rxqs)[i] = NULL;
			rxq_cleanup(tmp);
			rte_free(tmp);
		}
		priv->rxqs_n = 0;
		priv->rxqs = NULL;
	}
	if (priv->txqs != NULL) {
		/* XXX race condition if mlx5_tx_burst() is still running. */
		usleep(1000);
		for (i = 0; (i != priv->txqs_n); ++i) {
			tmp = (*priv->txqs)[i];
			if (tmp == NULL)
				continue;
			(*priv->txqs)[i] = NULL;
			txq_cleanup(tmp);
			rte_free(tmp);
		}
		priv->txqs_n = 0;
		priv->txqs = NULL;
	}
	if (priv->pd != NULL) {
		assert(priv->ctx != NULL);
		claim_zero(ibv_dealloc_pd(priv->pd));
		claim_zero(ibv_close_device(priv->ctx));
	} else
		assert(priv->ctx == NULL);
	if (priv->rss_conf != NULL) {
		for (i = 0; (i != hash_rxq_init_n); ++i)
			rte_free((*priv->rss_conf)[i]);
		rte_free(priv->rss_conf);
	}
	if (priv->reta_idx != NULL)
		rte_free(priv->reta_idx);
	priv_unlock(priv);
	memset(priv, 0, sizeof(*priv));
}

static const struct eth_dev_ops mlx5_dev_ops = {
	.dev_configure = mlx5_dev_configure,
	.dev_start = mlx5_dev_start,
	.dev_stop = mlx5_dev_stop,
	.dev_set_link_down = mlx5_set_link_down,
	.dev_set_link_up = mlx5_set_link_up,
	.dev_close = mlx5_dev_close,
	.promiscuous_enable = mlx5_promiscuous_enable,
	.promiscuous_disable = mlx5_promiscuous_disable,
	.allmulticast_enable = mlx5_allmulticast_enable,
	.allmulticast_disable = mlx5_allmulticast_disable,
	.link_update = mlx5_link_update,
	.stats_get = mlx5_stats_get,
	.stats_reset = mlx5_stats_reset,
	.dev_infos_get = mlx5_dev_infos_get,
	.dev_supported_ptypes_get = mlx5_dev_supported_ptypes_get,
	.vlan_filter_set = mlx5_vlan_filter_set,
	.rx_queue_setup = mlx5_rx_queue_setup,
	.tx_queue_setup = mlx5_tx_queue_setup,
	.rx_queue_release = mlx5_rx_queue_release,
	.tx_queue_release = mlx5_tx_queue_release,
	.flow_ctrl_get = mlx5_dev_get_flow_ctrl,
	.flow_ctrl_set = mlx5_dev_set_flow_ctrl,
	.mac_addr_remove = mlx5_mac_addr_remove,
	.mac_addr_add = mlx5_mac_addr_add,
	.mac_addr_set = mlx5_mac_addr_set,
	.mtu_set = mlx5_dev_set_mtu,
#ifdef HAVE_EXP_DEVICE_ATTR_VLAN_OFFLOADS
	.vlan_strip_queue_set = mlx5_vlan_strip_queue_set,
	.vlan_offload_set = mlx5_vlan_offload_set,
#endif /* HAVE_EXP_DEVICE_ATTR_VLAN_OFFLOADS */
	.reta_update = mlx5_dev_rss_reta_update,
	.reta_query = mlx5_dev_rss_reta_query,
	.rss_hash_update = mlx5_rss_hash_update,
	.rss_hash_conf_get = mlx5_rss_hash_conf_get,
#ifdef MLX5_FDIR_SUPPORT
	.filter_ctrl = mlx5_dev_filter_ctrl,
#endif /* MLX5_FDIR_SUPPORT */
};

static struct {
	struct rte_pci_addr pci_addr; /* associated PCI address */
	uint32_t ports; /* physical ports bitfield. */
} mlx5_dev[32];

/**
 * Get device index in mlx5_dev[] from PCI bus address.
 *
 * @param[in] pci_addr
 *   PCI bus address to look for.
 *
 * @return
 *   mlx5_dev[] index on success, -1 on failure.
 */
static int
mlx5_dev_idx(struct rte_pci_addr *pci_addr)
{
	unsigned int i;
	int ret = -1;

	assert(pci_addr != NULL);
	for (i = 0; (i != RTE_DIM(mlx5_dev)); ++i) {
		if ((mlx5_dev[i].pci_addr.domain == pci_addr->domain) &&
		    (mlx5_dev[i].pci_addr.bus == pci_addr->bus) &&
		    (mlx5_dev[i].pci_addr.devid == pci_addr->devid) &&
		    (mlx5_dev[i].pci_addr.function == pci_addr->function))
			return i;
		if ((mlx5_dev[i].ports == 0) && (ret == -1))
			ret = i;
	}
	return ret;
}

static struct eth_driver mlx5_driver;

/**
 * DPDK callback to register a PCI device.
 *
 * This function creates an Ethernet device for each port of a given
 * PCI device.
 *
 * @param[in] pci_drv
 *   PCI driver structure (mlx5_driver).
 * @param[in] pci_dev
 *   PCI device information.
 *
 * @return
 *   0 on success, negative errno value on failure.
 */
static int
mlx5_pci_devinit(struct rte_pci_driver *pci_drv, struct rte_pci_device *pci_dev)
{
	struct ibv_device **list;
	struct ibv_device *ibv_dev;
	int err = 0;
	struct ibv_context *attr_ctx = NULL;
	struct ibv_device_attr device_attr;
	unsigned int vf;
	int idx;
	int i;

	(void)pci_drv;
	assert(pci_drv == &mlx5_driver.pci_drv);
	/* Get mlx5_dev[] index. */
	idx = mlx5_dev_idx(&pci_dev->addr);
	if (idx == -1) {
		ERROR("this driver cannot support any more adapters");
		return -ENOMEM;
	}
	DEBUG("using driver device index %d", idx);

	/* Save PCI address. */
	mlx5_dev[idx].pci_addr = pci_dev->addr;
	list = ibv_get_device_list(&i);
	if (list == NULL) {
		assert(errno);
		if (errno == ENOSYS) {
			WARN("cannot list devices, is ib_uverbs loaded?");
			return 0;
		}
		return -errno;
	}
	assert(i >= 0);
	/*
	 * For each listed device, check related sysfs entry against
	 * the provided PCI ID.
	 */
	while (i != 0) {
		struct rte_pci_addr pci_addr;

		--i;
		DEBUG("checking device \"%s\"", list[i]->name);
		if (mlx5_ibv_device_to_pci_addr(list[i], &pci_addr))
			continue;
		if ((pci_dev->addr.domain != pci_addr.domain) ||
		    (pci_dev->addr.bus != pci_addr.bus) ||
		    (pci_dev->addr.devid != pci_addr.devid) ||
		    (pci_dev->addr.function != pci_addr.function))
			continue;
		vf = ((pci_dev->id.device_id ==
		       PCI_DEVICE_ID_MELLANOX_CONNECTX4VF) ||
		      (pci_dev->id.device_id ==
		       PCI_DEVICE_ID_MELLANOX_CONNECTX4LXVF));
		INFO("PCI information matches, using device \"%s\" (VF: %s)",
		     list[i]->name, (vf ? "true" : "false"));
		attr_ctx = ibv_open_device(list[i]);
		err = errno;
		break;
	}
	if (attr_ctx == NULL) {
		ibv_free_device_list(list);
		switch (err) {
		case 0:
			WARN("cannot access device, is mlx5_ib loaded?");
			return 0;
		case EINVAL:
			WARN("cannot use device, are drivers up to date?");
			return 0;
		}
		assert(err > 0);
		return -err;
	}
	ibv_dev = list[i];

	DEBUG("device opened");
	if (ibv_query_device(attr_ctx, &device_attr))
		goto error;
	INFO("%u port(s) detected", device_attr.phys_port_cnt);

	for (i = 0; i < device_attr.phys_port_cnt; i++) {
		uint32_t port = i + 1; /* ports are indexed from one */
		uint32_t test = (1 << i);
		struct ibv_context *ctx = NULL;
		struct ibv_port_attr port_attr;
		struct ibv_pd *pd = NULL;
		struct priv *priv = NULL;
		struct rte_eth_dev *eth_dev;
#ifdef HAVE_EXP_QUERY_DEVICE
		struct ibv_exp_device_attr exp_device_attr;
#endif /* HAVE_EXP_QUERY_DEVICE */
		struct ether_addr mac;

#ifdef HAVE_EXP_QUERY_DEVICE
		exp_device_attr.comp_mask =
			IBV_EXP_DEVICE_ATTR_EXP_CAP_FLAGS |
			IBV_EXP_DEVICE_ATTR_RX_HASH |
#ifdef HAVE_EXP_DEVICE_ATTR_VLAN_OFFLOADS
			IBV_EXP_DEVICE_ATTR_VLAN_OFFLOADS |
#endif /* HAVE_EXP_DEVICE_ATTR_VLAN_OFFLOADS */
			0;
#endif /* HAVE_EXP_QUERY_DEVICE */

		DEBUG("using port %u (%08" PRIx32 ")", port, test);

		ctx = ibv_open_device(ibv_dev);
		if (ctx == NULL)
			goto port_error;

		/* Check port status. */
		err = ibv_query_port(ctx, port, &port_attr);
		if (err) {
			ERROR("port query failed: %s", strerror(err));
			goto port_error;
		}

		if (port_attr.link_layer != IBV_LINK_LAYER_ETHERNET) {
			ERROR("port %d is not configured in Ethernet mode",
			      port);
			goto port_error;
		}

		if (port_attr.state != IBV_PORT_ACTIVE)
			DEBUG("port %d is not active: \"%s\" (%d)",
			      port, ibv_port_state_str(port_attr.state),
			      port_attr.state);

		/* Allocate protection domain. */
		pd = ibv_alloc_pd(ctx);
		if (pd == NULL) {
			ERROR("PD allocation failure");
			err = ENOMEM;
			goto port_error;
		}

		mlx5_dev[idx].ports |= test;

		/* from rte_ethdev.c */
		priv = rte_zmalloc("ethdev private structure",
				   sizeof(*priv),
				   RTE_CACHE_LINE_SIZE);
		if (priv == NULL) {
			ERROR("priv allocation failure");
			err = ENOMEM;
			goto port_error;
		}

		priv->ctx = ctx;
		priv->device_attr = device_attr;
		priv->port = port;
		priv->pd = pd;
		priv->mtu = ETHER_MTU;
#ifdef HAVE_EXP_QUERY_DEVICE
		if (ibv_exp_query_device(ctx, &exp_device_attr)) {
			ERROR("ibv_exp_query_device() failed");
			goto port_error;
		}

		priv->hw_csum =
			((exp_device_attr.exp_device_cap_flags &
			  IBV_EXP_DEVICE_RX_CSUM_TCP_UDP_PKT) &&
			 (exp_device_attr.exp_device_cap_flags &
			  IBV_EXP_DEVICE_RX_CSUM_IP_PKT));
		DEBUG("checksum offloading is %ssupported",
		      (priv->hw_csum ? "" : "not "));

		priv->hw_csum_l2tun = !!(exp_device_attr.exp_device_cap_flags &
					 IBV_EXP_DEVICE_VXLAN_SUPPORT);
		DEBUG("L2 tunnel checksum offloads are %ssupported",
		      (priv->hw_csum_l2tun ? "" : "not "));

		priv->ind_table_max_size = exp_device_attr.rx_hash_caps.max_rwq_indirection_table_size;
		/* Remove this check once DPDK supports larger/variable
		 * indirection tables. */
		if (priv->ind_table_max_size > (unsigned int)RSS_INDIRECTION_TABLE_SIZE)
			priv->ind_table_max_size = RSS_INDIRECTION_TABLE_SIZE;
		DEBUG("maximum RX indirection table size is %u",
		      priv->ind_table_max_size);
#ifdef HAVE_EXP_DEVICE_ATTR_VLAN_OFFLOADS
		priv->hw_vlan_strip = !!(exp_device_attr.wq_vlan_offloads_cap &
					 IBV_EXP_RECEIVE_WQ_CVLAN_STRIP);
#endif /* HAVE_EXP_DEVICE_ATTR_VLAN_OFFLOADS */
		DEBUG("VLAN stripping is %ssupported",
		      (priv->hw_vlan_strip ? "" : "not "));

#ifdef HAVE_VERBS_FCS
		priv->hw_fcs_strip = !!(exp_device_attr.exp_device_cap_flags &
					IBV_EXP_DEVICE_SCATTER_FCS);
#endif /* HAVE_VERBS_FCS */
		DEBUG("FCS stripping configuration is %ssupported",
		      (priv->hw_fcs_strip ? "" : "not "));

#else /* HAVE_EXP_QUERY_DEVICE */
		priv->ind_table_max_size = RSS_INDIRECTION_TABLE_SIZE;
#endif /* HAVE_EXP_QUERY_DEVICE */

		priv->vf = vf;
		/* Allocate and register default RSS hash keys. */
		priv->rss_conf = rte_calloc(__func__, hash_rxq_init_n,
					    sizeof((*priv->rss_conf)[0]), 0);
		if (priv->rss_conf == NULL) {
			err = ENOMEM;
			goto port_error;
		}
		err = rss_hash_rss_conf_new_key(priv,
						rss_hash_default_key,
						rss_hash_default_key_len,
						ETH_RSS_PROTO_MASK);
		if (err)
			goto port_error;
		/* Configure the first MAC address by default. */
		if (priv_get_mac(priv, &mac.addr_bytes)) {
			ERROR("cannot get MAC address, is mlx5_en loaded?"
			      " (errno: %s)", strerror(errno));
			goto port_error;
		}
		INFO("port %u MAC address is %02x:%02x:%02x:%02x:%02x:%02x",
		     priv->port,
		     mac.addr_bytes[0], mac.addr_bytes[1],
		     mac.addr_bytes[2], mac.addr_bytes[3],
		     mac.addr_bytes[4], mac.addr_bytes[5]);
		/* Register MAC address. */
		claim_zero(priv_mac_addr_add(priv, 0,
					     (const uint8_t (*)[ETHER_ADDR_LEN])
					     mac.addr_bytes));
		/* Initialize FD filters list. */
		err = fdir_init_filters_list(priv);
		if (err)
			goto port_error;
#ifndef NDEBUG
		{
			char ifname[IF_NAMESIZE];

			if (priv_get_ifname(priv, &ifname) == 0)
				DEBUG("port %u ifname is \"%s\"",
				      priv->port, ifname);
			else
				DEBUG("port %u ifname is unknown", priv->port);
		}
#endif
		/* Get actual MTU if possible. */
		priv_get_mtu(priv, &priv->mtu);
		DEBUG("port %u MTU is %u", priv->port, priv->mtu);

		/* from rte_ethdev.c */
		{
			char name[RTE_ETH_NAME_MAX_LEN];

			snprintf(name, sizeof(name), "%s port %u",
				 ibv_get_device_name(ibv_dev), port);
			eth_dev = rte_eth_dev_allocate(name, RTE_ETH_DEV_PCI);
		}
		if (eth_dev == NULL) {
			ERROR("can not allocate rte ethdev");
			err = ENOMEM;
			goto port_error;
		}

		/* Secondary processes have to use local storage for their
		 * private data as well as a copy of eth_dev->data, but this
		 * pointer must not be modified before burst functions are
		 * actually called. */
		if (mlx5_is_secondary()) {
			struct mlx5_secondary_data *sd =
				&mlx5_secondary_data[eth_dev->data->port_id];
			sd->primary_priv = eth_dev->data->dev_private;
			if (sd->primary_priv == NULL) {
				ERROR("no private data for port %u",
						eth_dev->data->port_id);
				err = EINVAL;
				goto port_error;
			}
			sd->shared_dev_data = eth_dev->data;
			rte_spinlock_init(&sd->lock);
			memcpy(sd->data.name, sd->shared_dev_data->name,
				   sizeof(sd->data.name));
			sd->data.dev_private = priv;
			sd->data.rx_mbuf_alloc_failed = 0;
			sd->data.mtu = ETHER_MTU;
			sd->data.port_id = sd->shared_dev_data->port_id;
			sd->data.mac_addrs = priv->mac;
			eth_dev->tx_pkt_burst = mlx5_tx_burst_secondary_setup;
			eth_dev->rx_pkt_burst = mlx5_rx_burst_secondary_setup;
		} else {
			eth_dev->data->dev_private = priv;
			eth_dev->data->rx_mbuf_alloc_failed = 0;
			eth_dev->data->mtu = ETHER_MTU;
			eth_dev->data->mac_addrs = priv->mac;
		}

		eth_dev->pci_dev = pci_dev;
		rte_eth_copy_pci_info(eth_dev, pci_dev);
		eth_dev->driver = &mlx5_driver;
		priv->dev = eth_dev;
		eth_dev->dev_ops = &mlx5_dev_ops;

		TAILQ_INIT(&eth_dev->link_intr_cbs);

		/* Bring Ethernet device up. */
		DEBUG("forcing Ethernet interface up");
		priv_set_flags(priv, ~IFF_UP, IFF_UP);
		continue;

port_error:
		if (priv) {
			rte_free(priv->rss_conf);
			rte_free(priv);
		}
		if (pd)
			claim_zero(ibv_dealloc_pd(pd));
		if (ctx)
			claim_zero(ibv_close_device(ctx));
		break;
	}

	/*
	 * XXX if something went wrong in the loop above, there is a resource
	 * leak (ctx, pd, priv, dpdk ethdev) but we can do nothing about it as
	 * long as the dpdk does not provide a way to deallocate a ethdev and a
	 * way to enumerate the registered ethdevs to free the previous ones.
	 */

	/* no port found, complain */
	if (!mlx5_dev[idx].ports) {
		err = ENODEV;
		goto error;
	}

error:
	if (attr_ctx)
		claim_zero(ibv_close_device(attr_ctx));
	if (list)
		ibv_free_device_list(list);
	assert(err >= 0);
	return -err;
}

static const struct rte_pci_id mlx5_pci_id_map[] = {
	{
		.vendor_id = PCI_VENDOR_ID_MELLANOX,
		.device_id = PCI_DEVICE_ID_MELLANOX_CONNECTX4,
		.subsystem_vendor_id = PCI_ANY_ID,
		.subsystem_device_id = PCI_ANY_ID
	},
	{
		.vendor_id = PCI_VENDOR_ID_MELLANOX,
		.device_id = PCI_DEVICE_ID_MELLANOX_CONNECTX4VF,
		.subsystem_vendor_id = PCI_ANY_ID,
		.subsystem_device_id = PCI_ANY_ID
	},
	{
		.vendor_id = PCI_VENDOR_ID_MELLANOX,
		.device_id = PCI_DEVICE_ID_MELLANOX_CONNECTX4LX,
		.subsystem_vendor_id = PCI_ANY_ID,
		.subsystem_device_id = PCI_ANY_ID
	},
	{
		.vendor_id = PCI_VENDOR_ID_MELLANOX,
		.device_id = PCI_DEVICE_ID_MELLANOX_CONNECTX4LXVF,
		.subsystem_vendor_id = PCI_ANY_ID,
		.subsystem_device_id = PCI_ANY_ID
	},
	{
		.vendor_id = 0
	}
};

static struct eth_driver mlx5_driver = {
	.pci_drv = {
		.name = MLX5_DRIVER_NAME,
		.id_table = mlx5_pci_id_map,
		.devinit = mlx5_pci_devinit,
		.drv_flags = RTE_PCI_DRV_INTR_LSC,
	},
	.dev_private_size = sizeof(struct priv)
};

/**
 * Driver initialization routine.
 */
static int
rte_mlx5_pmd_init(const char *name, const char *args)
{
	(void)name;
	(void)args;
	/*
	 * RDMAV_HUGEPAGES_SAFE tells ibv_fork_init() we intend to use
	 * huge pages. Calling ibv_fork_init() during init allows
	 * applications to use fork() safely for purposes other than
	 * using this PMD, which is not supported in forked processes.
	 */
	setenv("RDMAV_HUGEPAGES_SAFE", "1", 1);
	ibv_fork_init();
	rte_eal_pci_register(&mlx5_driver.pci_drv);
	return 0;
}

static struct rte_driver rte_mlx5_driver = {
	.type = PMD_PDEV,
	.name = MLX5_DRIVER_NAME,
	.init = rte_mlx5_pmd_init,
};

PMD_REGISTER_DRIVER(rte_mlx5_driver)
