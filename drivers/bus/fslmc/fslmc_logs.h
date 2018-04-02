/* SPDX-License-Identifier: BSD-3-Clause
 *
 *   Copyright 2016 NXP
 *
 */

#ifndef _FSLMC_LOGS_H_
#define _FSLMC_LOGS_H_

extern int dpaa2_logtype_bus;

#define DPAA2_BUS_LOG(level, fmt, args...) \
	rte_log(RTE_LOG_ ## level, dpaa2_logtype_bus, "fslmc: " fmt "\n", \
		##args)

/* Debug logs are with Function names */
#define DPAA2_BUS_DEBUG(fmt, args...) \
	rte_log(RTE_LOG_DEBUG, dpaa2_logtype_bus, "fslmc: %s(): " fmt "\n", \
		__func__, ##args)

#define BUS_INIT_FUNC_TRACE() DPAA2_BUS_LOG(DEBUG, " >>")

#define DPAA2_BUS_INFO(fmt, args...) \
	DPAA2_BUS_LOG(INFO, fmt, ## args)
#define DPAA2_BUS_ERR(fmt, args...) \
	DPAA2_BUS_LOG(ERR, fmt, ## args)
#define DPAA2_BUS_WARN(fmt, args...) \
	DPAA2_BUS_LOG(WARNING, fmt, ## args)

/* DP Logs, toggled out at compile time if level lower than current level */
#define DPAA2_BUS_DP_LOG(level, fmt, args...) \
	RTE_LOG_DP(level, PMD, fmt, ## args)

#define DPAA2_BUS_DP_DEBUG(fmt, args...) \
	DPAA2_BUS_DP_LOG(DEBUG, fmt, ## args)
#define DPAA2_BUS_DP_INFO(fmt, args...) \
	DPAA2_BUS_DP_LOG(INFO, fmt, ## args)
#define DPAA2_BUS_DP_WARN(fmt, args...) \
	DPAA2_BUS_DP_LOG(WARNING, fmt, ## args)

#define PMD_INIT_LOG(level, fmt, args...) \
	RTE_LOG(level, PMD, "%s(): " fmt "\n", __func__, ##args)

#ifdef RTE_LIBRTE_DPAA2_DEBUG_INIT
#define PMD_INIT_FUNC_TRACE() PMD_INIT_LOG(DEBUG, " >>")
#else
#define PMD_INIT_FUNC_TRACE() do { } while (0)
#endif

#ifdef RTE_LIBRTE_DPAA2_DEBUG_RX
#define PMD_RX_LOG(level, fmt, args...) \
	RTE_LOG(level, PMD, "%s(): " fmt "\n", __func__, ## args)
#else
#define PMD_RX_LOG(level, fmt, args...) do { } while (0)
#endif

#ifdef RTE_LIBRTE_DPAA2_DEBUG_TX
#define PMD_TX_LOG(level, fmt, args...) \
	RTE_LOG(level, PMD, "%s(): " fmt "\n", __func__, ## args)
#else
#define PMD_TX_LOG(level, fmt, args...) do { } while (0)
#endif

#ifdef RTE_LIBRTE_DPAA2_DEBUG_TX_FREE
#define PMD_TX_FREE_LOG(level, fmt, args...) \
	RTE_LOG(level, PMD, "%s(): " fmt "\n", __func__, ## args)
#else
#define PMD_TX_FREE_LOG(level, fmt, args...) do { } while (0)
#endif

#ifdef RTE_LIBRTE_DPAA2_DEBUG_DRIVER
#define PMD_DRV_LOG_RAW(level, fmt, args...) \
	RTE_LOG(level, PMD, "%s(): " fmt, __func__, ## args)
#else
#define PMD_DRV_LOG_RAW(level, fmt, args...) do { } while (0)
#endif

#define PMD_DRV_LOG(level, fmt, args...) \
	PMD_DRV_LOG_RAW(level, fmt "\n", ## args)

#endif /* _FSLMC_LOGS_H_ */
