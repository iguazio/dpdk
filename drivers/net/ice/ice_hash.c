/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation
 */

#include <sys/queue.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev_driver.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_eth_ctrl.h>
#include <rte_tailq.h>
#include <rte_flow_driver.h>

#include "ice_logs.h"
#include "base/ice_type.h"
#include "base/ice_flow.h"
#include "ice_ethdev.h"
#include "ice_generic_flow.h"

#define ICE_GTPU_EH_DWNLINK	0
#define ICE_GTPU_EH_UPLINK	1

struct rss_type_match_hdr {
	uint32_t hdr_mask;
	uint64_t eth_rss_hint;
};

struct ice_hash_match_type {
	uint64_t hash_type;
	uint64_t hash_flds;
};

struct rss_meta {
	uint32_t pkt_hdr;
	uint64_t hash_flds;
	uint8_t hash_function;
};

struct ice_hash_flow_cfg {
	bool simple_xor;
	struct ice_rss_cfg rss_cfg;
};

static int
ice_hash_init(struct ice_adapter *ad);

static int
ice_hash_create(struct ice_adapter *ad,
		struct rte_flow *flow,
		void *meta,
		struct rte_flow_error *error);

static int
ice_hash_destroy(struct ice_adapter *ad,
		struct rte_flow *flow,
		struct rte_flow_error *error);

static void
ice_hash_uninit(struct ice_adapter *ad);

static void
ice_hash_free(struct rte_flow *flow);

static int
ice_hash_parse_pattern_action(struct ice_adapter *ad,
			struct ice_pattern_match_item *array,
			uint32_t array_len,
			const struct rte_flow_item pattern[],
			const struct rte_flow_action actions[],
			void **meta,
			struct rte_flow_error *error);

/* The first member is protocol header, the second member is ETH_RSS_*. */
struct rss_type_match_hdr hint_empty = {
	ICE_FLOW_SEG_HDR_NONE,	0};
struct rss_type_match_hdr hint_eth_ipv4 = {
	ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV_OTHER,
	ETH_RSS_ETH | ETH_RSS_IPV4};
struct rss_type_match_hdr hint_eth_ipv4_udp = {
	ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_UDP,
	ETH_RSS_ETH | ETH_RSS_NONFRAG_IPV4_UDP};
struct rss_type_match_hdr hint_eth_ipv4_tcp = {
	ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_TCP,
	ETH_RSS_ETH | ETH_RSS_NONFRAG_IPV4_TCP};
struct rss_type_match_hdr hint_eth_ipv4_sctp = {
	ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_SCTP,
	ETH_RSS_ETH | ETH_RSS_NONFRAG_IPV4_SCTP};
struct rss_type_match_hdr hint_eth_ipv4_gtpu_ipv4 = {
	ICE_FLOW_SEG_HDR_GTPU_IP | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER,
	ETH_RSS_GTPU | ETH_RSS_IPV4};
struct rss_type_match_hdr hint_eth_ipv4_gtpu_eh_ipv4 = {
	ICE_FLOW_SEG_HDR_GTPU_EH | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER,
	ETH_RSS_GTPU | ETH_RSS_IPV4};
struct rss_type_match_hdr hint_eth_ipv4_gtpu_eh_ipv4_udp = {
	ICE_FLOW_SEG_HDR_GTPU_EH | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_UDP,
	ETH_RSS_GTPU | ETH_RSS_NONFRAG_IPV4_UDP};
struct rss_type_match_hdr hint_eth_ipv4_gtpu_eh_ipv4_tcp = {
	ICE_FLOW_SEG_HDR_GTPU_EH | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_TCP,
	ETH_RSS_GTPU | ETH_RSS_NONFRAG_IPV4_TCP};
struct rss_type_match_hdr hint_eth_pppoes_ipv4 = {
	ICE_FLOW_SEG_HDR_PPPOE | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER,
	ETH_RSS_ETH | ETH_RSS_PPPOE | ETH_RSS_IPV4};
struct rss_type_match_hdr hint_eth_pppoes_ipv4_udp = {
	ICE_FLOW_SEG_HDR_PPPOE | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_UDP,
	ETH_RSS_ETH | ETH_RSS_PPPOE | ETH_RSS_NONFRAG_IPV4_UDP};
struct rss_type_match_hdr hint_eth_pppoes_ipv4_tcp = {
	ICE_FLOW_SEG_HDR_PPPOE | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_TCP,
	ETH_RSS_ETH | ETH_RSS_PPPOE | ETH_RSS_NONFRAG_IPV4_TCP};
struct rss_type_match_hdr hint_eth_pppoes_ipv4_sctp = {
	ICE_FLOW_SEG_HDR_PPPOE | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_SCTP,
	ETH_RSS_ETH | ETH_RSS_PPPOE | ETH_RSS_NONFRAG_IPV4_SCTP};
struct rss_type_match_hdr hint_eth_ipv4_esp = {
	ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_ESP,
	ETH_RSS_ETH | ETH_RSS_IPV4 | ETH_RSS_ESP};
struct rss_type_match_hdr hint_eth_ipv4_udp_esp = {
	ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_NAT_T_ESP,
	ETH_RSS_ETH | ETH_RSS_NONFRAG_IPV4_UDP | ETH_RSS_ESP};
struct rss_type_match_hdr hint_eth_ipv4_ah = {
	ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_AH,
	ETH_RSS_ETH | ETH_RSS_IPV4 | ETH_RSS_AH};
struct rss_type_match_hdr hint_eth_ipv4_l2tpv3 = {
	ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_L2TPV3,
	ETH_RSS_ETH | ETH_RSS_IPV4 | ETH_RSS_L2TPV3};
struct rss_type_match_hdr hint_eth_ipv4_pfcp = {
	ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_PFCP_SESSION,
	ETH_RSS_ETH | ETH_RSS_NONFRAG_IPV4_UDP | ETH_RSS_PFCP};
struct rss_type_match_hdr hint_eth_vlan_ipv4 = {
	ICE_FLOW_SEG_HDR_VLAN | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER,
	ETH_RSS_ETH | ETH_RSS_IPV4 | ETH_RSS_C_VLAN};
struct rss_type_match_hdr hint_eth_vlan_ipv4_udp = {
	ICE_FLOW_SEG_HDR_VLAN | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_UDP,
	ETH_RSS_ETH | ETH_RSS_C_VLAN |
	ETH_RSS_NONFRAG_IPV4_UDP};
struct rss_type_match_hdr hint_eth_vlan_ipv4_tcp = {
	ICE_FLOW_SEG_HDR_VLAN | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_TCP,
	ETH_RSS_ETH | ETH_RSS_C_VLAN |
	ETH_RSS_NONFRAG_IPV4_TCP};
struct rss_type_match_hdr hint_eth_vlan_ipv4_sctp = {
	ICE_FLOW_SEG_HDR_VLAN | ICE_FLOW_SEG_HDR_IPV4 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_SCTP,
	ETH_RSS_ETH | ETH_RSS_C_VLAN |
	ETH_RSS_NONFRAG_IPV4_SCTP};
struct rss_type_match_hdr hint_eth_ipv6 = {
	ICE_FLOW_SEG_HDR_IPV6 | ICE_FLOW_SEG_HDR_IPV_OTHER,
	ETH_RSS_ETH | ETH_RSS_IPV6};
struct rss_type_match_hdr hint_eth_ipv6_udp = {
	ICE_FLOW_SEG_HDR_IPV6 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_UDP,
	ETH_RSS_ETH | ETH_RSS_NONFRAG_IPV6_UDP};
struct rss_type_match_hdr hint_eth_ipv6_tcp = {
	ICE_FLOW_SEG_HDR_IPV6 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_TCP,
	ETH_RSS_ETH | ETH_RSS_NONFRAG_IPV6_TCP};
struct rss_type_match_hdr hint_eth_ipv6_sctp = {
	ICE_FLOW_SEG_HDR_IPV6 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_SCTP,
	ETH_RSS_ETH | ETH_RSS_NONFRAG_IPV6_SCTP};
struct rss_type_match_hdr hint_eth_ipv6_esp = {
	ICE_FLOW_SEG_HDR_IPV6 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_ESP,
	ETH_RSS_ETH | ETH_RSS_IPV6 | ETH_RSS_ESP};
struct rss_type_match_hdr hint_eth_ipv6_udp_esp = {
	ICE_FLOW_SEG_HDR_IPV6 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_NAT_T_ESP,
	ETH_RSS_ETH | ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_ESP};
struct rss_type_match_hdr hint_eth_ipv6_ah = {
	ICE_FLOW_SEG_HDR_IPV6 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_AH,
	ETH_RSS_ETH | ETH_RSS_IPV6 | ETH_RSS_AH};
struct rss_type_match_hdr hint_eth_ipv6_l2tpv3 = {
	ICE_FLOW_SEG_HDR_IPV6 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_L2TPV3,
	ETH_RSS_ETH | ETH_RSS_IPV6 | ETH_RSS_L2TPV3};
struct rss_type_match_hdr hint_eth_ipv6_pfcp = {
	ICE_FLOW_SEG_HDR_IPV6 | ICE_FLOW_SEG_HDR_IPV_OTHER |
	ICE_FLOW_SEG_HDR_PFCP_SESSION,
	ETH_RSS_ETH | ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_PFCP};
struct rss_type_match_hdr hint_eth_vlan_ipv6 = {
	ICE_FLOW_SEG_HDR_VLAN | ICE_FLOW_SEG_HDR_IPV6 |
	ICE_FLOW_SEG_HDR_IPV_OTHER,
	ETH_RSS_ETH | ETH_RSS_IPV6 | ETH_RSS_C_VLAN};
struct rss_type_match_hdr hint_eth_vlan_ipv6_udp = {
	ICE_FLOW_SEG_HDR_VLAN | ICE_FLOW_SEG_HDR_IPV6 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_UDP,
	ETH_RSS_ETH | ETH_RSS_C_VLAN |
	ETH_RSS_NONFRAG_IPV6_UDP};
struct rss_type_match_hdr hint_eth_vlan_ipv6_tcp = {
	ICE_FLOW_SEG_HDR_VLAN | ICE_FLOW_SEG_HDR_IPV6 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_TCP,
	ETH_RSS_ETH | ETH_RSS_C_VLAN |
	ETH_RSS_NONFRAG_IPV6_TCP};
struct rss_type_match_hdr hint_eth_vlan_ipv6_sctp = {
	ICE_FLOW_SEG_HDR_VLAN | ICE_FLOW_SEG_HDR_IPV6 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_SCTP,
	ETH_RSS_ETH | ETH_RSS_C_VLAN |
	ETH_RSS_NONFRAG_IPV6_SCTP};
struct rss_type_match_hdr hint_eth_pppoes_ipv6 = {
	ICE_FLOW_SEG_HDR_PPPOE | ICE_FLOW_SEG_HDR_IPV6 |
	ICE_FLOW_SEG_HDR_IPV_OTHER,
	ETH_RSS_ETH | ETH_RSS_PPPOE | ETH_RSS_IPV6};
struct rss_type_match_hdr hint_eth_pppoes_ipv6_udp = {
	ICE_FLOW_SEG_HDR_PPPOE | ICE_FLOW_SEG_HDR_IPV6 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_UDP,
	ETH_RSS_ETH | ETH_RSS_PPPOE | ETH_RSS_NONFRAG_IPV6_UDP};
struct rss_type_match_hdr hint_eth_pppoes_ipv6_tcp = {
	ICE_FLOW_SEG_HDR_PPPOE | ICE_FLOW_SEG_HDR_IPV6 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_TCP,
	ETH_RSS_ETH | ETH_RSS_PPPOE | ETH_RSS_NONFRAG_IPV6_TCP};
struct rss_type_match_hdr hint_eth_pppoes_ipv6_sctp = {
	ICE_FLOW_SEG_HDR_PPPOE | ICE_FLOW_SEG_HDR_IPV6 |
	ICE_FLOW_SEG_HDR_IPV_OTHER | ICE_FLOW_SEG_HDR_SCTP,
	ETH_RSS_ETH | ETH_RSS_PPPOE | ETH_RSS_NONFRAG_IPV6_SCTP};
struct rss_type_match_hdr hint_eth_pppoes = {
	ICE_FLOW_SEG_HDR_PPPOE,
	ETH_RSS_ETH | ETH_RSS_PPPOE};

/* Supported pattern for os default package. */
static struct ice_pattern_match_item ice_hash_pattern_list_os[] = {
	{pattern_eth_ipv4,	ICE_INSET_NONE,	&hint_eth_ipv4},
	{pattern_eth_ipv4_udp,	ICE_INSET_NONE,	&hint_eth_ipv4_udp},
	{pattern_eth_ipv4_tcp,	ICE_INSET_NONE,	&hint_eth_ipv4_tcp},
	{pattern_eth_ipv4_sctp,	ICE_INSET_NONE,	&hint_eth_ipv4_sctp},
	{pattern_eth_ipv6,	ICE_INSET_NONE,	&hint_eth_ipv6},
	{pattern_eth_ipv6_udp,	ICE_INSET_NONE,	&hint_eth_ipv6_udp},
	{pattern_eth_ipv6_tcp,	ICE_INSET_NONE,	&hint_eth_ipv6_tcp},
	{pattern_eth_ipv6_sctp,	ICE_INSET_NONE,	&hint_eth_ipv6_sctp},
	{pattern_empty,		ICE_INSET_NONE,	&hint_empty},
};

/* Supported pattern for comms package. */
static struct ice_pattern_match_item ice_hash_pattern_list_comms[] = {
	{pattern_empty,			    ICE_INSET_NONE,
		&hint_empty},
	{pattern_eth_ipv4,		    ICE_INSET_NONE,
		&hint_eth_ipv4},
	{pattern_eth_ipv4_udp,		    ICE_INSET_NONE,
		&hint_eth_ipv4_udp},
	{pattern_eth_ipv4_tcp,		    ICE_INSET_NONE,
		&hint_eth_ipv4_tcp},
	{pattern_eth_ipv4_sctp,		    ICE_INSET_NONE,
		&hint_eth_ipv4_sctp},
	{pattern_eth_ipv4_gtpu_ipv4,	    ICE_INSET_NONE,
		&hint_eth_ipv4_gtpu_ipv4},
	{pattern_eth_ipv4_gtpu_eh_ipv4,	    ICE_INSET_NONE,
		&hint_eth_ipv4_gtpu_eh_ipv4},
	{pattern_eth_ipv4_gtpu_eh_ipv4_udp, ICE_INSET_NONE,
		&hint_eth_ipv4_gtpu_eh_ipv4_udp},
	{pattern_eth_ipv4_gtpu_eh_ipv4_tcp, ICE_INSET_NONE,
		&hint_eth_ipv4_gtpu_eh_ipv4_tcp},
	{pattern_eth_pppoes_ipv4,	    ICE_INSET_NONE,
		&hint_eth_pppoes_ipv4},
	{pattern_eth_pppoes_ipv4_udp,	    ICE_INSET_NONE,
		&hint_eth_pppoes_ipv4_udp},
	{pattern_eth_pppoes_ipv4_tcp,	    ICE_INSET_NONE,
		&hint_eth_pppoes_ipv4_tcp},
	{pattern_eth_pppoes_ipv4_sctp,	    ICE_INSET_NONE,
		&hint_eth_pppoes_ipv4_sctp},
	{pattern_eth_ipv4_esp,		    ICE_INSET_NONE,
		&hint_eth_ipv4_esp},
	{pattern_eth_ipv4_udp_esp,	    ICE_INSET_NONE,
		&hint_eth_ipv4_udp_esp},
	{pattern_eth_ipv4_ah,		    ICE_INSET_NONE,
		&hint_eth_ipv4_ah},
	{pattern_eth_ipv4_l2tp,		    ICE_INSET_NONE,
		&hint_eth_ipv4_l2tpv3},
	{pattern_eth_ipv4_pfcp,		    ICE_INSET_NONE,
		&hint_eth_ipv4_pfcp},
	{pattern_eth_vlan_ipv4,		    ICE_INSET_NONE,
		&hint_eth_vlan_ipv4},
	{pattern_eth_vlan_ipv4_udp,	    ICE_INSET_NONE,
		&hint_eth_vlan_ipv4_udp},
	{pattern_eth_vlan_ipv4_tcp,	    ICE_INSET_NONE,
		&hint_eth_vlan_ipv4_tcp},
	{pattern_eth_vlan_ipv4_sctp,	    ICE_INSET_NONE,
		&hint_eth_vlan_ipv4_sctp},
	{pattern_eth_ipv6,		    ICE_INSET_NONE,
		&hint_eth_ipv6},
	{pattern_eth_ipv6_udp,		    ICE_INSET_NONE,
		&hint_eth_ipv6_udp},
	{pattern_eth_ipv6_tcp,		    ICE_INSET_NONE,
		&hint_eth_ipv6_tcp},
	{pattern_eth_ipv6_sctp,		    ICE_INSET_NONE,
		&hint_eth_ipv6_sctp},
	{pattern_eth_ipv6_esp,		    ICE_INSET_NONE,
		&hint_eth_ipv6_esp},
	{pattern_eth_ipv6_udp_esp,	    ICE_INSET_NONE,
		&hint_eth_ipv6_udp_esp},
	{pattern_eth_ipv6_ah,		    ICE_INSET_NONE,
		&hint_eth_ipv6_ah},
	{pattern_eth_ipv6_l2tp,		    ICE_INSET_NONE,
		&hint_eth_ipv6_l2tpv3},
	{pattern_eth_ipv6_pfcp,		    ICE_INSET_NONE,
		&hint_eth_ipv6_pfcp},
	{pattern_eth_vlan_ipv6,		    ICE_INSET_NONE,
		&hint_eth_vlan_ipv6},
	{pattern_eth_vlan_ipv6_udp,	    ICE_INSET_NONE,
		&hint_eth_vlan_ipv6_udp},
	{pattern_eth_vlan_ipv6_tcp,	    ICE_INSET_NONE,
		&hint_eth_vlan_ipv6_tcp},
	{pattern_eth_vlan_ipv6_sctp,	    ICE_INSET_NONE,
		&hint_eth_vlan_ipv6_sctp},
	{pattern_eth_pppoes_ipv6,	    ICE_INSET_NONE,
		&hint_eth_pppoes_ipv6},
	{pattern_eth_pppoes_ipv6_udp,	    ICE_INSET_NONE,
		&hint_eth_pppoes_ipv6_udp},
	{pattern_eth_pppoes_ipv6_tcp,	    ICE_INSET_NONE,
		&hint_eth_pppoes_ipv6_tcp},
	{pattern_eth_pppoes_ipv6_sctp,	    ICE_INSET_NONE,
		&hint_eth_pppoes_ipv6_sctp},
	{pattern_eth_pppoes,		    ICE_INSET_NONE,
		&hint_eth_pppoes},
};

/**
 * The first member is input set combination,
 * the second member is hash fields.
 */
struct ice_hash_match_type ice_hash_type_list[] = {
	{ETH_RSS_L2_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_ETH_SA)},
	{ETH_RSS_L2_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_ETH_DA)},
	{ETH_RSS_ETH | ETH_RSS_L2_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_ETH_SA)},
	{ETH_RSS_ETH | ETH_RSS_L2_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_ETH_DA)},
	{ETH_RSS_ETH,
		BIT_ULL(ICE_FLOW_FIELD_IDX_ETH_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_ETH_DA)},
	{ETH_RSS_PPPOE,
		ICE_FLOW_HASH_PPPOE_SESS_ID},
	{ETH_RSS_ETH | ETH_RSS_PPPOE | ETH_RSS_L2_SRC_ONLY,
		ICE_FLOW_HASH_PPPOE_SESS_ID |
		BIT_ULL(ICE_FLOW_FIELD_IDX_ETH_SA)},
	{ETH_RSS_C_VLAN,
		BIT_ULL(ICE_FLOW_FIELD_IDX_C_VLAN)},
	{ETH_RSS_S_VLAN,
		BIT_ULL(ICE_FLOW_FIELD_IDX_S_VLAN)},
	{ETH_RSS_ESP,
		BIT_ULL(ICE_FLOW_FIELD_IDX_ESP_SPI)},
	{ETH_RSS_AH,
		BIT_ULL(ICE_FLOW_FIELD_IDX_AH_SPI)},
	{ETH_RSS_L2TPV3,
		BIT_ULL(ICE_FLOW_FIELD_IDX_L2TPV3_SESS_ID)},
	{ETH_RSS_PFCP,
		BIT_ULL(ICE_FLOW_FIELD_IDX_PFCP_SEID)},
	/* IPV4 */
	{ETH_RSS_IPV4 | ETH_RSS_L3_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_SA)},
	{ETH_RSS_IPV4 | ETH_RSS_L3_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_DA)},
	{ETH_RSS_IPV4, ICE_FLOW_HASH_IPV4},
	{ETH_RSS_NONFRAG_IPV4_UDP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV4_UDP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV4_UDP | ETH_RSS_L3_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_SA)},
	{ETH_RSS_NONFRAG_IPV4_UDP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV4_UDP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV4_UDP | ETH_RSS_L3_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_DA)},
	{ETH_RSS_NONFRAG_IPV4_UDP | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_SRC_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT)},
	{ETH_RSS_NONFRAG_IPV4_UDP | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_DST_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT)},
	{ETH_RSS_NONFRAG_IPV4_UDP,
		ICE_HASH_UDP_IPV4 |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT)},
	{ETH_RSS_NONFRAG_IPV4_TCP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV4_TCP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV4_TCP | ETH_RSS_L3_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_SA)},
	{ETH_RSS_NONFRAG_IPV4_TCP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV4_TCP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV4_TCP | ETH_RSS_L3_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_DA)},
	{ETH_RSS_NONFRAG_IPV4_TCP | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_SRC_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT)},
	{ETH_RSS_NONFRAG_IPV4_TCP | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_DST_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT)},
	{ETH_RSS_NONFRAG_IPV4_TCP,
		ICE_HASH_TCP_IPV4 |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT)},
	{ETH_RSS_NONFRAG_IPV4_SCTP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV4_SCTP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV4_SCTP | ETH_RSS_L3_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_SA)},
	{ETH_RSS_NONFRAG_IPV4_SCTP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV4_SCTP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV4_SCTP | ETH_RSS_L3_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_DA)},
	{ETH_RSS_NONFRAG_IPV4_SCTP | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_SRC_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT)},
	{ETH_RSS_NONFRAG_IPV4_SCTP | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_DST_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT)},
	{ETH_RSS_NONFRAG_IPV4_SCTP,
		ICE_HASH_SCTP_IPV4 |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV4_PROT)},
	/* IPV6 */
	{ETH_RSS_IPV6 | ETH_RSS_L3_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_SA)},
	{ETH_RSS_IPV6 | ETH_RSS_L3_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_DA)},
	{ETH_RSS_IPV6, ICE_FLOW_HASH_IPV6},
	{ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_L3_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_SA)},
	{ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_L3_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_DA)},
	{ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_SRC_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT)},
	{ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_UDP_DST_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT)},
	{ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_PFCP,
		BIT_ULL(ICE_FLOW_FIELD_IDX_PFCP_SEID)},
	{ETH_RSS_NONFRAG_IPV6_UDP,
		ICE_HASH_UDP_IPV6 |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT)},
	{ETH_RSS_NONFRAG_IPV6_TCP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV6_TCP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV6_TCP | ETH_RSS_L3_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_SA)},
	{ETH_RSS_NONFRAG_IPV6_TCP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV6_TCP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV6_TCP | ETH_RSS_L3_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_DA)},
	{ETH_RSS_NONFRAG_IPV6_TCP | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_SRC_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT)},
	{ETH_RSS_NONFRAG_IPV6_TCP | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_TCP_DST_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT)},
	{ETH_RSS_NONFRAG_IPV6_TCP,
		ICE_HASH_TCP_IPV6 |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT)},
	{ETH_RSS_NONFRAG_IPV6_SCTP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV6_SCTP | ETH_RSS_L3_SRC_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_SA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV6_SCTP | ETH_RSS_L3_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_SA)},
	{ETH_RSS_NONFRAG_IPV6_SCTP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_SRC_PORT)},
	{ETH_RSS_NONFRAG_IPV6_SCTP | ETH_RSS_L3_DST_ONLY | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_DA) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_DST_PORT)},
	{ETH_RSS_NONFRAG_IPV6_SCTP | ETH_RSS_L3_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_DA)},
	{ETH_RSS_NONFRAG_IPV6_SCTP | ETH_RSS_L4_SRC_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_SRC_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT)},
	{ETH_RSS_NONFRAG_IPV6_SCTP | ETH_RSS_L4_DST_ONLY,
		BIT_ULL(ICE_FLOW_FIELD_IDX_SCTP_DST_PORT) |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT)},
	{ETH_RSS_NONFRAG_IPV6_SCTP,
		ICE_HASH_SCTP_IPV6 |
		BIT_ULL(ICE_FLOW_FIELD_IDX_IPV6_PROT)},
};

static struct ice_flow_engine ice_hash_engine = {
	.init = ice_hash_init,
	.create = ice_hash_create,
	.destroy = ice_hash_destroy,
	.uninit = ice_hash_uninit,
	.free = ice_hash_free,
	.type = ICE_FLOW_ENGINE_HASH,
};

/* Register parser for os package. */
static struct ice_flow_parser ice_hash_parser_os = {
	.engine = &ice_hash_engine,
	.array = ice_hash_pattern_list_os,
	.array_len = RTE_DIM(ice_hash_pattern_list_os),
	.parse_pattern_action = ice_hash_parse_pattern_action,
	.stage = ICE_FLOW_STAGE_RSS,
};

/* Register parser for comms package. */
static struct ice_flow_parser ice_hash_parser_comms = {
	.engine = &ice_hash_engine,
	.array = ice_hash_pattern_list_comms,
	.array_len = RTE_DIM(ice_hash_pattern_list_comms),
	.parse_pattern_action = ice_hash_parse_pattern_action,
	.stage = ICE_FLOW_STAGE_RSS,
};

RTE_INIT(ice_hash_engine_init)
{
	struct ice_flow_engine *engine = &ice_hash_engine;
	ice_register_flow_engine(engine);
}

static int
ice_hash_init(struct ice_adapter *ad)
{
	struct ice_flow_parser *parser = NULL;

	if (ad->hw.dcf_enabled)
		return 0;

	if (ad->active_pkg_type == ICE_PKG_TYPE_OS_DEFAULT)
		parser = &ice_hash_parser_os;
	else if (ad->active_pkg_type == ICE_PKG_TYPE_COMMS)
		parser = &ice_hash_parser_comms;
	else
		return -EINVAL;

	return ice_register_parser(parser, ad);
}

static int
ice_hash_parse_pattern(struct ice_pattern_match_item *pattern_match_item,
		       const struct rte_flow_item pattern[], void **meta,
		       struct rte_flow_error *error)
{
	uint32_t hdr_mask = ((struct rss_type_match_hdr *)
		(pattern_match_item->meta))->hdr_mask;
	const struct rte_flow_item *item = pattern;
	const struct rte_flow_item_gtp_psc *psc;
	uint32_t hdrs = 0;

	for (item = pattern; item->type != RTE_FLOW_ITEM_TYPE_END; item++) {
		if (item->last) {
			rte_flow_error_set(error, EINVAL,
					RTE_FLOW_ERROR_TYPE_ITEM, item,
					"Not support range");
			return -rte_errno;
		}

		switch (item->type) {
		case RTE_FLOW_ITEM_TYPE_GTPU:
			hdrs |= ICE_FLOW_SEG_HDR_GTPU_IP;
			break;
		case RTE_FLOW_ITEM_TYPE_GTP_PSC:
			psc = item->spec;
			hdr_mask &= ~ICE_FLOW_SEG_HDR_GTPU_EH;
			hdrs &= ~ICE_FLOW_SEG_HDR_GTPU_IP;
			if (!psc)
				hdrs |= ICE_FLOW_SEG_HDR_GTPU_EH;
			else if (psc->pdu_type == ICE_GTPU_EH_UPLINK)
				hdrs |= ICE_FLOW_SEG_HDR_GTPU_UP;
			else if (psc->pdu_type == ICE_GTPU_EH_DWNLINK)
				hdrs |= ICE_FLOW_SEG_HDR_GTPU_DWN;
			break;
		default:
			break;
		}
	}

	/* Save protocol header to rss_meta. */
	((struct rss_meta *)*meta)->pkt_hdr |= hdr_mask | hdrs;

	return 0;
}

static int
ice_hash_parse_action(struct ice_pattern_match_item *pattern_match_item,
		const struct rte_flow_action actions[],
		void **meta,
		struct rte_flow_error *error)
{
	struct rss_type_match_hdr *m = (struct rss_type_match_hdr *)
				(pattern_match_item->meta);
	struct rss_meta *hash_meta = (struct rss_meta *)*meta;
	uint32_t type_list_len = RTE_DIM(ice_hash_type_list);
	enum rte_flow_action_type action_type;
	const struct rte_flow_action_rss *rss;
	const struct rte_flow_action *action;
	uint64_t combine_type;
	uint64_t rss_type;
	uint16_t i;

	/* Supported action is RSS. */
	for (action = actions; action->type !=
		RTE_FLOW_ACTION_TYPE_END; action++) {
		action_type = action->type;
		switch (action_type) {
		case RTE_FLOW_ACTION_TYPE_RSS:
			rss = action->conf;
			rss_type = rss->types;

			/* Check hash function and save it to rss_meta. */
			if (pattern_match_item->pattern_list !=
			    pattern_empty && rss->func ==
			    RTE_ETH_HASH_FUNCTION_SIMPLE_XOR) {
				return rte_flow_error_set(error, ENOTSUP,
					RTE_FLOW_ERROR_TYPE_ACTION, action,
					"Not supported flow");
			} else if (rss->func ==
				   RTE_ETH_HASH_FUNCTION_SIMPLE_XOR){
				((struct rss_meta *)*meta)->hash_function =
				RTE_ETH_HASH_FUNCTION_SIMPLE_XOR;
				return 0;
			} else if (rss->func ==
				   RTE_ETH_HASH_FUNCTION_SYMMETRIC_TOEPLITZ) {
				((struct rss_meta *)*meta)->hash_function =
				RTE_ETH_HASH_FUNCTION_SYMMETRIC_TOEPLITZ;
			}

			if (rss->level)
				return rte_flow_error_set(error, ENOTSUP,
					RTE_FLOW_ERROR_TYPE_ACTION, action,
					"a nonzero RSS encapsulation level is not supported");

			if (rss->key_len)
				return rte_flow_error_set(error, ENOTSUP,
					RTE_FLOW_ERROR_TYPE_ACTION, action,
					"a nonzero RSS key_len is not supported");

			if (rss->queue)
				return rte_flow_error_set(error, ENOTSUP,
					RTE_FLOW_ERROR_TYPE_ACTION, action,
					"a non-NULL RSS queue is not supported");

			/**
			 * Check simultaneous use of SRC_ONLY and DST_ONLY
			 * of the same level.
			 */
			rss_type = rte_eth_rss_hf_refine(rss_type);

			combine_type = ETH_RSS_L2_SRC_ONLY |
					ETH_RSS_L2_DST_ONLY |
					ETH_RSS_L3_SRC_ONLY |
					ETH_RSS_L3_DST_ONLY |
					ETH_RSS_L4_SRC_ONLY |
					ETH_RSS_L4_DST_ONLY;

			/* Check if rss types match pattern. */
			if (rss_type & ~combine_type & ~m->eth_rss_hint) {
				return rte_flow_error_set(error,
				ENOTSUP, RTE_FLOW_ERROR_TYPE_ACTION,
				action, "Not supported RSS types");
			}

			/* Find matched hash fields according to hash type. */
			for (i = 0; i < type_list_len; i++) {
				struct ice_hash_match_type *ht_map =
					&ice_hash_type_list[i];

				if (rss_type == ht_map->hash_type) {
					hash_meta->hash_flds =
							ht_map->hash_flds;
					break;
				}
			}

			/* update hash field for nat-t esp. */
			if (rss_type == ETH_RSS_ESP &&
			    (m->eth_rss_hint & ETH_RSS_NONFRAG_IPV4_UDP)) {
				hash_meta->hash_flds &=
				~(BIT_ULL(ICE_FLOW_FIELD_IDX_ESP_SPI));
				hash_meta->hash_flds |=
				BIT_ULL(ICE_FLOW_FIELD_IDX_NAT_T_ESP_SPI);
			}

			/* update hash field for gtpu-ip and gtpu-eh. */
			if (rss_type != ETH_RSS_GTPU)
				break;
			else if (hash_meta->pkt_hdr & ICE_FLOW_SEG_HDR_GTPU_IP)
				hash_meta->hash_flds |=
				BIT_ULL(ICE_FLOW_FIELD_IDX_GTPU_IP_TEID);
			else if (hash_meta->pkt_hdr & ICE_FLOW_SEG_HDR_GTPU_EH)
				hash_meta->hash_flds |=
				BIT_ULL(ICE_FLOW_FIELD_IDX_GTPU_EH_TEID);
			else if (hash_meta->pkt_hdr & ICE_FLOW_SEG_HDR_GTPU_DWN)
				hash_meta->hash_flds |=
				BIT_ULL(ICE_FLOW_FIELD_IDX_GTPU_DWN_TEID);
			else if (hash_meta->pkt_hdr & ICE_FLOW_SEG_HDR_GTPU_UP)
				hash_meta->hash_flds |=
				BIT_ULL(ICE_FLOW_FIELD_IDX_GTPU_UP_TEID);

			break;

		case RTE_FLOW_ACTION_TYPE_END:
			break;

		default:
			rte_flow_error_set(error, EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION, action,
					"Invalid action.");
			return -rte_errno;
		}
	}

	return 0;
}

static int
ice_hash_parse_pattern_action(__rte_unused struct ice_adapter *ad,
			struct ice_pattern_match_item *array,
			uint32_t array_len,
			const struct rte_flow_item pattern[],
			const struct rte_flow_action actions[],
			void **meta,
			struct rte_flow_error *error)
{
	int ret = 0;
	struct ice_pattern_match_item *pattern_match_item;
	struct rss_meta *rss_meta_ptr;

	rss_meta_ptr = rte_zmalloc(NULL, sizeof(*rss_meta_ptr), 0);
	if (!rss_meta_ptr) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_HANDLE, NULL,
				"No memory for rss_meta_ptr");
		return -ENOMEM;
	}

	/* Check rss supported pattern and find matched pattern. */
	pattern_match_item = ice_search_pattern_match_item(pattern,
					array, array_len, error);
	if (!pattern_match_item) {
		ret = -rte_errno;
		goto error;
	}

	ret = ice_hash_parse_pattern(pattern_match_item, pattern,
				     (void **)&rss_meta_ptr, error);
	if (ret)
		goto error;

	/* Check rss action. */
	ret = ice_hash_parse_action(pattern_match_item, actions,
				    (void **)&rss_meta_ptr, error);

error:
	if (!ret && meta)
		*meta = rss_meta_ptr;
	else
		rte_free(rss_meta_ptr);
	rte_free(pattern_match_item);

	return ret;
}

static int
ice_hash_create(struct ice_adapter *ad,
		struct rte_flow *flow,
		void *meta,
		struct rte_flow_error *error)
{
	struct ice_pf *pf = &ad->pf;
	struct ice_hw *hw = ICE_PF_TO_HW(pf);
	struct ice_vsi *vsi = pf->main_vsi;
	int ret;
	uint32_t reg;
	struct ice_hash_flow_cfg *filter_ptr;

	uint32_t headermask = ((struct rss_meta *)meta)->pkt_hdr;
	uint64_t hash_field = ((struct rss_meta *)meta)->hash_flds;
	uint8_t hash_function = ((struct rss_meta *)meta)->hash_function;

	filter_ptr = rte_zmalloc("ice_rss_filter",
				sizeof(struct ice_hash_flow_cfg), 0);
	if (!filter_ptr) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_HANDLE, NULL,
				"No memory for filter_ptr");
		return -ENOMEM;
	}

	if (hash_function == RTE_ETH_HASH_FUNCTION_SIMPLE_XOR) {
		/* Enable registers for simple_xor hash function. */
		reg = ICE_READ_REG(hw, VSIQF_HASH_CTL(vsi->vsi_id));
		reg = (reg & (~VSIQF_HASH_CTL_HASH_SCHEME_M)) |
			(2 << VSIQF_HASH_CTL_HASH_SCHEME_S);
		ICE_WRITE_REG(hw, VSIQF_HASH_CTL(vsi->vsi_id), reg);

		filter_ptr->simple_xor = 1;

		goto out;
	} else {
		filter_ptr->rss_cfg.packet_hdr = headermask;
		filter_ptr->rss_cfg.hashed_flds = hash_field;
		filter_ptr->rss_cfg.symm =
			(hash_function ==
				RTE_ETH_HASH_FUNCTION_SYMMETRIC_TOEPLITZ);

		ret = ice_add_rss_cfg(hw, vsi->idx,
				filter_ptr->rss_cfg.hashed_flds,
				filter_ptr->rss_cfg.packet_hdr,
				filter_ptr->rss_cfg.symm);
		if (ret) {
			rte_flow_error_set(error, EINVAL,
					RTE_FLOW_ERROR_TYPE_HANDLE, NULL,
					"rss flow create fail");
			goto error;
		}
	}

out:
	flow->rule = filter_ptr;
	rte_free(meta);
	return 0;

error:
	rte_free(filter_ptr);
	rte_free(meta);
	return -rte_errno;
}

static int
ice_hash_destroy(struct ice_adapter *ad,
		struct rte_flow *flow,
		struct rte_flow_error *error)
{
	struct ice_pf *pf = ICE_DEV_PRIVATE_TO_PF(ad);
	struct ice_hw *hw = ICE_PF_TO_HW(pf);
	struct ice_vsi *vsi = pf->main_vsi;
	int ret;
	uint32_t reg;
	struct ice_hash_flow_cfg *filter_ptr;

	filter_ptr = (struct ice_hash_flow_cfg *)flow->rule;

	if (filter_ptr->simple_xor == 1) {
		/* Return to symmetric_toeplitz state. */
		reg = ICE_READ_REG(hw, VSIQF_HASH_CTL(vsi->vsi_id));
		reg = (reg & (~VSIQF_HASH_CTL_HASH_SCHEME_M)) |
			(1 << VSIQF_HASH_CTL_HASH_SCHEME_S);
		ICE_WRITE_REG(hw, VSIQF_HASH_CTL(vsi->vsi_id), reg);
	} else {
		ret = ice_rem_rss_cfg(hw, vsi->idx,
				filter_ptr->rss_cfg.hashed_flds,
				filter_ptr->rss_cfg.packet_hdr);
		/* Fixme: Ignore the error if a rule does not exist.
		 * Currently a rule for inputset change or symm turn on/off
		 * will overwrite an exist rule, while application still
		 * have 2 rte_flow handles.
		 **/
		if (ret && ret != ICE_ERR_DOES_NOT_EXIST) {
			rte_flow_error_set(error, EINVAL,
					RTE_FLOW_ERROR_TYPE_HANDLE, NULL,
					"rss flow destroy fail");
			goto error;
		}
	}

	rte_free(filter_ptr);
	return 0;

error:
	rte_free(filter_ptr);
	return -rte_errno;
}

static void
ice_hash_uninit(struct ice_adapter *ad)
{
	if (ad->hw.dcf_enabled)
		return;

	if (ad->active_pkg_type == ICE_PKG_TYPE_OS_DEFAULT)
		ice_unregister_parser(&ice_hash_parser_os, ad);
	else if (ad->active_pkg_type == ICE_PKG_TYPE_COMMS)
		ice_unregister_parser(&ice_hash_parser_comms, ad);
}

static void
ice_hash_free(struct rte_flow *flow)
{
	rte_free(flow->rule);
}
