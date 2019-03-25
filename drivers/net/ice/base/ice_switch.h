/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2001-2018
 */

#ifndef _ICE_SWITCH_H_
#define _ICE_SWITCH_H_

#include "ice_common.h"
#include "ice_protocol_type.h"

#define ICE_SW_CFG_MAX_BUF_LEN 2048
#define ICE_MAX_SW 256
#define ICE_DFLT_VSI_INVAL 0xff


/* Worst case buffer length for ice_aqc_opc_get_res_alloc */
#define ICE_MAX_RES_TYPES 0x80
#define ICE_AQ_GET_RES_ALLOC_BUF_LEN \
	(ICE_MAX_RES_TYPES * sizeof(struct ice_aqc_get_res_resp_elem))

#define ICE_VSI_INVAL_ID 0xFFFF
#define ICE_INVAL_Q_HANDLE 0xFFFF

/* VSI queue context structure */
struct ice_q_ctx {
	u16  q_handle;
};

/* VSI context structure for add/get/update/free operations */
struct ice_vsi_ctx {
	u16 vsi_num;
	u16 vsis_allocd;
	u16 vsis_unallocated;
	u16 flags;
	struct ice_aqc_vsi_props info;
	struct ice_sched_vsi_info sched;
	u8 alloc_from_pool;
	u16 num_lan_q_entries[ICE_MAX_TRAFFIC_CLASS];
	struct ice_q_ctx *lan_q_ctx[ICE_MAX_TRAFFIC_CLASS];
	struct ice_lock rss_locks;	/* protect rss config in VSI ctx */
	struct LIST_HEAD_TYPE rss_list_head;
};

/* This is to be used by add/update mirror rule Admin Queue command */
struct ice_mir_rule_buf {
	u16 vsi_idx; /* VSI index */

	/* For each VSI, user can specify whether corresponding VSI
	 * should be added/removed to/from mirror rule
	 *
	 * add mirror rule: this should always be TRUE.
	 * update mirror rule:  add(true) or remove(false) VSI to/from
	 * mirror rule
	 */
	u8 add;
};

/* Switch recipe ID enum values are specific to hardware */
enum ice_sw_lkup_type {
	ICE_SW_LKUP_ETHERTYPE = 0,
	ICE_SW_LKUP_MAC = 1,
	ICE_SW_LKUP_MAC_VLAN = 2,
	ICE_SW_LKUP_PROMISC = 3,
	ICE_SW_LKUP_VLAN = 4,
	ICE_SW_LKUP_DFLT = 5,
	ICE_SW_LKUP_ETHERTYPE_MAC = 8,
	ICE_SW_LKUP_PROMISC_VLAN = 9,
	ICE_SW_LKUP_LAST
};

/* type of filter src ID */
enum ice_src_id {
	ICE_SRC_ID_UNKNOWN = 0,
	ICE_SRC_ID_VSI,
	ICE_SRC_ID_QUEUE,
	ICE_SRC_ID_LPORT,
};

struct ice_fltr_info {
	/* Look up information: how to look up packet */
	enum ice_sw_lkup_type lkup_type;
	/* Forward action: filter action to do after lookup */
	enum ice_sw_fwd_act_type fltr_act;
	/* rule ID returned by firmware once filter rule is created */
	u16 fltr_rule_id;
	u16 flag;
#define ICE_FLTR_RX		BIT(0)
#define ICE_FLTR_TX		BIT(1)
#define ICE_FLTR_TX_RX		(ICE_FLTR_RX | ICE_FLTR_TX)

	/* Source VSI for LOOKUP_TX or source port for LOOKUP_RX */
	u16 src;
	enum ice_src_id src_id;

	union {
		struct {
			u8 mac_addr[ETH_ALEN];
		} mac;
		struct {
			u8 mac_addr[ETH_ALEN];
			u16 vlan_id;
		} mac_vlan;
		struct {
			u16 vlan_id;
		} vlan;
		/* Set lkup_type as ICE_SW_LKUP_ETHERTYPE
		 * if just using ethertype as filter. Set lkup_type as
		 * ICE_SW_LKUP_ETHERTYPE_MAC if MAC also needs to be
		 * passed in as filter.
		 */
		struct {
			u16 ethertype;
			u8 mac_addr[ETH_ALEN]; /* optional */
		} ethertype_mac;
	} l_data; /* Make sure to zero out the memory of l_data before using
		   * it or only set the data associated with lookup match
		   * rest everything should be zero
		   */

	/* Depending on filter action */
	union {
		/* queue ID in case of ICE_FWD_TO_Q and starting
		 * queue ID in case of ICE_FWD_TO_QGRP.
		 */
		u16 q_id:11;
		u16 hw_vsi_id:10;
		u16 vsi_id:10;
		u16 vsi_list_id:10;
	} fwd_id;

	/* Sw VSI handle */
	u16 vsi_handle;

	/* Set to num_queues if action is ICE_FWD_TO_QGRP. This field
	 * determines the range of queues the packet needs to be forwarded to.
	 * Note that qgrp_size must be set to a power of 2.
	 */
	u8 qgrp_size;

	/* Rule creations populate these indicators basing on the switch type */
	u8 lb_en;	/* Indicate if packet can be looped back */
	u8 lan_en;	/* Indicate if packet can be forwarded to the uplink */
};

struct ice_adv_lkup_elem {
	enum ice_protocol_type type;
	union ice_prot_hdr h_u;	/* Header values */
	union ice_prot_hdr m_u;	/* Mask of header values to match */
};

struct ice_sw_act_ctrl {
	/* Source VSI for LOOKUP_TX or source port for LOOKUP_RX */
	u16 src;
	u16 flag;
#define ICE_FLTR_RX             BIT(0)
#define ICE_FLTR_TX             BIT(1)
#define ICE_FLTR_TX_RX (ICE_FLTR_RX | ICE_FLTR_TX)

	enum ice_sw_fwd_act_type fltr_act;
	/* Depending on filter action */
	union {
		/* This is a queue ID in case of ICE_FWD_TO_Q and starting
		 * queue ID in case of ICE_FWD_TO_QGRP.
		 */
		u16 q_id:11;
		u16 vsi_id:10;
		u16 hw_vsi_id:10;
		u16 vsi_list_id:10;
	} fwd_id;
	/* software VSI handle */
	u16 vsi_handle;
	u8 qgrp_size;
};

struct ice_adv_rule_info {
	enum ice_sw_tunnel_type tun_type;
	struct ice_sw_act_ctrl sw_act;
	u32 priority;
	u8 rx; /* true means LOOKUP_RX otherwise LOOKUP_TX */
};

/* A collection of one or more four word recipe */
struct ice_sw_recipe {
	/* For a chained recipe the root recipe is what should be used for
	 * programming rules
	 */
	u8 root_rid;
	u8 recp_created;

	/* Number of extraction words */
	u8 n_ext_words;
	/* Protocol ID and Offset pair (extraction word) to describe the
	 * recipe
	 */
	struct ice_fv_word ext_words[ICE_MAX_CHAIN_WORDS];

	/* if this recipe is a collection of other recipe */
	u8 big_recp;

	/* if this recipe is part of another bigger recipe then chain index
	 * corresponding to this recipe
	 */
	u8 chain_idx;

	/* if this recipe is a collection of other recipe then count of other
	 * recipes and recipe IDs of those recipes
	 */
	u8 n_grp_count;

	/* Bit map specifying the IDs associated with this group of recipe */
	ice_declare_bitmap(r_bitmap, ICE_MAX_NUM_RECIPES);

	enum ice_sw_tunnel_type tun_type;

	/* List of type ice_fltr_mgmt_list_entry or adv_rule */
	u8 adv_rule;
	struct LIST_HEAD_TYPE filt_rules;
	struct LIST_HEAD_TYPE filt_replay_rules;

	struct ice_lock filt_rule_lock;	/* protect filter rule structure */

	/* Profiles this recipe should be associated with */
	struct LIST_HEAD_TYPE fv_list;

	/* Profiles this recipe is associated with */
	u8 num_profs, *prof_ids;

	/* This allows user to specify the recipe priority.
	 * For now, this becomes 'fwd_priority' when recipe
	 * is created, usually recipes can have 'fwd' and 'join'
	 * priority.
	 */
	u8 priority;

	struct LIST_HEAD_TYPE rg_list;

	/* AQ buffer associated with this recipe */
	struct ice_aqc_recipe_data_elem *root_buf;
	/* This struct saves the fv_words for a given lookup */
	struct ice_prot_lkup_ext lkup_exts;
};

/* Bookkeeping structure to hold bitmap of VSIs corresponding to VSI list ID */
struct ice_vsi_list_map_info {
	struct LIST_ENTRY_TYPE list_entry;
	ice_declare_bitmap(vsi_map, ICE_MAX_VSI);
	u16 vsi_list_id;
	/* counter to track how many rules are reusing this VSI list */
	u16 ref_cnt;
};

struct ice_fltr_list_entry {
	struct LIST_ENTRY_TYPE list_entry;
	enum ice_status status;
	struct ice_fltr_info fltr_info;
};

/* This defines an entry in the list that maintains MAC or VLAN membership
 * to HW list mapping, since multiple VSIs can subscribe to the same MAC or
 * VLAN. As an optimization the VSI list should be created only when a
 * second VSI becomes a subscriber to the same MAC address. VSI lists are always
 * used for VLAN membership.
 */
struct ice_fltr_mgmt_list_entry {
	/* back pointer to VSI list ID to VSI list mapping */
	struct ice_vsi_list_map_info *vsi_list_info;
	u16 vsi_count;
#define ICE_INVAL_LG_ACT_INDEX 0xffff
	u16 lg_act_idx;
#define ICE_INVAL_SW_MARKER_ID 0xffff
	u16 sw_marker_id;
	struct LIST_ENTRY_TYPE list_entry;
	struct ice_fltr_info fltr_info;
#define ICE_INVAL_COUNTER_ID 0xff
	u8 counter_index;
};

struct ice_adv_fltr_mgmt_list_entry {
	struct LIST_ENTRY_TYPE list_entry;

	struct ice_adv_lkup_elem *lkups;
	struct ice_adv_rule_info rule_info;
	u16 lkups_cnt;
};

enum ice_promisc_flags {
	ICE_PROMISC_UCAST_RX = 0x1,
	ICE_PROMISC_UCAST_TX = 0x2,
	ICE_PROMISC_MCAST_RX = 0x4,
	ICE_PROMISC_MCAST_TX = 0x8,
	ICE_PROMISC_BCAST_RX = 0x10,
	ICE_PROMISC_BCAST_TX = 0x20,
	ICE_PROMISC_VLAN_RX = 0x40,
	ICE_PROMISC_VLAN_TX = 0x80,
};

/* VSI related commands */
enum ice_status
ice_aq_add_vsi(struct ice_hw *hw, struct ice_vsi_ctx *vsi_ctx,
	       struct ice_sq_cd *cd);
enum ice_status
ice_aq_free_vsi(struct ice_hw *hw, struct ice_vsi_ctx *vsi_ctx,
		bool keep_vsi_alloc, struct ice_sq_cd *cd);
enum ice_status
ice_aq_update_vsi(struct ice_hw *hw, struct ice_vsi_ctx *vsi_ctx,
		  struct ice_sq_cd *cd);
enum ice_status
ice_add_vsi(struct ice_hw *hw, u16 vsi_handle, struct ice_vsi_ctx *vsi_ctx,
	    struct ice_sq_cd *cd);
enum ice_status
ice_free_vsi(struct ice_hw *hw, u16 vsi_handle, struct ice_vsi_ctx *vsi_ctx,
	     bool keep_vsi_alloc, struct ice_sq_cd *cd);
enum ice_status
ice_update_vsi(struct ice_hw *hw, u16 vsi_handle, struct ice_vsi_ctx *vsi_ctx,
	       struct ice_sq_cd *cd);
struct ice_vsi_ctx *ice_get_vsi_ctx(struct ice_hw *hw, u16 vsi_handle);
void ice_clear_all_vsi_ctx(struct ice_hw *hw);
enum ice_status
ice_aq_get_vsi_params(struct ice_hw *hw, struct ice_vsi_ctx *vsi_ctx,
		      struct ice_sq_cd *cd);
enum ice_status
ice_aq_add_update_mir_rule(struct ice_hw *hw, u16 rule_type, u16 dest_vsi,
			   u16 count, struct ice_mir_rule_buf *mr_buf,
			   struct ice_sq_cd *cd, u16 *rule_id);
enum ice_status
ice_aq_delete_mir_rule(struct ice_hw *hw, u16 rule_id, bool keep_allocd,
		       struct ice_sq_cd *cd);
enum ice_status
ice_aq_get_storm_ctrl(struct ice_hw *hw, u32 *bcast_thresh, u32 *mcast_thresh,
		      u32 *ctl_bitmask);
enum ice_status
ice_aq_set_storm_ctrl(struct ice_hw *hw, u32 bcast_thresh, u32 mcast_thresh,
		      u32 ctl_bitmask);
/* Switch config */
enum ice_status ice_get_initial_sw_cfg(struct ice_hw *hw);

enum ice_status
ice_alloc_vlan_res_counter(struct ice_hw *hw, u16 *counter_id);
enum ice_status
ice_free_vlan_res_counter(struct ice_hw *hw, u16 counter_id);
enum ice_status
ice_alloc_res_cntr(struct ice_hw *hw, u8 type, u8 alloc_shared, u16 num_items,
		   u16 *counter_id);
enum ice_status
ice_free_res_cntr(struct ice_hw *hw, u8 type, u8 alloc_shared, u16 num_items,
		  u16 counter_id);

/* Switch/bridge related commands */
enum ice_status ice_update_sw_rule_bridge_mode(struct ice_hw *hw);
enum ice_status
ice_alloc_sw(struct ice_hw *hw, bool ena_stats, bool shared_res, u16 *sw_id,
	     u16 *counter_id);
enum ice_status
ice_free_sw(struct ice_hw *hw, u16 sw_id, u16 counter_id);
enum ice_status
ice_aq_get_res_alloc(struct ice_hw *hw, u16 *num_entries, void *buf,
		     u16 buf_size, struct ice_sq_cd *cd);
enum ice_status
ice_aq_get_res_descs(struct ice_hw *hw, u16 num_entries,
		     struct ice_aqc_get_allocd_res_desc_resp *buf,
		     u16 buf_size, u16 res_type, bool res_shared, u16 *desc_id,
		     struct ice_sq_cd *cd);
enum ice_status
ice_add_vlan(struct ice_hw *hw, struct LIST_HEAD_TYPE *m_list);
enum ice_status ice_add_mac(struct ice_hw *hw, struct LIST_HEAD_TYPE *m_lst);
enum ice_status ice_remove_mac(struct ice_hw *hw, struct LIST_HEAD_TYPE *m_lst);
enum ice_status
ice_add_eth_mac(struct ice_hw *hw, struct LIST_HEAD_TYPE *em_list);
enum ice_status
ice_remove_eth_mac(struct ice_hw *hw, struct LIST_HEAD_TYPE *em_list);
enum ice_status
ice_remove_vlan(struct ice_hw *hw, struct LIST_HEAD_TYPE *v_list);
#ifndef NO_MACVLAN_SUPPORT
enum ice_status
ice_add_mac_vlan(struct ice_hw *hw, struct LIST_HEAD_TYPE *m_list);
enum ice_status
ice_remove_mac_vlan(struct ice_hw *hw, struct LIST_HEAD_TYPE *v_list);
#endif /* !NO_MACVLAN_SUPPORT */

void ice_remove_vsi_fltr(struct ice_hw *hw, u16 vsi_handle);


/* Promisc/defport setup for VSIs */
enum ice_status
ice_cfg_dflt_vsi(struct ice_port_info *pi, u16 vsi_handle, bool set,
		 u8 direction);
enum ice_status
ice_set_vsi_promisc(struct ice_hw *hw, u16 vsi_handle, u8 promisc_mask,
		    u16 vid);
enum ice_status
ice_clear_vsi_promisc(struct ice_hw *hw, u16 vsi_handle, u8 promisc_mask,
		      u16 vid);
enum ice_status
ice_set_vlan_vsi_promisc(struct ice_hw *hw, u16 vsi_handle, u8 promisc_mask,
			 bool rm_vlan_promisc);

/* Get VSIs Promisc/defport settings */
enum ice_status
ice_get_vsi_promisc(struct ice_hw *hw, u16 vsi_handle, u8 *promisc_mask,
		    u16 *vid);
enum ice_status
ice_get_vsi_vlan_promisc(struct ice_hw *hw, u16 vsi_handle, u8 *promisc_mask,
			 u16 *vid);




enum ice_status ice_init_def_sw_recp(struct ice_hw *hw);
u16 ice_get_hw_vsi_num(struct ice_hw *hw, u16 vsi_handle);
bool ice_is_vsi_valid(struct ice_hw *hw, u16 vsi_handle);

enum ice_status ice_replay_vsi_all_fltr(struct ice_hw *hw, u16 vsi_handle);
void ice_rm_all_sw_replay_rule_info(struct ice_hw *hw);

#endif /* _ICE_SWITCH_H_ */
