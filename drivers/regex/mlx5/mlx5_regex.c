/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020 Mellanox Technologies, Ltd
 */

#include <rte_malloc.h>
#include <rte_log.h>
#include <rte_errno.h>
#include <rte_bus_pci.h>
#include <rte_pci.h>
#include <rte_regexdev.h>
#include <rte_regexdev_core.h>
#include <rte_regexdev_driver.h>

#include <mlx5_glue.h>
#include <mlx5_devx_cmds.h>
#include <mlx5_prm.h>

#include "mlx5_regex.h"
#include "mlx5_regex_utils.h"

int mlx5_regex_logtype;

const struct rte_regexdev_ops mlx5_regexdev_ops = {
	.dev_info_get = mlx5_regex_info_get,
};

static struct ibv_device *
mlx5_regex_get_ib_device_match(struct rte_pci_addr *addr)
{
	int n;
	struct ibv_device **ibv_list = mlx5_glue->get_device_list(&n);
	struct ibv_device *ibv_match = NULL;

	if (!ibv_list) {
		rte_errno = ENOSYS;
		return NULL;
	}
	while (n-- > 0) {
		struct rte_pci_addr pci_addr;

		DRV_LOG(DEBUG, "Checking device \"%s\"..", ibv_list[n]->name);
		if (mlx5_dev_to_pci_addr(ibv_list[n]->ibdev_path, &pci_addr))
			continue;
		if (rte_pci_addr_cmp(addr, &pci_addr))
			continue;
		ibv_match = ibv_list[n];
		break;
	}
	if (!ibv_match)
		rte_errno = ENOENT;
	mlx5_glue->free_device_list(ibv_list);
	return ibv_match;
}

static void
mlx5_regex_get_name(char *name, struct rte_pci_device *pci_dev __rte_unused)
{
	sprintf(name, "mlx5_regex_%02x:%02x.%02x", pci_dev->addr.bus,
		pci_dev->addr.devid, pci_dev->addr.function);
}

static int
mlx5_regex_pci_probe(struct rte_pci_driver *pci_drv __rte_unused,
		     struct rte_pci_device *pci_dev)
{
	struct ibv_device *ibv;
	struct mlx5_regex_priv *priv = NULL;
	struct ibv_context *ctx = NULL;
	struct mlx5_hca_attr attr;
	char name[RTE_REGEXDEV_NAME_MAX_LEN];
	int ret;

	ibv = mlx5_regex_get_ib_device_match(&pci_dev->addr);
	if (!ibv) {
		DRV_LOG(ERR, "No matching IB device for PCI slot "
			PCI_PRI_FMT ".", pci_dev->addr.domain,
			pci_dev->addr.bus, pci_dev->addr.devid,
			pci_dev->addr.function);
		return -rte_errno;
	}
	DRV_LOG(INFO, "PCI information matches for device \"%s\".",
		ibv->name);
	ctx = mlx5_glue->dv_open_device(ibv);
	if (!ctx) {
		DRV_LOG(ERR, "Failed to open IB device \"%s\".", ibv->name);
		rte_errno = ENODEV;
		return -rte_errno;
	}
	ret = mlx5_devx_cmd_query_hca_attr(ctx, &attr);
	if (ret) {
		DRV_LOG(ERR, "Unable to read HCA capabilities.");
		rte_errno = ENOTSUP;
		goto error;
	} else if (!attr.regex || attr.regexp_num_of_engines == 0) {
		DRV_LOG(ERR, "Not enough capabilities to support RegEx, maybe "
			"old FW/OFED version?");
		rte_errno = ENOTSUP;
		goto error;
	}
	priv = rte_zmalloc("mlx5 regex device private", sizeof(*priv),
			   RTE_CACHE_LINE_SIZE);
	if (!priv) {
		DRV_LOG(ERR, "Failed to allocate private memory.");
		rte_errno = ENOMEM;
		goto error;
	}
	priv->ctx = ctx;
	mlx5_regex_get_name(name, pci_dev);
	priv->regexdev = rte_regexdev_register(name);
	if (priv->regexdev == NULL) {
		DRV_LOG(ERR, "Failed to register RegEx device.");
		rte_errno = rte_errno ? rte_errno : EINVAL;
		goto error;
	}
	priv->regexdev->dev_ops = &mlx5_regexdev_ops;
	priv->regexdev->device = (struct rte_device *)pci_dev;
	priv->regexdev->data->dev_private = priv;
	return 0;

error:
	if (ctx)
		mlx5_glue->close_device(ctx);
	if (priv)
		rte_free(priv);
	return -rte_errno;
}

static int
mlx5_regex_pci_remove(struct rte_pci_device *pci_dev)
{
	char name[RTE_REGEXDEV_NAME_MAX_LEN];
	struct rte_regexdev *dev;
	struct mlx5_regex_priv *priv = NULL;

	mlx5_regex_get_name(name, pci_dev);
	dev = rte_regexdev_get_device_by_name(name);
	if (!dev)
		return 0;
	priv = dev->data->dev_private;
	if (priv) {
		if (priv->ctx)
			mlx5_glue->close_device(priv->ctx);
		if (priv->regexdev)
			rte_regexdev_unregister(priv->regexdev);
		rte_free(priv);
	}
	return 0;
}

static const struct rte_pci_id mlx5_regex_pci_id_map[] = {
	{
		RTE_PCI_DEVICE(PCI_VENDOR_ID_MELLANOX,
				PCI_DEVICE_ID_MELLANOX_CONNECTX6DXBF)
	},
	{
		.vendor_id = 0
	}
};

static struct rte_pci_driver mlx5_regex_driver = {
	.driver = {
		.name = "mlx5_regex",
	},
	.id_table = mlx5_regex_pci_id_map,
	.probe = mlx5_regex_pci_probe,
	.remove = mlx5_regex_pci_remove,
	.drv_flags = 0,
};

RTE_INIT(rte_mlx5_regex_init)
{
	if (mlx5_glue)
		rte_pci_register(&mlx5_regex_driver);
}

RTE_LOG_REGISTER(mlx5_regex_logtype, pmd.regex.mlx5, NOTICE)
RTE_PMD_EXPORT_NAME(net_mlx5_regex, __COUNTER__);
RTE_PMD_REGISTER_PCI_TABLE(net_mlx5_regex, mlx5_regex_pci_id_map);
RTE_PMD_REGISTER_KMOD_DEP(net_mlx5_regex, "* ib_uverbs & mlx5_core & mlx5_ib");
