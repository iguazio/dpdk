/*
 * Copyright (c) 2016 QLogic Corporation.
 * All rights reserved.
 * www.qlogic.com
 *
 * See LICENSE.qede_pmd for copyright and licensing details.
 */

#ifndef __ETH_COMMON__
#define __ETH_COMMON__
/********************/
/* ETH FW CONSTANTS */
/********************/

/* FP HSI version. FP HSI is compatible if (fwVer.major == drvVer.major &&
 * fwVer.minor >= drvVer.minor)
 */
/* ETH FP HSI Major version */
#define ETH_HSI_VER_MAJOR                   3
/* ETH FP HSI Minor version */
#define ETH_HSI_VER_MINOR                   10

/* Alias for 8.7.x.x/8.8.x.x ETH FP HSI MINOR version. In this version driver
 * is not required to set pkt_len field in eth_tx_1st_bd struct, and tunneling
 * offload is not supported.
 */
#define ETH_HSI_VER_NO_PKT_LEN_TUNN         5

#define ETH_CACHE_LINE_SIZE                 64
#define ETH_RX_CQE_GAP                      32
#define ETH_MAX_RAMROD_PER_CON              8
#define ETH_TX_BD_PAGE_SIZE_BYTES           4096
#define ETH_RX_BD_PAGE_SIZE_BYTES           4096
#define ETH_RX_CQE_PAGE_SIZE_BYTES          4096
#define ETH_RX_NUM_NEXT_PAGE_BDS            2

#define ETH_TX_MIN_BDS_PER_NON_LSO_PKT              1
#define ETH_TX_MAX_BDS_PER_NON_LSO_PACKET           18
#define ETH_TX_MAX_BDS_PER_LSO_PACKET               255
#define ETH_TX_MAX_LSO_HDR_NBD                      4
#define ETH_TX_MIN_BDS_PER_LSO_PKT                  3
#define ETH_TX_MIN_BDS_PER_TUNN_IPV6_WITH_EXT_PKT   3
#define ETH_TX_MIN_BDS_PER_IPV6_WITH_EXT_PKT        2
#define ETH_TX_MIN_BDS_PER_PKT_W_LOOPBACK_MODE      2
/* (QM_REG_TASKBYTECRDCOST_0, QM_VOQ_BYTE_CRD_TASK_COST) -
 * (VLAN-TAG + CRC + IPG + PREAMBLE)
 */
#define ETH_TX_MAX_NON_LSO_PKT_LEN                  (9700 - (4 + 4 + 12 + 8))
#define ETH_TX_MAX_LSO_HDR_BYTES                    510
/* Number of BDs to consider for LSO sliding window restriction is
 * (ETH_TX_LSO_WINDOW_BDS_NUM - hdr_nbd)
 */
#define ETH_TX_LSO_WINDOW_BDS_NUM                   (18 - 1)
/* Minimum data length (in bytes) in LSO sliding window */
#define ETH_TX_LSO_WINDOW_MIN_LEN                   9700
/* Maximum LSO packet TCP payload length (in bytes) */
#define ETH_TX_MAX_LSO_PAYLOAD_LEN                  0xFE000
/* Number of same-as-last resources in tx switching */
#define ETH_TX_NUM_SAME_AS_LAST_ENTRIES             320
/* Value for a connection for which same as last feature is disabled */
#define ETH_TX_INACTIVE_SAME_AS_LAST                0xFFFF

/* Maximum number of statistics counters */
#define ETH_NUM_STATISTIC_COUNTERS                  MAX_NUM_VPORTS
/* Maximum number of statistics counters when doubled VF zone used */
#define ETH_NUM_STATISTIC_COUNTERS_DOUBLE_VF_ZONE \
	(ETH_NUM_STATISTIC_COUNTERS - MAX_NUM_VFS / 2)
/* Maximum number of statistics counters when quad VF zone used */
#define ETH_NUM_STATISTIC_COUNTERS_QUAD_VF_ZONE \
	(ETH_NUM_STATISTIC_COUNTERS - 3 * MAX_NUM_VFS / 4)

/* Maximum number of buffers, used for RX packet placement */
#define ETH_RX_MAX_BUFF_PER_PKT             5

/* num of MAC/VLAN filters */
#define ETH_NUM_MAC_FILTERS                 512
#define ETH_NUM_VLAN_FILTERS                512

/* approx. multicast constants */
/* CRC seed for multicast bin calculation */
#define ETH_MULTICAST_BIN_FROM_MAC_SEED     0
#define ETH_MULTICAST_MAC_BINS              256
#define ETH_MULTICAST_MAC_BINS_IN_REGS      (ETH_MULTICAST_MAC_BINS / 32)

/*  ethernet vport update constants */
#define ETH_FILTER_RULES_COUNT              10
/* number of RSS indirection table entries, per Vport) */
#define ETH_RSS_IND_TABLE_ENTRIES_NUM       128
/* Length of RSS key (in regs) */
#define ETH_RSS_KEY_SIZE_REGS               10
/* number of available RSS engines in K2 */
#define ETH_RSS_ENGINE_NUM_K2               207
#define ETH_RSS_ENGINE_NUM_BB               127

/* TPA constants */
#define ETH_TPA_MAX_AGGS_NUM              64
#define ETH_TPA_CQE_START_LEN_LIST_SIZE   ETH_RX_MAX_BUFF_PER_PKT
#define ETH_TPA_CQE_CONT_LEN_LIST_SIZE    6
#define ETH_TPA_CQE_END_LEN_LIST_SIZE     4

/* Control frame check constants */
/* Number of etherType values configured by driver for control frame check */
#define ETH_CTL_FRAME_ETH_TYPE_NUM              4



/*
 * Destination port mode
 */
enum dest_port_mode {
	DEST_PORT_PHY /* Send to physical port. */,
	DEST_PORT_LOOPBACK /* Send to loopback port. */,
	DEST_PORT_PHY_LOOPBACK /* Send to physical and loopback port. */,
	DEST_PORT_DROP /* Drop the packet in PBF. */,
	MAX_DEST_PORT_MODE
};

/*
 * Ethernet address type
 */
enum eth_addr_type {
	BROADCAST_ADDRESS,
	MULTICAST_ADDRESS,
	UNICAST_ADDRESS,
	UNKNOWN_ADDRESS,
	MAX_ETH_ADDR_TYPE
};

struct eth_tx_1st_bd_flags {
	u8 bitfields;
#define ETH_TX_1ST_BD_FLAGS_START_BD_MASK         0x1
#define ETH_TX_1ST_BD_FLAGS_START_BD_SHIFT        0
#define ETH_TX_1ST_BD_FLAGS_FORCE_VLAN_MODE_MASK  0x1
#define ETH_TX_1ST_BD_FLAGS_FORCE_VLAN_MODE_SHIFT 1
#define ETH_TX_1ST_BD_FLAGS_IP_CSUM_MASK          0x1
#define ETH_TX_1ST_BD_FLAGS_IP_CSUM_SHIFT         2
#define ETH_TX_1ST_BD_FLAGS_L4_CSUM_MASK          0x1
#define ETH_TX_1ST_BD_FLAGS_L4_CSUM_SHIFT         3
#define ETH_TX_1ST_BD_FLAGS_VLAN_INSERTION_MASK   0x1
#define ETH_TX_1ST_BD_FLAGS_VLAN_INSERTION_SHIFT  4
#define ETH_TX_1ST_BD_FLAGS_LSO_MASK              0x1
#define ETH_TX_1ST_BD_FLAGS_LSO_SHIFT             5
#define ETH_TX_1ST_BD_FLAGS_TUNN_IP_CSUM_MASK     0x1
#define ETH_TX_1ST_BD_FLAGS_TUNN_IP_CSUM_SHIFT    6
#define ETH_TX_1ST_BD_FLAGS_TUNN_L4_CSUM_MASK     0x1
#define ETH_TX_1ST_BD_FLAGS_TUNN_L4_CSUM_SHIFT    7
};

/*
 * The parsing information data for the first tx bd of a given packet.
 */
struct eth_tx_data_1st_bd {
	__le16 vlan /* VLAN tag to insert to packet (if needed). */;
/* Number of BDs in packet. Should be at least 2 in non-LSO packet and at least
 * 3 in LSO (or Tunnel with IPv6+ext) packet.
 */
	u8 nbds;
	struct eth_tx_1st_bd_flags bd_flags;
	__le16 bitfields;
/* Indicates a tunneled packet. Must be set for encapsulated packet. */
#define ETH_TX_DATA_1ST_BD_TUNN_FLAG_MASK  0x1
#define ETH_TX_DATA_1ST_BD_TUNN_FLAG_SHIFT 0
#define ETH_TX_DATA_1ST_BD_RESERVED0_MASK  0x1
#define ETH_TX_DATA_1ST_BD_RESERVED0_SHIFT 1
/* Total packet length - must be filled for non-LSO packets. */
#define ETH_TX_DATA_1ST_BD_PKT_LEN_MASK    0x3FFF
#define ETH_TX_DATA_1ST_BD_PKT_LEN_SHIFT   2
};

/*
 * The parsing information data for the second tx bd of a given packet.
 */
struct eth_tx_data_2nd_bd {
	__le16 tunn_ip_size;
	__le16 bitfields1;
#define ETH_TX_DATA_2ND_BD_TUNN_INNER_L2_HDR_SIZE_W_MASK  0xF
#define ETH_TX_DATA_2ND_BD_TUNN_INNER_L2_HDR_SIZE_W_SHIFT 0
#define ETH_TX_DATA_2ND_BD_TUNN_INNER_ETH_TYPE_MASK       0x3
#define ETH_TX_DATA_2ND_BD_TUNN_INNER_ETH_TYPE_SHIFT      4
#define ETH_TX_DATA_2ND_BD_DEST_PORT_MODE_MASK            0x3
#define ETH_TX_DATA_2ND_BD_DEST_PORT_MODE_SHIFT           6
#define ETH_TX_DATA_2ND_BD_START_BD_MASK                  0x1
#define ETH_TX_DATA_2ND_BD_START_BD_SHIFT                 8
#define ETH_TX_DATA_2ND_BD_TUNN_TYPE_MASK                 0x3
#define ETH_TX_DATA_2ND_BD_TUNN_TYPE_SHIFT                9
#define ETH_TX_DATA_2ND_BD_TUNN_INNER_IPV6_MASK           0x1
#define ETH_TX_DATA_2ND_BD_TUNN_INNER_IPV6_SHIFT          11
#define ETH_TX_DATA_2ND_BD_IPV6_EXT_MASK                  0x1
#define ETH_TX_DATA_2ND_BD_IPV6_EXT_SHIFT                 12
#define ETH_TX_DATA_2ND_BD_TUNN_IPV6_EXT_MASK             0x1
#define ETH_TX_DATA_2ND_BD_TUNN_IPV6_EXT_SHIFT            13
#define ETH_TX_DATA_2ND_BD_L4_UDP_MASK                    0x1
#define ETH_TX_DATA_2ND_BD_L4_UDP_SHIFT                   14
#define ETH_TX_DATA_2ND_BD_L4_PSEUDO_CSUM_MODE_MASK       0x1
#define ETH_TX_DATA_2ND_BD_L4_PSEUDO_CSUM_MODE_SHIFT      15
	__le16 bitfields2;
#define ETH_TX_DATA_2ND_BD_L4_HDR_START_OFFSET_W_MASK     0x1FFF
#define ETH_TX_DATA_2ND_BD_L4_HDR_START_OFFSET_W_SHIFT    0
#define ETH_TX_DATA_2ND_BD_RESERVED0_MASK                 0x7
#define ETH_TX_DATA_2ND_BD_RESERVED0_SHIFT                13
};

/*
 * Firmware data for L2-EDPM packet.
 */
struct eth_edpm_fw_data {
/* Parsing information data from the 1st BD. */
	struct eth_tx_data_1st_bd data_1st_bd;
/* Parsing information data from the 2nd BD. */
	struct eth_tx_data_2nd_bd data_2nd_bd;
	__le32 reserved;
};

/*
 * FW debug.
 */
struct eth_fast_path_cqe_fw_debug {
	__le16 reserved2 /* FW reserved. */;
};


/*
 * tunneling parsing flags
 */
struct eth_tunnel_parsing_flags {
	u8 flags;
/* 0 - no tunneling, 1 - GENEVE, 2 - GRE, 3 - VXLAN
 * (use enum eth_rx_tunn_type)
 */
#define ETH_TUNNEL_PARSING_FLAGS_TYPE_MASK              0x3
#define ETH_TUNNEL_PARSING_FLAGS_TYPE_SHIFT             0
/*  If it s not an encapsulated packet then put 0x0. If it s an encapsulated
 *  packet but the tenant-id doesn t exist then put 0x0. Else put 0x1
 *
 */
#define ETH_TUNNEL_PARSING_FLAGS_TENNANT_ID_EXIST_MASK  0x1
#define ETH_TUNNEL_PARSING_FLAGS_TENNANT_ID_EXIST_SHIFT 2
/* Type of the next header above the tunneling: 0 - unknown, 1 - L2, 2 - Ipv4,
 * 3 - IPv6 (use enum tunnel_next_protocol)
 */
#define ETH_TUNNEL_PARSING_FLAGS_NEXT_PROTOCOL_MASK     0x3
#define ETH_TUNNEL_PARSING_FLAGS_NEXT_PROTOCOL_SHIFT    3
/* The result of comparing the DA-ip of the tunnel header. */
#define ETH_TUNNEL_PARSING_FLAGS_FIRSTHDRIPMATCH_MASK   0x1
#define ETH_TUNNEL_PARSING_FLAGS_FIRSTHDRIPMATCH_SHIFT  5
#define ETH_TUNNEL_PARSING_FLAGS_IPV4_FRAGMENT_MASK     0x1
#define ETH_TUNNEL_PARSING_FLAGS_IPV4_FRAGMENT_SHIFT    6
#define ETH_TUNNEL_PARSING_FLAGS_IPV4_OPTIONS_MASK      0x1
#define ETH_TUNNEL_PARSING_FLAGS_IPV4_OPTIONS_SHIFT     7
};

/*
 * PMD flow control bits
 */
struct eth_pmd_flow_flags {
	u8 flags;
#define ETH_PMD_FLOW_FLAGS_VALID_MASK     0x1 /* CQE valid bit */
#define ETH_PMD_FLOW_FLAGS_VALID_SHIFT    0
#define ETH_PMD_FLOW_FLAGS_TOGGLE_MASK    0x1 /* CQE ring toggle bit */
#define ETH_PMD_FLOW_FLAGS_TOGGLE_SHIFT   1
#define ETH_PMD_FLOW_FLAGS_RESERVED_MASK  0x3F
#define ETH_PMD_FLOW_FLAGS_RESERVED_SHIFT 2
};

/*
 * Regular ETH Rx FP CQE.
 */
struct eth_fast_path_rx_reg_cqe {
	u8 type /* CQE type */;
	u8 bitfields;
#define ETH_FAST_PATH_RX_REG_CQE_RSS_HASH_TYPE_MASK  0x7
#define ETH_FAST_PATH_RX_REG_CQE_RSS_HASH_TYPE_SHIFT 0
#define ETH_FAST_PATH_RX_REG_CQE_TC_MASK             0xF
#define ETH_FAST_PATH_RX_REG_CQE_TC_SHIFT            3
#define ETH_FAST_PATH_RX_REG_CQE_RESERVED0_MASK      0x1
#define ETH_FAST_PATH_RX_REG_CQE_RESERVED0_SHIFT     7
	__le16 pkt_len /* Total packet length (from the parser) */;
/* Parsing and error flags from the parser */
	struct parsing_and_err_flags pars_flags;
	__le16 vlan_tag /* 802.1q VLAN tag */;
	__le32 rss_hash /* RSS hash result */;
	__le16 len_on_first_bd /* Number of bytes placed on first BD */;
	u8 placement_offset /* Offset of placement from BD start */;
/* Tunnel Parsing Flags */
	struct eth_tunnel_parsing_flags tunnel_pars_flags;
	u8 bd_num /* Number of BDs, used for packet */;
	u8 reserved[9];
	struct eth_fast_path_cqe_fw_debug fw_debug /* FW reserved. */;
	u8 reserved1[3];
	struct eth_pmd_flow_flags pmd_flags /* CQE valid and toggle bits */;
};

/*
 * TPA-continue ETH Rx FP CQE.
 */
struct eth_fast_path_rx_tpa_cont_cqe {
	u8 type /* CQE type */;
	u8 tpa_agg_index /* TPA aggregation index */;
	__le16 len_list[ETH_TPA_CQE_CONT_LEN_LIST_SIZE]
	    /* List of the segment sizes */;
	u8 reserved;
	u8 reserved1 /* FW reserved. */;
	__le16 reserved2[ETH_TPA_CQE_CONT_LEN_LIST_SIZE] /* FW reserved. */;
	u8 reserved3[3];
	struct eth_pmd_flow_flags pmd_flags /* CQE valid and toggle bits */;
};

/*
 * TPA-end ETH Rx FP CQE .
 */
struct eth_fast_path_rx_tpa_end_cqe {
	u8 type /* CQE type */;
	u8 tpa_agg_index /* TPA aggregation index */;
	__le16 total_packet_len /* Total aggregated packet length */;
	u8 num_of_bds /* Total number of BDs comprising the packet */;
	u8 end_reason /* Aggregation end reason. Use enum eth_tpa_end_reason */
	  ;
	__le16 num_of_coalesced_segs /* Number of coalesced TCP segments */;
	__le32 ts_delta /* TCP timestamp delta */;
	__le16 len_list[ETH_TPA_CQE_END_LEN_LIST_SIZE]
	    /* List of the segment sizes */;
	__le16 reserved3[ETH_TPA_CQE_END_LEN_LIST_SIZE] /* FW reserved. */;
	__le16 reserved1;
	u8 reserved2 /* FW reserved. */;
	struct eth_pmd_flow_flags pmd_flags /* CQE valid and toggle bits */;
};

/*
 * TPA-start ETH Rx FP CQE.
 */
struct eth_fast_path_rx_tpa_start_cqe {
	u8 type /* CQE type */;
	u8 bitfields;
#define ETH_FAST_PATH_RX_TPA_START_CQE_RSS_HASH_TYPE_MASK  0x7
#define ETH_FAST_PATH_RX_TPA_START_CQE_RSS_HASH_TYPE_SHIFT 0
#define ETH_FAST_PATH_RX_TPA_START_CQE_TC_MASK             0xF
#define ETH_FAST_PATH_RX_TPA_START_CQE_TC_SHIFT            3
#define ETH_FAST_PATH_RX_TPA_START_CQE_RESERVED0_MASK      0x1
#define ETH_FAST_PATH_RX_TPA_START_CQE_RESERVED0_SHIFT     7
	__le16 seg_len /* Segment length (packetLen from the parser) */;
	struct parsing_and_err_flags pars_flags
	    /* Parsing and error flags from the parser */;
	__le16 vlan_tag /* 802.1q VLAN tag */;
	__le32 rss_hash /* RSS hash result */;
	__le16 len_on_first_bd /* Number of bytes placed on first BD */;
	u8 placement_offset /* Offset of placement from BD start */;
/* Tunnel Parsing Flags */
	struct eth_tunnel_parsing_flags tunnel_pars_flags;
	u8 tpa_agg_index /* TPA aggregation index */;
	u8 header_len /* Packet L2+L3+L4 header length */;
	__le16 ext_bd_len_list[ETH_TPA_CQE_START_LEN_LIST_SIZE]
	    /* Additional BDs length list. */;
	struct eth_fast_path_cqe_fw_debug fw_debug /* FW reserved. */;
	u8 reserved;
	struct eth_pmd_flow_flags pmd_flags /* CQE valid and toggle bits */;
};

/*
 * The L4 pseudo checksum mode for Ethernet
 */
enum eth_l4_pseudo_checksum_mode {
	ETH_L4_PSEUDO_CSUM_CORRECT_LENGTH
		/* Pseudo Header checksum on packet is calculated
		 * with the correct packet length field.
		*/
	   ,
	ETH_L4_PSEUDO_CSUM_ZERO_LENGTH
	    /* Pseudo Hdr checksum on packet is calc with zero len field. */
	   ,
	MAX_ETH_L4_PSEUDO_CHECKSUM_MODE
};

struct eth_rx_bd {
	struct regpair addr /* single continues buffer */;
};

/*
 * regular ETH Rx SP CQE
 */
struct eth_slow_path_rx_cqe {
	u8 type /* CQE type */;
	u8 ramrod_cmd_id;
	u8 error_flag;
	u8 reserved[25];
	__le16 echo;
	u8 reserved1;
	struct eth_pmd_flow_flags pmd_flags /* CQE valid and toggle bits */;
};

/*
 * union for all ETH Rx CQE types
 */
union eth_rx_cqe {
	struct eth_fast_path_rx_reg_cqe fast_path_regular /* Regular FP CQE */;
	struct eth_fast_path_rx_tpa_start_cqe fast_path_tpa_start
	    /* TPA-start CQE */;
	struct eth_fast_path_rx_tpa_cont_cqe fast_path_tpa_cont
	    /* TPA-continue CQE */;
	struct eth_fast_path_rx_tpa_end_cqe fast_path_tpa_end /* TPA-end CQE */
	  ;
	struct eth_slow_path_rx_cqe slow_path /* SP CQE */;
};

/*
 * ETH Rx CQE type
 */
enum eth_rx_cqe_type {
	ETH_RX_CQE_TYPE_UNUSED,
	ETH_RX_CQE_TYPE_REGULAR /* Regular FP ETH Rx CQE */,
	ETH_RX_CQE_TYPE_SLOW_PATH /* Slow path ETH Rx CQE */,
	ETH_RX_CQE_TYPE_TPA_START /* TPA start ETH Rx CQE */,
	ETH_RX_CQE_TYPE_TPA_CONT /* TPA Continue ETH Rx CQE */,
	ETH_RX_CQE_TYPE_TPA_END /* TPA end ETH Rx CQE */,
	MAX_ETH_RX_CQE_TYPE
};

/*
 * Wrapper for PD RX CQE - used in order to cover full cache line when writing
 * CQE
 */
struct eth_rx_pmd_cqe {
	union eth_rx_cqe cqe /* CQE data itself */;
	u8 reserved[ETH_RX_CQE_GAP];
};

/*
 * Eth RX Tunnel Type
 */
enum eth_rx_tunn_type {
	ETH_RX_NO_TUNN /* No Tunnel. */,
	ETH_RX_TUNN_GENEVE /* GENEVE Tunnel. */,
	ETH_RX_TUNN_GRE /* GRE Tunnel. */,
	ETH_RX_TUNN_VXLAN /* VXLAN Tunnel. */,
	MAX_ETH_RX_TUNN_TYPE
};

/*
 * Aggregation end reason.
 */
enum eth_tpa_end_reason {
	ETH_AGG_END_UNUSED,
	ETH_AGG_END_SP_UPDATE /* SP configuration update */,
	ETH_AGG_END_MAX_LEN
	    /* Maximum aggregation length or maximum buffer number used. */,
	ETH_AGG_END_LAST_SEG
	    /* TCP PSH flag or TCP payload length below continue threshold. */,
	ETH_AGG_END_TIMEOUT /* Timeout expiration. */,
	ETH_AGG_END_NOT_CONSISTENT,
	ETH_AGG_END_OUT_OF_ORDER,
	ETH_AGG_END_NON_TPA_SEG,
	MAX_ETH_TPA_END_REASON
};



/*
 * The first tx bd of a given packet
 */
struct eth_tx_1st_bd {
	struct regpair addr /* Single continuous buffer */;
	__le16 nbytes /* Number of bytes in this BD. */;
	struct eth_tx_data_1st_bd data /* Parsing information data. */;
};

/*
 * The second tx bd of a given packet
 */
struct eth_tx_2nd_bd {
	struct regpair addr /* Single continuous buffer */;
	__le16 nbytes /* Number of bytes in this BD. */;
	struct eth_tx_data_2nd_bd data /* Parsing information data. */;
};

/*
 * The parsing information data for the third tx bd of a given packet.
 */
struct eth_tx_data_3rd_bd {
	__le16 lso_mss /* For LSO packet - the MSS in bytes. */;
	__le16 bitfields;
#define ETH_TX_DATA_3RD_BD_TCP_HDR_LEN_DW_MASK  0xF
#define ETH_TX_DATA_3RD_BD_TCP_HDR_LEN_DW_SHIFT 0
#define ETH_TX_DATA_3RD_BD_HDR_NBD_MASK         0xF
#define ETH_TX_DATA_3RD_BD_HDR_NBD_SHIFT        4
#define ETH_TX_DATA_3RD_BD_START_BD_MASK        0x1
#define ETH_TX_DATA_3RD_BD_START_BD_SHIFT       8
#define ETH_TX_DATA_3RD_BD_RESERVED0_MASK       0x7F
#define ETH_TX_DATA_3RD_BD_RESERVED0_SHIFT      9
	u8 tunn_l4_hdr_start_offset_w;
	u8 tunn_hdr_size_w;
};

/*
 * The third tx bd of a given packet
 */
struct eth_tx_3rd_bd {
	struct regpair addr /* Single continuous buffer */;
	__le16 nbytes /* Number of bytes in this BD. */;
	struct eth_tx_data_3rd_bd data /* Parsing information data. */;
};

/*
 * Complementary information for the regular tx bd of a given packet.
 */
struct eth_tx_data_bd {
	__le16 reserved0;
	__le16 bitfields;
#define ETH_TX_DATA_BD_RESERVED1_MASK  0xFF
#define ETH_TX_DATA_BD_RESERVED1_SHIFT 0
#define ETH_TX_DATA_BD_START_BD_MASK   0x1
#define ETH_TX_DATA_BD_START_BD_SHIFT  8
#define ETH_TX_DATA_BD_RESERVED2_MASK  0x7F
#define ETH_TX_DATA_BD_RESERVED2_SHIFT 9
	__le16 reserved3;
};

/*
 * The common regular TX BD ring element
 */
struct eth_tx_bd {
	struct regpair addr /* Single continuous buffer */;
	__le16 nbytes /* Number of bytes in this BD. */;
	struct eth_tx_data_bd data /* Complementary information. */;
};

union eth_tx_bd_types {
	struct eth_tx_1st_bd first_bd /* The first tx bd of a given packet */;
	struct eth_tx_2nd_bd second_bd /* The second tx bd of a given packet */
	  ;
	struct eth_tx_3rd_bd third_bd /* The third tx bd of a given packet */;
	struct eth_tx_bd reg_bd /* The common non-special bd */;
};

/*
 * Eth Tx Tunnel Type
 */
enum eth_tx_tunn_type {
	ETH_TX_TUNN_GENEVE /* GENEVE Tunnel. */,
	ETH_TX_TUNN_TTAG /* T-Tag Tunnel. */,
	ETH_TX_TUNN_GRE /* GRE Tunnel. */,
	ETH_TX_TUNN_VXLAN /* VXLAN Tunnel. */,
	MAX_ETH_TX_TUNN_TYPE
};


/*
 * Ystorm Queue Zone
 */
struct xstorm_eth_queue_zone {
	struct coalescing_timeset int_coalescing_timeset
	    /* Tx interrupt coalescing TimeSet */;
	u8 reserved[7];
};

/*
 * ETH doorbell data
 */
struct eth_db_data {
	u8 params;
#define ETH_DB_DATA_DEST_MASK         0x3
#define ETH_DB_DATA_DEST_SHIFT        0
#define ETH_DB_DATA_AGG_CMD_MASK      0x3
#define ETH_DB_DATA_AGG_CMD_SHIFT     2
#define ETH_DB_DATA_BYPASS_EN_MASK    0x1
#define ETH_DB_DATA_BYPASS_EN_SHIFT   4
#define ETH_DB_DATA_RESERVED_MASK     0x1
#define ETH_DB_DATA_RESERVED_SHIFT    5
#define ETH_DB_DATA_AGG_VAL_SEL_MASK  0x3
#define ETH_DB_DATA_AGG_VAL_SEL_SHIFT 6
	u8 agg_flags;
	__le16 bd_prod;
};

#endif /* __ETH_COMMON__ */
