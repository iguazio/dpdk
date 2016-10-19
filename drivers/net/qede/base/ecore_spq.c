/*
 * Copyright (c) 2016 QLogic Corporation.
 * All rights reserved.
 * www.qlogic.com
 *
 * See LICENSE.qede_pmd for copyright and licensing details.
 */

#include "bcm_osal.h"
#include "reg_addr.h"
#include "ecore_gtt_reg_addr.h"
#include "ecore_hsi_common.h"
#include "ecore.h"
#include "ecore_sp_api.h"
#include "ecore_spq.h"
#include "ecore_iro.h"
#include "ecore_init_fw_funcs.h"
#include "ecore_cxt.h"
#include "ecore_int.h"
#include "ecore_dev_api.h"
#include "ecore_mcp.h"
#include "ecore_hw.h"
#include "ecore_sriov.h"

/***************************************************************************
 * Structures & Definitions
 ***************************************************************************/

#define SPQ_HIGH_PRI_RESERVE_DEFAULT	(1)
#define SPQ_BLOCK_SLEEP_LENGTH		(1000)

/***************************************************************************
 * Blocking Imp. (BLOCK/EBLOCK mode)
 ***************************************************************************/
static void ecore_spq_blocking_cb(struct ecore_hwfn *p_hwfn,
				  void *cookie,
				  union event_ring_data *data,
				  u8 fw_return_code)
{
	struct ecore_spq_comp_done *comp_done;

	comp_done = (struct ecore_spq_comp_done *)cookie;

	comp_done->done = 0x1;
	comp_done->fw_return_code = fw_return_code;

	/* make update visible to waiting thread */
	OSAL_SMP_WMB(p_hwfn->p_dev);
}

static enum _ecore_status_t ecore_spq_block(struct ecore_hwfn *p_hwfn,
					      struct ecore_spq_entry *p_ent,
					    u8 *p_fw_ret)
{
	int sleep_count = SPQ_BLOCK_SLEEP_LENGTH;
	struct ecore_spq_comp_done *comp_done;
	enum _ecore_status_t rc;

	comp_done = (struct ecore_spq_comp_done *)p_ent->comp_cb.cookie;
	while (sleep_count) {
		OSAL_POLL_MODE_DPC(p_hwfn);
		/* validate we receive completion update */
		OSAL_SMP_RMB(p_hwfn->p_dev);
		if (comp_done->done == 1) {
			if (p_fw_ret)
				*p_fw_ret = comp_done->fw_return_code;
			return ECORE_SUCCESS;
		}
		OSAL_MSLEEP(5);
		sleep_count--;
	}

	DP_INFO(p_hwfn, "Ramrod is stuck, requesting MCP drain\n");
	rc = ecore_mcp_drain(p_hwfn, p_hwfn->p_main_ptt);
	if (rc != ECORE_SUCCESS)
		DP_NOTICE(p_hwfn, true, "MCP drain failed\n");

	/* Retry after drain */
	sleep_count = SPQ_BLOCK_SLEEP_LENGTH;
	while (sleep_count) {
		/* validate we receive completion update */
		OSAL_SMP_RMB(p_hwfn->p_dev);
		if (comp_done->done == 1) {
			if (p_fw_ret)
				*p_fw_ret = comp_done->fw_return_code;
		return ECORE_SUCCESS;
		}
		OSAL_MSLEEP(5);
		sleep_count--;
	}

	if (comp_done->done == 1) {
		if (p_fw_ret)
			*p_fw_ret = comp_done->fw_return_code;
		return ECORE_SUCCESS;
	}

	DP_NOTICE(p_hwfn, true,
		  "Ramrod is stuck [CID %08x cmd %02x proto %02x echo %04x]\n",
		  OSAL_LE32_TO_CPU(p_ent->elem.hdr.cid),
		  p_ent->elem.hdr.cmd_id, p_ent->elem.hdr.protocol_id,
		  OSAL_LE16_TO_CPU(p_ent->elem.hdr.echo));

	ecore_hw_err_notify(p_hwfn, ECORE_HW_ERR_RAMROD_FAIL);

	return ECORE_BUSY;
}

/***************************************************************************
 * SPQ entries inner API
 ***************************************************************************/
static enum _ecore_status_t
ecore_spq_fill_entry(struct ecore_hwfn *p_hwfn, struct ecore_spq_entry *p_ent)
{
	p_ent->flags = 0;

	switch (p_ent->comp_mode) {
	case ECORE_SPQ_MODE_EBLOCK:
	case ECORE_SPQ_MODE_BLOCK:
		p_ent->comp_cb.function = ecore_spq_blocking_cb;
		break;
	case ECORE_SPQ_MODE_CB:
		break;
	default:
		DP_NOTICE(p_hwfn, true, "Unknown SPQE completion mode %d\n",
			  p_ent->comp_mode);
		return ECORE_INVAL;
	}

	DP_VERBOSE(p_hwfn, ECORE_MSG_SPQ,
		   "Ramrod header: [CID 0x%08x CMD 0x%02x protocol 0x%02x]"
		   " Data pointer: [%08x:%08x] Completion Mode: %s\n",
		   p_ent->elem.hdr.cid, p_ent->elem.hdr.cmd_id,
		   p_ent->elem.hdr.protocol_id,
		   p_ent->elem.data_ptr.hi, p_ent->elem.data_ptr.lo,
		   D_TRINE(p_ent->comp_mode, ECORE_SPQ_MODE_EBLOCK,
			   ECORE_SPQ_MODE_BLOCK, "MODE_EBLOCK", "MODE_BLOCK",
			   "MODE_CB"));

	return ECORE_SUCCESS;
}

/***************************************************************************
 * HSI access
 ***************************************************************************/
static void ecore_spq_hw_initialize(struct ecore_hwfn *p_hwfn,
				    struct ecore_spq *p_spq)
{
	u16 pq;
	struct ecore_cxt_info cxt_info;
	struct core_conn_context *p_cxt;
	union ecore_qm_pq_params pq_params;
	enum _ecore_status_t rc;

	cxt_info.iid = p_spq->cid;

	rc = ecore_cxt_get_cid_info(p_hwfn, &cxt_info);

	if (rc < 0) {
		DP_NOTICE(p_hwfn, true, "Cannot find context info for cid=%d\n",
			  p_spq->cid);
		return;
	}

	p_cxt = cxt_info.p_cxt;

	SET_FIELD(p_cxt->xstorm_ag_context.flags10,
		  XSTORM_CORE_CONN_AG_CTX_DQ_CF_EN, 1);
	SET_FIELD(p_cxt->xstorm_ag_context.flags1,
		  XSTORM_CORE_CONN_AG_CTX_DQ_CF_ACTIVE, 1);
	/* SET_FIELD(p_cxt->xstorm_ag_context.flags10,
	 *           XSTORM_CORE_CONN_AG_CTX_SLOW_PATH_EN, 1);
	 */
	SET_FIELD(p_cxt->xstorm_ag_context.flags9,
		  XSTORM_CORE_CONN_AG_CTX_CONSOLID_PROD_CF_EN, 1);

	/* CDU validation - FIXME currently disabled */

	/* QM physical queue */
	OSAL_MEMSET(&pq_params, 0, sizeof(pq_params));
	pq_params.core.tc = LB_TC;
	pq = ecore_get_qm_pq(p_hwfn, PROTOCOLID_CORE, &pq_params);
	p_cxt->xstorm_ag_context.physical_q0 = OSAL_CPU_TO_LE16(pq);

	p_cxt->xstorm_st_context.spq_base_lo =
	    DMA_LO_LE(p_spq->chain.p_phys_addr);
	p_cxt->xstorm_st_context.spq_base_hi =
	    DMA_HI_LE(p_spq->chain.p_phys_addr);

	p_cxt->xstorm_st_context.consolid_base_addr.lo =
	    DMA_LO_LE(p_hwfn->p_consq->chain.p_phys_addr);
	p_cxt->xstorm_st_context.consolid_base_addr.hi =
	    DMA_HI_LE(p_hwfn->p_consq->chain.p_phys_addr);
}

static enum _ecore_status_t ecore_spq_hw_post(struct ecore_hwfn *p_hwfn,
					      struct ecore_spq *p_spq,
					      struct ecore_spq_entry *p_ent)
{
	struct ecore_chain *p_chain = &p_hwfn->p_spq->chain;
	u16 echo = ecore_chain_get_prod_idx(p_chain);
	struct slow_path_element *elem;
	struct core_db_data db;

	p_ent->elem.hdr.echo = OSAL_CPU_TO_LE16(echo);
	elem = ecore_chain_produce(p_chain);
	if (!elem) {
		DP_NOTICE(p_hwfn, true, "Failed to produce from SPQ chain\n");
		return ECORE_INVAL;
	}

	*elem = p_ent->elem;	/* struct assignment */

	/* send a doorbell on the slow hwfn session */
	OSAL_MEMSET(&db, 0, sizeof(db));
	SET_FIELD(db.params, CORE_DB_DATA_DEST, DB_DEST_XCM);
	SET_FIELD(db.params, CORE_DB_DATA_AGG_CMD, DB_AGG_CMD_SET);
	SET_FIELD(db.params, CORE_DB_DATA_AGG_VAL_SEL,
		  DQ_XCM_CORE_SPQ_PROD_CMD);
	db.agg_flags = DQ_XCM_CORE_DQ_CF_CMD;

	/* validate producer is up to-date */
	OSAL_RMB(p_hwfn->p_dev);

	db.spq_prod = OSAL_CPU_TO_LE16(ecore_chain_get_prod_idx(p_chain));

	/* do not reorder */
	OSAL_BARRIER(p_hwfn->p_dev);

	DOORBELL(p_hwfn, DB_ADDR(p_spq->cid, DQ_DEMS_LEGACY), *(u32 *)&db);

	/* make sure doorbell is rang */
	OSAL_MMIOWB(p_hwfn->p_dev);

	DP_VERBOSE(p_hwfn, ECORE_MSG_SPQ,
		   "Doorbelled [0x%08x, CID 0x%08x] with Flags: %02x"
		   " agg_params: %02x, prod: %04x\n",
		   DB_ADDR(p_spq->cid, DQ_DEMS_LEGACY), p_spq->cid, db.params,
		   db.agg_flags, ecore_chain_get_prod_idx(p_chain));

	return ECORE_SUCCESS;
}

/***************************************************************************
 * Asynchronous events
 ***************************************************************************/

static enum _ecore_status_t
ecore_async_event_completion(struct ecore_hwfn *p_hwfn,
			     struct event_ring_entry *p_eqe)
{
	switch (p_eqe->protocol_id) {
	case PROTOCOLID_COMMON:
		return ecore_sriov_eqe_event(p_hwfn,
					     p_eqe->opcode,
					     p_eqe->echo, &p_eqe->data);
	default:
		DP_NOTICE(p_hwfn,
			  true, "Unknown Async completion for protocol: %d\n",
			  p_eqe->protocol_id);
		return ECORE_INVAL;
	}
}

/***************************************************************************
 * EQ API
 ***************************************************************************/
void ecore_eq_prod_update(struct ecore_hwfn *p_hwfn, u16 prod)
{
	u32 addr = GTT_BAR0_MAP_REG_USDM_RAM +
	    USTORM_EQE_CONS_OFFSET(p_hwfn->rel_pf_id);

	REG_WR16(p_hwfn, addr, prod);

	/* keep prod updates ordered */
	OSAL_MMIOWB(p_hwfn->p_dev);
}

enum _ecore_status_t ecore_eq_completion(struct ecore_hwfn *p_hwfn,
					 void *cookie)
{
	struct ecore_eq *p_eq = cookie;
	struct ecore_chain *p_chain = &p_eq->chain;
	enum _ecore_status_t rc = 0;

	/* take a snapshot of the FW consumer */
	u16 fw_cons_idx = OSAL_LE16_TO_CPU(*p_eq->p_fw_cons);

	DP_VERBOSE(p_hwfn, ECORE_MSG_SPQ, "fw_cons_idx %x\n", fw_cons_idx);

	/* Need to guarantee the fw_cons index we use points to a usuable
	 * element (to comply with our chain), so our macros would comply
	 */
	if ((fw_cons_idx & ecore_chain_get_usable_per_page(p_chain)) ==
	    ecore_chain_get_usable_per_page(p_chain)) {
		fw_cons_idx += ecore_chain_get_unusable_per_page(p_chain);
	}

	/* Complete current segment of eq entries */
	while (fw_cons_idx != ecore_chain_get_cons_idx(p_chain)) {
		struct event_ring_entry *p_eqe = ecore_chain_consume(p_chain);
		if (!p_eqe) {
			rc = ECORE_INVAL;
			break;
		}

		DP_VERBOSE(p_hwfn, ECORE_MSG_SPQ,
				"op %x prot %x res0 %x echo %x "
				"fwret %x flags %x\n", p_eqe->opcode,
			   p_eqe->protocol_id,	/* Event Protocol ID */
			   p_eqe->reserved0,	/* Reserved */
			   OSAL_LE16_TO_CPU(p_eqe->echo),
			   p_eqe->fw_return_code,    /* FW return code for SP
						      * ramrods
						      */
			   p_eqe->flags);

		if (GET_FIELD(p_eqe->flags, EVENT_RING_ENTRY_ASYNC)) {
			if (ecore_async_event_completion(p_hwfn, p_eqe))
				rc = ECORE_INVAL;
		} else if (ecore_spq_completion(p_hwfn,
						p_eqe->echo,
						p_eqe->fw_return_code,
						&p_eqe->data)) {
			rc = ECORE_INVAL;
		}

		ecore_chain_recycle_consumed(p_chain);
	}

	ecore_eq_prod_update(p_hwfn, ecore_chain_get_prod_idx(p_chain));

	return rc;
}

struct ecore_eq *ecore_eq_alloc(struct ecore_hwfn *p_hwfn, u16 num_elem)
{
	struct ecore_eq *p_eq;

	/* Allocate EQ struct */
	p_eq = OSAL_ZALLOC(p_hwfn->p_dev, GFP_KERNEL, sizeof(struct ecore_eq));
	if (!p_eq) {
		DP_NOTICE(p_hwfn, true,
			  "Failed to allocate `struct ecore_eq'\n");
		return OSAL_NULL;
	}

	/* Allocate and initialize EQ chain*/
	if (ecore_chain_alloc(p_hwfn->p_dev,
			      ECORE_CHAIN_USE_TO_PRODUCE,
			      ECORE_CHAIN_MODE_PBL,
			      ECORE_CHAIN_CNT_TYPE_U16,
			      num_elem,
			      sizeof(union event_ring_element), &p_eq->chain)) {
		DP_NOTICE(p_hwfn, true, "Failed to allocate eq chain\n");
		goto eq_allocate_fail;
	}

	/* register EQ completion on the SP SB */
	ecore_int_register_cb(p_hwfn,
			      ecore_eq_completion,
			      p_eq, &p_eq->eq_sb_index, &p_eq->p_fw_cons);

	return p_eq;

eq_allocate_fail:
	ecore_eq_free(p_hwfn, p_eq);
	return OSAL_NULL;
}

void ecore_eq_setup(struct ecore_hwfn *p_hwfn, struct ecore_eq *p_eq)
{
	ecore_chain_reset(&p_eq->chain);
}

void ecore_eq_free(struct ecore_hwfn *p_hwfn, struct ecore_eq *p_eq)
{
	if (!p_eq)
		return;
	ecore_chain_free(p_hwfn->p_dev, &p_eq->chain);
	OSAL_FREE(p_hwfn->p_dev, p_eq);
}

/***************************************************************************
* CQE API - manipulate EQ functionality
***************************************************************************/
static enum _ecore_status_t ecore_cqe_completion(struct ecore_hwfn *p_hwfn,
						 struct eth_slow_path_rx_cqe
						 *cqe,
						 enum protocol_type protocol)
{
	if (IS_VF(p_hwfn->p_dev))
		return OSAL_VF_CQE_COMPLETION(p_hwfn, cqe, protocol);

	/* @@@tmp - it's possible we'll eventually want to handle some
	 * actual commands that can arrive here, but for now this is only
	 * used to complete the ramrod using the echo value on the cqe
	 */
	return ecore_spq_completion(p_hwfn, cqe->echo, 0, OSAL_NULL);
}

enum _ecore_status_t ecore_eth_cqe_completion(struct ecore_hwfn *p_hwfn,
					      struct eth_slow_path_rx_cqe *cqe)
{
	enum _ecore_status_t rc;

	rc = ecore_cqe_completion(p_hwfn, cqe, PROTOCOLID_ETH);
	if (rc) {
		DP_NOTICE(p_hwfn, true,
			  "Failed to handle RXQ CQE [cmd 0x%02x]\n",
			  cqe->ramrod_cmd_id);
	}

	return rc;
}

/***************************************************************************
 * Slow hwfn Queue (spq)
 ***************************************************************************/
void ecore_spq_setup(struct ecore_hwfn *p_hwfn)
{
	struct ecore_spq *p_spq = p_hwfn->p_spq;
	struct ecore_spq_entry *p_virt = OSAL_NULL;
	dma_addr_t p_phys = 0;
	u32 i, capacity;

	OSAL_LIST_INIT(&p_spq->pending);
	OSAL_LIST_INIT(&p_spq->completion_pending);
	OSAL_LIST_INIT(&p_spq->free_pool);
	OSAL_LIST_INIT(&p_spq->unlimited_pending);
	OSAL_SPIN_LOCK_INIT(&p_spq->lock);

	/* SPQ empty pool */
	p_phys = p_spq->p_phys + OFFSETOF(struct ecore_spq_entry, ramrod);
	p_virt = p_spq->p_virt;

	capacity = ecore_chain_get_capacity(&p_spq->chain);
	for (i = 0; i < capacity; i++) {
		p_virt->elem.data_ptr.hi = DMA_HI_LE(p_phys);
		p_virt->elem.data_ptr.lo = DMA_LO_LE(p_phys);

		OSAL_LIST_PUSH_TAIL(&p_virt->list, &p_spq->free_pool);

		p_virt++;
		p_phys += sizeof(struct ecore_spq_entry);
	}

	/* Statistics */
	p_spq->normal_count = 0;
	p_spq->comp_count = 0;
	p_spq->comp_sent_count = 0;
	p_spq->unlimited_pending_count = 0;

	OSAL_MEM_ZERO(p_spq->p_comp_bitmap,
		      SPQ_COMP_BMAP_SIZE * sizeof(unsigned long));
	p_spq->comp_bitmap_idx = 0;

	/* SPQ cid, cannot fail */
	ecore_cxt_acquire_cid(p_hwfn, PROTOCOLID_CORE, &p_spq->cid);
	ecore_spq_hw_initialize(p_hwfn, p_spq);

	/* reset the chain itself */
	ecore_chain_reset(&p_spq->chain);
}

enum _ecore_status_t ecore_spq_alloc(struct ecore_hwfn *p_hwfn)
{
	struct ecore_spq_entry *p_virt = OSAL_NULL;
	struct ecore_spq *p_spq = OSAL_NULL;
	dma_addr_t p_phys = 0;
	u32 capacity;

	/* SPQ struct */
	p_spq =
	    OSAL_ZALLOC(p_hwfn->p_dev, GFP_KERNEL, sizeof(struct ecore_spq));
	if (!p_spq) {
		DP_NOTICE(p_hwfn, true,
			  "Failed to allocate `struct ecore_spq'\n");
		return ECORE_NOMEM;
	}

	/* SPQ ring  */
	if (ecore_chain_alloc(p_hwfn->p_dev, ECORE_CHAIN_USE_TO_PRODUCE,
			ECORE_CHAIN_MODE_SINGLE, ECORE_CHAIN_CNT_TYPE_U16, 0,
			/* N/A when the mode is SINGLE */
			sizeof(struct slow_path_element), &p_spq->chain)) {
		DP_NOTICE(p_hwfn, true, "Failed to allocate spq chain\n");
		goto spq_allocate_fail;
	}

	/* allocate and fill the SPQ elements (incl. ramrod data list) */
	capacity = ecore_chain_get_capacity(&p_spq->chain);
	p_virt = OSAL_DMA_ALLOC_COHERENT(p_hwfn->p_dev, &p_phys,
					 capacity *
					 sizeof(struct ecore_spq_entry));
	if (!p_virt)
		goto spq_allocate_fail;

	p_spq->p_virt = p_virt;
	p_spq->p_phys = p_phys;

	OSAL_SPIN_LOCK_ALLOC(p_hwfn, &p_spq->lock);

	p_hwfn->p_spq = p_spq;
	return ECORE_SUCCESS;

spq_allocate_fail:
	ecore_chain_free(p_hwfn->p_dev, &p_spq->chain);
	OSAL_FREE(p_hwfn->p_dev, p_spq);
	return ECORE_NOMEM;
}

void ecore_spq_free(struct ecore_hwfn *p_hwfn)
{
	struct ecore_spq *p_spq = p_hwfn->p_spq;
	u32 capacity;

	if (!p_spq)
		return;

	if (p_spq->p_virt) {
		capacity = ecore_chain_get_capacity(&p_spq->chain);
		OSAL_DMA_FREE_COHERENT(p_hwfn->p_dev,
				       p_spq->p_virt,
				       p_spq->p_phys,
				       capacity *
				       sizeof(struct ecore_spq_entry));
	}

	ecore_chain_free(p_hwfn->p_dev, &p_spq->chain);
	OSAL_SPIN_LOCK_DEALLOC(&p_spq->lock);
	OSAL_FREE(p_hwfn->p_dev, p_spq);
}

enum _ecore_status_t
ecore_spq_get_entry(struct ecore_hwfn *p_hwfn, struct ecore_spq_entry **pp_ent)
{
	struct ecore_spq *p_spq = p_hwfn->p_spq;
	struct ecore_spq_entry *p_ent = OSAL_NULL;

	OSAL_SPIN_LOCK(&p_spq->lock);

	if (OSAL_LIST_IS_EMPTY(&p_spq->free_pool)) {
		p_ent = OSAL_ZALLOC(p_hwfn->p_dev, GFP_ATOMIC,
				    sizeof(struct ecore_spq_entry));
		if (!p_ent) {
			OSAL_SPIN_UNLOCK(&p_spq->lock);
			DP_NOTICE(p_hwfn, true,
				  "Failed to allocate an SPQ entry"
				  " for a pending ramrod\n");
			return ECORE_NOMEM;
		}
		p_ent->queue = &p_spq->unlimited_pending;
	} else {
		p_ent = OSAL_LIST_FIRST_ENTRY(&p_spq->free_pool,
					      struct ecore_spq_entry, list);
		OSAL_LIST_REMOVE_ENTRY(&p_ent->list, &p_spq->free_pool);
		p_ent->queue = &p_spq->pending;
	}

	*pp_ent = p_ent;

	OSAL_SPIN_UNLOCK(&p_spq->lock);

	return ECORE_SUCCESS;
}

/* Locked variant; Should be called while the SPQ lock is taken */
static void __ecore_spq_return_entry(struct ecore_hwfn *p_hwfn,
				     struct ecore_spq_entry *p_ent)
{
	OSAL_LIST_PUSH_TAIL(&p_ent->list, &p_hwfn->p_spq->free_pool);
}

void ecore_spq_return_entry(struct ecore_hwfn *p_hwfn,
			    struct ecore_spq_entry *p_ent)
{
	OSAL_SPIN_LOCK(&p_hwfn->p_spq->lock);
	__ecore_spq_return_entry(p_hwfn, p_ent);
	OSAL_SPIN_UNLOCK(&p_hwfn->p_spq->lock);
}

/**
 * @brief ecore_spq_add_entry - adds a new entry to the pending
 *        list. Should be used while lock is being held.
 *
 * Addes an entry to the pending list is there is room (en empty
 * element is available in the free_pool), or else places the
 * entry in the unlimited_pending pool.
 *
 * @param p_hwfn
 * @param p_ent
 * @param priority
 *
 * @return enum _ecore_status_t
 */
static enum _ecore_status_t
ecore_spq_add_entry(struct ecore_hwfn *p_hwfn,
		    struct ecore_spq_entry *p_ent, enum spq_priority priority)
{
	struct ecore_spq *p_spq = p_hwfn->p_spq;

	if (p_ent->queue == &p_spq->unlimited_pending) {
		if (OSAL_LIST_IS_EMPTY(&p_spq->free_pool)) {
			OSAL_LIST_PUSH_TAIL(&p_ent->list,
					    &p_spq->unlimited_pending);
			p_spq->unlimited_pending_count++;

			return ECORE_SUCCESS;
		} else {
			struct ecore_spq_entry *p_en2;

			p_en2 = OSAL_LIST_FIRST_ENTRY(&p_spq->free_pool,
						     struct ecore_spq_entry,
						     list);
			OSAL_LIST_REMOVE_ENTRY(&p_en2->list, &p_spq->free_pool);

			/* Copy the ring element physical pointer to the new
			 * entry, since we are about to override the entire ring
			 * entry and don't want to lose the pointer.
			 */
			p_ent->elem.data_ptr = p_en2->elem.data_ptr;

			/* Setting the cookie to the comp_done of the
			 * new element.
			 */
			if (p_ent->comp_cb.cookie == &p_ent->comp_done)
				p_ent->comp_cb.cookie = &p_en2->comp_done;

			*p_en2 = *p_ent;

				OSAL_FREE(p_hwfn->p_dev, p_ent);

			p_ent = p_en2;
		}
	}

	/* entry is to be placed in 'pending' queue */
	switch (priority) {
	case ECORE_SPQ_PRIORITY_NORMAL:
		OSAL_LIST_PUSH_TAIL(&p_ent->list, &p_spq->pending);
		p_spq->normal_count++;
		break;
	case ECORE_SPQ_PRIORITY_HIGH:
		OSAL_LIST_PUSH_HEAD(&p_ent->list, &p_spq->pending);
		p_spq->high_count++;
		break;
	default:
		return ECORE_INVAL;
	}

	return ECORE_SUCCESS;
}

/***************************************************************************
 * Accessor
 ***************************************************************************/

u32 ecore_spq_get_cid(struct ecore_hwfn *p_hwfn)
{
	if (!p_hwfn->p_spq)
		return 0xffffffff;	/* illegal */
	return p_hwfn->p_spq->cid;
}

/***************************************************************************
 * Posting new Ramrods
 ***************************************************************************/

static enum _ecore_status_t ecore_spq_post_list(struct ecore_hwfn *p_hwfn,
						osal_list_t *head,
						u32 keep_reserve)
{
	struct ecore_spq *p_spq = p_hwfn->p_spq;
	enum _ecore_status_t rc;

	/* TODO - implementation might be wasteful; will always keep room
	 * for an additional high priority ramrod (even if one is already
	 * pending FW)
	 */
	while (ecore_chain_get_elem_left(&p_spq->chain) > keep_reserve &&
	       !OSAL_LIST_IS_EMPTY(head)) {
		struct ecore_spq_entry *p_ent =
		    OSAL_LIST_FIRST_ENTRY(head, struct ecore_spq_entry, list);
			OSAL_LIST_REMOVE_ENTRY(&p_ent->list, head);
		OSAL_LIST_PUSH_TAIL(&p_ent->list, &p_spq->completion_pending);
			p_spq->comp_sent_count++;

			rc = ecore_spq_hw_post(p_hwfn, p_spq, p_ent);
			if (rc) {
				OSAL_LIST_REMOVE_ENTRY(&p_ent->list,
						    &p_spq->completion_pending);
				__ecore_spq_return_entry(p_hwfn, p_ent);
				return rc;
			}
		}

	return ECORE_SUCCESS;
}

static enum _ecore_status_t ecore_spq_pend_post(struct ecore_hwfn *p_hwfn)
{
	enum _ecore_status_t rc = ECORE_NOTIMPL;
	struct ecore_spq *p_spq = p_hwfn->p_spq;
	struct ecore_spq_entry *p_ent = OSAL_NULL;

	while (!OSAL_LIST_IS_EMPTY(&p_spq->free_pool)) {
		if (OSAL_LIST_IS_EMPTY(&p_spq->unlimited_pending))
			break;

		p_ent = OSAL_LIST_FIRST_ENTRY(&p_spq->unlimited_pending,
					      struct ecore_spq_entry, list);
		if (!p_ent)
			return ECORE_INVAL;

		OSAL_LIST_REMOVE_ENTRY(&p_ent->list, &p_spq->unlimited_pending);

		ecore_spq_add_entry(p_hwfn, p_ent, p_ent->priority);
	}

	rc = ecore_spq_post_list(p_hwfn,
				 &p_spq->pending, SPQ_HIGH_PRI_RESERVE_DEFAULT);
	if (rc)
		return rc;

	return ECORE_SUCCESS;
}

enum _ecore_status_t ecore_spq_post(struct ecore_hwfn *p_hwfn,
				    struct ecore_spq_entry *p_ent,
				    u8 *fw_return_code)
{
	enum _ecore_status_t rc = ECORE_SUCCESS;
	struct ecore_spq *p_spq = p_hwfn ? p_hwfn->p_spq : OSAL_NULL;
	bool b_ret_ent = true;

	if (!p_hwfn)
		return ECORE_INVAL;

	if (!p_ent) {
		DP_NOTICE(p_hwfn, true, "Got a NULL pointer\n");
		return ECORE_INVAL;
	}

	if (p_hwfn->p_dev->recov_in_prog) {
		DP_VERBOSE(p_hwfn, ECORE_MSG_SPQ,
			   "Recovery is in progress -> skip spq post"
			   " [cmd %02x protocol %02x]\n",
			   p_ent->elem.hdr.cmd_id, p_ent->elem.hdr.protocol_id);
		/* Return success to let the flows to be completed successfully
		 * w/o any error handling.
		 */
		return ECORE_SUCCESS;
	}

	OSAL_SPIN_LOCK(&p_spq->lock);

	/* Complete the entry */
	rc = ecore_spq_fill_entry(p_hwfn, p_ent);

	/* Check return value after LOCK is taken for cleaner error flow */
	if (rc)
		goto spq_post_fail;

	/* Add the request to the pending queue */
	rc = ecore_spq_add_entry(p_hwfn, p_ent, p_ent->priority);
	if (rc)
		goto spq_post_fail;

	rc = ecore_spq_pend_post(p_hwfn);
	if (rc) {
		/* Since it's possible that pending failed for a different
		 * entry [although unlikely], the failed entry was already
		 * dealt with; No need to return it here.
		 */
		b_ret_ent = false;
		goto spq_post_fail;
	}

	OSAL_SPIN_UNLOCK(&p_spq->lock);

	if (p_ent->comp_mode == ECORE_SPQ_MODE_EBLOCK) {
		/* For entries in ECORE BLOCK mode, the completion code cannot
		 * perform the necessary cleanup - if it did, we couldn't
		 * access p_ent here to see whether it's successful or not.
		 * Thus, after gaining the answer perform the cleanup here.
		 */
		rc = ecore_spq_block(p_hwfn, p_ent, fw_return_code);
		if (rc)
			goto spq_post_fail2;

		/* return to pool */
		ecore_spq_return_entry(p_hwfn, p_ent);
	}
	return rc;

spq_post_fail2:
	OSAL_SPIN_LOCK(&p_spq->lock);
	OSAL_LIST_REMOVE_ENTRY(&p_ent->list, &p_spq->completion_pending);
	ecore_chain_return_produced(&p_spq->chain);

spq_post_fail:
	/* return to the free pool */
	if (b_ret_ent)
		__ecore_spq_return_entry(p_hwfn, p_ent);
	OSAL_SPIN_UNLOCK(&p_spq->lock);

	return rc;
}

enum _ecore_status_t ecore_spq_completion(struct ecore_hwfn *p_hwfn,
					  __le16 echo,
					  u8 fw_return_code,
					  union event_ring_data *p_data)
{
	struct ecore_spq *p_spq;
	struct ecore_spq_entry *p_ent = OSAL_NULL;
	struct ecore_spq_entry *tmp;
	struct ecore_spq_entry *found = OSAL_NULL;
	enum _ecore_status_t rc;

	if (!p_hwfn)
		return ECORE_INVAL;

	p_spq = p_hwfn->p_spq;
	if (!p_spq)
		return ECORE_INVAL;

	OSAL_SPIN_LOCK(&p_spq->lock);
	OSAL_LIST_FOR_EACH_ENTRY_SAFE(p_ent,
				      tmp,
				      &p_spq->completion_pending,
				      list, struct ecore_spq_entry) {
		if (p_ent->elem.hdr.echo == echo) {
			OSAL_LIST_REMOVE_ENTRY(&p_ent->list,
					       &p_spq->completion_pending);

			/* Avoid overriding of SPQ entries when getting
			 * out-of-order completions, by marking the completions
			 * in a bitmap and increasing the chain consumer only
			 * for the first successive completed entries.
			 */
			SPQ_COMP_BMAP_SET_BIT(p_spq, echo);
			while (SPQ_COMP_BMAP_TEST_BIT(p_spq,
						      p_spq->comp_bitmap_idx)) {
				SPQ_COMP_BMAP_CLEAR_BIT(p_spq,
							p_spq->comp_bitmap_idx);
				p_spq->comp_bitmap_idx++;
				ecore_chain_return_produced(&p_spq->chain);
			}

			p_spq->comp_count++;
			found = p_ent;
			break;
		}

		/* This is debug and should be relatively uncommon - depends
		 * on scenarios which have mutliple per-PF sent ramrods.
		 */
		DP_VERBOSE(p_hwfn, ECORE_MSG_SPQ,
			   "Got completion for echo %04x - doesn't match"
			   " echo %04x in completion pending list\n",
			   OSAL_LE16_TO_CPU(echo),
			   OSAL_LE16_TO_CPU(p_ent->elem.hdr.echo));
	}

	/* Release lock before callback, as callback may post
	 * an additional ramrod.
	 */
	OSAL_SPIN_UNLOCK(&p_spq->lock);

	if (!found) {
		DP_NOTICE(p_hwfn, true,
			  "Failed to find an entry this"
			  " EQE [echo %04x] completes\n",
			  OSAL_LE16_TO_CPU(echo));
		return ECORE_EXISTS;
	}

	DP_VERBOSE(p_hwfn, ECORE_MSG_SPQ,
		   "Complete EQE [echo %04x]: func %p cookie %p)\n",
		   OSAL_LE16_TO_CPU(echo),
		   p_ent->comp_cb.function, p_ent->comp_cb.cookie);
	if (found->comp_cb.function)
		found->comp_cb.function(p_hwfn, found->comp_cb.cookie, p_data,
					fw_return_code);

	if (found->comp_mode != ECORE_SPQ_MODE_EBLOCK) {
		/* EBLOCK is responsible for freeing its own entry */
		ecore_spq_return_entry(p_hwfn, found);
	}

	/* Attempt to post pending requests */
	OSAL_SPIN_LOCK(&p_spq->lock);
	rc = ecore_spq_pend_post(p_hwfn);
	OSAL_SPIN_UNLOCK(&p_spq->lock);

	return rc;
}

struct ecore_consq *ecore_consq_alloc(struct ecore_hwfn *p_hwfn)
{
	struct ecore_consq *p_consq;

	/* Allocate ConsQ struct */
	p_consq =
	    OSAL_ZALLOC(p_hwfn->p_dev, GFP_KERNEL, sizeof(struct ecore_consq));
	if (!p_consq) {
		DP_NOTICE(p_hwfn, true,
			  "Failed to allocate `struct ecore_consq'\n");
		return OSAL_NULL;
	}

	/* Allocate and initialize EQ chain */
	if (ecore_chain_alloc(p_hwfn->p_dev,
			      ECORE_CHAIN_USE_TO_PRODUCE,
			      ECORE_CHAIN_MODE_PBL,
			      ECORE_CHAIN_CNT_TYPE_U16,
			      ECORE_CHAIN_PAGE_SIZE / 0x80,
			      0x80, &p_consq->chain)) {
		DP_NOTICE(p_hwfn, true, "Failed to allocate consq chain");
		goto consq_allocate_fail;
	}

	return p_consq;

consq_allocate_fail:
	ecore_consq_free(p_hwfn, p_consq);
	return OSAL_NULL;
}

void ecore_consq_setup(struct ecore_hwfn *p_hwfn, struct ecore_consq *p_consq)
{
	ecore_chain_reset(&p_consq->chain);
}

void ecore_consq_free(struct ecore_hwfn *p_hwfn, struct ecore_consq *p_consq)
{
	if (!p_consq)
		return;
	ecore_chain_free(p_hwfn->p_dev, &p_consq->chain);
	OSAL_FREE(p_hwfn->p_dev, p_consq);
}
