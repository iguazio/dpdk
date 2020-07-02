/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019-2020 Broadcom
 * All rights reserved.
 */

#ifndef _TF_SESSION_H_
#define _TF_SESSION_H_

#include <stdint.h>
#include <stdlib.h>

#include "bitalloc.h"
#include "tf_core.h"
#include "tf_device.h"
#include "tf_rm.h"
#include "tf_tbl.h"
#include "stack.h"

/**
 * The Session module provides session control support. A session is
 * to the ULP layer known as a session_info instance. The session
 * private data is the actual session.
 *
 * Session manages:
 *   - The device and all the resources related to the device.
 *   - Any session sharing between ULP applications
 */

/** Session defines
 */
#define TF_SESSIONS_MAX	          1          /** max # sessions */
#define TF_SESSION_ID_INVALID     0xFFFFFFFF /** Invalid Session ID define */

/**
 * Number of EM entries. Static for now will be removed
 * when parameter added at a later date. At this stage we
 * are using fixed size entries so that each stack entry
 * represents 4 RT (f/n)blocks. So we take the total block
 * allocation for truflow and divide that by 4.
 */
#define TF_SESSION_TOTAL_FN_BLOCKS (1024 * 8) /* 8K blocks */
#define TF_SESSION_EM_ENTRY_SIZE 4 /* 4 blocks per entry */
#define TF_SESSION_EM_POOL_SIZE \
	(TF_SESSION_TOTAL_FN_BLOCKS / TF_SESSION_EM_ENTRY_SIZE)

/** Session
 *
 * Shared memory containing private TruFlow session information.
 * Through this structure the session can keep track of resource
 * allocations and (if so configured) any shadow copy of flow
 * information.
 *
 * Memory is assigned to the Truflow instance by way of
 * tf_open_session. Memory is allocated and owned by i.e. ULP.
 *
 * Access control to this shared memory is handled by the spin_lock in
 * tf_session_info.
 */
struct tf_session {
	/** TrueFlow Version. Used to control the structure layout
	 * when sharing sessions. No guarantee that a secondary
	 * process would come from the same version of an executable.
	 */
	struct tf_session_version ver;

	/** Session ID, allocated by FW on tf_open_session() */
	union tf_session_id session_id;

	/**
	 * String containing name of control channel interface to be
	 * used for this session to communicate with firmware.
	 *
	 * ctrl_chan_name will be used as part of a name for any
	 * shared memory allocation.
	 */
	char ctrl_chan_name[TF_SESSION_NAME_MAX];

	/**
	 * Boolean controlling the use and availability of shadow
	 * copy. Shadow copy will allow the TruFlow Core to keep track
	 * of resource content on the firmware side without having to
	 * query firmware. Additional private session core_data will
	 * be allocated if this boolean is set to 'true', default
	 * 'false'.
	 *
	 * Size of memory depends on the NVM Resource settings for the
	 * control channel.
	 */
	bool shadow_copy;

	/**
	 * Session Reference Count. To keep track of functions per
	 * session the ref_count is incremented. There is also a
	 * parallel TruFlow Firmware ref_count in case the TruFlow
	 * Core goes away without informing the Firmware.
	 */
	uint8_t ref_count;

	/** Device handle */
	struct tf_dev_info *dev;

	/** Session HW and SRAM resources */
	struct tf_rm_db resc;

	/* Session HW resource pools */

	/** RX L2 CTXT TCAM Pool */
	BITALLOC_INST(TF_L2_CTXT_TCAM_POOL_NAME_RX, TF_NUM_L2_CTXT_TCAM);
	/** TX L2 CTXT TCAM Pool */
	BITALLOC_INST(TF_L2_CTXT_TCAM_POOL_NAME_TX, TF_NUM_L2_CTXT_TCAM);

	/** RX Profile Func Pool */
	BITALLOC_INST(TF_PROF_FUNC_POOL_NAME_RX, TF_NUM_PROF_FUNC);
	/** TX Profile Func Pool */
	BITALLOC_INST(TF_PROF_FUNC_POOL_NAME_TX, TF_NUM_PROF_FUNC);

	/** RX Profile TCAM Pool */
	BITALLOC_INST(TF_PROF_TCAM_POOL_NAME_RX, TF_NUM_PROF_TCAM);
	/** TX Profile TCAM Pool */
	BITALLOC_INST(TF_PROF_TCAM_POOL_NAME_TX, TF_NUM_PROF_TCAM);

	/** RX EM Profile ID Pool */
	BITALLOC_INST(TF_EM_PROF_ID_POOL_NAME_RX, TF_NUM_EM_PROF_ID);
	/** TX EM Key Pool */
	BITALLOC_INST(TF_EM_PROF_ID_POOL_NAME_TX, TF_NUM_EM_PROF_ID);

	/** RX WC Profile Pool */
	BITALLOC_INST(TF_WC_TCAM_PROF_ID_POOL_NAME_RX, TF_NUM_WC_PROF_ID);
	/** TX WC Profile Pool */
	BITALLOC_INST(TF_WC_TCAM_PROF_ID_POOL_NAME_TX, TF_NUM_WC_PROF_ID);

	/* TBD, how do we want to handle EM records ?*/
	/* EM Records are not controlled by way of a pool */

	/** RX WC TCAM Pool */
	BITALLOC_INST(TF_WC_TCAM_POOL_NAME_RX, TF_NUM_WC_TCAM_ROW);
	/** TX WC TCAM Pool */
	BITALLOC_INST(TF_WC_TCAM_POOL_NAME_TX, TF_NUM_WC_TCAM_ROW);

	/** RX Meter Profile Pool */
	BITALLOC_INST(TF_METER_PROF_POOL_NAME_RX, TF_NUM_METER_PROF);
	/** TX Meter Profile Pool */
	BITALLOC_INST(TF_METER_PROF_POOL_NAME_TX, TF_NUM_METER_PROF);

	/** RX Meter Instance Pool */
	BITALLOC_INST(TF_METER_INST_POOL_NAME_RX, TF_NUM_METER);
	/** TX Meter Pool */
	BITALLOC_INST(TF_METER_INST_POOL_NAME_TX, TF_NUM_METER);

	/** RX Mirror Configuration Pool*/
	BITALLOC_INST(TF_MIRROR_POOL_NAME_RX, TF_NUM_MIRROR);
	/** RX Mirror Configuration Pool */
	BITALLOC_INST(TF_MIRROR_POOL_NAME_TX, TF_NUM_MIRROR);

	/** RX UPAR Pool */
	BITALLOC_INST(TF_UPAR_POOL_NAME_RX, TF_NUM_UPAR);
	/** TX UPAR Pool */
	BITALLOC_INST(TF_UPAR_POOL_NAME_TX, TF_NUM_UPAR);

	/** RX SP TCAM Pool */
	BITALLOC_INST(TF_SP_TCAM_POOL_NAME_RX, TF_NUM_SP_TCAM);
	/** TX SP TCAM Pool */
	BITALLOC_INST(TF_SP_TCAM_POOL_NAME_TX, TF_NUM_SP_TCAM);

	/** RX FKB Pool */
	BITALLOC_INST(TF_FKB_POOL_NAME_RX, TF_NUM_FKB);
	/** TX FKB Pool */
	BITALLOC_INST(TF_FKB_POOL_NAME_TX, TF_NUM_FKB);

	/** RX Table Scope Pool */
	BITALLOC_INST(TF_TBL_SCOPE_POOL_NAME_RX, TF_NUM_TBL_SCOPE);
	/** TX Table Scope Pool */
	BITALLOC_INST(TF_TBL_SCOPE_POOL_NAME_TX, TF_NUM_TBL_SCOPE);

	/** RX L2 Func Pool */
	BITALLOC_INST(TF_L2_FUNC_POOL_NAME_RX, TF_NUM_L2_FUNC);
	/** TX L2 Func Pool */
	BITALLOC_INST(TF_L2_FUNC_POOL_NAME_TX, TF_NUM_L2_FUNC);

	/** RX Epoch0 Pool */
	BITALLOC_INST(TF_EPOCH0_POOL_NAME_RX, TF_NUM_EPOCH0);
	/** TX Epoch0 Pool */
	BITALLOC_INST(TF_EPOCH0_POOL_NAME_TX, TF_NUM_EPOCH0);

	/** TX Epoch1 Pool */
	BITALLOC_INST(TF_EPOCH1_POOL_NAME_RX, TF_NUM_EPOCH1);
	/** TX Epoch1 Pool */
	BITALLOC_INST(TF_EPOCH1_POOL_NAME_TX, TF_NUM_EPOCH1);

	/** RX MetaData Profile Pool */
	BITALLOC_INST(TF_METADATA_POOL_NAME_RX, TF_NUM_METADATA);
	/** TX MetaData Profile Pool */
	BITALLOC_INST(TF_METADATA_POOL_NAME_TX, TF_NUM_METADATA);

	/** RX Connection Tracking State Pool */
	BITALLOC_INST(TF_CT_STATE_POOL_NAME_RX, TF_NUM_CT_STATE);
	/** TX Connection Tracking State Pool */
	BITALLOC_INST(TF_CT_STATE_POOL_NAME_TX, TF_NUM_CT_STATE);

	/** RX Range Profile Pool */
	BITALLOC_INST(TF_RANGE_PROF_POOL_NAME_RX, TF_NUM_RANGE_PROF);
	/** TX Range Profile Pool */
	BITALLOC_INST(TF_RANGE_PROF_POOL_NAME_TX, TF_NUM_RANGE_PROF);

	/** RX Range Pool */
	BITALLOC_INST(TF_RANGE_ENTRY_POOL_NAME_RX, TF_NUM_RANGE_ENTRY);
	/** TX Range Pool */
	BITALLOC_INST(TF_RANGE_ENTRY_POOL_NAME_TX, TF_NUM_RANGE_ENTRY);

	/** RX LAG Pool */
	BITALLOC_INST(TF_LAG_ENTRY_POOL_NAME_RX, TF_NUM_LAG_ENTRY);
	/** TX LAG Pool */
	BITALLOC_INST(TF_LAG_ENTRY_POOL_NAME_TX, TF_NUM_LAG_ENTRY);

	/* Session SRAM pools */

	/** RX Full Action Record Pool */
	BITALLOC_INST(TF_SRAM_FULL_ACTION_POOL_NAME_RX,
		      TF_RSVD_SRAM_FULL_ACTION_RX);
	/** TX Full Action Record Pool */
	BITALLOC_INST(TF_SRAM_FULL_ACTION_POOL_NAME_TX,
		      TF_RSVD_SRAM_FULL_ACTION_TX);

	/** RX Multicast Group Pool, only RX is supported */
	BITALLOC_INST(TF_SRAM_MCG_POOL_NAME_RX,
		      TF_RSVD_SRAM_MCG_RX);

	/** RX Encap 8B Pool*/
	BITALLOC_INST(TF_SRAM_ENCAP_8B_POOL_NAME_RX,
		      TF_RSVD_SRAM_ENCAP_8B_RX);
	/** TX Encap 8B Pool*/
	BITALLOC_INST(TF_SRAM_ENCAP_8B_POOL_NAME_TX,
		      TF_RSVD_SRAM_ENCAP_8B_TX);

	/** RX Encap 16B Pool */
	BITALLOC_INST(TF_SRAM_ENCAP_16B_POOL_NAME_RX,
		      TF_RSVD_SRAM_ENCAP_16B_RX);
	/** TX Encap 16B Pool */
	BITALLOC_INST(TF_SRAM_ENCAP_16B_POOL_NAME_TX,
		      TF_RSVD_SRAM_ENCAP_16B_TX);

	/** TX Encap 64B Pool, only TX is supported */
	BITALLOC_INST(TF_SRAM_ENCAP_64B_POOL_NAME_TX,
		      TF_RSVD_SRAM_ENCAP_64B_TX);

	/** RX Source Properties SMAC Pool */
	BITALLOC_INST(TF_SRAM_SP_SMAC_POOL_NAME_RX,
		      TF_RSVD_SRAM_SP_SMAC_RX);
	/** TX Source Properties SMAC Pool */
	BITALLOC_INST(TF_SRAM_SP_SMAC_POOL_NAME_TX,
		      TF_RSVD_SRAM_SP_SMAC_TX);

	/** TX Source Properties SMAC IPv4 Pool, only TX is supported */
	BITALLOC_INST(TF_SRAM_SP_SMAC_IPV4_POOL_NAME_TX,
		      TF_RSVD_SRAM_SP_SMAC_IPV4_TX);

	/** TX Source Properties SMAC IPv6 Pool, only TX is supported */
	BITALLOC_INST(TF_SRAM_SP_SMAC_IPV6_POOL_NAME_TX,
		      TF_RSVD_SRAM_SP_SMAC_IPV6_TX);

	/** RX Counter 64B Pool */
	BITALLOC_INST(TF_SRAM_STATS_64B_POOL_NAME_RX,
		      TF_RSVD_SRAM_COUNTER_64B_RX);
	/** TX Counter 64B Pool */
	BITALLOC_INST(TF_SRAM_STATS_64B_POOL_NAME_TX,
		      TF_RSVD_SRAM_COUNTER_64B_TX);

	/** RX NAT Source Port Pool */
	BITALLOC_INST(TF_SRAM_NAT_SPORT_POOL_NAME_RX,
		      TF_RSVD_SRAM_NAT_SPORT_RX);
	/** TX NAT Source Port Pool */
	BITALLOC_INST(TF_SRAM_NAT_SPORT_POOL_NAME_TX,
		      TF_RSVD_SRAM_NAT_SPORT_TX);

	/** RX NAT Destination Port Pool */
	BITALLOC_INST(TF_SRAM_NAT_DPORT_POOL_NAME_RX,
		      TF_RSVD_SRAM_NAT_DPORT_RX);
	/** TX NAT Destination Port Pool */
	BITALLOC_INST(TF_SRAM_NAT_DPORT_POOL_NAME_TX,
		      TF_RSVD_SRAM_NAT_DPORT_TX);

	/** RX NAT Source IPv4 Pool */
	BITALLOC_INST(TF_SRAM_NAT_S_IPV4_POOL_NAME_RX,
		      TF_RSVD_SRAM_NAT_S_IPV4_RX);
	/** TX NAT Source IPv4 Pool */
	BITALLOC_INST(TF_SRAM_NAT_S_IPV4_POOL_NAME_TX,
		      TF_RSVD_SRAM_NAT_S_IPV4_TX);

	/** RX NAT Destination IPv4 Pool */
	BITALLOC_INST(TF_SRAM_NAT_D_IPV4_POOL_NAME_RX,
		      TF_RSVD_SRAM_NAT_D_IPV4_RX);
	/** TX NAT IPv4 Destination Pool */
	BITALLOC_INST(TF_SRAM_NAT_D_IPV4_POOL_NAME_TX,
		      TF_RSVD_SRAM_NAT_D_IPV4_TX);

	/**
	 * Pools not allocated from HCAPI RM
	 */

	/** RX L2 Ctx Remap ID  Pool */
	BITALLOC_INST(TF_L2_CTXT_REMAP_POOL_NAME_RX, TF_NUM_L2_CTXT_TCAM);
	/** TX L2 Ctx Remap ID Pool */
	BITALLOC_INST(TF_L2_CTXT_REMAP_POOL_NAME_TX, TF_NUM_L2_CTXT_TCAM);

	/** CRC32 seed table */
#define TF_LKUP_SEED_MEM_SIZE 512
	uint32_t lkup_em_seed_mem[TF_DIR_MAX][TF_LKUP_SEED_MEM_SIZE];

	/** Lookup3 init values */
	uint32_t lkup_lkup3_init_cfg[TF_DIR_MAX];

	/** Table scope array */
	struct tf_tbl_scope_cb tbl_scopes[TF_NUM_TBL_SCOPE];

	/**
	 * EM Pools
	 */
	struct stack em_pool[TF_DIR_MAX];
};

/**
 * Session open parameter definition
 */
struct tf_session_open_session_parms {
	/**
	 * [in] Pointer to the TF open session configuration
	 */
	struct tf_open_session_parms *open_cfg;
};

/**
 * Session attach parameter definition
 */
struct tf_session_attach_session_parms {
	/**
	 * [in] Pointer to the TF attach session configuration
	 */
	struct tf_attach_session_parms *attach_cfg;
};

/**
 * Session close parameter definition
 */
struct tf_session_close_session_parms {
	uint8_t *ref_count;
	union tf_session_id *session_id;
};

/**
 * @page session Session Management
 *
 * @ref tf_session_open_session
 *
 * @ref tf_session_attach_session
 *
 * @ref tf_session_close_session
 *
 * @ref tf_session_get_session
 *
 * @ref tf_session_get_device
 *
 * @ref tf_session_get_fw_session_id
 */

/**
 * Creates a host session with a corresponding firmware session.
 *
 * [in] tfp
 *   Pointer to TF handle
 *
 * [in] parms
 *   Pointer to the session open parameters
 *
 * Returns
 *   - (0) if successful.
 *   - (-EINVAL) on failure.
 */
int tf_session_open_session(struct tf *tfp,
			    struct tf_session_open_session_parms *parms);

/**
 * Attaches a previous created session.
 *
 * [in] tfp
 *   Pointer to TF handle
 *
 * [in] parms
 *   Pointer to the session attach parameters
 *
 * Returns
 *   - (0) if successful.
 *   - (-EINVAL) on failure.
 */
int tf_session_attach_session(struct tf *tfp,
			      struct tf_session_attach_session_parms *parms);

/**
 * Closes a previous created session.
 *
 * [in] tfp
 *   Pointer to TF handle
 *
 * [in/out] parms
 *   Pointer to the session close parameters.
 *
 * Returns
 *   - (0) if successful.
 *   - (-EINVAL) on failure.
 */
int tf_session_close_session(struct tf *tfp,
			     struct tf_session_close_session_parms *parms);

/**
 * Looks up the private session information from the TF session info.
 *
 * [in] tfp
 *   Pointer to TF handle
 *
 * [out] tfs
 *   Pointer pointer to the session
 *
 * Returns
 *   - (0) if successful.
 *   - (-EINVAL) on failure.
 */
int tf_session_get_session(struct tf *tfp,
			   struct tf_session **tfs);

/**
 * Looks up the device information from the TF Session.
 *
 * [in] tfp
 *   Pointer to TF handle
 *
 * [out] tfd
 *   Pointer pointer to the device
 *
 * Returns
 *   - (0) if successful.
 *   - (-EINVAL) on failure.
 */
int tf_session_get_device(struct tf_session *tfs,
			  struct tf_dev_info **tfd);

/**
 * Looks up the FW session id of the firmware connection for the
 * requested TF handle.
 *
 * [in] tfp
 *   Pointer to TF handle
 *
 * [out] session_id
 *   Pointer to the session_id
 *
 * Returns
 *   - (0) if successful.
 *   - (-EINVAL) on failure.
 */
int tf_session_get_fw_session_id(struct tf *tfp,
				 uint8_t *fw_session_id);

#endif /* _TF_SESSION_H_ */
