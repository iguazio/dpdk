/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020 Mellanox Technologies, Ltd
 */

#ifndef MLX5_REGEX_H
#define MLX5_REGEX_H

struct mlx5_regex_priv {
	TAILQ_ENTRY(mlx5_regex_priv) next;
	struct ibv_context *ctx; /* Device context. */
	struct rte_pci_device *pci_dev;
	struct rte_regexdev *regexdev; /* Pointer to the RegEx dev. */
};

/* mlx5_rxp.c */
int mlx5_regex_info_get(struct rte_regexdev *dev,
			struct rte_regexdev_info *info);

#endif /* MLX5_REGEX_H */
