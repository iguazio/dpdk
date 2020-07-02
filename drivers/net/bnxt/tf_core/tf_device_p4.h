/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019-2020 Broadcom
 * All rights reserved.
 */

#ifndef _TF_DEVICE_P4_H_
#define _TF_DEVICE_P4_H_

#include <cfa_resource_types.h>

#include "tf_core.h"
#include "tf_rm_new.h"

struct tf_rm_element_cfg tf_ident_p4[TF_IDENT_TYPE_MAX] = {
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_L2_CTXT_REMAP },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_PROF_FUNC },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_WC_TCAM_PROF_ID },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_EM_PROF_ID },
	/* CFA_RESOURCE_TYPE_P4_L2_FUNC */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID }
};

struct tf_rm_element_cfg tf_tcam_p4[TF_TCAM_TBL_TYPE_MAX] = {
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_L2_CTXT_TCAM },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_PROF_TCAM },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_WC_TCAM },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_SP_TCAM },
	/* CFA_RESOURCE_TYPE_P4_CT_RULE_TCAM */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_VEB_TCAM */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID }
};

struct tf_rm_element_cfg tf_tbl_p4[TF_TBL_TYPE_MAX] = {
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_FULL_ACTION },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_MCG },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_ENCAP_8B },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_ENCAP_16B },
	/* CFA_RESOURCE_TYPE_P4_ENCAP_32B */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_ENCAP_64B },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_SP_MAC },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_SP_MAC_IPV4 },
	/* CFA_RESOURCE_TYPE_P4_SP_MAC_IPV6 */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_COUNTER_64B },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_NAT_SPORT },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_NAT_DPORT },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_NAT_S_IPV4 },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_NAT_D_IPV4 },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_NAT_S_IPV6 },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_NAT_D_IPV6 },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_METER_PROF },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_METER },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_MIRROR },
	/* CFA_RESOURCE_TYPE_P4_UPAR */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_EPOC */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_METADATA */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_CT_STATE */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_RANGE_PROF */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_RANGE_ENTRY */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_LAG */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_VNIC_SVIF */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_EM_FBK */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_WC_FKB */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	/* CFA_RESOURCE_TYPE_P4_EXT */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID }
};

struct tf_rm_element_cfg tf_em_ext_p4[TF_EM_TBL_TYPE_MAX] = {
	/* CFA_RESOURCE_TYPE_P4_EM_REC */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_TBL_SCOPE },
};

struct tf_rm_element_cfg tf_em_int_p4[TF_EM_TBL_TYPE_MAX] = {
	{ TF_RM_ELEM_CFG_HCAPI, CFA_RESOURCE_TYPE_P4_EM_REC },
	/* CFA_RESOURCE_TYPE_P4_TBL_SCOPE */
	{ TF_RM_ELEM_CFG_NULL, CFA_RESOURCE_TYPE_INVALID },
};

#endif /* _TF_DEVICE_P4_H_ */
