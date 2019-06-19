/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2001-2019
 */

#include "ice_common.h"
#include "ice_fdir.h"

/* These are dummy packet headers used to program flow director filters. */
static const u8 ice_fdir_tcpv4_pkt[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x45, 0x00,
	0x00, 0x28, 0x00, 0x01, 0x00, 0x00, 0x40, 0x06,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00,
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const u8 ice_fdir_udpv4_pkt[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x45, 0x00,
	0x00, 0x1C, 0x00, 0x00, 0x40, 0x00, 0x40, 0x11,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

static const u8 ice_fdir_sctpv4_pkt[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x45, 0x00,
	0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x40, 0x84,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const u8 ice_fdir_ipv4_pkt[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x45, 0x00,
	0x00, 0x14, 0x00, 0x00, 0x40, 0x00, 0x40, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

static const u8 ice_fdir_tcpv6_pkt[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x86, 0xDD, 0x60, 0x00,
	0x00, 0x00, 0x00, 0x14, 0x06, 0x40, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x50, 0x00, 0x20, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

static const u8 ice_fdir_udpv6_pkt[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x86, 0xDD, 0x60, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x11, 0x40, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
};

static const u8 ice_fdir_sctpv6_pkt[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x86, 0xDD, 0x60, 0x00,
	0x00, 0x00, 0x00, 0x0C, 0x84, 0x40, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

static const u8 ice_fdir_ipv6_pkt[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x86, 0xDD, 0x60, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x3B, 0x40, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* Flow Director dummy packet table */
static const struct ice_fdir_base_pkt ice_fdir_pkt[] = {
	{
		ICE_FLTR_PTYPE_NONF_IPV4_TCP,
		sizeof(ice_fdir_tcpv4_pkt),
		ice_fdir_tcpv4_pkt,
	},
	{
		ICE_FLTR_PTYPE_NONF_IPV4_UDP,
		sizeof(ice_fdir_udpv4_pkt),
		ice_fdir_udpv4_pkt,
	},
	{
		ICE_FLTR_PTYPE_NONF_IPV4_SCTP,
		sizeof(ice_fdir_sctpv4_pkt),
		ice_fdir_sctpv4_pkt,
	},
	{
		ICE_FLTR_PTYPE_NONF_IPV4_OTHER,
		sizeof(ice_fdir_ipv4_pkt),
		ice_fdir_ipv4_pkt,
	},
	{
		ICE_FLTR_PTYPE_NONF_IPV6_TCP,
		sizeof(ice_fdir_tcpv6_pkt),
		ice_fdir_tcpv6_pkt,
	},
	{
		ICE_FLTR_PTYPE_NONF_IPV6_UDP,
		sizeof(ice_fdir_udpv6_pkt),
		ice_fdir_udpv6_pkt,
	},
	{
		ICE_FLTR_PTYPE_NONF_IPV6_SCTP,
		sizeof(ice_fdir_sctpv6_pkt),
		ice_fdir_sctpv6_pkt,
	},
	{
		ICE_FLTR_PTYPE_NONF_IPV6_OTHER,
		sizeof(ice_fdir_ipv6_pkt),
		ice_fdir_ipv6_pkt,
	},
};

#define ICE_FDIR_NUM_PKT ARRAY_SIZE(ice_fdir_pkt)

/* Flow Direcotr (FD) filter program descriptor Context */
static const struct ice_ctx_ele ice_fd_fltr_desc_ctx_info[] = {
					   /* Field		Width	LSB */
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, qindex,		11,	0),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, comp_q,		1,	11),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, comp_report,	2,	12),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, fd_space,		2,	14),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, cnt_index,		13,	16),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, cnt_ena,		2,	29),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, evict_ena,		1,	31),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, toq,		3,	32),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, toq_prio,		3,	35),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, dpu_recipe,		2,	38),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, drop,		1,	40),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, flex_prio,		3,	41),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, flex_mdid,		4,	44),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, flex_val,		16,	48),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, dtype,		4,	64),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, pcmd,		1,	68),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, desc_prof_prio,	3,	69),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, desc_prof,		6,	72),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, fd_vsi,		10,	78),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, swap,		1,	88),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, fdid_prio,		3,	89),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, fdid_mdid,		4,	92),
	ICE_CTX_STORE(ice_fd_fltr_desc_ctx, fdid,		32,	96),
	{ 0 }
};

/**
 * ice_set_dflt_val_fd_desc
 * @fd_fltr_ctx: pointer to fd filter descriptor
 */
void
ice_set_dflt_val_fd_desc(struct ice_fd_fltr_desc_ctx *fd_fltr_ctx)
{
	fd_fltr_ctx->comp_q = ICE_FXD_FLTR_QW0_COMP_Q_ZERO;
	fd_fltr_ctx->comp_report = ICE_FXD_FLTR_QW0_COMP_REPORT_SW_FAIL;
	fd_fltr_ctx->fd_space = ICE_FXD_FLTR_QW0_FD_SPACE_GUAR_BEST;
	fd_fltr_ctx->cnt_ena = ICE_FXD_FLTR_QW0_STAT_ENA_PKTS;
	fd_fltr_ctx->evict_ena = ICE_FXD_FLTR_QW0_EVICT_ENA_TRUE;
	fd_fltr_ctx->toq = ICE_FXD_FLTR_QW0_TO_Q_EQUALS_QINDEX;
	fd_fltr_ctx->toq_prio = ICE_FXD_FLTR_QW0_TO_Q_PRIO1;
	fd_fltr_ctx->dpu_recipe = ICE_FXD_FLTR_QW0_DPU_RECIPE_DFLT;
	fd_fltr_ctx->drop = ICE_FXD_FLTR_QW0_DROP_NO;
	fd_fltr_ctx->flex_prio = ICE_FXD_FLTR_QW0_FLEX_PRI_NONE;
	fd_fltr_ctx->flex_mdid = ICE_FXD_FLTR_QW0_FLEX_MDID0;
	fd_fltr_ctx->flex_val = ICE_FXD_FLTR_QW0_FLEX_VAL0;
	fd_fltr_ctx->dtype = ICE_TX_DESC_DTYPE_FLTR_PROG;
	fd_fltr_ctx->desc_prof_prio = ICE_FXD_FLTR_QW1_PROF_PRIO_ZERO;
	fd_fltr_ctx->desc_prof = ICE_FXD_FLTR_QW1_PROF_ZERO;
	fd_fltr_ctx->swap = ICE_FXD_FLTR_QW1_SWAP_SET;
	fd_fltr_ctx->fdid_prio = ICE_FXD_FLTR_QW1_FDID_PRI_ONE;
	fd_fltr_ctx->fdid_mdid = ICE_FXD_FLTR_QW1_FDID_MDID_FD;
	fd_fltr_ctx->fdid = ICE_FXD_FLTR_QW1_FDID_ZERO;
}

/**
 * ice_fdir_get_prgm_desc - set a fdir descriptor from a fdir filter struct
 * @hw: pointer to the hardware structure
 * @input: filter
 * @fdesc: filter descriptor
 * @add: if add is true, this is an add operation, false implies delete
 */
void
ice_fdir_get_prgm_desc(struct ice_hw *hw, struct ice_fdir_fltr *input,
		       struct ice_fltr_desc *fdesc, bool add)
{
	struct ice_fd_fltr_desc_ctx fdir_fltr_ctx = { 0 };

	/* set default context info */
	ice_set_dflt_val_fd_desc(&fdir_fltr_ctx);

	/* change sideband filtering values */
	fdir_fltr_ctx.fdid = input->fltr_id;
	if (input->dest_ctl == ICE_FLTR_PRGM_DESC_DEST_DROP_PKT) {
		fdir_fltr_ctx.drop = ICE_FXD_FLTR_QW0_DROP_YES;
		fdir_fltr_ctx.qindex = 0;
	} else {
		fdir_fltr_ctx.drop = ICE_FXD_FLTR_QW0_DROP_NO;
		fdir_fltr_ctx.qindex = input->q_index;
	}
	fdir_fltr_ctx.cnt_ena = ICE_FXD_FLTR_QW0_STAT_ENA_PKTS;
	fdir_fltr_ctx.cnt_index = input->cnt_index;
	fdir_fltr_ctx.fd_vsi = ice_get_hw_vsi_num(hw, input->dest_vsi);
	fdir_fltr_ctx.evict_ena = ICE_FXD_FLTR_QW0_EVICT_ENA_FALSE;
	fdir_fltr_ctx.toq_prio = 3;
	fdir_fltr_ctx.pcmd = (add) ? ICE_FXD_FLTR_QW1_PCMD_ADD :
		ICE_FXD_FLTR_QW1_PCMD_REMOVE;
	fdir_fltr_ctx.swap = ICE_FXD_FLTR_QW1_SWAP_NOT_SET;
	fdir_fltr_ctx.comp_q = ICE_FXD_FLTR_QW0_COMP_Q_ZERO;
	fdir_fltr_ctx.comp_report = ICE_FXD_FLTR_QW0_COMP_REPORT_SW;
	fdir_fltr_ctx.fdid_prio = 3;
	fdir_fltr_ctx.desc_prof = 1;
	fdir_fltr_ctx.desc_prof_prio = 3;
	ice_set_fd_desc_val(&fdir_fltr_ctx, fdesc);
}

/**
 * ice_set_fd_desc_val
 * @fd_fltr_ctx: pointer to fd filter descriptor context
 * @fdir_desc: populated with fd filter descriptor values
 */
void
ice_set_fd_desc_val(struct ice_fd_fltr_desc_ctx *fd_fltr_ctx,
		    struct ice_fltr_desc *fdir_desc)
{
	u64 ctx_buf[2] = { 0 };

	ice_set_ctx((u8 *)fd_fltr_ctx, (u8 *)ctx_buf,
		    ice_fd_fltr_desc_ctx_info);
	fdir_desc->qidx_compq_space_stat = CPU_TO_LE64(ctx_buf[0]);
	fdir_desc->dtype_cmd_vsi_fdid = CPU_TO_LE64(ctx_buf[1]);
}

/**
 * ice_alloc_fd_res_cntr - obtain counter resource for FD type
 * @hw: pointer to the hardware structure
 * @cntr_id: returns counter index
 */
enum ice_status ice_alloc_fd_res_cntr(struct ice_hw *hw, u16 *cntr_id)
{
	return ice_alloc_res_cntr(hw, ICE_AQC_RES_TYPE_FDIR_COUNTER_BLOCK,
				  ICE_AQC_RES_TYPE_FLAG_DEDICATED, 1, cntr_id);
}

/**
 * ice_free_fd_res_cntr - Free counter resource for FD type
 * @hw: pointer to the hardware structure
 * @cntr_id: counter index to be freed
 */
enum ice_status ice_free_fd_res_cntr(struct ice_hw *hw, u16 cntr_id)
{
	return ice_free_res_cntr(hw, ICE_AQC_RES_TYPE_FDIR_COUNTER_BLOCK,
				 ICE_AQC_RES_TYPE_FLAG_DEDICATED, 1, cntr_id);
}

/**
 * ice_alloc_fd_guar_item - allocate resource for FD guaranteed entries
 * @hw: pointer to the hardware structure
 * @cntr_id: returns counter index
 * @num_fltr: number of filter entries to be allocated
 */
enum ice_status
ice_alloc_fd_guar_item(struct ice_hw *hw, u16 *cntr_id, u16 num_fltr)
{
	return ice_alloc_res_cntr(hw, ICE_AQC_RES_TYPE_FDIR_GUARANTEED_ENTRIES,
				  ICE_AQC_RES_TYPE_FLAG_DEDICATED, num_fltr,
				  cntr_id);
}

/**
 * ice_free_fd_guar_item - Free flow director guaranteed entries
 * @hw: pointer to the hardware structure
 * @cntr_id: counter index that needs to be freed
 * @num_fltr: number of filters to be freed
 */
enum ice_status
ice_free_fd_guar_item(struct ice_hw *hw, u16 cntr_id, u16 num_fltr)
{
	return ice_free_res_cntr(hw, ICE_AQC_RES_TYPE_FDIR_GUARANTEED_ENTRIES,
				 ICE_AQC_RES_TYPE_FLAG_DEDICATED, num_fltr,
				 cntr_id);
}

/**
 * ice_alloc_fd_shrd_item - allocate resource for flow director shared entries
 * @hw: pointer to the hardware structure
 * @cntr_id: returns counter index
 * @num_fltr: number of filter entries to be allocated
 */
enum ice_status
ice_alloc_fd_shrd_item(struct ice_hw *hw, u16 *cntr_id, u16 num_fltr)
{
	return ice_alloc_res_cntr(hw, ICE_AQC_RES_TYPE_FDIR_SHARED_ENTRIES,
				  ICE_AQC_RES_TYPE_FLAG_DEDICATED, num_fltr,
				  cntr_id);
}

/**
 * ice_free_fd_shrd_item - Free flow director shared entries
 * @hw: pointer to the hardware structure
 * @cntr_id: counter index that needs to be freed
 * @num_fltr: number of filters to be freed
 */
enum ice_status
ice_free_fd_shrd_item(struct ice_hw *hw, u16 cntr_id, u16 num_fltr)
{
	return ice_free_res_cntr(hw, ICE_AQC_RES_TYPE_FDIR_SHARED_ENTRIES,
				 ICE_AQC_RES_TYPE_FLAG_DEDICATED, num_fltr,
				 cntr_id);
}

/**
 * ice_get_fdir_cnt_all - get the number of Flow Director filters
 * @hw: hardware data structure
 *
 * Returns the number of filters available on device
 */
int ice_get_fdir_cnt_all(struct ice_hw *hw)
{
	return hw->func_caps.fd_fltr_guar +
	       hw->func_caps.fd_fltr_best_effort;
}

/**
 * ice_pkt_insert_ipv6_addr - insert a be32 IPv6 address into a memory buffer.
 * @pkt: packet buffer
 * @offset: offset into buffer
 * @addr: IPv6 address to convert and insert into pkt at offset
 */
static void ice_pkt_insert_ipv6_addr(u8 *pkt, int offset, __be32 *addr)
{
	int idx;

	for (idx = 0; idx < ICE_IPV6_ADDR_LEN_AS_U32; idx++)
		ice_memcpy(pkt + offset + idx * sizeof(*addr), &addr[idx],
			   sizeof(*addr), ICE_NONDMA_TO_NONDMA);
}

/**
 * ice_pkt_insert_u16 - insert a be16 value into a memory buffer.
 * @pkt: packet buffer
 * @offset: offset into buffer
 * @data: 16 bit value to convert and insert into pkt at offset
 */
static void ice_pkt_insert_u16(u8 *pkt, int offset, __be16 data)
{
	ice_memcpy(pkt + offset, &data, sizeof(data), ICE_NONDMA_TO_NONDMA);
}

/**
 * ice_pkt_insert_u32 - insert a be32 value into a memory buffer.
 * @pkt: packet buffer
 * @offset: offset into buffer
 * @data: 32 bit value to convert and insert into pkt at offset
 */
static void ice_pkt_insert_u32(u8 *pkt, int offset, __be32 data)
{
	ice_memcpy(pkt + offset, &data, sizeof(data), ICE_NONDMA_TO_NONDMA);
}

/**
 * ice_fdir_get_prgm_pkt - generate a dummy packet
 * @input: flow director filter data structure
 * @pkt: pointer to return filter packet
 * @frag: generate a fragment packet
 */
enum ice_status
ice_fdir_get_prgm_pkt(struct ice_fdir_fltr *input, u8 *pkt, bool frag)
{
	enum ice_fltr_ptype flow;
	u16 idx;

	if (input->flow_type == ICE_FLTR_PTYPE_NONF_IPV4_OTHER) {
		switch (input->ip.v4.proto) {
		case ICE_IP_PROTO_TCP:
			flow = ICE_FLTR_PTYPE_NONF_IPV4_TCP;
			break;
		case ICE_IP_PROTO_UDP:
			flow = ICE_FLTR_PTYPE_NONF_IPV4_UDP;
			break;
		case ICE_IP_PROTO_SCTP:
			flow = ICE_FLTR_PTYPE_NONF_IPV4_SCTP;
			break;
		case ICE_IP_PROTO_IP:
			flow = ICE_FLTR_PTYPE_NONF_IPV4_OTHER;
			break;
		default:
			return ICE_ERR_PARAM;
		}
	} else if (input->flow_type == ICE_FLTR_PTYPE_NONF_IPV6_OTHER) {
		switch (input->ip.v6.proto) {
		case ICE_IP_PROTO_TCP:
			flow = ICE_FLTR_PTYPE_NONF_IPV6_TCP;
			break;
		case ICE_IP_PROTO_UDP:
			flow = ICE_FLTR_PTYPE_NONF_IPV6_UDP;
			break;
		case ICE_IP_PROTO_SCTP:
			flow = ICE_FLTR_PTYPE_NONF_IPV6_SCTP;
			break;
		case ICE_IP_PROTO_IP:
			flow = ICE_FLTR_PTYPE_NONF_IPV6_OTHER;
			break;
		default:
			return ICE_ERR_PARAM;
		}
	} else {
		flow = input->flow_type;
	}

	for (idx = 0; idx < ICE_FDIR_NUM_PKT; idx++)
		if (ice_fdir_pkt[idx].flow == flow)
			break;
	if (idx == ICE_FDIR_NUM_PKT)
		return ICE_ERR_PARAM;
	ice_memcpy(pkt, ice_fdir_pkt[idx].pkt, ice_fdir_pkt[idx].pkt_len,
		   ICE_NONDMA_TO_NONDMA);

	switch (flow) {
	case ICE_FLTR_PTYPE_NONF_IPV4_TCP:
		ice_pkt_insert_u32(pkt, ICE_IPV4_DST_ADDR_OFFSET,
				   input->ip.v4.dst_ip);
		ice_pkt_insert_u16(pkt, ICE_IPV4_TCP_DST_PORT_OFFSET,
				   input->ip.v4.dst_port);
		ice_pkt_insert_u32(pkt, ICE_IPV4_SRC_ADDR_OFFSET,
				   input->ip.v4.src_ip);
		ice_pkt_insert_u16(pkt, ICE_IPV4_TCP_SRC_PORT_OFFSET,
				   input->ip.v4.src_port);
		if (frag)
			pkt[20] = ICE_FDIR_IPV4_PKT_FLAG_DF;
		break;
	case ICE_FLTR_PTYPE_NONF_IPV4_UDP:
		ice_pkt_insert_u32(pkt, ICE_IPV4_DST_ADDR_OFFSET,
				   input->ip.v4.dst_ip);
		ice_pkt_insert_u16(pkt, ICE_IPV4_UDP_DST_PORT_OFFSET,
				   input->ip.v4.dst_port);
		ice_pkt_insert_u32(pkt, ICE_IPV4_SRC_ADDR_OFFSET,
				   input->ip.v4.src_ip);
		ice_pkt_insert_u16(pkt, ICE_IPV4_UDP_SRC_PORT_OFFSET,
				   input->ip.v4.src_port);
		break;
	case ICE_FLTR_PTYPE_NONF_IPV4_SCTP:
		ice_pkt_insert_u32(pkt, ICE_IPV4_DST_ADDR_OFFSET,
				   input->ip.v4.dst_ip);
		ice_pkt_insert_u16(pkt, ICE_IPV4_SCTP_DST_PORT_OFFSET,
				   input->ip.v4.dst_port);
		ice_pkt_insert_u32(pkt, ICE_IPV4_SRC_ADDR_OFFSET,
				   input->ip.v4.src_ip);
		ice_pkt_insert_u16(pkt, ICE_IPV4_SCTP_SRC_PORT_OFFSET,
				   input->ip.v4.src_port);
		break;
	case ICE_FLTR_PTYPE_NONF_IPV4_OTHER:
		ice_pkt_insert_u32(pkt, ICE_IPV4_DST_ADDR_OFFSET,
				   input->ip.v4.dst_ip);
		ice_pkt_insert_u32(pkt, ICE_IPV4_SRC_ADDR_OFFSET,
				   input->ip.v4.src_ip);
		ice_pkt_insert_u16(pkt, ICE_IPV4_PROTO_OFFSET, 0);
		break;
	case ICE_FLTR_PTYPE_NONF_IPV6_TCP:
		ice_pkt_insert_ipv6_addr(pkt, ICE_IPV6_DST_ADDR_OFFSET,
					 input->ip.v6.dst_ip);
		ice_pkt_insert_ipv6_addr(pkt, ICE_IPV6_SRC_ADDR_OFFSET,
					 input->ip.v6.src_ip);
		ice_pkt_insert_u16(pkt, ICE_IPV6_TCP_DST_PORT_OFFSET,
				   input->ip.v6.dst_port);
		ice_pkt_insert_u16(pkt, ICE_IPV6_TCP_SRC_PORT_OFFSET,
				   input->ip.v6.src_port);
		break;
	case ICE_FLTR_PTYPE_NONF_IPV6_UDP:
		ice_pkt_insert_ipv6_addr(pkt, ICE_IPV6_DST_ADDR_OFFSET,
					 input->ip.v6.dst_ip);
		ice_pkt_insert_ipv6_addr(pkt, ICE_IPV6_SRC_ADDR_OFFSET,
					 input->ip.v6.src_ip);
		ice_pkt_insert_u16(pkt, ICE_IPV6_UDP_DST_PORT_OFFSET,
				   input->ip.v6.dst_port);
		ice_pkt_insert_u16(pkt, ICE_IPV6_UDP_SRC_PORT_OFFSET,
				   input->ip.v6.src_port);
		break;
	case ICE_FLTR_PTYPE_NONF_IPV6_SCTP:
		ice_pkt_insert_ipv6_addr(pkt, ICE_IPV6_DST_ADDR_OFFSET,
					 input->ip.v6.dst_ip);
		ice_pkt_insert_ipv6_addr(pkt, ICE_IPV6_SRC_ADDR_OFFSET,
					 input->ip.v6.src_ip);
		ice_pkt_insert_u16(pkt, ICE_IPV6_SCTP_DST_PORT_OFFSET,
				   input->ip.v6.dst_port);
		ice_pkt_insert_u16(pkt, ICE_IPV6_SCTP_SRC_PORT_OFFSET,
				   input->ip.v6.src_port);
		break;
	case ICE_FLTR_PTYPE_NONF_IPV6_OTHER:
		ice_pkt_insert_ipv6_addr(pkt, ICE_IPV6_DST_ADDR_OFFSET,
					 input->ip.v6.dst_ip);
		ice_pkt_insert_ipv6_addr(pkt, ICE_IPV6_SRC_ADDR_OFFSET,
					 input->ip.v6.src_ip);
		break;
	default:
		return ICE_ERR_PARAM;
	}

	if (input->flex_fltr)
		ice_pkt_insert_u16(pkt, input->flex_offset, input->flex_word);

	return ICE_SUCCESS;
}

/**
 * ice_fdir_has_frag - does flow type have 2 ptypes
 * @flow: flow ptype
 *
 * returns true is there is a fragment packet for this ptype
 */
bool ice_fdir_has_frag(enum ice_fltr_ptype flow)
{
	if (flow == ICE_FLTR_PTYPE_NONF_IPV4_OTHER)
		return true;
	else
		return false;
}

/**
 * ice_fdir_find_by_idx - find filter with idx
 * @hw: pointer to hardware structure
 * @fltr_idx: index to find.
 *
 * Returns pointer to filter if found or null
 */
struct ice_fdir_fltr *
ice_fdir_find_fltr_by_idx(struct ice_hw *hw, u32 fltr_idx)
{
	struct ice_fdir_fltr *rule = NULL;

	LIST_FOR_EACH_ENTRY(rule, &hw->fdir_list_head, ice_fdir_fltr,
			    fltr_node) {
		/* rule ID found in the list */
		if (fltr_idx == rule->fltr_id)
			return rule;
		if (fltr_idx < rule->fltr_id)
			break;
	}
	return NULL;
}

/**
 * ice_fdir_list_add_fltr - add a new node to the flow director filter list
 * @hw: hardware structure
 * @fltr: filter node to add to structure
 */
void ice_fdir_list_add_fltr(struct ice_hw *hw, struct ice_fdir_fltr *fltr)
{
	struct ice_fdir_fltr *rule, *parent = NULL;

	LIST_FOR_EACH_ENTRY(rule, &hw->fdir_list_head, ice_fdir_fltr,
			    fltr_node) {
		/* rule ID found or pass its spot in the list */
		if (rule->fltr_id >= fltr->fltr_id)
			break;
		parent = rule;
	}

	if (parent)
		LIST_ADD_AFTER(&fltr->fltr_node, &parent->fltr_node);
	else
		LIST_ADD(&fltr->fltr_node, &hw->fdir_list_head);
}

/**
 * ice_fdir_update_cntrs - increment / decrement filter counter
 * @hw: pointer to hardware structure
 * @flow: filter flow type
 * @add: true implies filters added
 */
void
ice_fdir_update_cntrs(struct ice_hw *hw, enum ice_fltr_ptype flow, bool add)
{
	int incr;

	incr = (add) ? 1 : -1;
	hw->fdir_active_fltr += incr;
	if (flow == ICE_FLTR_PTYPE_NONF_NONE || flow >= ICE_FLTR_PTYPE_MAX)
		ice_debug(hw, ICE_DBG_SW, "Unknown filter type %d\n", flow);
	else
		hw->fdir_fltr_cnt[flow] += incr;
}

/**
 * ice_cmp_ipv6_addr - compare 2 IP v6 addresses
 * @a: IP v6 address
 * @b: IP v6 address
 *
 * Returns 0 on equal, returns non-0 if different
 */
static int ice_cmp_ipv6_addr(__be32 *a, __be32 *b)
{
	return memcmp(a, b, 4 * sizeof(__be32));
}

/**
 * ice_fdir_comp_ipv6_rules - compare 2 filters
 * @a: a Flow Director filter data structure
 * @b: a Flow Director filter data structure
 * @v6: bool true if v6 filter
 *
 * Returns true if the filters match
 */
static bool
ice_fdir_comp_rules(struct ice_fdir_fltr *a,  struct ice_fdir_fltr *b, bool v6)
{
	enum ice_fltr_ptype flow_type = a->flow_type;

	/* The calling function already checks that the two filters have the
	 * same flow_type.
	 */
	if (!v6) {
		if (flow_type == ICE_FLTR_PTYPE_NONF_IPV4_TCP ||
		    flow_type == ICE_FLTR_PTYPE_NONF_IPV4_UDP ||
		    flow_type == ICE_FLTR_PTYPE_NONF_IPV4_SCTP) {
			if (a->ip.v4.dst_ip == b->ip.v4.dst_ip &&
			    a->ip.v4.src_ip == b->ip.v4.src_ip &&
			    a->ip.v4.dst_port == b->ip.v4.dst_port &&
			    a->ip.v4.src_port == b->ip.v4.src_port)
				return true;
		} else if (flow_type == ICE_FLTR_PTYPE_NONF_IPV4_OTHER) {
			if (a->ip.v4.dst_ip == b->ip.v4.dst_ip &&
			    a->ip.v4.src_ip == b->ip.v4.src_ip &&
			    a->ip.v4.l4_header == b->ip.v4.l4_header &&
			    a->ip.v4.proto == b->ip.v4.proto &&
			    a->ip.v4.ip_ver == b->ip.v4.ip_ver &&
			    a->ip.v4.tos == b->ip.v4.tos)
				return true;
		}
	} else {
		if (flow_type == ICE_FLTR_PTYPE_NONF_IPV6_UDP ||
		    flow_type == ICE_FLTR_PTYPE_NONF_IPV6_TCP ||
		    flow_type == ICE_FLTR_PTYPE_NONF_IPV6_SCTP) {
			if (a->ip.v6.dst_port == b->ip.v6.dst_port &&
			    a->ip.v6.src_port == b->ip.v6.src_port &&
			    !ice_cmp_ipv6_addr(a->ip.v6.dst_ip,
					       b->ip.v6.dst_ip) &&
			    !ice_cmp_ipv6_addr(a->ip.v6.src_ip,
					       b->ip.v6.src_ip))
				return true;
		} else if (flow_type == ICE_FLTR_PTYPE_NONF_IPV6_OTHER) {
			if (a->ip.v6.dst_port == b->ip.v6.dst_port &&
			    a->ip.v6.src_port == b->ip.v6.src_port)
				return true;
		}
	}

	return false;
}

/**
 * ice_fdir_is_dup_fltr - test if filter is already in list for PF
 * @hw: hardware data structure
 * @input: Flow Director filter data structure
 *
 * Returns true if the filter is found in the list
 */
bool ice_fdir_is_dup_fltr(struct ice_hw *hw, struct ice_fdir_fltr *input)
{
	enum ice_fltr_ptype flow_type;
	struct ice_fdir_fltr *rule;
	bool ret = false;

	rule = NULL;

	LIST_FOR_EACH_ENTRY(rule, &hw->fdir_list_head, ice_fdir_fltr,
			    fltr_node) {
		if (rule->flow_type == input->flow_type) {
			flow_type = input->flow_type;
			if (flow_type == ICE_FLTR_PTYPE_NONF_IPV4_TCP ||
			    flow_type == ICE_FLTR_PTYPE_NONF_IPV4_UDP ||
			    flow_type == ICE_FLTR_PTYPE_NONF_IPV4_SCTP ||
			    flow_type == ICE_FLTR_PTYPE_NONF_IPV4_OTHER)
				ret = ice_fdir_comp_rules(rule, input, false);
			else
				ret = ice_fdir_comp_rules(rule, input, true);
			if (ret)
				break;
		}
	}

	return ret;
}

/**
 * ice_clear_vsi_fd_table - admin command to clear FD table for a VSI
 * @hw: hardware data structure
 * @vsi_num: vsi_num (HW VSI num)
 *
 * Clears FD table entries by issuing admin command (direct, 0x0B06)
 * Must to pass valid vsi_num as returned by "AddVSI".
 */
enum ice_status ice_clear_vsi_fd_table(struct ice_hw *hw, u16 vsi_num)
{
	struct ice_aqc_clear_fd_table *cmd;
	struct ice_aq_desc desc;

	cmd = &desc.params.clear_fd_table;
	ice_fill_dflt_direct_cmd_desc(&desc, ice_aqc_opc_clear_fd_table);
	cmd->clear_type = CL_FD_VM_VF_TYPE_VSI_IDX;

	cmd->vsi_index = CPU_TO_LE16(vsi_num);
	return ice_aq_send_cmd(hw, &desc, NULL, 0, NULL);
}

/**
 * ice_clear_pf_fd_table - admin command to clear FD table for PF
 * @hw: hardware data structure
 *
 * Clears FD table entries for a PF by issuing admin command (direct, 0x0B06)
 */
enum ice_status ice_clear_pf_fd_table(struct ice_hw *hw)
{
	struct ice_aqc_clear_fd_table *cmd;
	struct ice_aq_desc desc;

	cmd = &desc.params.clear_fd_table;
	ice_fill_dflt_direct_cmd_desc(&desc, ice_aqc_opc_clear_fd_table);
	cmd->clear_type = CL_FD_VM_VF_TYPE_PF_IDX;
	/* vsi_index must be 0 to clear FD table for a PF */
	cmd->vsi_index = CPU_TO_LE16(0);

	return ice_aq_send_cmd(hw, &desc, NULL, 0, NULL);
}
