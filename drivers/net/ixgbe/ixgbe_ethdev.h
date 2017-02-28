/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2016 Intel Corporation. All rights reserved.
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
 *     * Neither the name of Intel Corporation nor the names of its
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

#ifndef _IXGBE_ETHDEV_H_
#define _IXGBE_ETHDEV_H_
#include "base/ixgbe_dcb.h"
#include "base/ixgbe_dcb_82599.h"
#include "base/ixgbe_dcb_82598.h"
#include "ixgbe_bypass.h"
#include <rte_time.h>
#include <rte_hash.h>

/* need update link, bit flag */
#define IXGBE_FLAG_NEED_LINK_UPDATE (uint32_t)(1 << 0)
#define IXGBE_FLAG_MAILBOX          (uint32_t)(1 << 1)
#define IXGBE_FLAG_PHY_INTERRUPT    (uint32_t)(1 << 2)
#define IXGBE_FLAG_MACSEC           (uint32_t)(1 << 3)

/*
 * Defines that were not part of ixgbe_type.h as they are not used by the
 * FreeBSD driver.
 */
#define IXGBE_ADVTXD_MAC_1588       0x00080000 /* IEEE1588 Timestamp packet */
#define IXGBE_RXD_STAT_TMST         0x10000    /* Timestamped Packet indication */
#define IXGBE_ADVTXD_TUCMD_L4T_RSV  0x00001800 /* L4 Packet TYPE, resvd  */
#define IXGBE_RXDADV_ERR_CKSUM_BIT  30
#define IXGBE_RXDADV_ERR_CKSUM_MSK  3
#define IXGBE_ADVTXD_MACLEN_SHIFT   9          /* Bit shift for l2_len */
#define IXGBE_NB_STAT_MAPPING_REGS  32
#define IXGBE_EXTENDED_VLAN	  (uint32_t)(1 << 26) /* EXTENDED VLAN ENABLE */
#define IXGBE_VFTA_SIZE 128
#define IXGBE_VLAN_TAG_SIZE 4
#define IXGBE_MAX_RX_QUEUE_NUM	128
#define IXGBE_MAX_INTR_QUEUE_NUM	15
#define IXGBE_VMDQ_DCB_NB_QUEUES     IXGBE_MAX_RX_QUEUE_NUM
#define IXGBE_DCB_NB_QUEUES          IXGBE_MAX_RX_QUEUE_NUM
#define IXGBE_NONE_MODE_TX_NB_QUEUES 64

#ifndef NBBY
#define NBBY	8	/* number of bits in a byte */
#endif
#define IXGBE_HWSTRIP_BITMAP_SIZE (IXGBE_MAX_RX_QUEUE_NUM / (sizeof(uint32_t) * NBBY))

/* EITR Inteval is in 2048ns uinits for 1G and 10G link */
#define IXGBE_EITR_INTERVAL_UNIT_NS	2048
#define IXGBE_EITR_ITR_INT_SHIFT       3
#define IXGBE_EITR_INTERVAL_US(us) \
	(((us) * 1000 / IXGBE_EITR_INTERVAL_UNIT_NS << IXGBE_EITR_ITR_INT_SHIFT) & \
		IXGBE_EITR_ITR_INT_MASK)


/* Loopback operation modes */
/* 82599 specific loopback operation types */
#define IXGBE_LPBK_82599_NONE   0x0 /* Default value. Loopback is disabled. */
#define IXGBE_LPBK_82599_TX_RX  0x1 /* Tx->Rx loopback operation is enabled. */

#define IXGBE_MAX_JUMBO_FRAME_SIZE      0x2600 /* Maximum Jumbo frame size. */

#define IXGBE_RTTBCNRC_RF_INT_MASK_BASE 0x000003FF
#define IXGBE_RTTBCNRC_RF_INT_MASK_M \
	(IXGBE_RTTBCNRC_RF_INT_MASK_BASE << IXGBE_RTTBCNRC_RF_INT_SHIFT)

#define IXGBE_MAX_QUEUE_NUM_PER_VF  8

#define IXGBE_SYN_FILTER_ENABLE         0x00000001 /* syn filter enable field */
#define IXGBE_SYN_FILTER_QUEUE          0x000000FE /* syn filter queue field */
#define IXGBE_SYN_FILTER_QUEUE_SHIFT    1          /* syn filter queue field shift */
#define IXGBE_SYN_FILTER_SYNQFP         0x80000000 /* syn filter SYNQFP */

#define IXGBE_ETQF_UP                   0x00070000 /* ethertype filter priority field */
#define IXGBE_ETQF_SHIFT                16
#define IXGBE_ETQF_UP_EN                0x00080000
#define IXGBE_ETQF_ETHERTYPE            0x0000FFFF /* ethertype filter ethertype field */
#define IXGBE_ETQF_MAX_PRI              7

#define IXGBE_SDPQF_DSTPORT             0xFFFF0000 /* dst port field */
#define IXGBE_SDPQF_DSTPORT_SHIFT       16         /* dst port field shift */
#define IXGBE_SDPQF_SRCPORT             0x0000FFFF /* src port field */

#define IXGBE_L34T_IMIR_SIZE_BP         0x00001000
#define IXGBE_L34T_IMIR_RESERVE         0x00080000 /* bit 13 to 19 must be set to 1000000b. */
#define IXGBE_L34T_IMIR_LLI             0x00100000
#define IXGBE_L34T_IMIR_QUEUE           0x0FE00000
#define IXGBE_L34T_IMIR_QUEUE_SHIFT     21
#define IXGBE_5TUPLE_MAX_PRI            7
#define IXGBE_5TUPLE_MIN_PRI            1

#define IXGBE_RSS_OFFLOAD_ALL ( \
	ETH_RSS_IPV4 | \
	ETH_RSS_NONFRAG_IPV4_TCP | \
	ETH_RSS_NONFRAG_IPV4_UDP | \
	ETH_RSS_IPV6 | \
	ETH_RSS_NONFRAG_IPV6_TCP | \
	ETH_RSS_NONFRAG_IPV6_UDP | \
	ETH_RSS_IPV6_EX | \
	ETH_RSS_IPV6_TCP_EX | \
	ETH_RSS_IPV6_UDP_EX)

#define IXGBE_VF_IRQ_ENABLE_MASK        3          /* vf irq enable mask */
#define IXGBE_VF_MAXMSIVECTOR           1

#define IXGBE_MISC_VEC_ID               RTE_INTR_VEC_ZERO_OFFSET
#define IXGBE_RX_VEC_START              RTE_INTR_VEC_RXTX_OFFSET

#define IXGBE_SECTX_MINSECIFG_MASK      0x0000000F

#define IXGBE_MACSEC_PNTHRSH            0xFFFFFE00

#define IXGBE_MAX_FDIR_FILTER_NUM       (1024 * 32)
#define IXGBE_MAX_L2_TN_FILTER_NUM      128

/*
 * Information about the fdir mode.
 */
struct ixgbe_hw_fdir_mask {
	uint16_t vlan_tci_mask;
	uint32_t src_ipv4_mask;
	uint32_t dst_ipv4_mask;
	uint16_t src_ipv6_mask;
	uint16_t dst_ipv6_mask;
	uint16_t src_port_mask;
	uint16_t dst_port_mask;
	uint16_t flex_bytes_mask;
	uint8_t  mac_addr_byte_mask;
	uint32_t tunnel_id_mask;
	uint8_t  tunnel_type_mask;
};

struct ixgbe_fdir_filter {
	TAILQ_ENTRY(ixgbe_fdir_filter) entries;
	union ixgbe_atr_input ixgbe_fdir; /* key of fdir filter*/
	uint32_t fdirflags; /* drop or forward */
	uint32_t fdirhash; /* hash value for fdir */
	uint8_t queue; /* assigned rx queue */
};

/* list of fdir filters */
TAILQ_HEAD(ixgbe_fdir_filter_list, ixgbe_fdir_filter);

struct ixgbe_fdir_rule {
	struct ixgbe_hw_fdir_mask mask;
	union ixgbe_atr_input ixgbe_fdir; /* key of fdir filter*/
	bool b_spec; /* If TRUE, ixgbe_fdir, fdirflags, queue have meaning. */
	bool b_mask; /* If TRUE, mask has meaning. */
	enum rte_fdir_mode mode; /* IP, MAC VLAN, Tunnel */
	uint32_t fdirflags; /* drop or forward */
	uint32_t soft_id; /* an unique value for this rule */
	uint8_t queue; /* assigned rx queue */
};

struct ixgbe_hw_fdir_info {
	struct ixgbe_hw_fdir_mask mask;
	uint8_t     flex_bytes_offset;
	uint16_t    collision;
	uint16_t    free;
	uint16_t    maxhash;
	uint8_t     maxlen;
	uint64_t    add;
	uint64_t    remove;
	uint64_t    f_add;
	uint64_t    f_remove;
	struct ixgbe_fdir_filter_list fdir_list; /* filter list*/
	/* store the pointers of the filters, index is the hash value. */
	struct ixgbe_fdir_filter **hash_map;
	struct rte_hash *hash_handle; /* cuckoo hash handler */
	bool mask_added; /* If already got mask from consistent filter */
};

/* structure for interrupt relative data */
struct ixgbe_interrupt {
	uint32_t flags;
	uint32_t mask;
	/*to save original mask during delayed handler */
	uint32_t mask_original;
};

struct ixgbe_stat_mapping_registers {
	uint32_t tqsm[IXGBE_NB_STAT_MAPPING_REGS];
	uint32_t rqsmr[IXGBE_NB_STAT_MAPPING_REGS];
};

struct ixgbe_vfta {
	uint32_t vfta[IXGBE_VFTA_SIZE];
};

struct ixgbe_hwstrip {
	uint32_t bitmap[IXGBE_HWSTRIP_BITMAP_SIZE];
};

/*
 * VF data which used by PF host only
 */
#define IXGBE_MAX_VF_MC_ENTRIES		30
#define IXGBE_MAX_MR_RULE_ENTRIES	4 /* number of mirroring rules supported */
#define IXGBE_MAX_UTA                   128

struct ixgbe_uta_info {
	uint8_t  uc_filter_type;
	uint16_t uta_in_use;
	uint32_t uta_shadow[IXGBE_MAX_UTA];
};

#define IXGBE_MAX_MIRROR_RULES 4  /* Maximum nb. of mirror rules. */

struct ixgbe_mirror_info {
	struct rte_eth_mirror_conf mr_conf[IXGBE_MAX_MIRROR_RULES];
	/**< store PF mirror rules configuration*/
};

struct ixgbe_vf_info {
	uint8_t vf_mac_addresses[ETHER_ADDR_LEN];
	uint16_t vf_mc_hashes[IXGBE_MAX_VF_MC_ENTRIES];
	uint16_t num_vf_mc_hashes;
	uint16_t default_vf_vlan_id;
	uint16_t vlans_enabled;
	bool clear_to_send;
	uint16_t tx_rate[IXGBE_MAX_QUEUE_NUM_PER_VF];
	uint16_t vlan_count;
	uint8_t spoofchk_enabled;
	uint8_t api_version;
};

/*
 *  Possible l4type of 5tuple filters.
 */
enum ixgbe_5tuple_protocol {
	IXGBE_FILTER_PROTOCOL_TCP = 0,
	IXGBE_FILTER_PROTOCOL_UDP,
	IXGBE_FILTER_PROTOCOL_SCTP,
	IXGBE_FILTER_PROTOCOL_NONE,
};

TAILQ_HEAD(ixgbe_5tuple_filter_list, ixgbe_5tuple_filter);

struct ixgbe_5tuple_filter_info {
	uint32_t dst_ip;
	uint32_t src_ip;
	uint16_t dst_port;
	uint16_t src_port;
	enum ixgbe_5tuple_protocol proto;        /* l4 protocol. */
	uint8_t priority;        /* seven levels (001b-111b), 111b is highest,
				      used when more than one filter matches. */
	uint8_t dst_ip_mask:1,   /* if mask is 1b, do not compare dst ip. */
		src_ip_mask:1,   /* if mask is 1b, do not compare src ip. */
		dst_port_mask:1, /* if mask is 1b, do not compare dst port. */
		src_port_mask:1, /* if mask is 1b, do not compare src port. */
		proto_mask:1;    /* if mask is 1b, do not compare protocol. */
};

/* 5tuple filter structure */
struct ixgbe_5tuple_filter {
	TAILQ_ENTRY(ixgbe_5tuple_filter) entries;
	uint16_t index;       /* the index of 5tuple filter */
	struct ixgbe_5tuple_filter_info filter_info;
	uint16_t queue;       /* rx queue assigned to */
};

#define IXGBE_5TUPLE_ARRAY_SIZE \
	(RTE_ALIGN(IXGBE_MAX_FTQF_FILTERS, (sizeof(uint32_t) * NBBY)) / \
	 (sizeof(uint32_t) * NBBY))

struct ixgbe_ethertype_filter {
	uint16_t ethertype;
	uint32_t etqf;
	uint32_t etqs;
	/**
	 * If this filter is added by configuration,
	 * it should not be removed.
	 */
	bool     conf;
};

/*
 * Structure to store filters' info.
 */
struct ixgbe_filter_info {
	uint8_t ethertype_mask;  /* Bit mask for every used ethertype filter */
	/* store used ethertype filters*/
	struct ixgbe_ethertype_filter ethertype_filters[IXGBE_MAX_ETQF_FILTERS];
	/* Bit mask for every used 5tuple filter */
	uint32_t fivetuple_mask[IXGBE_5TUPLE_ARRAY_SIZE];
	struct ixgbe_5tuple_filter_list fivetuple_list;
	/* store the SYN filter info */
	uint32_t syn_info;
};

struct ixgbe_l2_tn_key {
	enum rte_eth_tunnel_type          l2_tn_type;
	uint32_t                          tn_id;
};

struct ixgbe_l2_tn_filter {
	TAILQ_ENTRY(ixgbe_l2_tn_filter)    entries;
	struct ixgbe_l2_tn_key             key;
	uint32_t                           pool;
};

TAILQ_HEAD(ixgbe_l2_tn_filter_list, ixgbe_l2_tn_filter);

struct ixgbe_l2_tn_info {
	struct ixgbe_l2_tn_filter_list      l2_tn_list;
	struct ixgbe_l2_tn_filter         **hash_map;
	struct rte_hash                    *hash_handle;
	bool e_tag_en; /* e-tag enabled */
	bool e_tag_fwd_en; /* e-tag based forwarding enabled */
	bool e_tag_ether_type; /* ether type for e-tag */
};

struct rte_flow {
	enum rte_filter_type filter_type;
	void *rule;
};
/* ntuple filter list structure */
struct ixgbe_ntuple_filter_ele {
	TAILQ_ENTRY(ixgbe_ntuple_filter_ele) entries;
	struct rte_eth_ntuple_filter filter_info;
};
/* ethertype filter list structure */
struct ixgbe_ethertype_filter_ele {
	TAILQ_ENTRY(ixgbe_ethertype_filter_ele) entries;
	struct rte_eth_ethertype_filter filter_info;
};
/* syn filter list structure */
struct ixgbe_eth_syn_filter_ele {
	TAILQ_ENTRY(ixgbe_eth_syn_filter_ele) entries;
	struct rte_eth_syn_filter filter_info;
};
/* fdir filter list structure */
struct ixgbe_fdir_rule_ele {
	TAILQ_ENTRY(ixgbe_fdir_rule_ele) entries;
	struct ixgbe_fdir_rule filter_info;
};
/* l2_tunnel filter list structure */
struct ixgbe_eth_l2_tunnel_conf_ele {
	TAILQ_ENTRY(ixgbe_eth_l2_tunnel_conf_ele) entries;
	struct rte_eth_l2_tunnel_conf filter_info;
};
/* ixgbe_flow memory list structure */
struct ixgbe_flow_mem {
	TAILQ_ENTRY(ixgbe_flow_mem) entries;
	struct rte_flow *flow;
};

TAILQ_HEAD(ixgbe_ntuple_filter_list, ixgbe_ntuple_filter_ele);
struct ixgbe_ntuple_filter_list filter_ntuple_list;
TAILQ_HEAD(ixgbe_ethertype_filter_list, ixgbe_ethertype_filter_ele);
struct ixgbe_ethertype_filter_list filter_ethertype_list;
TAILQ_HEAD(ixgbe_syn_filter_list, ixgbe_eth_syn_filter_ele);
struct ixgbe_syn_filter_list filter_syn_list;
TAILQ_HEAD(ixgbe_fdir_rule_filter_list, ixgbe_fdir_rule_ele);
struct ixgbe_fdir_rule_filter_list filter_fdir_list;
TAILQ_HEAD(ixgbe_l2_tunnel_filter_list, ixgbe_eth_l2_tunnel_conf_ele);
struct ixgbe_l2_tunnel_filter_list filter_l2_tunnel_list;
TAILQ_HEAD(ixgbe_flow_mem_list, ixgbe_flow_mem);
struct ixgbe_flow_mem_list ixgbe_flow_list;

/*
 * Statistics counters collected by the MACsec
 */
struct ixgbe_macsec_stats {
	/* TX port statistics */
	uint64_t out_pkts_untagged;
	uint64_t out_pkts_encrypted;
	uint64_t out_pkts_protected;
	uint64_t out_octets_encrypted;
	uint64_t out_octets_protected;

	/* RX port statistics */
	uint64_t in_pkts_untagged;
	uint64_t in_pkts_badtag;
	uint64_t in_pkts_nosci;
	uint64_t in_pkts_unknownsci;
	uint64_t in_octets_decrypted;
	uint64_t in_octets_validated;

	/* RX SC statistics */
	uint64_t in_pkts_unchecked;
	uint64_t in_pkts_delayed;
	uint64_t in_pkts_late;

	/* RX SA statistics */
	uint64_t in_pkts_ok;
	uint64_t in_pkts_invalid;
	uint64_t in_pkts_notvalid;
	uint64_t in_pkts_unusedsa;
	uint64_t in_pkts_notusingsa;
};

/*
 * Structure to store private data for each driver instance (for each port).
 */
struct ixgbe_adapter {
	struct ixgbe_hw             hw;
	struct ixgbe_hw_stats       stats;
	struct ixgbe_macsec_stats   macsec_stats;
	struct ixgbe_hw_fdir_info   fdir;
	struct ixgbe_interrupt      intr;
	struct ixgbe_stat_mapping_registers stat_mappings;
	struct ixgbe_vfta           shadow_vfta;
	struct ixgbe_hwstrip		hwstrip;
	struct ixgbe_dcb_config     dcb_config;
	struct ixgbe_mirror_info    mr_data;
	struct ixgbe_vf_info        *vfdata;
	struct ixgbe_uta_info       uta_info;
#ifdef RTE_NIC_BYPASS
	struct ixgbe_bypass_info    bps;
#endif /* RTE_NIC_BYPASS */
	struct ixgbe_filter_info    filter;
	struct ixgbe_l2_tn_info     l2_tn;

	bool rx_bulk_alloc_allowed;
	bool rx_vec_allowed;
	struct rte_timecounter      systime_tc;
	struct rte_timecounter      rx_tstamp_tc;
	struct rte_timecounter      tx_tstamp_tc;
};

#define IXGBE_DEV_TO_PCI(eth_dev) \
	RTE_DEV_TO_PCI((eth_dev)->device)

#define IXGBE_DEV_PRIVATE_TO_HW(adapter)\
	(&((struct ixgbe_adapter *)adapter)->hw)

#define IXGBE_DEV_PRIVATE_TO_STATS(adapter) \
	(&((struct ixgbe_adapter *)adapter)->stats)

#define IXGBE_DEV_PRIVATE_TO_MACSEC_STATS(adapter) \
	(&((struct ixgbe_adapter *)adapter)->macsec_stats)

#define IXGBE_DEV_PRIVATE_TO_INTR(adapter) \
	(&((struct ixgbe_adapter *)adapter)->intr)

#define IXGBE_DEV_PRIVATE_TO_FDIR_INFO(adapter) \
	(&((struct ixgbe_adapter *)adapter)->fdir)

#define IXGBE_DEV_PRIVATE_TO_STAT_MAPPINGS(adapter) \
	(&((struct ixgbe_adapter *)adapter)->stat_mappings)

#define IXGBE_DEV_PRIVATE_TO_VFTA(adapter) \
	(&((struct ixgbe_adapter *)adapter)->shadow_vfta)

#define IXGBE_DEV_PRIVATE_TO_HWSTRIP_BITMAP(adapter) \
	(&((struct ixgbe_adapter *)adapter)->hwstrip)

#define IXGBE_DEV_PRIVATE_TO_DCB_CFG(adapter) \
	(&((struct ixgbe_adapter *)adapter)->dcb_config)

#define IXGBE_DEV_PRIVATE_TO_P_VFDATA(adapter) \
	(&((struct ixgbe_adapter *)adapter)->vfdata)

#define IXGBE_DEV_PRIVATE_TO_PFDATA(adapter) \
	(&((struct ixgbe_adapter *)adapter)->mr_data)

#define IXGBE_DEV_PRIVATE_TO_UTA(adapter) \
	(&((struct ixgbe_adapter *)adapter)->uta_info)

#define IXGBE_DEV_PRIVATE_TO_FILTER_INFO(adapter) \
	(&((struct ixgbe_adapter *)adapter)->filter)

#define IXGBE_DEV_PRIVATE_TO_L2_TN_INFO(adapter) \
	(&((struct ixgbe_adapter *)adapter)->l2_tn)

/*
 * RX/TX function prototypes
 */
void ixgbe_dev_clear_queues(struct rte_eth_dev *dev);

void ixgbe_dev_free_queues(struct rte_eth_dev *dev);

void ixgbe_dev_rx_queue_release(void *rxq);

void ixgbe_dev_tx_queue_release(void *txq);

int  ixgbe_dev_rx_queue_setup(struct rte_eth_dev *dev, uint16_t rx_queue_id,
		uint16_t nb_rx_desc, unsigned int socket_id,
		const struct rte_eth_rxconf *rx_conf,
		struct rte_mempool *mb_pool);

int  ixgbe_dev_tx_queue_setup(struct rte_eth_dev *dev, uint16_t tx_queue_id,
		uint16_t nb_tx_desc, unsigned int socket_id,
		const struct rte_eth_txconf *tx_conf);

uint32_t ixgbe_dev_rx_queue_count(struct rte_eth_dev *dev,
		uint16_t rx_queue_id);

int ixgbe_dev_rx_descriptor_done(void *rx_queue, uint16_t offset);

int ixgbe_dev_rx_descriptor_status(void *rx_queue, uint16_t offset);
int ixgbe_dev_tx_descriptor_status(void *tx_queue, uint16_t offset);

int ixgbe_dev_rx_init(struct rte_eth_dev *dev);

void ixgbe_dev_tx_init(struct rte_eth_dev *dev);

int ixgbe_dev_rxtx_start(struct rte_eth_dev *dev);

int ixgbe_dev_rx_queue_start(struct rte_eth_dev *dev, uint16_t rx_queue_id);

int ixgbe_dev_rx_queue_stop(struct rte_eth_dev *dev, uint16_t rx_queue_id);

int ixgbe_dev_tx_queue_start(struct rte_eth_dev *dev, uint16_t tx_queue_id);

int ixgbe_dev_tx_queue_stop(struct rte_eth_dev *dev, uint16_t tx_queue_id);

void ixgbe_rxq_info_get(struct rte_eth_dev *dev, uint16_t queue_id,
	struct rte_eth_rxq_info *qinfo);

void ixgbe_txq_info_get(struct rte_eth_dev *dev, uint16_t queue_id,
	struct rte_eth_txq_info *qinfo);

int ixgbevf_dev_rx_init(struct rte_eth_dev *dev);

void ixgbevf_dev_tx_init(struct rte_eth_dev *dev);

void ixgbevf_dev_rxtx_start(struct rte_eth_dev *dev);

uint16_t ixgbe_recv_pkts(void *rx_queue, struct rte_mbuf **rx_pkts,
		uint16_t nb_pkts);

uint16_t ixgbe_recv_pkts_bulk_alloc(void *rx_queue, struct rte_mbuf **rx_pkts,
				    uint16_t nb_pkts);

uint16_t ixgbe_recv_pkts_lro_single_alloc(void *rx_queue,
		struct rte_mbuf **rx_pkts, uint16_t nb_pkts);
uint16_t ixgbe_recv_pkts_lro_bulk_alloc(void *rx_queue,
		struct rte_mbuf **rx_pkts, uint16_t nb_pkts);

uint16_t ixgbe_xmit_pkts(void *tx_queue, struct rte_mbuf **tx_pkts,
		uint16_t nb_pkts);

uint16_t ixgbe_xmit_pkts_simple(void *tx_queue, struct rte_mbuf **tx_pkts,
		uint16_t nb_pkts);

uint16_t ixgbe_prep_pkts(void *tx_queue, struct rte_mbuf **tx_pkts,
		uint16_t nb_pkts);

int ixgbe_dev_rss_hash_update(struct rte_eth_dev *dev,
			      struct rte_eth_rss_conf *rss_conf);

int ixgbe_dev_rss_hash_conf_get(struct rte_eth_dev *dev,
				struct rte_eth_rss_conf *rss_conf);

uint16_t ixgbe_reta_size_get(enum ixgbe_mac_type mac_type);

uint32_t ixgbe_reta_reg_get(enum ixgbe_mac_type mac_type, uint16_t reta_idx);

uint32_t ixgbe_mrqc_reg_get(enum ixgbe_mac_type mac_type);

uint32_t ixgbe_rssrk_reg_get(enum ixgbe_mac_type mac_type, uint8_t i);

bool ixgbe_rss_update_sp(enum ixgbe_mac_type mac_type);

int ixgbe_add_del_ntuple_filter(struct rte_eth_dev *dev,
			struct rte_eth_ntuple_filter *filter,
			bool add);
int ixgbe_add_del_ethertype_filter(struct rte_eth_dev *dev,
			struct rte_eth_ethertype_filter *filter,
			bool add);
int ixgbe_syn_filter_set(struct rte_eth_dev *dev,
			struct rte_eth_syn_filter *filter,
			bool add);
int
ixgbe_dev_l2_tunnel_filter_add(struct rte_eth_dev *dev,
			       struct rte_eth_l2_tunnel_conf *l2_tunnel,
			       bool restore);
int
ixgbe_dev_l2_tunnel_filter_del(struct rte_eth_dev *dev,
			       struct rte_eth_l2_tunnel_conf *l2_tunnel);
void ixgbe_filterlist_flush(void);
/*
 * Flow director function prototypes
 */
int ixgbe_fdir_configure(struct rte_eth_dev *dev);
int ixgbe_fdir_set_input_mask(struct rte_eth_dev *dev);
int ixgbe_fdir_filter_program(struct rte_eth_dev *dev,
			      struct ixgbe_fdir_rule *rule,
			      bool del, bool update);

void ixgbe_configure_dcb(struct rte_eth_dev *dev);

/*
 * misc function prototypes
 */
void ixgbe_vlan_hw_filter_enable(struct rte_eth_dev *dev);

void ixgbe_vlan_hw_filter_disable(struct rte_eth_dev *dev);

void ixgbe_vlan_hw_strip_enable_all(struct rte_eth_dev *dev);

void ixgbe_vlan_hw_strip_disable_all(struct rte_eth_dev *dev);

void ixgbe_pf_host_init(struct rte_eth_dev *eth_dev);

void ixgbe_pf_host_uninit(struct rte_eth_dev *eth_dev);

void ixgbe_pf_mbx_process(struct rte_eth_dev *eth_dev);

int ixgbe_pf_host_configure(struct rte_eth_dev *eth_dev);

uint32_t ixgbe_convert_vm_rx_mask_to_val(uint16_t rx_mask, uint32_t orig_val);

int ixgbe_fdir_ctrl_func(struct rte_eth_dev *dev,
			enum rte_filter_op filter_op, void *arg);
void ixgbe_fdir_filter_restore(struct rte_eth_dev *dev);
int ixgbe_clear_all_fdir_filter(struct rte_eth_dev *dev);

extern const struct rte_flow_ops ixgbe_flow_ops;

void ixgbe_clear_all_ethertype_filter(struct rte_eth_dev *dev);
void ixgbe_clear_all_ntuple_filter(struct rte_eth_dev *dev);
void ixgbe_clear_syn_filter(struct rte_eth_dev *dev);
int ixgbe_clear_all_l2_tn_filter(struct rte_eth_dev *dev);

int ixgbe_disable_sec_tx_path_generic(struct ixgbe_hw *hw);

int ixgbe_enable_sec_tx_path_generic(struct ixgbe_hw *hw);

static inline int
ixgbe_ethertype_filter_lookup(struct ixgbe_filter_info *filter_info,
			      uint16_t ethertype)
{
	int i;

	for (i = 0; i < IXGBE_MAX_ETQF_FILTERS; i++) {
		if (filter_info->ethertype_filters[i].ethertype == ethertype &&
		    (filter_info->ethertype_mask & (1 << i)))
			return i;
	}
	return -1;
}

static inline int
ixgbe_ethertype_filter_insert(struct ixgbe_filter_info *filter_info,
			      struct ixgbe_ethertype_filter *ethertype_filter)
{
	int i;

	for (i = 0; i < IXGBE_MAX_ETQF_FILTERS; i++) {
		if (!(filter_info->ethertype_mask & (1 << i))) {
			filter_info->ethertype_mask |= 1 << i;
			filter_info->ethertype_filters[i].ethertype =
				ethertype_filter->ethertype;
			filter_info->ethertype_filters[i].etqf =
				ethertype_filter->etqf;
			filter_info->ethertype_filters[i].etqs =
				ethertype_filter->etqs;
			filter_info->ethertype_filters[i].conf =
				ethertype_filter->conf;
			return i;
		}
	}
	return -1;
}

static inline int
ixgbe_ethertype_filter_remove(struct ixgbe_filter_info *filter_info,
			      uint8_t idx)
{
	if (idx >= IXGBE_MAX_ETQF_FILTERS)
		return -1;
	filter_info->ethertype_mask &= ~(1 << idx);
	filter_info->ethertype_filters[idx].ethertype = 0;
	filter_info->ethertype_filters[idx].etqf = 0;
	filter_info->ethertype_filters[idx].etqs = 0;
	filter_info->ethertype_filters[idx].etqs = FALSE;
	return idx;
}

#endif /* _IXGBE_ETHDEV_H_ */
