/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2015 6WIND S.A.
 * Copyright 2015 Mellanox.
 */

#include <stddef.h>
#include <errno.h>
#include <string.h>

/* Verbs header. */
/* ISO C doesn't support unnamed structs/unions, disabling -pedantic. */
#ifdef PEDANTIC
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <infiniband/verbs.h>
#ifdef PEDANTIC
#pragma GCC diagnostic error "-Wpedantic"
#endif

#include <rte_ethdev_driver.h>

#include "mlx5.h"
#include "mlx5_rxtx.h"
#include "mlx5_utils.h"

/**
 * DPDK callback to enable promiscuous mode.
 *
 * @param dev
 *   Pointer to Ethernet device structure.
 */
void
mlx5_promiscuous_enable(struct rte_eth_dev *dev)
{
	int ret;

	dev->data->promiscuous = 1;
	ret = mlx5_traffic_restart(dev);
	if (ret)
		ERROR("%p cannot enable promiscuous mode: %s", (void *)dev,
		      strerror(rte_errno));
}

/**
 * DPDK callback to disable promiscuous mode.
 *
 * @param dev
 *   Pointer to Ethernet device structure.
 */
void
mlx5_promiscuous_disable(struct rte_eth_dev *dev)
{
	int ret;

	dev->data->promiscuous = 0;
	ret = mlx5_traffic_restart(dev);
	if (ret)
		ERROR("%p cannot disable promiscuous mode: %s", (void *)dev,
		      strerror(rte_errno));
}

/**
 * DPDK callback to enable allmulti mode.
 *
 * @param dev
 *   Pointer to Ethernet device structure.
 */
void
mlx5_allmulticast_enable(struct rte_eth_dev *dev)
{
	int ret;

	dev->data->all_multicast = 1;
	ret = mlx5_traffic_restart(dev);
	if (ret)
		ERROR("%p cannot enable allmulicast mode: %s", (void *)dev,
		      strerror(rte_errno));
}

/**
 * DPDK callback to disable allmulti mode.
 *
 * @param dev
 *   Pointer to Ethernet device structure.
 */
void
mlx5_allmulticast_disable(struct rte_eth_dev *dev)
{
	int ret;

	dev->data->all_multicast = 0;
	ret = mlx5_traffic_restart(dev);
	if (ret)
		ERROR("%p cannot disable allmulicast mode: %s", (void *)dev,
		      strerror(rte_errno));
}
