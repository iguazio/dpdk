/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2001-2019
 */

#include "ice_common.h"
#include "ice_flow.h"

/* Size of known protocol header fields */
#define ICE_FLOW_FLD_SZ_ETH_TYPE	2
#define ICE_FLOW_FLD_SZ_VLAN		2
#define ICE_FLOW_FLD_SZ_IPV4_ADDR	4
#define ICE_FLOW_FLD_SZ_IPV6_ADDR	16
#define ICE_FLOW_FLD_SZ_IP_DSCP		1
#define ICE_FLOW_FLD_SZ_IP_TTL		1
#define ICE_FLOW_FLD_SZ_IP_PROT		1
#define ICE_FLOW_FLD_SZ_PORT		2
#define ICE_FLOW_FLD_SZ_TCP_FLAGS	1
#define ICE_FLOW_FLD_SZ_ICMP_TYPE	1
#define ICE_FLOW_FLD_SZ_ICMP_CODE	1
#define ICE_FLOW_FLD_SZ_ARP_OPER	2
#define ICE_FLOW_FLD_SZ_GRE_KEYID	4

/* Protocol header fields are extracted at the word boundaries as word-sized
 * values. Specify the displacement value of some non-word-aligned fields needed
 * to compute the offset of words containing the fields in the corresponding
 * protocol headers. Displacement values are expressed in number of bits.
 */
#define ICE_FLOW_FLD_IPV6_TTL_DSCP_DISP	(-4)
#define ICE_FLOW_FLD_IPV6_TTL_PROT_DISP	((-2) * 8)
#define ICE_FLOW_FLD_IPV6_TTL_TTL_DISP	((-1) * 8)

/* Describe properties of a protocol header field */
struct ice_flow_field_info {
	enum ice_flow_seg_hdr hdr;
	s16 off;	/* Offset from start of a protocol header, in bits */
	u16 size;	/* Size of fields in bits */
};

/* Table containing properties of supported protocol header fields */
static const
struct ice_flow_field_info ice_flds_info[ICE_FLOW_FIELD_IDX_MAX] = {
	/* Ether */
	/* ICE_FLOW_FIELD_IDX_ETH_DA */
	{ ICE_FLOW_SEG_HDR_ETH, 0, ETH_ALEN * 8 },
	/* ICE_FLOW_FIELD_IDX_ETH_SA */
	{ ICE_FLOW_SEG_HDR_ETH, ETH_ALEN * 8, ETH_ALEN * 8 },
	/* ICE_FLOW_FIELD_IDX_S_VLAN */
	{ ICE_FLOW_SEG_HDR_VLAN, 12 * 8, ICE_FLOW_FLD_SZ_VLAN * 8 },
	/* ICE_FLOW_FIELD_IDX_C_VLAN */
	{ ICE_FLOW_SEG_HDR_VLAN, 14 * 8, ICE_FLOW_FLD_SZ_VLAN * 8 },
	/* ICE_FLOW_FIELD_IDX_ETH_TYPE */
	{ ICE_FLOW_SEG_HDR_ETH, 12 * 8, ICE_FLOW_FLD_SZ_ETH_TYPE * 8 },
	/* IPv4 */
	/* ICE_FLOW_FIELD_IDX_IP_DSCP */
	{ ICE_FLOW_SEG_HDR_IPV4, 1 * 8, 1 * 8 },
	/* ICE_FLOW_FIELD_IDX_IP_TTL */
	{ ICE_FLOW_SEG_HDR_NONE, 8 * 8, 1 * 8 },
	/* ICE_FLOW_FIELD_IDX_IP_PROT */
	{ ICE_FLOW_SEG_HDR_NONE, 9 * 8, ICE_FLOW_FLD_SZ_IP_PROT * 8 },
	/* ICE_FLOW_FIELD_IDX_IPV4_SA */
	{ ICE_FLOW_SEG_HDR_IPV4, 12 * 8, ICE_FLOW_FLD_SZ_IPV4_ADDR * 8 },
	/* ICE_FLOW_FIELD_IDX_IPV4_DA */
	{ ICE_FLOW_SEG_HDR_IPV4, 16 * 8, ICE_FLOW_FLD_SZ_IPV4_ADDR * 8 },
	/* IPv6 */
	/* ICE_FLOW_FIELD_IDX_IPV6_SA */
	{ ICE_FLOW_SEG_HDR_IPV6, 8 * 8, ICE_FLOW_FLD_SZ_IPV6_ADDR * 8 },
	/* ICE_FLOW_FIELD_IDX_IPV6_DA */
	{ ICE_FLOW_SEG_HDR_IPV6, 24 * 8, ICE_FLOW_FLD_SZ_IPV6_ADDR * 8 },
	/* Transport */
	/* ICE_FLOW_FIELD_IDX_TCP_SRC_PORT */
	{ ICE_FLOW_SEG_HDR_TCP, 0 * 8, ICE_FLOW_FLD_SZ_PORT * 8 },
	/* ICE_FLOW_FIELD_IDX_TCP_DST_PORT */
	{ ICE_FLOW_SEG_HDR_TCP, 2 * 8, ICE_FLOW_FLD_SZ_PORT * 8 },
	/* ICE_FLOW_FIELD_IDX_UDP_SRC_PORT */
	{ ICE_FLOW_SEG_HDR_UDP, 0 * 8, ICE_FLOW_FLD_SZ_PORT * 8 },
	/* ICE_FLOW_FIELD_IDX_UDP_DST_PORT */
	{ ICE_FLOW_SEG_HDR_UDP, 2 * 8, ICE_FLOW_FLD_SZ_PORT * 8 },
	/* ICE_FLOW_FIELD_IDX_SCTP_SRC_PORT */
	{ ICE_FLOW_SEG_HDR_SCTP, 0 * 8, ICE_FLOW_FLD_SZ_PORT * 8 },
	/* ICE_FLOW_FIELD_IDX_SCTP_DST_PORT */
	{ ICE_FLOW_SEG_HDR_SCTP, 2 * 8, ICE_FLOW_FLD_SZ_PORT * 8 },
	/* ICE_FLOW_FIELD_IDX_TCP_FLAGS */
	{ ICE_FLOW_SEG_HDR_TCP, 13 * 8, ICE_FLOW_FLD_SZ_TCP_FLAGS * 8 },
	/* ARP */
	/* ICE_FLOW_FIELD_IDX_ARP_SIP */
	{ ICE_FLOW_SEG_HDR_ARP, 14 * 8, ICE_FLOW_FLD_SZ_IPV4_ADDR * 8 },
	/* ICE_FLOW_FIELD_IDX_ARP_DIP */
	{ ICE_FLOW_SEG_HDR_ARP, 24 * 8, ICE_FLOW_FLD_SZ_IPV4_ADDR * 8 },
	/* ICE_FLOW_FIELD_IDX_ARP_SHA */
	{ ICE_FLOW_SEG_HDR_ARP, 8 * 8, ETH_ALEN * 8 },
	/* ICE_FLOW_FIELD_IDX_ARP_DHA */
	{ ICE_FLOW_SEG_HDR_ARP, 18 * 8, ETH_ALEN * 8 },
	/* ICE_FLOW_FIELD_IDX_ARP_OP */
	{ ICE_FLOW_SEG_HDR_ARP, 6 * 8, ICE_FLOW_FLD_SZ_ARP_OPER * 8 },
	/* ICMP */
	/* ICE_FLOW_FIELD_IDX_ICMP_TYPE */
	{ ICE_FLOW_SEG_HDR_ICMP, 0 * 8, ICE_FLOW_FLD_SZ_ICMP_TYPE * 8 },
	/* ICE_FLOW_FIELD_IDX_ICMP_CODE */
	{ ICE_FLOW_SEG_HDR_ICMP, 1 * 8, ICE_FLOW_FLD_SZ_ICMP_CODE * 8 },
	/* GRE */
	/* ICE_FLOW_FIELD_IDX_GRE_KEYID */
	{ ICE_FLOW_SEG_HDR_GRE, 12 * 8, ICE_FLOW_FLD_SZ_GRE_KEYID * 8 },
};

/* Bitmaps indicating relevant packet types for a particular protocol header
 *
 * Packet types for packets with an Outer/First/Single MAC header
 */
static const u32 ice_ptypes_mac_ofos[] = {
	0xFDC00CC6, 0xBFBF7F7E, 0xF7EFDFDF, 0xFEFDFDFB,
	0x03BF7F7E, 0x00000000, 0x00000000, 0x00000000,
	0x000B0F0F, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Innermost/Last MAC VLAN header */
static const u32 ice_ptypes_macvlan_il[] = {
	0x00000000, 0xBC000000, 0x000001DF, 0xF0000000,
	0x0000077E, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Outer/First/Single IPv4 header */
static const u32 ice_ptypes_ipv4_ofos[] = {
	0xFDC00000, 0xBFBF7F7E, 0x00EFDFDF, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x0003000F, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Innermost/Last IPv4 header */
static const u32 ice_ptypes_ipv4_il[] = {
	0xE0000000, 0xB807700E, 0x8001DC03, 0xE01DC03B,
	0x0007700E, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Outer/First/Single IPv6 header */
static const u32 ice_ptypes_ipv6_ofos[] = {
	0x00000000, 0x00000000, 0xF7000000, 0xFEFDFDFB,
	0x03BF7F7E, 0x00000000, 0x00000000, 0x00000000,
	0x00080F00, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Innermost/Last IPv6 header */
static const u32 ice_ptypes_ipv6_il[] = {
	0x00000000, 0x03B80770, 0x00EE01DC, 0x0EE00000,
	0x03B80770, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Outermost/First ARP header */
static const u32 ice_ptypes_arp_of[] = {
	0x00000800, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Outermost/First UDP header */
static const u32 ice_ptypes_udp_of[] = {
	0x81000000, 0x00000000, 0x04000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Innermost/Last UDP header */
static const u32 ice_ptypes_udp_il[] = {
	0x80000000, 0x20204040, 0x00081010, 0x80810102,
	0x00204040, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Innermost/Last TCP header */
static const u32 ice_ptypes_tcp_il[] = {
	0x04000000, 0x80810102, 0x10204040, 0x42040408,
	0x00810002, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Innermost/Last SCTP header */
static const u32 ice_ptypes_sctp_il[] = {
	0x08000000, 0x01020204, 0x20408081, 0x04080810,
	0x01020204, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Outermost/First ICMP header */
static const u32 ice_ptypes_icmp_of[] = {
	0x10000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Innermost/Last ICMP header */
static const u32 ice_ptypes_icmp_il[] = {
	0x00000000, 0x02040408, 0x40810102, 0x08101020,
	0x02040408, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Outermost/First GRE header */
static const u32 ice_ptypes_gre_of[] = {
	0x00000000, 0xBFBF7800, 0x00EFDFDF, 0xFEFDE000,
	0x03BF7F7E, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Packet types for packets with an Innermost/Last MAC header */
static const u32 ice_ptypes_mac_il[] = {
	0x00000000, 0x00000000, 0x00EFDE00, 0x00000000,
	0x03BF7800, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/* Manage parameters and info. used during the creation of a flow profile */
struct ice_flow_prof_params {
	enum ice_block blk;
	struct ice_flow_prof *prof;

	u16 entry_length; /* # of bytes formatted entry will require */
	u8 es_cnt;
	/* For ACL, the es[0] will have the data of ICE_RX_MDID_PKT_FLAGS_15_0
	 * This will give us the direction flags.
	 */
	struct ice_fv_word es[ICE_MAX_FV_WORDS];

	ice_declare_bitmap(ptypes, ICE_FLOW_PTYPE_MAX);
};

/**
 * ice_is_pow2 - check if integer value is a power of 2
 * @val: unsigned integer to be validated
 */
static bool ice_is_pow2(u64 val)
{
	return (val && !(val & (val - 1)));
}

#define ICE_FLOW_SEG_HDRS_L2_MASK	\
	(ICE_FLOW_SEG_HDR_ETH | ICE_FLOW_SEG_HDR_VLAN)
#define ICE_FLOW_SEG_HDRS_L3_MASK	\
	(ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV6 | ICE_FLOW_SEG_HDR_ARP)
#define ICE_FLOW_SEG_HDRS_L4_MASK	\
	(ICE_FLOW_SEG_HDR_ICMP | ICE_FLOW_SEG_HDR_TCP | ICE_FLOW_SEG_HDR_UDP | \
	 ICE_FLOW_SEG_HDR_SCTP)

/**
 * ice_flow_val_hdrs - validates packet segments for valid protocol headers
 * @segs: array of one or more packet segments that describe the flow
 * @segs_cnt: number of packet segments provided
 */
static enum ice_status
ice_flow_val_hdrs(struct ice_flow_seg_info *segs, u8 segs_cnt)
{
	const u32 masks = (ICE_FLOW_SEG_HDRS_L2_MASK |
			   ICE_FLOW_SEG_HDRS_L3_MASK |
			   ICE_FLOW_SEG_HDRS_L4_MASK);
	u8 i;

	for (i = 0; i < segs_cnt; i++) {
		/* No header specified */
		if (!(segs[i].hdrs & masks) || (segs[i].hdrs & ~masks))
			return ICE_ERR_PARAM;

		/* Multiple L3 headers */
		if (segs[i].hdrs & ICE_FLOW_SEG_HDRS_L3_MASK &&
		    !ice_is_pow2(segs[i].hdrs & ICE_FLOW_SEG_HDRS_L3_MASK))
			return ICE_ERR_PARAM;

		/* Multiple L4 headers */
		if (segs[i].hdrs & ICE_FLOW_SEG_HDRS_L4_MASK &&
		    !ice_is_pow2(segs[i].hdrs & ICE_FLOW_SEG_HDRS_L4_MASK))
			return ICE_ERR_PARAM;
	}

	return ICE_SUCCESS;
}

/* Sizes of fixed known protocol headers without header options */
#define ICE_FLOW_PROT_HDR_SZ_MAC	14
#define ICE_FLOW_PROT_HDR_SZ_MAC_VLAN	(ICE_FLOW_PROT_HDR_SZ_MAC + 2)
#define ICE_FLOW_PROT_HDR_SZ_IPV4	20
#define ICE_FLOW_PROT_HDR_SZ_IPV6	40
#define ICE_FLOW_PROT_HDR_SZ_ARP	28
#define ICE_FLOW_PROT_HDR_SZ_ICMP	8
#define ICE_FLOW_PROT_HDR_SZ_TCP	20
#define ICE_FLOW_PROT_HDR_SZ_UDP	8
#define ICE_FLOW_PROT_HDR_SZ_SCTP	12

/**
 * ice_flow_calc_seg_sz - calculates size of a packet segment based on headers
 * @params: information about the flow to be processed
 * @seg: index of packet segment whose header size is to be determined
 */
static u16 ice_flow_calc_seg_sz(struct ice_flow_prof_params *params, u8 seg)
{
	u16 sz;

	/* L2 headers */
	sz = (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDR_VLAN) ?
		ICE_FLOW_PROT_HDR_SZ_MAC_VLAN : ICE_FLOW_PROT_HDR_SZ_MAC;

	/* L3 headers */
	if (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDR_IPV4)
		sz += ICE_FLOW_PROT_HDR_SZ_IPV4;
	else if (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDR_IPV6)
		sz += ICE_FLOW_PROT_HDR_SZ_IPV6;
	else if (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDR_ARP)
		sz += ICE_FLOW_PROT_HDR_SZ_ARP;
	else if (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDRS_L4_MASK)
		/* A L3 header is required if L4 is specified */
		return 0;

	/* L4 headers */
	if (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDR_ICMP)
		sz += ICE_FLOW_PROT_HDR_SZ_ICMP;
	else if (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDR_TCP)
		sz += ICE_FLOW_PROT_HDR_SZ_TCP;
	else if (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDR_UDP)
		sz += ICE_FLOW_PROT_HDR_SZ_UDP;
	else if (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDR_SCTP)
		sz += ICE_FLOW_PROT_HDR_SZ_SCTP;

	return sz;
}

/**
 * ice_flow_proc_seg_hdrs - process protocol headers present in pkt segments
 * @params: information about the flow to be processed
 *
 * This function identifies the packet types associated with the protocol
 * headers being present in packet segments of the specified flow profile.
 */
static enum ice_status
ice_flow_proc_seg_hdrs(struct ice_flow_prof_params *params)
{
	struct ice_flow_prof *prof;
	u8 i;

	ice_memset(params->ptypes, 0xff, sizeof(params->ptypes),
		   ICE_NONDMA_MEM);

	prof = params->prof;

	for (i = 0; i < params->prof->segs_cnt; i++) {
		const ice_bitmap_t *src;
		u32 hdrs;

		if (i > 0 && (i + 1) < prof->segs_cnt)
			continue;

		hdrs = prof->segs[i].hdrs;

		if (hdrs & ICE_FLOW_SEG_HDR_ETH) {
			src = !i ? (const ice_bitmap_t *)ice_ptypes_mac_ofos :
				(const ice_bitmap_t *)ice_ptypes_mac_il;
			ice_and_bitmap(params->ptypes, params->ptypes, src,
				       ICE_FLOW_PTYPE_MAX);
			hdrs &= ~ICE_FLOW_SEG_HDR_ETH;
		}

		if (i && hdrs & ICE_FLOW_SEG_HDR_VLAN) {
			src = (const ice_bitmap_t *)ice_ptypes_macvlan_il;
			ice_and_bitmap(params->ptypes, params->ptypes, src,
				       ICE_FLOW_PTYPE_MAX);
			hdrs &= ~ICE_FLOW_SEG_HDR_VLAN;
		}

		if (!i && hdrs & ICE_FLOW_SEG_HDR_ARP) {
			ice_and_bitmap(params->ptypes, params->ptypes,
				       (const ice_bitmap_t *)ice_ptypes_arp_of,
				       ICE_FLOW_PTYPE_MAX);
			hdrs &= ~ICE_FLOW_SEG_HDR_ARP;
		}

		if (hdrs & ICE_FLOW_SEG_HDR_IPV4) {
			src = !i ? (const ice_bitmap_t *)ice_ptypes_ipv4_ofos :
				(const ice_bitmap_t *)ice_ptypes_ipv4_il;
			ice_and_bitmap(params->ptypes, params->ptypes, src,
				       ICE_FLOW_PTYPE_MAX);
			hdrs &= ~ICE_FLOW_SEG_HDR_IPV4;
		} else if (hdrs & ICE_FLOW_SEG_HDR_IPV6) {
			src = !i ? (const ice_bitmap_t *)ice_ptypes_ipv6_ofos :
				(const ice_bitmap_t *)ice_ptypes_ipv6_il;
			ice_and_bitmap(params->ptypes, params->ptypes, src,
				       ICE_FLOW_PTYPE_MAX);
			hdrs &= ~ICE_FLOW_SEG_HDR_IPV6;
		}

		if (hdrs & ICE_FLOW_SEG_HDR_ICMP) {
			src = !i ? (const ice_bitmap_t *)ice_ptypes_icmp_of :
				(const ice_bitmap_t *)ice_ptypes_icmp_il;
			ice_and_bitmap(params->ptypes, params->ptypes, src,
				       ICE_FLOW_PTYPE_MAX);
			hdrs &= ~ICE_FLOW_SEG_HDR_ICMP;
		} else if (hdrs & ICE_FLOW_SEG_HDR_UDP) {
			src = !i ? (const ice_bitmap_t *)ice_ptypes_udp_of :
				(const ice_bitmap_t *)ice_ptypes_udp_il;
			ice_and_bitmap(params->ptypes, params->ptypes, src,
				       ICE_FLOW_PTYPE_MAX);
			hdrs &= ~ICE_FLOW_SEG_HDR_UDP;
		} else if (hdrs & ICE_FLOW_SEG_HDR_TCP) {
			ice_and_bitmap(params->ptypes, params->ptypes,
				       (const ice_bitmap_t *)ice_ptypes_tcp_il,
				       ICE_FLOW_PTYPE_MAX);
			hdrs &= ~ICE_FLOW_SEG_HDR_TCP;
		} else if (hdrs & ICE_FLOW_SEG_HDR_SCTP) {
			src = (const ice_bitmap_t *)ice_ptypes_sctp_il;
			ice_and_bitmap(params->ptypes, params->ptypes, src,
				       ICE_FLOW_PTYPE_MAX);
			hdrs &= ~ICE_FLOW_SEG_HDR_SCTP;
		} else if (hdrs & ICE_FLOW_SEG_HDR_GRE) {
			if (!i) {
				src = (const ice_bitmap_t *)ice_ptypes_gre_of;
				ice_and_bitmap(params->ptypes, params->ptypes,
					       src, ICE_FLOW_PTYPE_MAX);
			}
			hdrs &= ~ICE_FLOW_SEG_HDR_GRE;
		}
	}

	return ICE_SUCCESS;
}

/**
 * ice_flow_xtract_fld - Create an extraction sequence entry for the given field
 * @hw: pointer to the HW struct
 * @params: information about the flow to be processed
 * @seg: packet segment index of the field to be extracted
 * @fld: ID of field to be extracted
 *
 * This function determines the protocol ID, offset, and size of the given
 * field. It then allocates one or more extraction sequence entries for the
 * given field, and fill the entries with protocol ID and offset information.
 */
static enum ice_status
ice_flow_xtract_fld(struct ice_hw *hw, struct ice_flow_prof_params *params,
		    u8 seg, enum ice_flow_field fld)
{
	enum ice_flow_field sib = ICE_FLOW_FIELD_IDX_MAX;
	enum ice_prot_id prot_id = ICE_PROT_ID_INVAL;
	u8 fv_words = hw->blk[params->blk].es.fvw;
	struct ice_flow_fld_info *flds;
	u16 cnt, ese_bits, i;
	s16 adj = 0;
	u8 off;

	flds = params->prof->segs[seg].fields;

	switch (fld) {
	case ICE_FLOW_FIELD_IDX_ETH_DA:
	case ICE_FLOW_FIELD_IDX_ETH_SA:
	case ICE_FLOW_FIELD_IDX_S_VLAN:
	case ICE_FLOW_FIELD_IDX_C_VLAN:
		prot_id = seg == 0 ? ICE_PROT_MAC_OF_OR_S : ICE_PROT_MAC_IL;
		break;
	case ICE_FLOW_FIELD_IDX_ETH_TYPE:
		prot_id = seg == 0 ? ICE_PROT_ETYPE_OL : ICE_PROT_ETYPE_IL;
		break;
	case ICE_FLOW_FIELD_IDX_IP_DSCP:
		if (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDR_IPV6)
			adj = ICE_FLOW_FLD_IPV6_TTL_DSCP_DISP;
		/* Fall through */
	case ICE_FLOW_FIELD_IDX_IP_TTL:
	case ICE_FLOW_FIELD_IDX_IP_PROT:
		/* Some fields are located at different offsets in IPv4 and
		 * IPv6
		 */
		if (params->prof->segs[seg].hdrs & ICE_FLOW_SEG_HDR_IPV4) {
			prot_id = seg == 0 ? ICE_PROT_IPV4_OF_OR_S :
				ICE_PROT_IPV4_IL;
			/* TTL and PROT share the same extraction seq. entry.
			 * Each is considered a sibling to the other in term
			 * sharing the same extraction sequence entry.
			 */
			if (fld == ICE_FLOW_FIELD_IDX_IP_TTL)
				sib = ICE_FLOW_FIELD_IDX_IP_PROT;
			else if (fld == ICE_FLOW_FIELD_IDX_IP_PROT)
				sib = ICE_FLOW_FIELD_IDX_IP_TTL;
		} else if (params->prof->segs[seg].hdrs &
			   ICE_FLOW_SEG_HDR_IPV6) {
			prot_id = seg == 0 ? ICE_PROT_IPV6_OF_OR_S :
				ICE_PROT_IPV6_IL;
			if (fld == ICE_FLOW_FIELD_IDX_IP_TTL)
				adj = ICE_FLOW_FLD_IPV6_TTL_TTL_DISP;
			else if (fld == ICE_FLOW_FIELD_IDX_IP_PROT)
				adj = ICE_FLOW_FLD_IPV6_TTL_PROT_DISP;
		}
		break;
	case ICE_FLOW_FIELD_IDX_IPV4_SA:
	case ICE_FLOW_FIELD_IDX_IPV4_DA:
		prot_id = seg == 0 ? ICE_PROT_IPV4_OF_OR_S : ICE_PROT_IPV4_IL;
		break;
	case ICE_FLOW_FIELD_IDX_IPV6_SA:
	case ICE_FLOW_FIELD_IDX_IPV6_DA:
		prot_id = seg == 0 ? ICE_PROT_IPV6_OF_OR_S : ICE_PROT_IPV6_IL;
		break;
	case ICE_FLOW_FIELD_IDX_TCP_SRC_PORT:
	case ICE_FLOW_FIELD_IDX_TCP_DST_PORT:
	case ICE_FLOW_FIELD_IDX_TCP_FLAGS:
		prot_id = ICE_PROT_TCP_IL;
		break;
	case ICE_FLOW_FIELD_IDX_UDP_SRC_PORT:
	case ICE_FLOW_FIELD_IDX_UDP_DST_PORT:
		prot_id = seg == 0 ? ICE_PROT_UDP_IL_OR_S : ICE_PROT_UDP_OF;
		break;
	case ICE_FLOW_FIELD_IDX_SCTP_SRC_PORT:
	case ICE_FLOW_FIELD_IDX_SCTP_DST_PORT:
		prot_id = ICE_PROT_SCTP_IL;
		break;
	case ICE_FLOW_FIELD_IDX_ARP_SIP:
	case ICE_FLOW_FIELD_IDX_ARP_DIP:
	case ICE_FLOW_FIELD_IDX_ARP_SHA:
	case ICE_FLOW_FIELD_IDX_ARP_DHA:
	case ICE_FLOW_FIELD_IDX_ARP_OP:
		prot_id = ICE_PROT_ARP_OF;
		break;
	case ICE_FLOW_FIELD_IDX_ICMP_TYPE:
	case ICE_FLOW_FIELD_IDX_ICMP_CODE:
		/* ICMP type and code share the same extraction seq. entry */
		prot_id = (params->prof->segs[seg].hdrs &
			   ICE_FLOW_SEG_HDR_IPV4) ?
			ICE_PROT_ICMP_IL : ICE_PROT_ICMPV6_IL;
		sib = fld == ICE_FLOW_FIELD_IDX_ICMP_TYPE ?
			ICE_FLOW_FIELD_IDX_ICMP_CODE :
			ICE_FLOW_FIELD_IDX_ICMP_TYPE;
		break;
	case ICE_FLOW_FIELD_IDX_GRE_KEYID:
		prot_id = ICE_PROT_GRE_OF;
		break;
	default:
		return ICE_ERR_NOT_IMPL;
	}

	/* Each extraction sequence entry is a word in size, and extracts a
	 * word-aligned offset from a protocol header.
	 */
	ese_bits = ICE_FLOW_FV_EXTRACT_SZ * 8;

	flds[fld].xtrct.prot_id = prot_id;
	flds[fld].xtrct.off = (ice_flds_info[fld].off / ese_bits) *
		ICE_FLOW_FV_EXTRACT_SZ;
	flds[fld].xtrct.disp = (u8)((ice_flds_info[fld].off + adj) % ese_bits);
	flds[fld].xtrct.idx = params->es_cnt;

	/* Adjust the next field-entry index after accommodating the number of
	 * entries this field consumes
	 */
	cnt = DIVIDE_AND_ROUND_UP(flds[fld].xtrct.disp +
				  ice_flds_info[fld].size, ese_bits);

	/* Fill in the extraction sequence entries needed for this field */
	off = flds[fld].xtrct.off;
	for (i = 0; i < cnt; i++) {
		/* Only consume an extraction sequence entry if there is no
		 * sibling field associated with this field or the sibling entry
		 * already extracts the word shared with this field.
		 */
		if (sib == ICE_FLOW_FIELD_IDX_MAX ||
		    flds[sib].xtrct.prot_id == ICE_PROT_ID_INVAL ||
		    flds[sib].xtrct.off != off) {
			u8 idx;

			/* Make sure the number of extraction sequence required
			 * does not exceed the block's capability
			 */
			if (params->es_cnt >= fv_words)
				return ICE_ERR_MAX_LIMIT;

			/* some blocks require a reversed field vector layout */
			if (hw->blk[params->blk].es.reverse)
				idx = fv_words - params->es_cnt - 1;
			else
				idx = params->es_cnt;

			params->es[idx].prot_id = prot_id;
			params->es[idx].off = off;
			params->es_cnt++;
		}

		off += ICE_FLOW_FV_EXTRACT_SZ;
	}

	return ICE_SUCCESS;
}

/**
 * ice_flow_xtract_raws - Create extract sequence entries for raw bytes
 * @hw: pointer to the HW struct
 * @params: information about the flow to be processed
 * @seg: index of packet segment whose raw fields are to be be extracted
 */
static enum ice_status
ice_flow_xtract_raws(struct ice_hw *hw, struct ice_flow_prof_params *params,
		     u8 seg)
{
	u16 hdrs_sz;
	u8 i;

	if (!params->prof->segs[seg].raws_cnt)
		return ICE_SUCCESS;

	if (params->prof->segs[seg].raws_cnt >
	    ARRAY_SIZE(params->prof->segs[seg].raws))
		return ICE_ERR_MAX_LIMIT;

	/* Offsets within the segment headers are not supported */
	hdrs_sz = ice_flow_calc_seg_sz(params, seg);
	if (!hdrs_sz)
		return ICE_ERR_PARAM;

	for (i = 0; i < params->prof->segs[seg].raws_cnt; i++) {
		struct ice_flow_seg_fld_raw *raw;
		u16 off, cnt, j;

		raw = &params->prof->segs[seg].raws[i];

		/* Only support matching raw fields in the payload */
		if (raw->off < hdrs_sz)
			return ICE_ERR_PARAM;

		/* Convert the segment-relative offset into payload-relative
		 * offset.
		 */
		off = raw->off - hdrs_sz;

		/* Storing extraction information */
		raw->info.xtrct.prot_id = ICE_PROT_PAY;
		raw->info.xtrct.off = (off / ICE_FLOW_FV_EXTRACT_SZ) *
			ICE_FLOW_FV_EXTRACT_SZ;
		raw->info.xtrct.disp = (off % ICE_FLOW_FV_EXTRACT_SZ) * 8;
		raw->info.xtrct.idx = params->es_cnt;

		/* Determine the number of field vector entries this raw field
		 * consumes.
		 */
		cnt = DIVIDE_AND_ROUND_UP(raw->info.xtrct.disp +
					  (raw->info.src.last * 8),
					  ICE_FLOW_FV_EXTRACT_SZ * 8);
		off = raw->info.xtrct.off;
		for (j = 0; j < cnt; j++) {
			/* Make sure the number of extraction sequence required
			 * does not exceed the block's capability
			 */
			if (params->es_cnt >= hw->blk[params->blk].es.count ||
			    params->es_cnt >= ICE_MAX_FV_WORDS)
				return ICE_ERR_MAX_LIMIT;

			params->es[params->es_cnt].prot_id = ICE_PROT_PAY;
			params->es[params->es_cnt].off = off;
			params->es_cnt++;
			off += ICE_FLOW_FV_EXTRACT_SZ;
		}
	}

	return ICE_SUCCESS;
}

/**
 * ice_flow_create_xtrct_seq - Create an extraction sequence for given segments
 * @hw: pointer to the HW struct
 * @params: information about the flow to be processed
 *
 * This function iterates through all matched fields in the given segments, and
 * creates an extraction sequence for the fields.
 */
static enum ice_status
ice_flow_create_xtrct_seq(struct ice_hw *hw,
			  struct ice_flow_prof_params *params)
{
	enum ice_status status = ICE_SUCCESS;
	u8 i;

	for (i = 0; i < params->prof->segs_cnt; i++) {
		u64 match = params->prof->segs[i].match;
		u16 j;

		for (j = 0; j < ICE_FLOW_FIELD_IDX_MAX && match; j++) {
			const u64 bit = BIT_ULL(j);

			if (match & bit) {
				status = ice_flow_xtract_fld
					(hw, params, i, (enum ice_flow_field)j);
				if (status)
					return status;
				match &= ~bit;
			}
		}

		/* Process raw matching bytes */
		status = ice_flow_xtract_raws(hw, params, i);
		if (status)
			return status;
	}

	return status;
}

/**
 * ice_flow_proc_segs - process all packet segments associated with a profile
 * @hw: pointer to the HW struct
 * @params: information about the flow to be processed
 */
static enum ice_status
ice_flow_proc_segs(struct ice_hw *hw, struct ice_flow_prof_params *params)
{
	enum ice_status status;

	status = ice_flow_proc_seg_hdrs(params);
	if (status)
		return status;

	status = ice_flow_create_xtrct_seq(hw, params);
	if (status)
		return status;

	switch (params->blk) {
	case ICE_BLK_RSS:
		/* Only header information is provided for RSS configuration.
		 * No further processing is needed.
		 */
		status = ICE_SUCCESS;
		break;
	case ICE_BLK_FD:
		status = ICE_SUCCESS;
		break;
	case ICE_BLK_SW:
	default:
		return ICE_ERR_NOT_IMPL;
	}

	return status;
}

#define ICE_FLOW_FIND_PROF_CHK_FLDS	0x00000001
#define ICE_FLOW_FIND_PROF_CHK_VSI	0x00000002

/**
 * ice_flow_find_prof_conds - Find a profile matching headers and conditions
 * @hw: pointer to the HW struct
 * @blk: classification stage
 * @dir: flow direction
 * @segs: array of one or more packet segments that describe the flow
 * @segs_cnt: number of packet segments provided
 * @vsi_handle: software VSI handle to check VSI (ICE_FLOW_FIND_PROF_CHK_VSI)
 * @conds: additional conditions to be checked (ICE_FLOW_FIND_PROF_CHK_*)
 */
static struct ice_flow_prof *
ice_flow_find_prof_conds(struct ice_hw *hw, enum ice_block blk,
			 enum ice_flow_dir dir, struct ice_flow_seg_info *segs,
			 u8 segs_cnt, u16 vsi_handle, u32 conds)
{
	struct ice_flow_prof *p;

	LIST_FOR_EACH_ENTRY(p, &hw->fl_profs[blk], ice_flow_prof, l_entry) {
		if (p->dir == dir && segs_cnt && segs_cnt == p->segs_cnt) {
			u8 i;

			/* Check for profile-VSI association if specified */
			if ((conds & ICE_FLOW_FIND_PROF_CHK_VSI) &&
			    ice_is_vsi_valid(hw, vsi_handle) &&
			    !ice_is_bit_set(p->vsis, vsi_handle))
				continue;

			/* Protocol headers must be checked. Matched fields are
			 * checked if specified.
			 */
			for (i = 0; i < segs_cnt; i++)
				if (segs[i].hdrs != p->segs[i].hdrs ||
				    ((conds & ICE_FLOW_FIND_PROF_CHK_FLDS) &&
				     segs[i].match != p->segs[i].match))
					break;

			/* A match is found if all segments are matched */
			if (i == segs_cnt)
				return p;
		}
	}

	return NULL;
}

/**
 * ice_flow_find_prof - Look up a profile matching headers and matched fields
 * @hw: pointer to the HW struct
 * @blk: classification stage
 * @dir: flow direction
 * @segs: array of one or more packet segments that describe the flow
 * @segs_cnt: number of packet segments provided
 */
u64
ice_flow_find_prof(struct ice_hw *hw, enum ice_block blk, enum ice_flow_dir dir,
		   struct ice_flow_seg_info *segs, u8 segs_cnt)
{
	struct ice_flow_prof *p;

	ice_acquire_lock(&hw->fl_profs_locks[blk]);
	p = ice_flow_find_prof_conds(hw, blk, dir, segs, segs_cnt,
				     ICE_MAX_VSI, ICE_FLOW_FIND_PROF_CHK_FLDS);
	ice_release_lock(&hw->fl_profs_locks[blk]);

	return p ? p->id : ICE_FLOW_PROF_ID_INVAL;
}

/**
 * ice_flow_find_prof_id - Look up a profile with given profile ID
 * @hw: pointer to the HW struct
 * @blk: classification stage
 * @prof_id: unique ID to identify this flow profile
 */
static struct ice_flow_prof *
ice_flow_find_prof_id(struct ice_hw *hw, enum ice_block blk, u64 prof_id)
{
	struct ice_flow_prof *p;

	LIST_FOR_EACH_ENTRY(p, &hw->fl_profs[blk], ice_flow_prof, l_entry) {
		if (p->id == prof_id)
			return p;
	}

	return NULL;
}

/**
 * ice_flow_rem_entry_sync - Remove a flow entry
 * @hw: pointer to the HW struct
 * @entry: flow entry to be removed
 */
static enum ice_status
ice_flow_rem_entry_sync(struct ice_hw *hw, struct ice_flow_entry *entry)
{
	if (!entry)
		return ICE_ERR_BAD_PTR;

	LIST_DEL(&entry->l_entry);

	if (entry->entry)
		ice_free(hw, entry->entry);

	if (entry->acts)
		ice_free(hw, entry->acts);

	ice_free(hw, entry);

	return ICE_SUCCESS;
}

/**
 * ice_flow_add_prof_sync - Add a flow profile for packet segments and fields
 * @hw: pointer to the HW struct
 * @blk: classification stage
 * @dir: flow direction
 * @prof_id: unique ID to identify this flow profile
 * @segs: array of one or more packet segments that describe the flow
 * @segs_cnt: number of packet segments provided
 * @acts: array of default actions
 * @acts_cnt: number of default actions
 * @prof: stores the returned flow profile added
 *
 * Assumption: the caller has acquired the lock to the profile list
 */
static enum ice_status
ice_flow_add_prof_sync(struct ice_hw *hw, enum ice_block blk,
		       enum ice_flow_dir dir, u64 prof_id,
		       struct ice_flow_seg_info *segs, u8 segs_cnt,
		       struct ice_flow_action *acts, u8 acts_cnt,
		       struct ice_flow_prof **prof)
{
	struct ice_flow_prof_params params;
	enum ice_status status = ICE_SUCCESS;
	u8 i;

	if (!prof || (acts_cnt && !acts))
		return ICE_ERR_BAD_PTR;

	ice_memset(&params, 0, sizeof(params), ICE_NONDMA_MEM);
	params.prof = (struct ice_flow_prof *)
		ice_malloc(hw, sizeof(*params.prof));
	if (!params.prof)
		return ICE_ERR_NO_MEMORY;

	/* initialize extraction sequence to all invalid (0xff) */
	ice_memset(params.es, 0xff, sizeof(params.es), ICE_NONDMA_MEM);

	params.blk = blk;
	params.prof->id = prof_id;
	params.prof->dir = dir;
	params.prof->segs_cnt = segs_cnt;

	/* Make a copy of the segments that need to be persistent in the flow
	 * profile instance
	 */
	for (i = 0; i < segs_cnt; i++)
		ice_memcpy(&params.prof->segs[i], &segs[i], sizeof(*segs),
			   ICE_NONDMA_TO_NONDMA);

	/* Make a copy of the actions that need to be persistent in the flow
	 * profile instance.
	 */
	if (acts_cnt) {
		params.prof->acts = (struct ice_flow_action *)
			ice_memdup(hw, acts, acts_cnt * sizeof(*acts),
				   ICE_NONDMA_TO_NONDMA);

		if (!params.prof->acts) {
			status = ICE_ERR_NO_MEMORY;
			goto out;
		}
	}

	status = ice_flow_proc_segs(hw, &params);
	if (status) {
		ice_debug(hw, ICE_DBG_FLOW,
			  "Error processing a flow's packet segments\n");
		goto out;
	}

	/* Add a HW profile for this flow profile */
	status = ice_add_prof(hw, blk, prof_id, (u8 *)params.ptypes, params.es);
	if (status) {
		ice_debug(hw, ICE_DBG_FLOW, "Error adding a HW flow profile\n");
		goto out;
	}

	INIT_LIST_HEAD(&params.prof->entries);
	ice_init_lock(&params.prof->entries_lock);
	*prof = params.prof;

out:
	if (status) {
		if (params.prof->acts)
			ice_free(hw, params.prof->acts);
		ice_free(hw, params.prof);
	}

	return status;
}

/**
 * ice_flow_rem_prof_sync - remove a flow profile
 * @hw: pointer to the hardware structure
 * @blk: classification stage
 * @prof: pointer to flow profile to remove
 *
 * Assumption: the caller has acquired the lock to the profile list
 */
static enum ice_status
ice_flow_rem_prof_sync(struct ice_hw *hw, enum ice_block blk,
		       struct ice_flow_prof *prof)
{
	enum ice_status status = ICE_SUCCESS;

	/* Remove all remaining flow entries before removing the flow profile */
	if (!LIST_EMPTY(&prof->entries)) {
		struct ice_flow_entry *e, *t;

		ice_acquire_lock(&prof->entries_lock);

		LIST_FOR_EACH_ENTRY_SAFE(e, t, &prof->entries, ice_flow_entry,
					 l_entry) {
			status = ice_flow_rem_entry_sync(hw, e);
			if (status)
				break;
		}

		ice_release_lock(&prof->entries_lock);
	}

	/* Remove all hardware profiles associated with this flow profile */
	status = ice_rem_prof(hw, blk, prof->id);
	if (!status) {
		LIST_DEL(&prof->l_entry);
		ice_destroy_lock(&prof->entries_lock);
		if (prof->acts)
			ice_free(hw, prof->acts);
		ice_free(hw, prof);
	}

	return status;
}

/**
 * ice_flow_assoc_prof - associate a VSI with a flow profile
 * @hw: pointer to the hardware structure
 * @blk: classification stage
 * @prof: pointer to flow profile
 * @vsi_handle: software VSI handle
 *
 * Assumption: the caller has acquired the lock to the profile list
 * and the software VSI handle has been validated
 */
static enum ice_status
ice_flow_assoc_prof(struct ice_hw *hw, enum ice_block blk,
		    struct ice_flow_prof *prof, u16 vsi_handle)
{
	enum ice_status status = ICE_SUCCESS;

	if (!ice_is_bit_set(prof->vsis, vsi_handle)) {
		status = ice_add_prof_id_flow(hw, blk,
					      ice_get_hw_vsi_num(hw,
								 vsi_handle),
					      prof->id);
		if (!status)
			ice_set_bit(vsi_handle, prof->vsis);
		else
			ice_debug(hw, ICE_DBG_FLOW,
				  "HW profile add failed, %d\n",
				  status);
	}

	return status;
}

/**
 * ice_flow_disassoc_prof - disassociate a VSI from a flow profile
 * @hw: pointer to the hardware structure
 * @blk: classification stage
 * @prof: pointer to flow profile
 * @vsi_handle: software VSI handle
 *
 * Assumption: the caller has acquired the lock to the profile list
 * and the software VSI handle has been validated
 */
static enum ice_status
ice_flow_disassoc_prof(struct ice_hw *hw, enum ice_block blk,
		       struct ice_flow_prof *prof, u16 vsi_handle)
{
	enum ice_status status = ICE_SUCCESS;

	if (ice_is_bit_set(prof->vsis, vsi_handle)) {
		status = ice_rem_prof_id_flow(hw, blk,
					      ice_get_hw_vsi_num(hw,
								 vsi_handle),
					      prof->id);
		if (!status)
			ice_clear_bit(vsi_handle, prof->vsis);
		else
			ice_debug(hw, ICE_DBG_FLOW,
				  "HW profile remove failed, %d\n",
				  status);
	}

	return status;
}

/**
 * ice_flow_add_prof - Add a flow profile for packet segments and matched fields
 * @hw: pointer to the HW struct
 * @blk: classification stage
 * @dir: flow direction
 * @prof_id: unique ID to identify this flow profile
 * @segs: array of one or more packet segments that describe the flow
 * @segs_cnt: number of packet segments provided
 * @acts: array of default actions
 * @acts_cnt: number of default actions
 * @prof: stores the returned flow profile added
 */
enum ice_status
ice_flow_add_prof(struct ice_hw *hw, enum ice_block blk, enum ice_flow_dir dir,
		  u64 prof_id, struct ice_flow_seg_info *segs, u8 segs_cnt,
		  struct ice_flow_action *acts, u8 acts_cnt,
		  struct ice_flow_prof **prof)
{
	enum ice_status status;

	if (segs_cnt > ICE_FLOW_SEG_MAX)
		return ICE_ERR_MAX_LIMIT;

	if (!segs_cnt)
		return ICE_ERR_PARAM;

	if (!segs)
		return ICE_ERR_BAD_PTR;

	status = ice_flow_val_hdrs(segs, segs_cnt);
	if (status)
		return status;

	ice_acquire_lock(&hw->fl_profs_locks[blk]);

	status = ice_flow_add_prof_sync(hw, blk, dir, prof_id, segs, segs_cnt,
					acts, acts_cnt, prof);
	if (!status)
		LIST_ADD(&(*prof)->l_entry, &hw->fl_profs[blk]);

	ice_release_lock(&hw->fl_profs_locks[blk]);

	return status;
}

/**
 * ice_flow_rem_prof - Remove a flow profile and all entries associated with it
 * @hw: pointer to the HW struct
 * @blk: the block for which the flow profile is to be removed
 * @prof_id: unique ID of the flow profile to be removed
 */
enum ice_status
ice_flow_rem_prof(struct ice_hw *hw, enum ice_block blk, u64 prof_id)
{
	struct ice_flow_prof *prof;
	enum ice_status status;

	ice_acquire_lock(&hw->fl_profs_locks[blk]);

	prof = ice_flow_find_prof_id(hw, blk, prof_id);
	if (!prof) {
		status = ICE_ERR_DOES_NOT_EXIST;
		goto out;
	}

	/* prof becomes invalid after the call */
	status = ice_flow_rem_prof_sync(hw, blk, prof);

out:
	ice_release_lock(&hw->fl_profs_locks[blk]);

	return status;
}

/**
 * ice_flow_get_hw_prof - return the HW profile for a specific profile ID handle
 * @hw: pointer to the HW struct
 * @blk: classification stage
 * @prof_id: the profile ID handle
 * @hw_prof_id: pointer to variable to receive the HW profile ID
 */
enum ice_status
ice_flow_get_hw_prof(struct ice_hw *hw, enum ice_block blk, u64 prof_id,
		     u8 *hw_prof_id)
{
	struct ice_prof_map *map;

	map = ice_search_prof_id(hw, blk, prof_id);
	if (map) {
		*hw_prof_id = map->prof_id;
		return ICE_SUCCESS;
	}

	return ICE_ERR_DOES_NOT_EXIST;
}

/**
 * ice_flow_find_entry - look for a flow entry using its unique ID
 * @hw: pointer to the HW struct
 * @blk: classification stage
 * @entry_id: unique ID to identify this flow entry
 *
 * This function looks for the flow entry with the specified unique ID in all
 * flow profiles of the specified classification stage. If the entry is found,
 * and it returns the handle to the flow entry. Otherwise, it returns
 * ICE_FLOW_ENTRY_ID_INVAL.
 */
u64 ice_flow_find_entry(struct ice_hw *hw, enum ice_block blk, u64 entry_id)
{
	struct ice_flow_entry *found = NULL;
	struct ice_flow_prof *p;

	ice_acquire_lock(&hw->fl_profs_locks[blk]);

	LIST_FOR_EACH_ENTRY(p, &hw->fl_profs[blk], ice_flow_prof, l_entry) {
		struct ice_flow_entry *e;

		ice_acquire_lock(&p->entries_lock);
		LIST_FOR_EACH_ENTRY(e, &p->entries, ice_flow_entry, l_entry)
			if (e->id == entry_id) {
				found = e;
				break;
			}
		ice_release_lock(&p->entries_lock);

		if (found)
			break;
	}

	ice_release_lock(&hw->fl_profs_locks[blk]);

	return found ? ICE_FLOW_ENTRY_HNDL(found) : ICE_FLOW_ENTRY_HANDLE_INVAL;
}

/**
 * ice_flow_add_entry - Add a flow entry
 * @hw: pointer to the HW struct
 * @blk: classification stage
 * @prof_id: ID of the profile to add a new flow entry to
 * @entry_id: unique ID to identify this flow entry
 * @vsi_handle: software VSI handle for the flow entry
 * @prio: priority of the flow entry
 * @data: pointer to a data buffer containing flow entry's match values/masks
 * @acts: arrays of actions to be performed on a match
 * @acts_cnt: number of actions
 * @entry_h: pointer to buffer that receives the new flow entry's handle
 */
enum ice_status
ice_flow_add_entry(struct ice_hw *hw, enum ice_block blk, u64 prof_id,
		   u64 entry_id, u16 vsi_handle, enum ice_flow_priority prio,
		   void *data, struct ice_flow_action *acts, u8 acts_cnt,
		   u64 *entry_h)
{
	struct ice_flow_prof *prof = NULL;
	struct ice_flow_entry *e = NULL;
	enum ice_status status = ICE_SUCCESS;

	if (acts_cnt && !acts)
		return ICE_ERR_PARAM;

	/* No flow entry data is expected for RSS */
	if (!entry_h || (!data && blk != ICE_BLK_RSS))
		return ICE_ERR_BAD_PTR;

	if (!ice_is_vsi_valid(hw, vsi_handle))
		return ICE_ERR_PARAM;

	ice_acquire_lock(&hw->fl_profs_locks[blk]);

	prof = ice_flow_find_prof_id(hw, blk, prof_id);
	if (!prof) {
		status = ICE_ERR_DOES_NOT_EXIST;
	} else {
		/* Allocate memory for the entry being added and associate
		 * the VSI to the found flow profile
		 */
		e = (struct ice_flow_entry *)ice_malloc(hw, sizeof(*e));
		if (!e)
			status = ICE_ERR_NO_MEMORY;
		else
			status = ice_flow_assoc_prof(hw, blk, prof, vsi_handle);
	}

	ice_release_lock(&hw->fl_profs_locks[blk]);
	if (status)
		goto out;

	e->id = entry_id;
	e->vsi_handle = vsi_handle;
	e->prof = prof;

	switch (blk) {
	case ICE_BLK_RSS:
		/* RSS will add only one entry per VSI per profile */
		break;
	case ICE_BLK_FD:
		break;
	case ICE_BLK_SW:
	case ICE_BLK_PE:
	default:
		status = ICE_ERR_NOT_IMPL;
		goto out;
	}

	ice_acquire_lock(&prof->entries_lock);
	LIST_ADD(&e->l_entry, &prof->entries);
	ice_release_lock(&prof->entries_lock);

	*entry_h = ICE_FLOW_ENTRY_HNDL(e);

out:
	if (status && e) {
		if (e->entry)
			ice_free(hw, e->entry);
		ice_free(hw, e);
	}

	return status;
}

/**
 * ice_flow_rem_entry - Remove a flow entry
 * @hw: pointer to the HW struct
 * @entry_h: handle to the flow entry to be removed
 */
enum ice_status ice_flow_rem_entry(struct ice_hw *hw, u64 entry_h)
{
	struct ice_flow_entry *entry;
	struct ice_flow_prof *prof;
	enum ice_status status;

	if (entry_h == ICE_FLOW_ENTRY_HANDLE_INVAL)
		return ICE_ERR_PARAM;

	entry = ICE_FLOW_ENTRY_PTR((unsigned long)entry_h);

	/* Retain the pointer to the flow profile as the entry will be freed */
	prof = entry->prof;

	ice_acquire_lock(&prof->entries_lock);
	status = ice_flow_rem_entry_sync(hw, entry);
	ice_release_lock(&prof->entries_lock);

	return status;
}

/**
 * ice_flow_set_fld_ext - specifies locations of field from entry's input buffer
 * @seg: packet segment the field being set belongs to
 * @fld: field to be set
 * @type: type of the field
 * @val_loc: if not ICE_FLOW_FLD_OFF_INVAL, location of the value to match from
 *           entry's input buffer
 * @mask_loc: if not ICE_FLOW_FLD_OFF_INVAL, location of mask value from entry's
 *            input buffer
 * @last_loc: if not ICE_FLOW_FLD_OFF_INVAL, location of last/upper value from
 *            entry's input buffer
 *
 * This helper function stores information of a field being matched, including
 * the type of the field and the locations of the value to match, the mask, and
 * and the upper-bound value in the start of the input buffer for a flow entry.
 * This function should only be used for fixed-size data structures.
 *
 * This function also opportunistically determines the protocol headers to be
 * present based on the fields being set. Some fields cannot be used alone to
 * determine the protocol headers present. Sometimes, fields for particular
 * protocol headers are not matched. In those cases, the protocol headers
 * must be explicitly set.
 */
static void
ice_flow_set_fld_ext(struct ice_flow_seg_info *seg, enum ice_flow_field fld,
		     enum ice_flow_fld_match_type type, u16 val_loc,
		     u16 mask_loc, u16 last_loc)
{
	u64 bit = BIT_ULL(fld);

	seg->match |= bit;
	if (type == ICE_FLOW_FLD_TYPE_RANGE)
		seg->range |= bit;

	seg->fields[fld].type = type;
	seg->fields[fld].src.val = val_loc;
	seg->fields[fld].src.mask = mask_loc;
	seg->fields[fld].src.last = last_loc;

	ICE_FLOW_SET_HDRS(seg, ice_flds_info[fld].hdr);
}

/**
 * ice_flow_set_fld - specifies locations of field from entry's input buffer
 * @seg: packet segment the field being set belongs to
 * @fld: field to be set
 * @val_loc: if not ICE_FLOW_FLD_OFF_INVAL, location of the value to match from
 *           entry's input buffer
 * @mask_loc: if not ICE_FLOW_FLD_OFF_INVAL, location of mask value from entry's
 *            input buffer
 * @last_loc: if not ICE_FLOW_FLD_OFF_INVAL, location of last/upper value from
 *            entry's input buffer
 * @range: indicate if field being matched is to be in a range
 *
 * This function specifies the locations, in the form of byte offsets from the
 * start of the input buffer for a flow entry, from where the value to match,
 * the mask value, and upper value can be extracted. These locations are then
 * stored in the flow profile. When adding a flow entry associated with the
 * flow profile, these locations will be used to quickly extract the values and
 * create the content of a match entry. This function should only be used for
 * fixed-size data structures.
 */
void
ice_flow_set_fld(struct ice_flow_seg_info *seg, enum ice_flow_field fld,
		 u16 val_loc, u16 mask_loc, u16 last_loc, bool range)
{
	enum ice_flow_fld_match_type t = range ?
		ICE_FLOW_FLD_TYPE_RANGE : ICE_FLOW_FLD_TYPE_REG;

	ice_flow_set_fld_ext(seg, fld, t, val_loc, mask_loc, last_loc);
}

/**
 * ice_flow_set_fld_prefix - sets locations of prefix field from entry's buf
 * @seg: packet segment the field being set belongs to
 * @fld: field to be set
 * @val_loc: if not ICE_FLOW_FLD_OFF_INVAL, location of the value to match from
 *           entry's input buffer
 * @pref_loc: location of prefix value from entry's input buffer
 * @pref_sz: size of the location holding the prefix value
 *
 * This function specifies the locations, in the form of byte offsets from the
 * start of the input buffer for a flow entry, from where the value to match
 * and the IPv4 prefix value can be extracted. These locations are then stored
 * in the flow profile. When adding flow entries to the associated flow profile,
 * these locations can be used to quickly extract the values to create the
 * content of a match entry. This function should only be used for fixed-size
 * data structures.
 */
void
ice_flow_set_fld_prefix(struct ice_flow_seg_info *seg, enum ice_flow_field fld,
			u16 val_loc, u16 pref_loc, u8 pref_sz)
{
	/* For this type of field, the "mask" location is for the prefix value's
	 * location and the "last" location is for the size of the location of
	 * the prefix value.
	 */
	ice_flow_set_fld_ext(seg, fld, ICE_FLOW_FLD_TYPE_PREFIX, val_loc,
			     pref_loc, (u16)pref_sz);
}

/**
 * ice_flow_add_fld_raw - sets locations of a raw field from entry's input buf
 * @seg: packet segment the field being set belongs to
 * @off: offset of the raw field from the beginning of the segment in bytes
 * @len: length of the raw pattern to be matched
 * @val_loc: location of the value to match from entry's input buffer
 * @mask_loc: location of mask value from entry's input buffer
 *
 * This function specifies the offset of the raw field to be match from the
 * beginning of the specified packet segment, and the locations, in the form of
 * byte offsets from the start of the input buffer for a flow entry, from where
 * the value to match and the mask value to be extracted. These locations are
 * then stored in the flow profile. When adding flow entries to the associated
 * flow profile, these locations can be used to quickly extract the values to
 * create the content of a match entry. This function should only be used for
 * fixed-size data structures.
 */
void
ice_flow_add_fld_raw(struct ice_flow_seg_info *seg, u16 off, u8 len,
		     u16 val_loc, u16 mask_loc)
{
	if (seg->raws_cnt < ICE_FLOW_SEG_RAW_FLD_MAX) {
		seg->raws[seg->raws_cnt].off = off;
		seg->raws[seg->raws_cnt].info.type = ICE_FLOW_FLD_TYPE_SIZE;
		seg->raws[seg->raws_cnt].info.src.val = val_loc;
		seg->raws[seg->raws_cnt].info.src.mask = mask_loc;
		/* The "last" field is used to store the length of the field */
		seg->raws[seg->raws_cnt].info.src.last = len;
	}

	/* Overflows of "raws" will be handled as an error condition later in
	 * the flow when this information is processed.
	 */
	seg->raws_cnt++;
}

#define ICE_FLOW_RSS_SEG_HDR_L3_MASKS \
	(ICE_FLOW_SEG_HDR_IPV4 | ICE_FLOW_SEG_HDR_IPV6)

#define ICE_FLOW_RSS_SEG_HDR_L4_MASKS \
	(ICE_FLOW_SEG_HDR_TCP | ICE_FLOW_SEG_HDR_UDP | \
	 ICE_FLOW_SEG_HDR_SCTP)

#define ICE_FLOW_RSS_SEG_HDR_VAL_MASKS \
	(ICE_FLOW_RSS_SEG_HDR_L3_MASKS | \
	 ICE_FLOW_RSS_SEG_HDR_L4_MASKS)

/**
 * ice_flow_set_rss_seg_info - setup packet segments for RSS
 * @segs: pointer to the flow field segment(s)
 * @hash_fields: fields to be hashed on for the segment(s)
 * @flow_hdr: protocol header fields within a packet segment
 *
 * Helper function to extract fields from hash bitmap and use flow
 * header value to set flow field segment for further use in flow
 * profile entry or removal.
 */
static enum ice_status
ice_flow_set_rss_seg_info(struct ice_flow_seg_info *segs, u64 hash_fields,
			  u32 flow_hdr)
{
	u64 val = hash_fields;
	u8 i;

	for (i = 0; val && i < ICE_FLOW_FIELD_IDX_MAX; i++) {
		u64 bit = BIT_ULL(i);

		if (val & bit) {
			ice_flow_set_fld(segs, (enum ice_flow_field)i,
					 ICE_FLOW_FLD_OFF_INVAL,
					 ICE_FLOW_FLD_OFF_INVAL,
					 ICE_FLOW_FLD_OFF_INVAL, false);
			val &= ~bit;
		}
	}
	ICE_FLOW_SET_HDRS(segs, flow_hdr);

	if (segs->hdrs & ~ICE_FLOW_RSS_SEG_HDR_VAL_MASKS)
		return ICE_ERR_PARAM;

	val = (u64)(segs->hdrs & ICE_FLOW_RSS_SEG_HDR_L3_MASKS);
	if (!ice_is_pow2(val))
		return ICE_ERR_CFG;

	val = (u64)(segs->hdrs & ICE_FLOW_RSS_SEG_HDR_L4_MASKS);
	if (val && !ice_is_pow2(val))
		return ICE_ERR_CFG;

	return ICE_SUCCESS;
}

/**
 * ice_rem_all_rss_vsi_ctx - remove all RSS configurations from VSI context
 * @hw: pointer to the hardware structure
 * @vsi_handle: software VSI handle
 *
 */
void ice_rem_all_rss_vsi_ctx(struct ice_hw *hw, u16 vsi_handle)
{
	struct ice_rss_cfg *r, *tmp;

	if (!ice_is_vsi_valid(hw, vsi_handle) ||
	    LIST_EMPTY(&hw->vsi_ctx[vsi_handle]->rss_list_head))
		return;

	ice_acquire_lock(&hw->vsi_ctx[vsi_handle]->rss_locks);
	LIST_FOR_EACH_ENTRY_SAFE(r, tmp,
				 &hw->vsi_ctx[vsi_handle]->rss_list_head,
				 ice_rss_cfg, l_entry) {
		LIST_DEL(&r->l_entry);
		ice_free(hw, r);
	}
	ice_release_lock(&hw->vsi_ctx[vsi_handle]->rss_locks);
}

/**
 * ice_rem_vsi_rss_cfg - remove RSS configurations associated with VSI
 * @hw: pointer to the hardware structure
 * @vsi_handle: software VSI handle
 *
 * This function will iterate through all flow profiles and disassociate
 * the VSI from that profile. If the flow profile has no VSIs it will
 * be removed.
 */
enum ice_status ice_rem_vsi_rss_cfg(struct ice_hw *hw, u16 vsi_handle)
{
	const enum ice_block blk = ICE_BLK_RSS;
	struct ice_flow_prof *p, *t;
	enum ice_status status = ICE_SUCCESS;

	if (!ice_is_vsi_valid(hw, vsi_handle))
		return ICE_ERR_PARAM;

	ice_acquire_lock(&hw->fl_profs_locks[blk]);
	LIST_FOR_EACH_ENTRY_SAFE(p, t, &hw->fl_profs[blk], ice_flow_prof,
				 l_entry) {
		if (ice_is_bit_set(p->vsis, vsi_handle)) {
			status = ice_flow_disassoc_prof(hw, blk, p, vsi_handle);
			if (status)
				break;

			if (!ice_is_any_bit_set(p->vsis, ICE_MAX_VSI)) {
				status = ice_flow_rem_prof_sync(hw, blk, p);
				if (status)
					break;
			}
		}
	}
	ice_release_lock(&hw->fl_profs_locks[blk]);

	return status;
}

/**
 * ice_rem_rss_cfg_vsi_ctx - remove RSS configuration from VSI context
 * @hw: pointer to the hardware structure
 * @vsi_handle: software VSI handle
 * @prof: pointer to flow profile
 *
 * Assumption: lock has already been acquired for RSS list
 */
static void
ice_rem_rss_cfg_vsi_ctx(struct ice_hw *hw, u16 vsi_handle,
			struct ice_flow_prof *prof)
{
	struct ice_rss_cfg *r, *tmp;

	/* Search for RSS hash fields associated to the VSI that match the
	 * hash configurations associated to the flow profile. If found
	 * remove from the RSS entry list of the VSI context and delete entry.
	 */
	LIST_FOR_EACH_ENTRY_SAFE(r, tmp,
				 &hw->vsi_ctx[vsi_handle]->rss_list_head,
				 ice_rss_cfg, l_entry) {
		if (r->hashed_flds == prof->segs[prof->segs_cnt - 1].match &&
		    r->packet_hdr == prof->segs[prof->segs_cnt - 1].hdrs) {
			LIST_DEL(&r->l_entry);
			ice_free(hw, r);
			return;
		}
	}
}

/**
 * ice_add_rss_vsi_ctx - add RSS configuration to VSI context
 * @hw: pointer to the hardware structure
 * @vsi_handle: software VSI handle
 * @prof: pointer to flow profile
 *
 * Assumption: lock has already been acquired for RSS list
 */
static enum ice_status
ice_add_rss_vsi_ctx(struct ice_hw *hw, u16 vsi_handle,
		    struct ice_flow_prof *prof)
{
	struct ice_rss_cfg *r, *rss_cfg;

	LIST_FOR_EACH_ENTRY(r, &hw->vsi_ctx[vsi_handle]->rss_list_head,
			    ice_rss_cfg, l_entry)
		if (r->hashed_flds == prof->segs[prof->segs_cnt - 1].match &&
		    r->packet_hdr == prof->segs[prof->segs_cnt - 1].hdrs)
			return ICE_SUCCESS;

	rss_cfg = (struct ice_rss_cfg *)ice_malloc(hw, sizeof(*rss_cfg));
	if (!rss_cfg)
		return ICE_ERR_NO_MEMORY;

	rss_cfg->hashed_flds = prof->segs[prof->segs_cnt - 1].match;
	rss_cfg->packet_hdr = prof->segs[prof->segs_cnt - 1].hdrs;
	LIST_ADD(&rss_cfg->l_entry, &hw->vsi_ctx[vsi_handle]->rss_list_head);

	return ICE_SUCCESS;
}

#define ICE_FLOW_PROF_HASH_S	0
#define ICE_FLOW_PROF_HASH_M	(0xFFFFFFFFULL << ICE_FLOW_PROF_HASH_S)
#define ICE_FLOW_PROF_HDR_S	32
#define ICE_FLOW_PROF_HDR_M	(0xFFFFFFFFULL << ICE_FLOW_PROF_HDR_S)

#define ICE_FLOW_GEN_PROFID(hash, hdr) \
	(u64)(((u64)(hash) & ICE_FLOW_PROF_HASH_M) | \
	      (((u64)(hdr) << ICE_FLOW_PROF_HDR_S) & ICE_FLOW_PROF_HDR_M))

/**
 * ice_add_rss_cfg_sync - add an RSS configuration
 * @hw: pointer to the hardware structure
 * @vsi_handle: software VSI handle
 * @hashed_flds: hash bit fields (ICE_FLOW_HASH_*) to configure
 * @addl_hdrs: protocol header fields
 *
 * Assumption: lock has already been acquired for RSS list
 */
static enum ice_status
ice_add_rss_cfg_sync(struct ice_hw *hw, u16 vsi_handle, u64 hashed_flds,
		     u32 addl_hdrs)
{
	const enum ice_block blk = ICE_BLK_RSS;
	struct ice_flow_prof *prof = NULL;
	struct ice_flow_seg_info *segs;
	enum ice_status status = ICE_SUCCESS;

	segs = (struct ice_flow_seg_info *)ice_malloc(hw, sizeof(*segs));
	if (!segs)
		return ICE_ERR_NO_MEMORY;

	/* Construct the packet segment info from the hashed fields */
	status = ice_flow_set_rss_seg_info(segs, hashed_flds, addl_hdrs);
	if (status)
		goto exit;

	/* Search for a flow profile that has matching headers, hash fields
	 * and has the input VSI associated to it. If found, no further
	 * operations required and exit.
	 */
	prof = ice_flow_find_prof_conds(hw, blk, ICE_FLOW_RX, segs, 1,
					vsi_handle,
					ICE_FLOW_FIND_PROF_CHK_FLDS |
					ICE_FLOW_FIND_PROF_CHK_VSI);
	if (prof)
		goto exit;

	/* Check if a flow profile exists with the same protocol headers and
	 * associated with the input VSI. If so disasscociate the VSI from
	 * this profile. The VSI will be added to a new profile created with
	 * the protocol header and new hash field configuration.
	 */
	prof = ice_flow_find_prof_conds(hw, blk, ICE_FLOW_RX, segs, 1,
					vsi_handle, ICE_FLOW_FIND_PROF_CHK_VSI);
	if (prof) {
		status = ice_flow_disassoc_prof(hw, blk, prof, vsi_handle);
		if (!status)
			ice_rem_rss_cfg_vsi_ctx(hw, vsi_handle, prof);
		else
			goto exit;

		/* Remove profile if it has no VSIs associated */
		if (!ice_is_any_bit_set(prof->vsis, ICE_MAX_VSI)) {
			status = ice_flow_rem_prof_sync(hw, blk, prof);
			if (status)
				goto exit;
		}
	}

	/* Search for a profile that has same match fields only. If this
	 * exists then associate the VSI to this profile.
	 */
	prof = ice_flow_find_prof_conds(hw, blk, ICE_FLOW_RX, segs, 1,
					vsi_handle,
					ICE_FLOW_FIND_PROF_CHK_FLDS);
	if (prof) {
		status = ice_flow_assoc_prof(hw, blk, prof, vsi_handle);
		if (!status)
			status = ice_add_rss_vsi_ctx(hw, vsi_handle, prof);
		goto exit;
	}

	/* Create a new flow profile with generated profile and packet
	 * segment information.
	 */
	status = ice_flow_add_prof(hw, blk, ICE_FLOW_RX,
				   ICE_FLOW_GEN_PROFID(hashed_flds, segs->hdrs),
				   segs, 1, NULL, 0, &prof);
	if (status)
		goto exit;

	status = ice_flow_assoc_prof(hw, blk, prof, vsi_handle);
	/* If association to a new flow profile failed then this profile can
	 * be removed.
	 */
	if (status) {
		ice_flow_rem_prof_sync(hw, blk, prof);
		goto exit;
	}

	status = ice_add_rss_vsi_ctx(hw, vsi_handle, prof);

exit:
	ice_free(hw, segs);
	return status;
}

/**
 * ice_add_rss_cfg - add an RSS configuration with specified hashed fields
 * @hw: pointer to the hardware structure
 * @vsi_handle: software VSI handle
 * @hashed_flds: hash bit fields (ICE_FLOW_HASH_*) to configure
 * @addl_hdrs: protocol header fields
 *
 * This function will generate a flow profile based on fields associated with
 * the input fields to hash on, the flow type and use the VSI number to add
 * a flow entry to the profile.
 */
enum ice_status
ice_add_rss_cfg(struct ice_hw *hw, u16 vsi_handle, u64 hashed_flds,
		u32 addl_hdrs)
{
	enum ice_status status;

	if (hashed_flds == ICE_HASH_INVALID ||
	    !ice_is_vsi_valid(hw, vsi_handle))
		return ICE_ERR_PARAM;

	ice_acquire_lock(&hw->vsi_ctx[vsi_handle]->rss_locks);
	status = ice_add_rss_cfg_sync(hw, vsi_handle, hashed_flds, addl_hdrs);
	ice_release_lock(&hw->vsi_ctx[vsi_handle]->rss_locks);

	return status;
}

/**
 * ice_rem_rss_cfg_sync - remove an existing RSS configuration
 * @hw: pointer to the hardware structure
 * @vsi_handle: software VSI handle
 * @hashed_flds: Packet hash types (ICE_FLOW_HASH_*) to remove
 * @addl_hdrs: Protocol header fields within a packet segment
 *
 * Assumption: lock has already been acquired for RSS list
 */
static enum ice_status
ice_rem_rss_cfg_sync(struct ice_hw *hw, u16 vsi_handle, u64 hashed_flds,
		     u32 addl_hdrs)
{
	const enum ice_block blk = ICE_BLK_RSS;
	struct ice_flow_seg_info *segs;
	struct ice_flow_prof *prof;
	enum ice_status status;

	segs = (struct ice_flow_seg_info *)ice_malloc(hw, sizeof(*segs));
	if (!segs)
		return ICE_ERR_NO_MEMORY;

	/* Construct the packet segment info from the hashed fields */
	status = ice_flow_set_rss_seg_info(segs, hashed_flds, addl_hdrs);
	if (status)
		goto out;

	prof = ice_flow_find_prof_conds(hw, blk, ICE_FLOW_RX, segs, 1,
					vsi_handle,
					ICE_FLOW_FIND_PROF_CHK_FLDS);
	if (!prof) {
		status = ICE_ERR_DOES_NOT_EXIST;
		goto out;
	}

	status = ice_flow_disassoc_prof(hw, blk, prof, vsi_handle);
	if (status)
		goto out;

	if (!ice_is_any_bit_set(prof->vsis, ICE_MAX_VSI))
		status = ice_flow_rem_prof_sync(hw, blk, prof);

	ice_rem_rss_cfg_vsi_ctx(hw, vsi_handle, prof);

out:
	ice_free(hw, segs);
	return status;
}

/**
 * ice_rem_rss_cfg - remove an existing RSS config with matching hashed fields
 * @hw: pointer to the hardware structure
 * @vsi_handle: software VSI handle
 * @hashed_flds: Packet hash types (ICE_FLOW_HASH_*) to remove
 * @addl_hdrs: Protocol header fields within a packet segment
 *
 * This function will lookup the flow profile based on the input
 * hash field bitmap, iterate through the profile entry list of
 * that profile and find entry associated with input VSI to be
 * removed. Calls are made to underlying flow apis which will in
 * turn build or update buffers for RSS XLT1 section.
 */
enum ice_status
ice_rem_rss_cfg(struct ice_hw *hw, u16 vsi_handle, u64 hashed_flds,
		u32 addl_hdrs)
{
	enum ice_status status;

	if (hashed_flds == ICE_HASH_INVALID ||
	    !ice_is_vsi_valid(hw, vsi_handle))
		return ICE_ERR_PARAM;

	ice_acquire_lock(&hw->vsi_ctx[vsi_handle]->rss_locks);
	status = ice_rem_rss_cfg_sync(hw, vsi_handle, hashed_flds, addl_hdrs);
	ice_release_lock(&hw->vsi_ctx[vsi_handle]->rss_locks);

	return status;
}

/**
 * ice_replay_rss_cfg - remove RSS configurations associated with VSI
 * @hw: pointer to the hardware structure
 * @vsi_handle: software VSI handle
 */
enum ice_status ice_replay_rss_cfg(struct ice_hw *hw, u16 vsi_handle)
{
	enum ice_status status = ICE_SUCCESS;
	struct ice_rss_cfg *r;

	if (!ice_is_vsi_valid(hw, vsi_handle))
		return ICE_ERR_PARAM;

	ice_acquire_lock(&hw->vsi_ctx[vsi_handle]->rss_locks);
	LIST_FOR_EACH_ENTRY(r, &hw->vsi_ctx[vsi_handle]->rss_list_head,
			    ice_rss_cfg, l_entry) {
		status = ice_add_rss_cfg_sync(hw, vsi_handle, r->hashed_flds,
					      r->packet_hdr);
		if (status)
			break;
	}
	ice_release_lock(&hw->vsi_ctx[vsi_handle]->rss_locks);

	return status;
}

/**
 * ice_get_rss_cfg - returns hashed fields for the given header types
 * @hw: pointer to the hardware structure
 * @vsi_handle: software VSI handle
 * @hdrs: protocol header type
 *
 * This function will return the match fields of the first instance of flow
 * profile having the given header types and containing input VSI
 */
u64 ice_get_rss_cfg(struct ice_hw *hw, u16 vsi_handle, u32 hdrs)
{
	struct ice_rss_cfg *r, *rss_cfg = NULL;

	/* verify if the protocol header is non zero and VSI is valid */
	if (hdrs == ICE_FLOW_SEG_HDR_NONE || !ice_is_vsi_valid(hw, vsi_handle))
		return ICE_HASH_INVALID;

	ice_acquire_lock(&hw->vsi_ctx[vsi_handle]->rss_locks);
	LIST_FOR_EACH_ENTRY(r, &hw->vsi_ctx[vsi_handle]->rss_list_head,
			    ice_rss_cfg, l_entry)
		if (r->packet_hdr == hdrs) {
			rss_cfg = r;
			break;
		}
	ice_release_lock(&hw->vsi_ctx[vsi_handle]->rss_locks);

	return rss_cfg ? rss_cfg->hashed_flds : ICE_HASH_INVALID;
}
