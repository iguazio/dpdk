/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2017 Intel Corporation
 */

/**
 * @file Header file containing DPDK compilation parameters
 *
 * Header file containing DPDK compilation parameters. Also include the
 * meson-generated header file containing the detected parameters that
 * are variable across builds or build environments.
 *
 * NOTE: This file is only used for meson+ninja builds. For builds done
 * using make/gmake, the rte_config.h file is autogenerated from the
 * defconfig_* files in the config directory.
 */
#ifndef _RTE_CONFIG_H_
#define _RTE_CONFIG_H_

#include <rte_build_config.h>

/****** library defines ********/

/* EAL defines */
#define RTE_MAX_MEMSEG_LISTS 128
#define RTE_MAX_MEMSEG_PER_LIST 8192
#define RTE_MAX_MEM_MB_PER_LIST 32768
#define RTE_MAX_MEMSEG_PER_TYPE 32768
#define RTE_MAX_MEM_MB_PER_TYPE 65536
#define RTE_MAX_MEM_MB 524288
#define RTE_MAX_MEMZONE 2560
#define RTE_MAX_TAILQ 32
#define RTE_LOG_DP_LEVEL RTE_LOG_INFO
#define RTE_BACKTRACE 1
#define RTE_EAL_VFIO 1

/* bsd module defines */
#define RTE_CONTIGMEM_MAX_NUM_BUFS 64
#define RTE_CONTIGMEM_DEFAULT_NUM_BUFS 1
#define RTE_CONTIGMEM_DEFAULT_BUF_SIZE (512*1024*1024)

/* mempool defines */
#define RTE_MEMPOOL_CACHE_MAX_SIZE 512

/* mbuf defines */
#define RTE_MBUF_DEFAULT_MEMPOOL_OPS "ring_mp_mc"
#define RTE_MBUF_REFCNT_ATOMIC 1
#define RTE_PKTMBUF_HEADROOM 128

/* ether defines */
#define RTE_MAX_ETHPORTS 32
#define RTE_MAX_QUEUES_PER_PORT 1024
#define RTE_ETHDEV_QUEUE_STAT_CNTRS 16
#define RTE_ETHDEV_RXTX_CALLBACKS 1

/* cryptodev defines */
#define RTE_CRYPTO_MAX_DEVS 64
#define RTE_CRYPTODEV_NAME_LEN 64

/* eventdev defines */
#define RTE_EVENT_MAX_DEVS 16
#define RTE_EVENT_MAX_QUEUES_PER_DEV 64
#define RTE_EVENT_TIMER_ADAPTER_NUM_MAX 32

/* rawdev defines */
#define RTE_RAWDEV_MAX_DEVS 10

/* ip_fragmentation defines */
#define RTE_LIBRTE_IP_FRAG_MAX_FRAG 4
#undef RTE_LIBRTE_IP_FRAG_TBL_STAT

/* rte_power defines */
#define RTE_MAX_LCORE_FREQS 64

/* rte_sched defines */
#undef RTE_SCHED_RED
#undef RTE_SCHED_COLLECT_STATS
#undef RTE_SCHED_SUBPORT_TC_OV
#define RTE_SCHED_PORT_N_GRINDERS 8
#undef RTE_SCHED_VECTOR

/****** driver defines ********/

/*
 * Number of sessions to create in the session memory pool
 * on a single instance of crypto HW device.
 */
/* QuickAssist device */
#define RTE_QAT_PMD_MAX_NB_SESSIONS 2048

/* DPAA2_SEC */
#define RTE_DPAA2_SEC_PMD_MAX_NB_SESSIONS 2048

/* DPAA_SEC */
#define RTE_DPAA_SEC_PMD_MAX_NB_SESSIONS 2048

/* DPAA SEC max cryptodev devices*/
#define RTE_LIBRTE_DPAA_MAX_CRYPTODEV	4

/* fm10k defines */
#define RTE_LIBRTE_FM10K_RX_OLFLAGS_ENABLE 1

/* i40e defines */
#define RTE_LIBRTE_I40E_RX_ALLOW_BULK_ALLOC 1
#undef RTE_LIBRTE_I40E_16BYTE_RX_DESC
#define RTE_LIBRTE_I40E_QUEUE_NUM_PER_PF 64
#define RTE_LIBRTE_I40E_QUEUE_NUM_PER_VF 4
#define RTE_LIBRTE_I40E_QUEUE_NUM_PER_VM 4
/* interval up to 8160 us, aligned to 2 (or default value) */
#define RTE_LIBRTE_I40E_ITR_INTERVAL -1

/* Ring net PMD settings */
#define RTE_PMD_RING_MAX_RX_RINGS 16
#define RTE_PMD_RING_MAX_TX_RINGS 16

#endif /* _RTE_CONFIG_H_ */
