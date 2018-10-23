/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Gaëtan Rivet
 */

#ifndef _RTE_ETH_PRIVATE_H_
#define _RTE_ETH_PRIVATE_H_

#include "rte_ethdev.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Convert rte_eth_dev pointer to port id.
 * NULL will be translated to RTE_MAX_ETHPORTS.
 */
uint16_t eth_dev_to_id(const struct rte_eth_dev *dev);

/* Generic rte_eth_dev comparison function. */
typedef int (*rte_eth_cmp_t)(const struct rte_eth_dev *, const void *);

/* Generic rte_eth_dev iterator. */
struct rte_eth_dev *
eth_find_device(const struct rte_eth_dev *_start, rte_eth_cmp_t cmp,
		const void *data);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_ETH_PRIVATE_H_ */
