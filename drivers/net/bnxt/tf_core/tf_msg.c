/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019-2020 Broadcom
 * All rights reserved.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "tf_msg_common.h"
#include "tf_device.h"
#include "tf_msg.h"
#include "tf_util.h"
#include "tf_common.h"
#include "tf_session.h"
#include "tfp.h"
#include "hwrm_tf.h"
#include "tf_em.h"

/**
 * Endian converts min and max values from the HW response to the query
 */
#define TF_HW_RESP_TO_QUERY(query, index, response, element) do {            \
	(query)->hw_query[index].min =                                       \
		tfp_le_to_cpu_16(response. element ## _min);                 \
	(query)->hw_query[index].max =                                       \
		tfp_le_to_cpu_16(response. element ## _max);                 \
} while (0)

/**
 * Endian converts the number of entries from the alloc to the request
 */
#define TF_HW_ALLOC_TO_REQ(alloc, index, request, element)                   \
	(request. num_ ## element = tfp_cpu_to_le_16((alloc)->hw_num[index]))

/**
 * Endian converts the start and stride value from the free to the request
 */
#define TF_HW_FREE_TO_REQ(hw_entry, index, request, element) do {            \
	request.element ## _start =                                          \
		tfp_cpu_to_le_16(hw_entry[index].start);                     \
	request.element ## _stride =                                         \
		tfp_cpu_to_le_16(hw_entry[index].stride);                    \
} while (0)

/**
 * Endian converts the start and stride from the HW response to the
 * alloc
 */
#define TF_HW_RESP_TO_ALLOC(hw_entry, index, response, element) do {         \
	hw_entry[index].start =                                              \
		tfp_le_to_cpu_16(response.element ## _start);                \
	hw_entry[index].stride =                                             \
		tfp_le_to_cpu_16(response.element ## _stride);               \
} while (0)

/**
 * Endian converts min and max values from the SRAM response to the
 * query
 */
#define TF_SRAM_RESP_TO_QUERY(query, index, response, element) do {          \
	(query)->sram_query[index].min =                                     \
		tfp_le_to_cpu_16(response.element ## _min);                  \
	(query)->sram_query[index].max =                                     \
		tfp_le_to_cpu_16(response.element ## _max);                  \
} while (0)

/**
 * Endian converts the number of entries from the action (alloc) to
 * the request
 */
#define TF_SRAM_ALLOC_TO_REQ(action, index, request, element)                \
	(request. num_ ## element = tfp_cpu_to_le_16((action)->sram_num[index]))

/**
 * Endian converts the start and stride value from the free to the request
 */
#define TF_SRAM_FREE_TO_REQ(sram_entry, index, request, element) do {        \
	request.element ## _start =                                          \
		tfp_cpu_to_le_16(sram_entry[index].start);                   \
	request.element ## _stride =                                         \
		tfp_cpu_to_le_16(sram_entry[index].stride);                  \
} while (0)

/**
 * Endian converts the start and stride from the HW response to the
 * alloc
 */
#define TF_SRAM_RESP_TO_ALLOC(sram_entry, index, response, element) do {     \
	sram_entry[index].start =                                            \
		tfp_le_to_cpu_16(response.element ## _start);                \
	sram_entry[index].stride =                                           \
		tfp_le_to_cpu_16(response.element ## _stride);               \
} while (0)

/**
 * This is the MAX data we can transport across regular HWRM
 */
#define TF_PCI_BUF_SIZE_MAX 88

/**
 * If data bigger than TF_PCI_BUF_SIZE_MAX then use DMA method
 */
struct tf_msg_dma_buf {
	void *va_addr;
	uint64_t pa_addr;
};

static int
tf_tcam_tbl_2_hwrm(enum tf_tcam_tbl_type tcam_type,
		   uint32_t *hwrm_type)
{
	int rc = 0;

	switch (tcam_type) {
	case TF_TCAM_TBL_TYPE_L2_CTXT_TCAM:
		*hwrm_type = TF_DEV_DATA_TYPE_TF_L2_CTX_ENTRY;
		break;
	case TF_TCAM_TBL_TYPE_PROF_TCAM:
		*hwrm_type = TF_DEV_DATA_TYPE_TF_PROF_TCAM_ENTRY;
		break;
	case TF_TCAM_TBL_TYPE_WC_TCAM:
		*hwrm_type = TF_DEV_DATA_TYPE_TF_WC_ENTRY;
		break;
	case TF_TCAM_TBL_TYPE_VEB_TCAM:
		rc = -EOPNOTSUPP;
		break;
	case TF_TCAM_TBL_TYPE_SP_TCAM:
		rc = -EOPNOTSUPP;
		break;
	case TF_TCAM_TBL_TYPE_CT_RULE_TCAM:
		rc = -EOPNOTSUPP;
		break;
	default:
		rc = -EOPNOTSUPP;
		break;
	}

	return rc;
}

/**
 * Allocates a DMA buffer that can be used for message transfer.
 *
 * [in] buf
 *   Pointer to DMA buffer structure
 *
 * [in] size
 *   Requested size of the buffer in bytes
 *
 * Returns:
 *    0      - Success
 *   -ENOMEM - Unable to allocate buffer, no memory
 */
static int
tf_msg_alloc_dma_buf(struct tf_msg_dma_buf *buf, int size)
{
	struct tfp_calloc_parms alloc_parms;
	int rc;

	/* Allocate session */
	alloc_parms.nitems = 1;
	alloc_parms.size = size;
	alloc_parms.alignment = 4096;
	rc = tfp_calloc(&alloc_parms);
	if (rc)
		return -ENOMEM;

	buf->pa_addr = (uintptr_t)alloc_parms.mem_pa;
	buf->va_addr = alloc_parms.mem_va;

	return 0;
}

/**
 * Free's a previous allocated DMA buffer.
 *
 * [in] buf
 *   Pointer to DMA buffer structure
 */
static void
tf_msg_free_dma_buf(struct tf_msg_dma_buf *buf)
{
	tfp_free(buf->va_addr);
}

/**
 * NEW HWRM direct messages
 */

/**
 * Sends session open request to TF Firmware
 */
int
tf_msg_session_open(struct tf *tfp,
		    char *ctrl_chan_name,
		    uint8_t *fw_session_id)
{
	int rc;
	struct hwrm_tf_session_open_input req = { 0 };
	struct hwrm_tf_session_open_output resp = { 0 };
	struct tfp_send_msg_parms parms = { 0 };

	/* Populate the request */
	tfp_memcpy(&req.session_name, ctrl_chan_name, TF_SESSION_NAME_MAX);

	parms.tf_type = HWRM_TF_SESSION_OPEN;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	if (rc)
		return rc;

	*fw_session_id = resp.fw_session_id;

	return rc;
}

/**
 * Sends session attach request to TF Firmware
 */
int
tf_msg_session_attach(struct tf *tfp __rte_unused,
		      char *ctrl_chan_name __rte_unused,
		      uint8_t tf_fw_session_id __rte_unused)
{
	return -1;
}

/**
 * Sends session close request to TF Firmware
 */
int
tf_msg_session_close(struct tf *tfp)
{
	int rc;
	struct hwrm_tf_session_close_input req = { 0 };
	struct hwrm_tf_session_close_output resp = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);
	struct tfp_send_msg_parms parms = { 0 };

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);

	parms.tf_type = HWRM_TF_SESSION_CLOSE;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	return rc;
}

/**
 * Sends session query config request to TF Firmware
 */
int
tf_msg_session_qcfg(struct tf *tfp)
{
	int rc;
	struct hwrm_tf_session_qcfg_input  req = { 0 };
	struct hwrm_tf_session_qcfg_output resp = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);
	struct tfp_send_msg_parms parms = { 0 };

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);

	parms.tf_type = HWRM_TF_SESSION_QCFG,
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	return rc;
}

/**
 * Sends session HW resource query capability request to TF Firmware
 */
int
tf_msg_session_hw_resc_qcaps(struct tf *tfp,
			     enum tf_dir dir,
			     struct tf_rm_hw_query *query)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_session_hw_resc_qcaps_input req = { 0 };
	struct tf_session_hw_resc_qcaps_output resp = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);

	memset(query, 0, sizeof(*query));

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);

	MSG_PREP(parms,
		 TF_KONG_MB,
		 HWRM_TF,
		 HWRM_TFT_SESSION_HW_RESC_QCAPS,
		 req,
		 resp);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	/* Process the response */
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_L2_CTXT_TCAM, resp,
			    l2_ctx_tcam_entries);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_PROF_FUNC, resp,
			    prof_func);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_PROF_TCAM, resp,
			    prof_tcam_entries);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_EM_PROF_ID, resp,
			    em_prof_id);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_EM_REC, resp,
			    em_record_entries);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_WC_TCAM_PROF_ID, resp,
			    wc_tcam_prof_id);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_WC_TCAM, resp,
			    wc_tcam_entries);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_METER_PROF, resp,
			    meter_profiles);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_METER_INST,
			    resp, meter_inst);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_MIRROR, resp,
			    mirrors);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_UPAR, resp,
			    upar);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_SP_TCAM, resp,
			    sp_tcam_entries);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_L2_FUNC, resp,
			    l2_func);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_FKB, resp,
			    flex_key_templ);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_TBL_SCOPE, resp,
			    tbl_scope);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_EPOCH0, resp,
			    epoch0_entries);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_EPOCH1, resp,
			    epoch1_entries);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_METADATA, resp,
			    metadata);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_CT_STATE, resp,
			    ct_state);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_RANGE_PROF, resp,
			    range_prof);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_RANGE_ENTRY, resp,
			    range_entries);
	TF_HW_RESP_TO_QUERY(query, TF_RESC_TYPE_HW_LAG_ENTRY, resp,
			    lag_tbl_entries);

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

/**
 * Sends session HW resource allocation request to TF Firmware
 */
int
tf_msg_session_hw_resc_alloc(struct tf *tfp __rte_unused,
			     enum tf_dir dir,
			     struct tf_rm_hw_alloc *hw_alloc __rte_unused,
			     struct tf_rm_entry *hw_entry __rte_unused)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_session_hw_resc_alloc_input req = { 0 };
	struct tf_session_hw_resc_alloc_output resp = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);

	memset(hw_entry, 0, sizeof(*hw_entry));

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);

	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_L2_CTXT_TCAM, req,
			   l2_ctx_tcam_entries);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_PROF_FUNC, req,
			   prof_func_entries);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_PROF_TCAM, req,
			   prof_tcam_entries);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_EM_PROF_ID, req,
			   em_prof_id);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_EM_REC, req,
			   em_record_entries);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_WC_TCAM_PROF_ID, req,
			   wc_tcam_prof_id);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_WC_TCAM, req,
			   wc_tcam_entries);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_METER_PROF, req,
			   meter_profiles);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_METER_INST, req,
			   meter_inst);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_MIRROR, req,
			   mirrors);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_UPAR, req,
			   upar);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_SP_TCAM, req,
			   sp_tcam_entries);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_L2_FUNC, req,
			   l2_func);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_FKB, req,
			   flex_key_templ);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_TBL_SCOPE, req,
			   tbl_scope);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_EPOCH0, req,
			   epoch0_entries);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_EPOCH1, req,
			   epoch1_entries);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_METADATA, req,
			   metadata);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_CT_STATE, req,
			   ct_state);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_RANGE_PROF, req,
			   range_prof);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_RANGE_ENTRY, req,
			   range_entries);
	TF_HW_ALLOC_TO_REQ(hw_alloc, TF_RESC_TYPE_HW_LAG_ENTRY, req,
			   lag_tbl_entries);

	MSG_PREP(parms,
		 TF_KONG_MB,
		 HWRM_TF,
		 HWRM_TFT_SESSION_HW_RESC_ALLOC,
		 req,
		 resp);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	/* Process the response */
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_L2_CTXT_TCAM, resp,
			    l2_ctx_tcam_entries);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_PROF_FUNC, resp,
			    prof_func);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_PROF_TCAM, resp,
			    prof_tcam_entries);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_EM_PROF_ID, resp,
			    em_prof_id);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_EM_REC, resp,
			    em_record_entries);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_WC_TCAM_PROF_ID, resp,
			    wc_tcam_prof_id);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_WC_TCAM, resp,
			    wc_tcam_entries);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_METER_PROF, resp,
			    meter_profiles);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_METER_INST, resp,
			    meter_inst);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_MIRROR, resp,
			    mirrors);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_UPAR, resp,
			    upar);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_SP_TCAM, resp,
			    sp_tcam_entries);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_L2_FUNC, resp,
			    l2_func);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_FKB, resp,
			    flex_key_templ);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_TBL_SCOPE, resp,
			    tbl_scope);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_EPOCH0, resp,
			    epoch0_entries);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_EPOCH1, resp,
			    epoch1_entries);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_METADATA, resp,
			    metadata);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_CT_STATE, resp,
			    ct_state);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_RANGE_PROF, resp,
			    range_prof);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_RANGE_ENTRY, resp,
			    range_entries);
	TF_HW_RESP_TO_ALLOC(hw_entry, TF_RESC_TYPE_HW_LAG_ENTRY, resp,
			    lag_tbl_entries);

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

/**
 * Sends session HW resource free request to TF Firmware
 */
int
tf_msg_session_hw_resc_free(struct tf *tfp,
			    enum tf_dir dir,
			    struct tf_rm_entry *hw_entry)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_session_hw_resc_free_input req = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);

	memset(hw_entry, 0, sizeof(*hw_entry));

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);

	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_L2_CTXT_TCAM, req,
			  l2_ctx_tcam_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_PROF_FUNC, req,
			  prof_func);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_PROF_TCAM, req,
			  prof_tcam_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_EM_PROF_ID, req,
			  em_prof_id);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_EM_REC, req,
			  em_record_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_WC_TCAM_PROF_ID, req,
			  wc_tcam_prof_id);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_WC_TCAM, req,
			  wc_tcam_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_METER_PROF, req,
			  meter_profiles);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_METER_INST, req,
			  meter_inst);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_MIRROR, req,
			  mirrors);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_UPAR, req,
			  upar);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_SP_TCAM, req,
			  sp_tcam_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_L2_FUNC, req,
			  l2_func);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_FKB, req,
			  flex_key_templ);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_TBL_SCOPE, req,
			  tbl_scope);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_EPOCH0, req,
			  epoch0_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_EPOCH1, req,
			  epoch1_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_METADATA, req,
			  metadata);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_CT_STATE, req,
			  ct_state);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_RANGE_PROF, req,
			  range_prof);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_RANGE_ENTRY, req,
			  range_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_LAG_ENTRY, req,
			  lag_tbl_entries);

	MSG_PREP_NO_RESP(parms,
			 TF_KONG_MB,
			 HWRM_TF,
			 HWRM_TFT_SESSION_HW_RESC_FREE,
			 req);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

/**
 * Sends session HW resource flush request to TF Firmware
 */
int
tf_msg_session_hw_resc_flush(struct tf *tfp,
			     enum tf_dir dir,
			     struct tf_rm_entry *hw_entry)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_session_hw_resc_free_input req = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);

	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_L2_CTXT_TCAM, req,
			  l2_ctx_tcam_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_PROF_FUNC, req,
			  prof_func);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_PROF_TCAM, req,
			  prof_tcam_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_EM_PROF_ID, req,
			  em_prof_id);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_EM_REC, req,
			  em_record_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_WC_TCAM_PROF_ID, req,
			  wc_tcam_prof_id);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_WC_TCAM, req,
			  wc_tcam_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_METER_PROF, req,
			  meter_profiles);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_METER_INST, req,
			  meter_inst);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_MIRROR, req,
			  mirrors);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_UPAR, req,
			  upar);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_SP_TCAM, req,
			  sp_tcam_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_L2_FUNC, req,
			  l2_func);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_FKB, req,
			  flex_key_templ);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_TBL_SCOPE, req,
			  tbl_scope);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_EPOCH0, req,
			  epoch0_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_EPOCH1, req,
			  epoch1_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_METADATA, req,
			  metadata);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_CT_STATE, req,
			  ct_state);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_RANGE_PROF, req,
			  range_prof);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_RANGE_ENTRY, req,
			  range_entries);
	TF_HW_FREE_TO_REQ(hw_entry, TF_RESC_TYPE_HW_LAG_ENTRY, req,
			  lag_tbl_entries);

	MSG_PREP_NO_RESP(parms,
			 TF_KONG_MB,
			 TF_TYPE_TRUFLOW,
			 HWRM_TFT_SESSION_HW_RESC_FLUSH,
			 req);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

/**
 * Sends session SRAM resource query capability request to TF Firmware
 */
int
tf_msg_session_sram_resc_qcaps(struct tf *tfp __rte_unused,
			       enum tf_dir dir,
			       struct tf_rm_sram_query *query __rte_unused)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_session_sram_resc_qcaps_input req = { 0 };
	struct tf_session_sram_resc_qcaps_output resp = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);

	MSG_PREP(parms,
		 TF_KONG_MB,
		 HWRM_TF,
		 HWRM_TFT_SESSION_SRAM_RESC_QCAPS,
		 req,
		 resp);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	/* Process the response */
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_FULL_ACTION, resp,
			      full_action);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_MCG, resp,
			      mcg);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_ENCAP_8B, resp,
			      encap_8b);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_ENCAP_16B, resp,
			      encap_16b);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_ENCAP_64B, resp,
			      encap_64b);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_SP_SMAC, resp,
			      sp_smac);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_SP_SMAC_IPV4, resp,
			      sp_smac_ipv4);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_SP_SMAC_IPV6, resp,
			      sp_smac_ipv6);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_COUNTER_64B, resp,
			      counter_64b);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_NAT_SPORT, resp,
			      nat_sport);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_NAT_DPORT, resp,
			      nat_dport);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_NAT_S_IPV4, resp,
			      nat_s_ipv4);
	TF_SRAM_RESP_TO_QUERY(query, TF_RESC_TYPE_SRAM_NAT_D_IPV4, resp,
			      nat_d_ipv4);

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

/**
 * Sends session SRAM resource allocation request to TF Firmware
 */
int
tf_msg_session_sram_resc_alloc(struct tf *tfp __rte_unused,
			       enum tf_dir dir,
			       struct tf_rm_sram_alloc *sram_alloc __rte_unused,
			       struct tf_rm_entry *sram_entry __rte_unused)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_session_sram_resc_alloc_input req = { 0 };
	struct tf_session_sram_resc_alloc_output resp;
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);

	memset(&resp, 0, sizeof(resp));

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);

	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_FULL_ACTION, req,
			     full_action);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_MCG, req,
			     mcg);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_ENCAP_8B, req,
			     encap_8b);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_ENCAP_16B, req,
			     encap_16b);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_ENCAP_64B, req,
			     encap_64b);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_SP_SMAC, req,
			     sp_smac);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_SP_SMAC_IPV4,
			     req, sp_smac_ipv4);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_SP_SMAC_IPV6,
			     req, sp_smac_ipv6);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_COUNTER_64B,
			     req, counter_64b);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_NAT_SPORT, req,
			     nat_sport);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_NAT_DPORT, req,
			     nat_dport);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_NAT_S_IPV4, req,
			     nat_s_ipv4);
	TF_SRAM_ALLOC_TO_REQ(sram_alloc, TF_RESC_TYPE_SRAM_NAT_D_IPV4, req,
			     nat_d_ipv4);

	MSG_PREP(parms,
		 TF_KONG_MB,
		 HWRM_TF,
		 HWRM_TFT_SESSION_SRAM_RESC_ALLOC,
		 req,
		 resp);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	/* Process the response */
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_FULL_ACTION,
			      resp, full_action);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_MCG, resp,
			      mcg);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_ENCAP_8B, resp,
			      encap_8b);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_ENCAP_16B, resp,
			      encap_16b);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_ENCAP_64B, resp,
			      encap_64b);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_SP_SMAC, resp,
			      sp_smac);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_SP_SMAC_IPV4,
			      resp, sp_smac_ipv4);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_SP_SMAC_IPV6,
			      resp, sp_smac_ipv6);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_COUNTER_64B, resp,
			      counter_64b);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_NAT_SPORT, resp,
			      nat_sport);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_NAT_DPORT, resp,
			      nat_dport);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_NAT_S_IPV4, resp,
			      nat_s_ipv4);
	TF_SRAM_RESP_TO_ALLOC(sram_entry, TF_RESC_TYPE_SRAM_NAT_D_IPV4, resp,
			      nat_d_ipv4);

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

/**
 * Sends session SRAM resource free request to TF Firmware
 */
int
tf_msg_session_sram_resc_free(struct tf *tfp __rte_unused,
			      enum tf_dir dir,
			      struct tf_rm_entry *sram_entry __rte_unused)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_session_sram_resc_free_input req = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);

	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_FULL_ACTION, req,
			    full_action);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_MCG, req,
			    mcg);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_ENCAP_8B, req,
			    encap_8b);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_ENCAP_16B, req,
			    encap_16b);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_ENCAP_64B, req,
			    encap_64b);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_SP_SMAC, req,
			    sp_smac);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_SP_SMAC_IPV4, req,
			    sp_smac_ipv4);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_SP_SMAC_IPV6, req,
			    sp_smac_ipv6);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_COUNTER_64B, req,
			    counter_64b);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_NAT_SPORT, req,
			    nat_sport);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_NAT_DPORT, req,
			    nat_dport);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_NAT_S_IPV4, req,
			    nat_s_ipv4);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_NAT_D_IPV4, req,
			    nat_d_ipv4);

	MSG_PREP_NO_RESP(parms,
			 TF_KONG_MB,
			 HWRM_TF,
			 HWRM_TFT_SESSION_SRAM_RESC_FREE,
			 req);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

/**
 * Sends session SRAM resource flush request to TF Firmware
 */
int
tf_msg_session_sram_resc_flush(struct tf *tfp,
			       enum tf_dir dir,
			       struct tf_rm_entry *sram_entry)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_session_sram_resc_free_input req = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);

	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_FULL_ACTION, req,
			    full_action);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_MCG, req,
			    mcg);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_ENCAP_8B, req,
			    encap_8b);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_ENCAP_16B, req,
			    encap_16b);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_ENCAP_64B, req,
			    encap_64b);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_SP_SMAC, req,
			    sp_smac);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_SP_SMAC_IPV4, req,
			    sp_smac_ipv4);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_SP_SMAC_IPV6, req,
			    sp_smac_ipv6);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_COUNTER_64B, req,
			    counter_64b);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_NAT_SPORT, req,
			    nat_sport);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_NAT_DPORT, req,
			    nat_dport);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_NAT_S_IPV4, req,
			    nat_s_ipv4);
	TF_SRAM_FREE_TO_REQ(sram_entry, TF_RESC_TYPE_SRAM_NAT_D_IPV4, req,
			    nat_d_ipv4);

	MSG_PREP_NO_RESP(parms,
			 TF_KONG_MB,
			 TF_TYPE_TRUFLOW,
			 HWRM_TFT_SESSION_SRAM_RESC_FLUSH,
			 req);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

int
tf_msg_session_resc_qcaps(struct tf *tfp,
			  enum tf_dir dir,
			  uint16_t size,
			  struct tf_rm_resc_req_entry *query,
			  enum tf_rm_resc_resv_strategy *resv_strategy)
{
	int rc;
	int i;
	struct tfp_send_msg_parms parms = { 0 };
	struct hwrm_tf_session_resc_qcaps_input req = { 0 };
	struct hwrm_tf_session_resc_qcaps_output resp = { 0 };
	uint8_t fw_session_id;
	struct tf_msg_dma_buf qcaps_buf = { 0 };
	struct tf_rm_resc_req_entry *data;
	int dma_size;

	TF_CHECK_PARMS3(tfp, query, resv_strategy);

	rc = tf_session_get_fw_session_id(tfp, &fw_session_id);
	if (rc) {
		TFP_DRV_LOG(ERR,
			    "%s: Unable to lookup FW id, rc:%s\n",
			    tf_dir_2_str(dir),
			    strerror(-rc));
		return rc;
	}

	/* Prepare DMA buffer */
	dma_size = size * sizeof(struct tf_rm_resc_req_entry);
	rc = tf_msg_alloc_dma_buf(&qcaps_buf, dma_size);
	if (rc)
		return rc;

	/* Populate the request */
	req.fw_session_id = tfp_cpu_to_le_32(fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);
	req.qcaps_size = size;
	req.qcaps_addr = tfp_cpu_to_le_64(qcaps_buf.pa_addr);

	parms.tf_type = HWRM_TF_SESSION_RESC_QCAPS;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp, &parms);
	if (rc)
		return rc;

	/* Process the response
	 * Should always get expected number of entries
	 */
	if (resp.size != size) {
		TFP_DRV_LOG(ERR,
			    "%s: QCAPS message size error, rc:%s\n",
			    tf_dir_2_str(dir),
			    strerror(-EINVAL));
		return -EINVAL;
	}

	printf("size: %d\n", resp.size);

	/* Post process the response */
	data = (struct tf_rm_resc_req_entry *)qcaps_buf.va_addr;

	printf("\nQCAPS\n");
	for (i = 0; i < size; i++) {
		query[i].type = tfp_cpu_to_le_32(data[i].type);
		query[i].min = tfp_le_to_cpu_16(data[i].min);
		query[i].max = tfp_le_to_cpu_16(data[i].max);

		printf("type: %d(0x%x) %d %d\n",
		       query[i].type,
		       query[i].type,
		       query[i].min,
		       query[i].max);

	}

	*resv_strategy = resp.flags &
	      HWRM_TF_SESSION_RESC_QCAPS_OUTPUT_FLAGS_SESS_RESV_STRATEGY_MASK;

	tf_msg_free_dma_buf(&qcaps_buf);

	return rc;
}

int
tf_msg_session_resc_alloc(struct tf *tfp,
			  enum tf_dir dir,
			  uint16_t size,
			  struct tf_rm_resc_req_entry *request,
			  struct tf_rm_resc_entry *resv)
{
	int rc;
	int i;
	struct tfp_send_msg_parms parms = { 0 };
	struct hwrm_tf_session_resc_alloc_input req = { 0 };
	struct hwrm_tf_session_resc_alloc_output resp = { 0 };
	uint8_t fw_session_id;
	struct tf_msg_dma_buf req_buf = { 0 };
	struct tf_msg_dma_buf resv_buf = { 0 };
	struct tf_rm_resc_req_entry *req_data;
	struct tf_rm_resc_entry *resv_data;
	int dma_size;

	TF_CHECK_PARMS3(tfp, request, resv);

	rc = tf_session_get_fw_session_id(tfp, &fw_session_id);
	if (rc) {
		TFP_DRV_LOG(ERR,
			    "%s: Unable to lookup FW id, rc:%s\n",
			    tf_dir_2_str(dir),
			    strerror(-rc));
		return rc;
	}

	/* Prepare DMA buffers */
	dma_size = size * sizeof(struct tf_rm_resc_req_entry);
	rc = tf_msg_alloc_dma_buf(&req_buf, dma_size);
	if (rc)
		return rc;

	dma_size = size * sizeof(struct tf_rm_resc_entry);
	rc = tf_msg_alloc_dma_buf(&resv_buf, dma_size);
	if (rc)
		return rc;

	/* Populate the request */
	req.fw_session_id = tfp_cpu_to_le_32(fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);
	req.req_size = size;

	req_data = (struct tf_rm_resc_req_entry *)req_buf.va_addr;
	for (i = 0; i < size; i++) {
		req_data[i].type = tfp_cpu_to_le_32(request[i].type);
		req_data[i].min = tfp_cpu_to_le_16(request[i].min);
		req_data[i].max = tfp_cpu_to_le_16(request[i].max);
	}

	req.req_addr = tfp_cpu_to_le_64(req_buf.pa_addr);
	req.resc_addr = tfp_cpu_to_le_64(resv_buf.pa_addr);

	parms.tf_type = HWRM_TF_SESSION_RESC_ALLOC;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp, &parms);
	if (rc)
		return rc;

	/* Process the response
	 * Should always get expected number of entries
	 */
	if (resp.size != size) {
		TFP_DRV_LOG(ERR,
			    "%s: Alloc message size error, rc:%s\n",
			    tf_dir_2_str(dir),
			    strerror(-EINVAL));
		return -EINVAL;
	}

	printf("\nRESV\n");
	printf("size: %d\n", resp.size);

	/* Post process the response */
	resv_data = (struct tf_rm_resc_entry *)resv_buf.va_addr;
	for (i = 0; i < size; i++) {
		resv[i].type = tfp_cpu_to_le_32(resv_data[i].type);
		resv[i].start = tfp_cpu_to_le_16(resv_data[i].start);
		resv[i].stride = tfp_cpu_to_le_16(resv_data[i].stride);

		printf("%d type: %d(0x%x) %d %d\n",
		       i,
		       resv[i].type,
		       resv[i].type,
		       resv[i].start,
		       resv[i].stride);
	}

	tf_msg_free_dma_buf(&req_buf);
	tf_msg_free_dma_buf(&resv_buf);

	return rc;
}

/**
 * Sends EM mem register request to Firmware
 */
int tf_msg_em_mem_rgtr(struct tf *tfp,
		       int           page_lvl,
		       int           page_size,
		       uint64_t      dma_addr,
		       uint16_t     *ctx_id)
{
	int rc;
	struct hwrm_tf_ctxt_mem_rgtr_input req = { 0 };
	struct hwrm_tf_ctxt_mem_rgtr_output resp = { 0 };
	struct tfp_send_msg_parms parms = { 0 };

	req.page_level = page_lvl;
	req.page_size = page_size;
	req.page_dir = tfp_cpu_to_le_64(dma_addr);

	parms.tf_type = HWRM_TF_CTXT_MEM_RGTR;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	if (rc)
		return rc;

	*ctx_id = tfp_le_to_cpu_16(resp.ctx_id);

	return rc;
}

/**
 * Sends EM mem unregister request to Firmware
 */
int tf_msg_em_mem_unrgtr(struct tf *tfp,
			 uint16_t  *ctx_id)
{
	int rc;
	struct hwrm_tf_ctxt_mem_unrgtr_input req = {0};
	struct hwrm_tf_ctxt_mem_unrgtr_output resp = {0};
	struct tfp_send_msg_parms parms = { 0 };

	req.ctx_id = tfp_cpu_to_le_32(*ctx_id);

	parms.tf_type = HWRM_TF_CTXT_MEM_UNRGTR;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	return rc;
}

/**
 * Sends EM qcaps request to Firmware
 */
int tf_msg_em_qcaps(struct tf *tfp,
		    int dir,
		    struct tf_em_caps *em_caps)
{
	int rc;
	struct hwrm_tf_ext_em_qcaps_input  req = {0};
	struct hwrm_tf_ext_em_qcaps_output resp = { 0 };
	uint32_t             flags;
	struct tfp_send_msg_parms parms = { 0 };

	flags = (dir == TF_DIR_TX ? HWRM_TF_EXT_EM_QCAPS_INPUT_FLAGS_DIR_TX :
		 HWRM_TF_EXT_EM_QCAPS_INPUT_FLAGS_DIR_RX);
	req.flags = tfp_cpu_to_le_32(flags);

	parms.tf_type = HWRM_TF_EXT_EM_QCAPS;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	if (rc)
		return rc;

	em_caps->supported = tfp_le_to_cpu_32(resp.supported);
	em_caps->max_entries_supported =
		tfp_le_to_cpu_32(resp.max_entries_supported);
	em_caps->key_entry_size = tfp_le_to_cpu_16(resp.key_entry_size);
	em_caps->record_entry_size =
		tfp_le_to_cpu_16(resp.record_entry_size);
	em_caps->efc_entry_size = tfp_le_to_cpu_16(resp.efc_entry_size);

	return rc;
}

/**
 * Sends EM config request to Firmware
 */
int tf_msg_em_cfg(struct tf *tfp,
		  uint32_t   num_entries,
		  uint16_t   key0_ctx_id,
		  uint16_t   key1_ctx_id,
		  uint16_t   record_ctx_id,
		  uint16_t   efc_ctx_id,
		  uint8_t    flush_interval,
		  int        dir)
{
	int rc;
	struct hwrm_tf_ext_em_cfg_input  req = {0};
	struct hwrm_tf_ext_em_cfg_output resp = {0};
	uint32_t flags;
	struct tfp_send_msg_parms parms = { 0 };

	flags = (dir == TF_DIR_TX ? HWRM_TF_EXT_EM_CFG_INPUT_FLAGS_DIR_TX :
		 HWRM_TF_EXT_EM_CFG_INPUT_FLAGS_DIR_RX);
	flags |= HWRM_TF_EXT_EM_QCAPS_INPUT_FLAGS_PREFERRED_OFFLOAD;

	req.flags = tfp_cpu_to_le_32(flags);
	req.num_entries = tfp_cpu_to_le_32(num_entries);

	req.flush_interval = flush_interval;

	req.key0_ctx_id = tfp_cpu_to_le_16(key0_ctx_id);
	req.key1_ctx_id = tfp_cpu_to_le_16(key1_ctx_id);
	req.record_ctx_id = tfp_cpu_to_le_16(record_ctx_id);
	req.efc_ctx_id = tfp_cpu_to_le_16(efc_ctx_id);

	parms.tf_type = HWRM_TF_EXT_EM_CFG;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	return rc;
}

/**
 * Sends EM internal insert request to Firmware
 */
int tf_msg_insert_em_internal_entry(struct tf *tfp,
				struct tf_insert_em_entry_parms *em_parms,
				uint16_t *rptr_index,
				uint8_t *rptr_entry,
				uint8_t *num_of_entries)
{
	int                         rc;
	struct tfp_send_msg_parms        parms = { 0 };
	struct hwrm_tf_em_insert_input   req = { 0 };
	struct hwrm_tf_em_insert_output  resp = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);
	struct tf_em_64b_entry *em_result =
		(struct tf_em_64b_entry *)em_parms->em_record;
	uint32_t flags;

	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	tfp_memcpy(req.em_key,
		   em_parms->key,
		   ((em_parms->key_sz_in_bits + 7) / 8));

	flags = (em_parms->dir == TF_DIR_TX ?
		 HWRM_TF_EM_INSERT_INPUT_FLAGS_DIR_TX :
		 HWRM_TF_EM_INSERT_INPUT_FLAGS_DIR_RX);
	req.flags = tfp_cpu_to_le_16(flags);
	req.strength =
		(em_result->hdr.word1 & CFA_P4_EEM_ENTRY_STRENGTH_MASK) >>
		CFA_P4_EEM_ENTRY_STRENGTH_SHIFT;
	req.em_key_bitlen = em_parms->key_sz_in_bits;
	req.action_ptr = em_result->hdr.pointer;
	req.em_record_idx = *rptr_index;

	parms.tf_type = HWRM_TF_EM_INSERT;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	if (rc)
		return rc;

	*rptr_entry = resp.rptr_entry;
	*rptr_index = resp.rptr_index;
	*num_of_entries = resp.num_of_entries;

	return 0;
}

/**
 * Sends EM delete insert request to Firmware
 */
int tf_msg_delete_em_entry(struct tf *tfp,
			   struct tf_delete_em_entry_parms *em_parms)
{
	int                             rc;
	struct tfp_send_msg_parms       parms = { 0 };
	struct hwrm_tf_em_delete_input  req = { 0 };
	struct hwrm_tf_em_delete_output resp = { 0 };
	uint32_t flags;
	struct tf_session *tfs =
		(struct tf_session *)(tfp->session->core_data);

	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);

	flags = (em_parms->dir == TF_DIR_TX ?
		 HWRM_TF_EM_DELETE_INPUT_FLAGS_DIR_TX :
		 HWRM_TF_EM_DELETE_INPUT_FLAGS_DIR_RX);
	req.flags = tfp_cpu_to_le_16(flags);
	req.flow_handle = tfp_cpu_to_le_64(em_parms->flow_handle);

	parms.tf_type = HWRM_TF_EM_DELETE;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	if (rc)
		return rc;

	em_parms->index = tfp_le_to_cpu_16(resp.em_index);

	return 0;
}

/**
 * Sends EM operation request to Firmware
 */
int tf_msg_em_op(struct tf *tfp,
		 int dir,
		 uint16_t op)
{
	int rc;
	struct hwrm_tf_ext_em_op_input req = {0};
	struct hwrm_tf_ext_em_op_output resp = {0};
	uint32_t flags;
	struct tfp_send_msg_parms parms = { 0 };

	flags = (dir == TF_DIR_TX ? HWRM_TF_EXT_EM_CFG_INPUT_FLAGS_DIR_TX :
		 HWRM_TF_EXT_EM_CFG_INPUT_FLAGS_DIR_RX);
	req.flags = tfp_cpu_to_le_32(flags);
	req.op = tfp_cpu_to_le_16(op);

	parms.tf_type = HWRM_TF_EXT_EM_OP;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	return rc;
}

int
tf_msg_set_tbl_entry(struct tf *tfp,
		     enum tf_dir dir,
		     enum tf_tbl_type type,
		     uint16_t size,
		     uint8_t *data,
		     uint32_t index)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_tbl_type_set_input req = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);
	req.type = tfp_cpu_to_le_32(type);
	req.size = tfp_cpu_to_le_16(size);
	req.index = tfp_cpu_to_le_32(index);

	tfp_memcpy(&req.data,
		   data,
		   size);

	MSG_PREP_NO_RESP(parms,
			 TF_KONG_MB,
			 HWRM_TF,
			 HWRM_TFT_TBL_TYPE_SET,
			 req);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

int
tf_msg_get_tbl_entry(struct tf *tfp,
		     enum tf_dir dir,
		     enum tf_tbl_type type,
		     uint16_t size,
		     uint8_t *data,
		     uint32_t index)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_tbl_type_get_input req = { 0 };
	struct tf_tbl_type_get_output resp = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(dir);
	req.type = tfp_cpu_to_le_32(type);
	req.index = tfp_cpu_to_le_32(index);

	MSG_PREP(parms,
		 TF_KONG_MB,
		 HWRM_TF,
		 HWRM_TFT_TBL_TYPE_GET,
		 req,
		 resp);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	/* Verify that we got enough buffer to return the requested data */
	if (resp.size < size)
		return -EINVAL;

	tfp_memcpy(data,
		   &resp.data,
		   resp.size);

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

int
tf_msg_bulk_get_tbl_entry(struct tf *tfp,
			  struct tf_bulk_get_tbl_entry_parms *params)
{
	int rc;
	struct tfp_send_msg_parms parms = { 0 };
	struct tf_tbl_type_bulk_get_input req = { 0 };
	struct tf_tbl_type_bulk_get_output resp = { 0 };
	struct tf_session *tfs = (struct tf_session *)(tfp->session->core_data);
	int data_size = 0;

	/* Populate the request */
	req.fw_session_id =
		tfp_cpu_to_le_32(tfs->session_id.internal.fw_session_id);
	req.flags = tfp_cpu_to_le_16(params->dir);
	req.type = tfp_cpu_to_le_32(params->type);
	req.start_index = tfp_cpu_to_le_32(params->starting_idx);
	req.num_entries = tfp_cpu_to_le_32(params->num_entries);

	data_size = params->num_entries * params->entry_sz_in_bytes;

	req.host_addr = tfp_cpu_to_le_64(params->physical_mem_addr);

	MSG_PREP(parms,
		 TF_KONG_MB,
		 HWRM_TF,
		 HWRM_TFT_TBL_TYPE_BULK_GET,
		 req,
		 resp);

	rc = tfp_send_msg_tunneled(tfp, &parms);
	if (rc)
		return rc;

	/* Verify that we got enough buffer to return the requested data */
	if (resp.size < data_size)
		return -EINVAL;

	return tfp_le_to_cpu_32(parms.tf_resp_code);
}

int
tf_msg_tcam_entry_set(struct tf *tfp,
		      struct tf_tcam_set_parms *parms)
{
	int rc;
	struct tfp_send_msg_parms mparms = { 0 };
	struct hwrm_tf_tcam_set_input req = { 0 };
	struct hwrm_tf_tcam_set_output resp = { 0 };
	struct tf_msg_dma_buf buf = { 0 };
	uint8_t *data = NULL;
	int data_size = 0;

	rc = tf_tcam_tbl_2_hwrm(parms->type, &req.type);
	if (rc != 0)
		return rc;

	req.idx = tfp_cpu_to_le_16(parms->idx);
	if (parms->dir == TF_DIR_TX)
		req.flags |= HWRM_TF_TCAM_SET_INPUT_FLAGS_DIR_TX;

	req.key_size = parms->key_size;
	req.mask_offset = parms->key_size;
	/* Result follows after key and mask, thus multiply by 2 */
	req.result_offset = 2 * parms->key_size;
	req.result_size = parms->result_size;
	data_size = 2 * req.key_size + req.result_size;

	if (data_size <= TF_PCI_BUF_SIZE_MAX) {
		/* use pci buffer */
		data = &req.dev_data[0];
	} else {
		/* use dma buffer */
		req.flags |= HWRM_TF_TCAM_SET_INPUT_FLAGS_DMA;
		rc = tf_msg_alloc_dma_buf(&buf, data_size);
		if (rc)
			goto cleanup;
		data = buf.va_addr;
		tfp_memcpy(&req.dev_data[0],
			   &buf.pa_addr,
			   sizeof(buf.pa_addr));
	}

	tfp_memcpy(&data[0], parms->key, parms->key_size);
	tfp_memcpy(&data[parms->key_size], parms->mask, parms->key_size);
	tfp_memcpy(&data[req.result_offset], parms->result, parms->result_size);

	mparms.tf_type = HWRM_TF_TCAM_SET;
	mparms.req_data = (uint32_t *)&req;
	mparms.req_size = sizeof(req);
	mparms.resp_data = (uint32_t *)&resp;
	mparms.resp_size = sizeof(resp);
	mparms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &mparms);
	if (rc)
		goto cleanup;

cleanup:
	tf_msg_free_dma_buf(&buf);

	return rc;
}

int
tf_msg_tcam_entry_free(struct tf *tfp,
		       struct tf_tcam_free_parms *in_parms)
{
	int rc;
	struct hwrm_tf_tcam_free_input req =  { 0 };
	struct hwrm_tf_tcam_free_output resp = { 0 };
	struct tfp_send_msg_parms parms = { 0 };

	/* Populate the request */
	rc = tf_tcam_tbl_2_hwrm(in_parms->type, &req.type);
	if (rc != 0)
		return rc;

	req.count = 1;
	req.idx_list[0] = tfp_cpu_to_le_16(in_parms->idx);
	if (in_parms->dir == TF_DIR_TX)
		req.flags |= HWRM_TF_TCAM_FREE_INPUT_FLAGS_DIR_TX;

	parms.tf_type = HWRM_TF_TCAM_FREE;
	parms.req_data = (uint32_t *)&req;
	parms.req_size = sizeof(req);
	parms.resp_data = (uint32_t *)&resp;
	parms.resp_size = sizeof(resp);
	parms.mailbox = TF_KONG_MB;

	rc = tfp_send_msg_direct(tfp,
				 &parms);
	return rc;
}
