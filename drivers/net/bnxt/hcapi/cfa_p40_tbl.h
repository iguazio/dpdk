/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019-2020 Broadcom
 * All rights reserved.
 */
/*
 * Name:  cfa_p40_tbl.h
 *
 * Description: header for SWE based on Truflow
 *
 * Date:  12/16/19 17:18:12
 *
 * Note:  This file was originally generated by tflib_decode.py.
 *        Remainder is hand coded due to lack of availability of xml for
 *        additional tables at this time (EEM Record and union fields)
 *
 **/
#ifndef _CFA_P40_TBL_H_
#define _CFA_P40_TBL_H_

#include "cfa_p40_hw.h"

#include "hcapi_cfa_defs.h"

const struct hcapi_cfa_field cfa_p40_prof_l2_ctxt_tcam_layout[] = {
	{CFA_P40_PROF_L2_CTXT_TCAM_VALID_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_VALID_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_KEY_TYPE_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_KEY_TYPE_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_TUN_HDR_TYPE_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_TUN_HDR_TYPE_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_T_L2_NUMTAGS_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_T_L2_NUMTAGS_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_L2_NUMTAGS_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_L2_NUMTAGS_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_MAC1_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_MAC1_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_T_OVID_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_T_OVID_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_T_IVID_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_T_IVID_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_SPARIF_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_SPARIF_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_SVIF_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_SVIF_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_MAC0_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_MAC0_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_OVID_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_OVID_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_IVID_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_IVID_NUM_BITS},
};

const struct hcapi_cfa_field cfa_p40_act_veb_tcam_layout[] = {
	{CFA_P40_ACT_VEB_TCAM_VALID_BITPOS,
	 CFA_P40_ACT_VEB_TCAM_VALID_NUM_BITS},
	{CFA_P40_ACT_VEB_TCAM_RESERVED_BITPOS,
	 CFA_P40_ACT_VEB_TCAM_RESERVED_NUM_BITS},
	{CFA_P40_ACT_VEB_TCAM_PARIF_IN_BITPOS,
	 CFA_P40_ACT_VEB_TCAM_PARIF_IN_NUM_BITS},
	{CFA_P40_ACT_VEB_TCAM_NUM_VTAGS_BITPOS,
	 CFA_P40_ACT_VEB_TCAM_NUM_VTAGS_NUM_BITS},
	{CFA_P40_ACT_VEB_TCAM_MAC_BITPOS,
	 CFA_P40_ACT_VEB_TCAM_MAC_NUM_BITS},
	{CFA_P40_ACT_VEB_TCAM_OVID_BITPOS,
	 CFA_P40_ACT_VEB_TCAM_OVID_NUM_BITS},
	{CFA_P40_ACT_VEB_TCAM_IVID_BITPOS,
	 CFA_P40_ACT_VEB_TCAM_IVID_NUM_BITS},
};

const struct hcapi_cfa_field cfa_p40_lkup_tcam_record_mem_layout[] = {
	{CFA_P40_LKUP_TCAM_RECORD_MEM_VALID_BITPOS,
	 CFA_P40_LKUP_TCAM_RECORD_MEM_VALID_NUM_BITS},
	{CFA_P40_LKUP_TCAM_RECORD_MEM_ACT_REC_PTR_BITPOS,
	 CFA_P40_LKUP_TCAM_RECORD_MEM_ACT_REC_PTR_NUM_BITS},
	{CFA_P40_LKUP_TCAM_RECORD_MEM_STRENGTH_BITPOS,
	 CFA_P40_LKUP_TCAM_RECORD_MEM_STRENGTH_NUM_BITS},
};

const struct hcapi_cfa_field cfa_p40_prof_ctxt_remap_mem_layout[] = {
	{CFA_P40_PROF_CTXT_REMAP_MEM_TPID_ANTI_SPOOF_CTL_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_TPID_ANTI_SPOOF_CTL_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_PRI_ANTI_SPOOF_CTL_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_PRI_ANTI_SPOOF_CTL_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_BYP_SP_LKUP_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_BYP_SP_LKUP_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_SP_REC_PTR_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_SP_REC_PTR_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_BD_ACT_EN_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_BD_ACT_EN_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_DEFAULT_TPID_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_DEFAULT_TPID_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_ALLOWED_TPID_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_ALLOWED_TPID_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_DEFAULT_PRI_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_DEFAULT_PRI_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_ALLOWED_PRI_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_ALLOWED_PRI_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_PARIF_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_PARIF_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_BYP_LKUP_EN_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_BYP_LKUP_EN_NUM_BITS},
	/* Fields below not generated through automation */
	{CFA_P40_PROF_CTXT_REMAP_MEM_PROF_VNIC_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_PROF_VNIC_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_PROF_FUNC_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_PROF_FUNC_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_L2_CTXT_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_L2_CTXT_NUM_BITS},
	{CFA_P40_PROF_CTXT_REMAP_MEM_ARP_BITPOS,
	 CFA_P40_PROF_CTXT_REMAP_MEM_ARP_NUM_BITS},
};

const struct hcapi_cfa_field cfa_p40_prof_profile_tcam_remap_mem_layout[] = {
	{CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_PL_BYP_LKUP_EN_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_PL_BYP_LKUP_EN_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_EM_SEARCH_ENB_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_EM_SEARCH_ENB_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_EM_PROFILE_ID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_EM_PROFILE_ID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_EM_KEY_ID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_EM_KEY_ID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_EM_KEY_MASK_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_EM_KEY_MASK_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_TCAM_SEARCH_ENB_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_TCAM_SEARCH_ENB_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_TCAM_PROFILE_ID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_TCAM_PROFILE_ID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_TCAM_KEY_ID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_TCAM_KEY_ID_NUM_BITS},
	/* Fields below not generated through automation */
	{CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_BYPASS_OPT_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_BYPASS_OPT_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_ACT_REC_PTR_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_REMAP_MEM_ACT_REC_PTR_NUM_BITS},
};

const struct hcapi_cfa_field cfa_p40_prof_profile_tcam_layout[] = {
	{CFA_P40_PROF_PROFILE_TCAM_VALID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_VALID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_PKT_TYPE_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_PKT_TYPE_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_RECYCLE_CNT_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_RECYCLE_CNT_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_AGG_ERROR_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_AGG_ERROR_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_PROF_FUNC_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_PROF_FUNC_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_RESERVED_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_RESERVED_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_HREC_NEXT_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_HREC_NEXT_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL2_HDR_VALID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL2_HDR_VALID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL2_HDR_TYPE_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL2_HDR_TYPE_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL2_UC_MC_BC_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL2_UC_MC_BC_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL2_VTAG_PRESENT_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL2_VTAG_PRESENT_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL2_TWO_VTAGS_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL2_TWO_VTAGS_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL3_VALID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL3_VALID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL3_ERROR_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL3_ERROR_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL3_HDR_TYPE_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL3_HDR_TYPE_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL3_HDR_ISIP_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL3_HDR_ISIP_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL3_IPV6_CMP_SRC_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL3_IPV6_CMP_SRC_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL3_IPV6_CMP_DEST_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL3_IPV6_CMP_DEST_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL4_HDR_VALID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL4_HDR_VALID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL4_HDR_ERROR_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL4_HDR_ERROR_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL4_HDR_TYPE_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL4_HDR_TYPE_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TL4_HDR_IS_UDP_TCP_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TL4_HDR_IS_UDP_TCP_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TUN_HDR_VALID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TUN_HDR_VALID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TUN_HDR_ERR_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TUN_HDR_ERR_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TUN_HDR_TYPE_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TUN_HDR_TYPE_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_TUN_HDR_FLAGS_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_TUN_HDR_FLAGS_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L2_HDR_VALID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L2_HDR_VALID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L2_HDR_ERROR_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L2_HDR_ERROR_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L2_HDR_TYPE_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L2_HDR_TYPE_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L2_UC_MC_BC_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L2_UC_MC_BC_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L2_VTAG_PRESENT_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L2_VTAG_PRESENT_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L2_TWO_VTAGS_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L2_TWO_VTAGS_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L3_VALID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L3_VALID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L3_ERROR_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L3_ERROR_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L3_HDR_TYPE_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L3_HDR_TYPE_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L3_HDR_ISIP_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L3_HDR_ISIP_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L3_IPV6_CMP_SRC_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L3_IPV6_CMP_SRC_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L3_IPV6_CMP_DEST_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L3_IPV6_CMP_DEST_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L4_HDR_VALID_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L4_HDR_VALID_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L4_HDR_ERROR_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L4_HDR_ERROR_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L4_HDR_TYPE_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L4_HDR_TYPE_NUM_BITS},
	{CFA_P40_PROF_PROFILE_TCAM_L4_HDR_IS_UDP_TCP_BITPOS,
	 CFA_P40_PROF_PROFILE_TCAM_L4_HDR_IS_UDP_TCP_NUM_BITS},
};

/**************************************************************************/
/**
 * Non-autogenerated fields
 */

const struct hcapi_cfa_field cfa_p40_eem_key_tbl_layout[] = {
	{CFA_P40_EEM_KEY_TBL_VALID_BITPOS,
	 CFA_P40_EEM_KEY_TBL_VALID_NUM_BITS},

	{CFA_P40_EEM_KEY_TBL_L1_CACHEABLE_BITPOS,
	 CFA_P40_EEM_KEY_TBL_L1_CACHEABLE_NUM_BITS},

	{CFA_P40_EEM_KEY_TBL_STRENGTH_BITPOS,
	 CFA_P40_EEM_KEY_TBL_STRENGTH_NUM_BITS},

	{CFA_P40_EEM_KEY_TBL_KEY_SZ_BITPOS,
	 CFA_P40_EEM_KEY_TBL_KEY_SZ_NUM_BITS},

	{CFA_P40_EEM_KEY_TBL_REC_SZ_BITPOS,
	 CFA_P40_EEM_KEY_TBL_REC_SZ_NUM_BITS},

	{CFA_P40_EEM_KEY_TBL_ACT_REC_INT_BITPOS,
	 CFA_P40_EEM_KEY_TBL_ACT_REC_INT_NUM_BITS},

	{CFA_P40_EEM_KEY_TBL_EXT_FLOW_CTR_BITPOS,
	 CFA_P40_EEM_KEY_TBL_EXT_FLOW_CTR_NUM_BITS},

	{CFA_P40_EEM_KEY_TBL_AR_PTR_BITPOS,
	 CFA_P40_EEM_KEY_TBL_AR_PTR_NUM_BITS},

};

const struct hcapi_cfa_field cfa_p40_mirror_tbl_layout[] = {
	{CFA_P40_MIRROR_TBL_SP_PTR_BITPOS,
	 CFA_P40_MIRROR_TBL_SP_PTR_NUM_BITS},

	{CFA_P40_MIRROR_TBL_IGN_DROP_BITPOS,
	 CFA_P40_MIRROR_TBL_IGN_DROP_NUM_BITS},

	{CFA_P40_MIRROR_TBL_COPY_BITPOS,
	 CFA_P40_MIRROR_TBL_COPY_NUM_BITS},

	{CFA_P40_MIRROR_TBL_EN_BITPOS,
	 CFA_P40_MIRROR_TBL_EN_NUM_BITS},

	{CFA_P40_MIRROR_TBL_AR_PTR_BITPOS,
	 CFA_P40_MIRROR_TBL_AR_PTR_NUM_BITS},
};

/* P45 Defines */

const struct hcapi_cfa_field cfa_p45_prof_l2_ctxt_tcam_layout[] = {
	{CFA_P45_PROF_L2_CTXT_TCAM_VALID_BITPOS,
	 CFA_P45_PROF_L2_CTXT_TCAM_VALID_NUM_BITS},
	{CFA_P45_PROF_L2_CTXT_TCAM_SPARIF_BITPOS,
	 CFA_P45_PROF_L2_CTXT_TCAM_SPARIF_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_KEY_TYPE_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_KEY_TYPE_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_TUN_HDR_TYPE_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_TUN_HDR_TYPE_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_T_L2_NUMTAGS_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_T_L2_NUMTAGS_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_L2_NUMTAGS_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_L2_NUMTAGS_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_MAC1_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_MAC1_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_T_OVID_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_T_OVID_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_T_IVID_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_T_IVID_NUM_BITS},
	{CFA_P45_PROF_L2_CTXT_TCAM_SVIF_BITPOS,
	 CFA_P45_PROF_L2_CTXT_TCAM_SVIF_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_MAC0_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_MAC0_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_OVID_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_OVID_NUM_BITS},
	{CFA_P40_PROF_L2_CTXT_TCAM_IVID_BITPOS,
	 CFA_P40_PROF_L2_CTXT_TCAM_IVID_NUM_BITS},
};
#endif /* _CFA_P40_TBL_H_ */
