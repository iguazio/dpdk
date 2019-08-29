/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2001-2019
 */

#ifndef _ICE_COMMON_H_
#define _ICE_COMMON_H_

#include "ice_type.h"
#include "ice_flex_pipe.h"
#include "ice_switch.h"
#include "ice_fdir.h"

enum ice_fw_modes {
	ICE_FW_MODE_NORMAL,
	ICE_FW_MODE_DBG,
	ICE_FW_MODE_REC,
	ICE_FW_MODE_ROLLBACK
};

enum ice_status ice_nvm_validate_checksum(struct ice_hw *hw);

enum ice_status ice_init_hw(struct ice_hw *hw);
void ice_deinit_hw(struct ice_hw *hw);
enum ice_status
ice_get_pfa_module_tlv(struct ice_hw *hw, u16 *module_tlv, u16 *module_tlv_len,
		       u16 module_type);
enum ice_status ice_check_reset(struct ice_hw *hw);
enum ice_status ice_reset(struct ice_hw *hw, enum ice_reset_req req);

enum ice_status ice_create_all_ctrlq(struct ice_hw *hw);
enum ice_status ice_init_all_ctrlq(struct ice_hw *hw);
void ice_shutdown_all_ctrlq(struct ice_hw *hw);
void ice_destroy_all_ctrlq(struct ice_hw *hw);
enum ice_status
ice_clean_rq_elem(struct ice_hw *hw, struct ice_ctl_q_info *cq,
		  struct ice_rq_event_info *e, u16 *pending);
enum ice_status
ice_get_link_status(struct ice_port_info *pi, bool *link_up);
enum ice_status ice_update_link_info(struct ice_port_info *pi);
enum ice_status
ice_acquire_res(struct ice_hw *hw, enum ice_aq_res_ids res,
		enum ice_aq_res_access_type access, u32 timeout);
void ice_release_res(struct ice_hw *hw, enum ice_aq_res_ids res);
enum ice_status
ice_alloc_hw_res(struct ice_hw *hw, u16 type, u16 num, bool btm, u16 *res);
enum ice_status
ice_free_hw_res(struct ice_hw *hw, u16 type, u16 num, u16 *res);
enum ice_status
ice_aq_alloc_free_res(struct ice_hw *hw, u16 num_entries,
		      struct ice_aqc_alloc_free_res_elem *buf, u16 buf_size,
		      enum ice_adminq_opc opc, struct ice_sq_cd *cd);
enum ice_status ice_init_nvm(struct ice_hw *hw);
enum ice_status ice_read_sr_word(struct ice_hw *hw, u16 offset, u16 *data);
enum ice_status
ice_read_sr_buf(struct ice_hw *hw, u16 offset, u16 *words, u16 *data);
enum ice_status
ice_sq_send_cmd(struct ice_hw *hw, struct ice_ctl_q_info *cq,
		struct ice_aq_desc *desc, void *buf, u16 buf_size,
		struct ice_sq_cd *cd);
void ice_clear_pxe_mode(struct ice_hw *hw);

enum ice_status ice_get_caps(struct ice_hw *hw);

/* Define a macro that will align a pointer to point to the next memory address
 * that falls on the given power of 2 (i.e., 2, 4, 8, 16, 32, 64...). For
 * example, given the variable pointer = 0x1006, then after the following call:
 *
 *      pointer = ICE_ALIGN(pointer, 4)
 *
 * ... the value of pointer would equal 0x1008, since 0x1008 is the next
 * address after 0x1006 which is divisible by 4.
 */
#define ICE_ALIGN(ptr, align)	(((ptr) + ((align) - 1)) & ~((align) - 1))

enum ice_status
ice_write_rxq_ctx(struct ice_hw *hw, struct ice_rlan_ctx *rlan_ctx,
		  u32 rxq_index);
#if !defined(NO_UNUSED_CTX_CODE) || defined(AE_DRIVER)
enum ice_status ice_clear_rxq_ctx(struct ice_hw *hw, u32 rxq_index);
enum ice_status
ice_clear_tx_cmpltnq_ctx(struct ice_hw *hw, u32 tx_cmpltnq_index);
enum ice_status
ice_write_tx_cmpltnq_ctx(struct ice_hw *hw,
			 struct ice_tx_cmpltnq_ctx *tx_cmpltnq_ctx,
			 u32 tx_cmpltnq_index);
enum ice_status
ice_clear_tx_drbell_q_ctx(struct ice_hw *hw, u32 tx_drbell_q_index);
enum ice_status
ice_write_tx_drbell_q_ctx(struct ice_hw *hw,
			  struct ice_tx_drbell_q_ctx *tx_drbell_q_ctx,
			  u32 tx_drbell_q_index);
#endif /* !NO_UNUSED_CTX_CODE || AE_DRIVER */

enum ice_status
ice_aq_get_rss_lut(struct ice_hw *hw, u16 vsi_handle, u8 lut_type, u8 *lut,
		   u16 lut_size);
enum ice_status
ice_aq_set_rss_lut(struct ice_hw *hw, u16 vsi_handle, u8 lut_type, u8 *lut,
		   u16 lut_size);
enum ice_status
ice_aq_get_rss_key(struct ice_hw *hw, u16 vsi_handle,
		   struct ice_aqc_get_set_rss_keys *keys);
enum ice_status
ice_aq_set_rss_key(struct ice_hw *hw, u16 vsi_handle,
		   struct ice_aqc_get_set_rss_keys *keys);
enum ice_status
ice_aq_add_lan_txq(struct ice_hw *hw, u8 count,
		   struct ice_aqc_add_tx_qgrp *qg_list, u16 buf_size,
		   struct ice_sq_cd *cd);

bool ice_check_sq_alive(struct ice_hw *hw, struct ice_ctl_q_info *cq);
enum ice_status ice_aq_q_shutdown(struct ice_hw *hw, bool unloading);
void ice_fill_dflt_direct_cmd_desc(struct ice_aq_desc *desc, u16 opcode);
extern const struct ice_ctx_ele ice_tlan_ctx_info[];
enum ice_status
ice_set_ctx(u8 *src_ctx, u8 *dest_ctx, const struct ice_ctx_ele *ce_info);
enum ice_status
ice_aq_send_cmd(struct ice_hw *hw, struct ice_aq_desc *desc,
		void *buf, u16 buf_size, struct ice_sq_cd *cd);
enum ice_status ice_aq_get_fw_ver(struct ice_hw *hw, struct ice_sq_cd *cd);

enum ice_status
ice_aq_send_driver_ver(struct ice_hw *hw, struct ice_driver_ver *dv,
		       struct ice_sq_cd *cd);
enum ice_status
ice_aq_get_phy_caps(struct ice_port_info *pi, bool qual_mods, u8 report_mode,
		    struct ice_aqc_get_phy_caps_data *caps,
		    struct ice_sq_cd *cd);
void
ice_update_phy_type(u64 *phy_type_low, u64 *phy_type_high,
		    u16 link_speeds_bitmap);
enum ice_status
ice_aq_manage_mac_write(struct ice_hw *hw, const u8 *mac_addr, u8 flags,
			struct ice_sq_cd *cd);

enum ice_status ice_clear_pf_cfg(struct ice_hw *hw);
enum ice_status
ice_aq_set_phy_cfg(struct ice_hw *hw, struct ice_port_info *pi,
		   struct ice_aqc_set_phy_cfg_data *cfg, struct ice_sq_cd *cd);
enum ice_status
ice_set_fc(struct ice_port_info *pi, u8 *aq_failures,
	   bool ena_auto_link_update);
void
ice_cfg_phy_fec(struct ice_aqc_set_phy_cfg_data *cfg, enum ice_fec_mode fec);
void
ice_copy_phy_caps_to_cfg(struct ice_aqc_get_phy_caps_data *caps,
			 struct ice_aqc_set_phy_cfg_data *cfg);
enum ice_status
ice_aq_set_link_restart_an(struct ice_port_info *pi, bool ena_link,
			   struct ice_sq_cd *cd);
enum ice_status
ice_aq_set_mac_cfg(struct ice_hw *hw, u16 max_frame_size, struct ice_sq_cd *cd);
enum ice_status
ice_aq_get_link_info(struct ice_port_info *pi, bool ena_lse,
		     struct ice_link_status *link, struct ice_sq_cd *cd);
enum ice_status
ice_aq_set_event_mask(struct ice_hw *hw, u8 port_num, u16 mask,
		      struct ice_sq_cd *cd);
enum ice_status
ice_aq_set_mac_loopback(struct ice_hw *hw, bool ena_lpbk, struct ice_sq_cd *cd);


enum ice_status
ice_aq_set_port_id_led(struct ice_port_info *pi, bool is_orig_mode,
		       struct ice_sq_cd *cd);



enum ice_status
ice_get_ctx(u8 *src_ctx, u8 *dest_ctx, struct ice_ctx_ele *ce_info);
enum ice_status
ice_dis_vsi_txq(struct ice_port_info *pi, u16 vsi_handle, u8 tc, u8 num_queues,
		u16 *q_handle, u16 *q_ids, u32 *q_teids,
		enum ice_disq_rst_src rst_src, u16 vmvf_num,
		struct ice_sq_cd *cd);
enum ice_status
ice_cfg_vsi_lan(struct ice_port_info *pi, u16 vsi_handle, u8 tc_bitmap,
		u16 *max_lanqs);
enum ice_status
ice_ena_vsi_txq(struct ice_port_info *pi, u16 vsi_handle, u8 tc, u16 q_handle,
		u8 num_qgrps, struct ice_aqc_add_tx_qgrp *buf, u16 buf_size,
		struct ice_sq_cd *cd);
enum ice_status ice_replay_vsi(struct ice_hw *hw, u16 vsi_handle);
void ice_replay_post(struct ice_hw *hw);
void ice_sched_replay_agg_vsi_preinit(struct ice_hw *hw);
void ice_sched_replay_agg(struct ice_hw *hw);
enum ice_status ice_sched_replay_tc_node_bw(struct ice_hw *hw);
enum ice_status ice_replay_vsi_agg(struct ice_hw *hw, u16 vsi_handle);
enum ice_status
ice_sched_replay_q_bw(struct ice_port_info *pi, struct ice_q_ctx *q_ctx);
struct ice_q_ctx *
ice_get_lan_q_ctx(struct ice_hw *hw, u16 vsi_handle, u8 tc, u16 q_handle);
enum ice_status
ice_cfg_tc_node_bw_alloc(struct ice_port_info *pi, u8 tc,
			 enum ice_rl_type rl_type, u8 bw_alloc);
enum ice_status ice_cfg_rl_burst_size(struct ice_hw *hw, u32 bytes);
void ice_output_fw_log(struct ice_hw *hw, struct ice_aq_desc *desc, void *buf);
void
ice_stat_update40(struct ice_hw *hw, u32 reg, bool prev_stat_loaded,
		  u64 *prev_stat, u64 *cur_stat);
void
ice_stat_update32(struct ice_hw *hw, u32 reg, bool prev_stat_loaded,
		  u64 *prev_stat, u64 *cur_stat);
void
ice_get_nvm_version(struct ice_hw *hw, u8 *oem_ver, u16 *oem_build,
		    u8 *oem_patch, u8 *ver_hi, u8 *ver_lo);
enum ice_fw_modes ice_get_fw_mode(struct ice_hw *hw);
void ice_print_rollback_msg(struct ice_hw *hw);
enum ice_status
ice_sched_query_elem(struct ice_hw *hw, u32 node_teid,
		     struct ice_aqc_get_elem *buf);
#endif /* _ICE_COMMON_H_ */
