/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2017 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cmdline_parse.h>
#include <cmdline_parse_num.h>
#include <cmdline_parse_string.h>

#include <rte_ethdev.h>
#include <rte_flow.h>
#include <rte_tm.h>

#include "testpmd.h"
#include "cmdline_tm.h"

/** Display TM Error Message */
static void
print_err_msg(struct rte_tm_error *error)
{
	static const char *const errstrlist[] = {
		[RTE_TM_ERROR_TYPE_NONE] = "no error",
		[RTE_TM_ERROR_TYPE_UNSPECIFIED] = "cause unspecified",
		[RTE_TM_ERROR_TYPE_CAPABILITIES]
			= "capability parameter null",
		[RTE_TM_ERROR_TYPE_LEVEL_ID] = "level id",
		[RTE_TM_ERROR_TYPE_WRED_PROFILE]
			= "wred profile null",
		[RTE_TM_ERROR_TYPE_WRED_PROFILE_GREEN] = "wred profile(green)",
		[RTE_TM_ERROR_TYPE_WRED_PROFILE_YELLOW]
			= "wred profile(yellow)",
		[RTE_TM_ERROR_TYPE_WRED_PROFILE_RED] = "wred profile(red)",
		[RTE_TM_ERROR_TYPE_WRED_PROFILE_ID] = "wred profile id",
		[RTE_TM_ERROR_TYPE_SHARED_WRED_CONTEXT_ID]
			= "shared wred context id",
		[RTE_TM_ERROR_TYPE_SHAPER_PROFILE] = "shaper profile null",
		[RTE_TM_ERROR_TYPE_SHAPER_PROFILE_COMMITTED_RATE]
			= "committed rate field (shaper profile)",
		[RTE_TM_ERROR_TYPE_SHAPER_PROFILE_COMMITTED_SIZE]
			= "committed size field (shaper profile)",
		[RTE_TM_ERROR_TYPE_SHAPER_PROFILE_PEAK_RATE]
			= "peak rate field (shaper profile)",
		[RTE_TM_ERROR_TYPE_SHAPER_PROFILE_PEAK_SIZE]
			= "peak size field (shaper profile)",
		[RTE_TM_ERROR_TYPE_SHAPER_PROFILE_PKT_ADJUST_LEN]
			= "packet adjust length field (shaper profile)",
		[RTE_TM_ERROR_TYPE_SHAPER_PROFILE_ID] = "shaper profile id",
		[RTE_TM_ERROR_TYPE_SHARED_SHAPER_ID] = "shared shaper id",
		[RTE_TM_ERROR_TYPE_NODE_PARENT_NODE_ID] = "parent node id",
		[RTE_TM_ERROR_TYPE_NODE_PRIORITY] = "node priority",
		[RTE_TM_ERROR_TYPE_NODE_WEIGHT] = "node weight",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS] = "node parameter null",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS_SHAPER_PROFILE_ID]
			= "shaper profile id field (node params)",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS_SHARED_SHAPER_ID]
			= "shared shaper id field (node params)",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS_N_SHARED_SHAPERS]
			= "num shared shapers field (node params)",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS_WFQ_WEIGHT_MODE]
			= "wfq weght mode field (node params)",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS_N_SP_PRIORITIES]
			= "num strict priorities field (node params)",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS_CMAN]
			= "congestion management mode field (node params)",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS_WRED_PROFILE_ID] =
			"wred profile id field (node params)",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS_SHARED_WRED_CONTEXT_ID]
			= "shared wred context id field (node params)",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS_N_SHARED_WRED_CONTEXTS]
			= "num shared wred contexts field (node params)",
		[RTE_TM_ERROR_TYPE_NODE_PARAMS_STATS]
			= "stats field (node params)",
		[RTE_TM_ERROR_TYPE_NODE_ID] = "node id",
	};

	const char *errstr;
	char buf[64];

	if ((unsigned int)error->type >= RTE_DIM(errstrlist) ||
		!errstrlist[error->type])
		errstr = "unknown type";
	else
		errstr = errstrlist[error->type];

	if (error->cause)
		snprintf(buf, sizeof(buf), "cause: %p, ", error->cause);

	printf("%s: %s%s (error %d)\n", errstr, error->cause ? buf : "",
		error->message ? error->message : "(no stated reason)",
		error->type);
}

/* *** Port TM Capability *** */
struct cmd_show_port_tm_cap_result {
	cmdline_fixed_string_t show;
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t tm;
	cmdline_fixed_string_t cap;
	uint16_t port_id;
};

cmdline_parse_token_string_t cmd_show_port_tm_cap_show =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_cap_result,
		show, "show");
cmdline_parse_token_string_t cmd_show_port_tm_cap_port =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_cap_result,
		port, "port");
cmdline_parse_token_string_t cmd_show_port_tm_cap_tm =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_cap_result,
		tm, "tm");
cmdline_parse_token_string_t cmd_show_port_tm_cap_cap =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_cap_result,
		cap, "cap");
cmdline_parse_token_num_t cmd_show_port_tm_cap_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_show_port_tm_cap_result,
		 port_id, UINT16);

static void cmd_show_port_tm_cap_parsed(void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_show_port_tm_cap_result *res = parsed_result;
	struct rte_tm_capabilities cap;
	struct rte_tm_error error;
	portid_t port_id = res->port_id;
	uint32_t i;
	int ret;

	if (port_id_is_invalid(port_id, ENABLED_WARN))
		return;

	memset(&cap, 0, sizeof(struct rte_tm_capabilities));
	ret = rte_tm_capabilities_get(port_id, &cap, &error);
	if (ret) {
		print_err_msg(&error);
		return;
	}

	printf("\n****   Port TM Capabilities ****\n\n");
	printf("cap.n_nodes_max %" PRIu32 "\n", cap.n_nodes_max);
	printf("cap.n_levels_max %" PRIu32 "\n", cap.n_levels_max);
	printf("cap.non_leaf_nodes_identical %" PRId32 "\n",
		cap.non_leaf_nodes_identical);
	printf("cap.leaf_nodes_identical %" PRId32 "\n",
		cap.leaf_nodes_identical);
	printf("cap.shaper_n_max %u\n", cap.shaper_n_max);
	printf("cap.shaper_private_n_max %" PRIu32 "\n",
		cap.shaper_private_n_max);
	printf("cap.shaper_private_dual_rate_n_max %" PRId32 "\n",
		cap.shaper_private_dual_rate_n_max);
	printf("cap.shaper_private_rate_min %" PRIu64 "\n",
		cap.shaper_private_rate_min);
	printf("cap.shaper_private_rate_max %" PRIu64 "\n",
		cap.shaper_private_rate_max);
	printf("cap.shaper_shared_n_max %" PRIu32 "\n",
		cap.shaper_shared_n_max);
	printf("cap.shaper_shared_n_nodes_per_shaper_max %" PRIu32 "\n",
		cap.shaper_shared_n_nodes_per_shaper_max);
	printf("cap.shaper_shared_n_shapers_per_node_max %" PRIu32 "\n",
		cap.shaper_shared_n_shapers_per_node_max);
	printf("cap.shaper_shared_dual_rate_n_max %" PRIu32 "\n",
		cap.shaper_shared_dual_rate_n_max);
	printf("cap.shaper_shared_rate_min %" PRIu64 "\n",
		cap.shaper_shared_rate_min);
	printf("cap.shaper_shared_rate_max %" PRIu64 "\n",
		cap.shaper_shared_rate_max);
	printf("cap.shaper_pkt_length_adjust_min %" PRId32 "\n",
		cap.shaper_pkt_length_adjust_min);
	printf("cap.shaper_pkt_length_adjust_max %" PRId32 "\n",
		cap.shaper_pkt_length_adjust_max);
	printf("cap.sched_n_children_max %" PRIu32 "\n",
		cap.sched_n_children_max);
	printf("cap.sched_sp_n_priorities_max %" PRIu32 "\n",
		cap.sched_sp_n_priorities_max);
	printf("cap.sched_wfq_n_children_per_group_max %" PRIu32 "\n",
		cap.sched_wfq_n_children_per_group_max);
	printf("cap.sched_wfq_n_groups_max %" PRIu32 "\n",
		cap.sched_wfq_n_groups_max);
	printf("cap.sched_wfq_weight_max %" PRIu32 "\n",
		cap.sched_wfq_weight_max);
	printf("cap.cman_head_drop_supported %" PRId32 "\n",
		cap.cman_head_drop_supported);
	printf("cap.cman_wred_context_n_max %" PRIu32 "\n",
		cap.cman_wred_context_n_max);
	printf("cap.cman_wred_context_private_n_max %" PRIu32 "\n",
		cap.cman_wred_context_private_n_max);
	printf("cap.cman_wred_context_shared_n_max %" PRIu32 "\n",
		cap.cman_wred_context_shared_n_max);
	printf("cap.cman_wred_context_shared_n_nodes_per_context_max %" PRIu32
		"\n", cap.cman_wred_context_shared_n_nodes_per_context_max);
	printf("cap.cman_wred_context_shared_n_contexts_per_node_max %" PRIu32
		"\n", cap.cman_wred_context_shared_n_contexts_per_node_max);

	for (i = 0; i < RTE_TM_COLORS; i++) {
		printf("cap.mark_vlan_dei_supported %" PRId32 "\n",
			cap.mark_vlan_dei_supported[i]);
		printf("cap.mark_ip_ecn_tcp_supported %" PRId32 "\n",
			cap.mark_ip_ecn_tcp_supported[i]);
		printf("cap.mark_ip_ecn_sctp_supported %" PRId32 "\n",
			cap.mark_ip_ecn_sctp_supported[i]);
		printf("cap.mark_ip_dscp_supported %" PRId32 "\n",
			cap.mark_ip_dscp_supported[i]);
	}

	printf("cap.dynamic_update_mask %" PRIx64 "\n",
		cap.dynamic_update_mask);
	printf("cap.stats_mask %" PRIx64 "\n", cap.stats_mask);
}

cmdline_parse_inst_t cmd_show_port_tm_cap = {
	.f = cmd_show_port_tm_cap_parsed,
	.data = NULL,
	.help_str = "Show Port TM Capabilities",
	.tokens = {
		(void *)&cmd_show_port_tm_cap_show,
		(void *)&cmd_show_port_tm_cap_port,
		(void *)&cmd_show_port_tm_cap_tm,
		(void *)&cmd_show_port_tm_cap_cap,
		(void *)&cmd_show_port_tm_cap_port_id,
		NULL,
	},
};

/* *** Port TM Hierarchical Level Capability *** */
struct cmd_show_port_tm_level_cap_result {
	cmdline_fixed_string_t show;
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t tm;
	cmdline_fixed_string_t level;
	cmdline_fixed_string_t cap;
	uint16_t port_id;
	uint32_t level_id;
};

cmdline_parse_token_string_t cmd_show_port_tm_level_cap_show =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_level_cap_result,
		show, "show");
cmdline_parse_token_string_t cmd_show_port_tm_level_cap_port =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_level_cap_result,
		port, "port");
cmdline_parse_token_string_t cmd_show_port_tm_level_cap_tm =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_level_cap_result,
		tm, "tm");
cmdline_parse_token_string_t cmd_show_port_tm_level_cap_level =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_level_cap_result,
		level, "level");
cmdline_parse_token_string_t cmd_show_port_tm_level_cap_cap =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_level_cap_result,
		cap, "cap");
cmdline_parse_token_num_t cmd_show_port_tm_level_cap_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_show_port_tm_level_cap_result,
		 port_id, UINT16);
cmdline_parse_token_num_t cmd_show_port_tm_level_cap_level_id =
	TOKEN_NUM_INITIALIZER(struct cmd_show_port_tm_level_cap_result,
		 level_id, UINT32);


static void cmd_show_port_tm_level_cap_parsed(void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_show_port_tm_level_cap_result *res = parsed_result;
	struct rte_tm_level_capabilities lcap;
	struct rte_tm_error error;
	portid_t port_id = res->port_id;
	uint32_t level_id = res->level_id;
	int ret;

	if (port_id_is_invalid(port_id, ENABLED_WARN))
		return;

	memset(&lcap, 0, sizeof(struct rte_tm_level_capabilities));
	ret = rte_tm_level_capabilities_get(port_id, level_id, &lcap, &error);
	if (ret) {
		print_err_msg(&error);
		return;
	}
	printf("\n**   Port TM Hierarchy level %" PRIu32 " Capability **\n\n",
		level_id);

	printf("cap.n_nodes_max %" PRIu32 "\n", lcap.n_nodes_max);
	printf("cap.n_nodes_nonleaf_max %" PRIu32 "\n",
		lcap.n_nodes_nonleaf_max);
	printf("cap.n_nodes_leaf_max %" PRIu32 "\n", lcap.n_nodes_leaf_max);
	printf("cap.non_leaf_nodes_identical %" PRId32 "\n",
		lcap.non_leaf_nodes_identical);
	printf("cap.leaf_nodes_identical %" PRId32 "\n",
		lcap.leaf_nodes_identical);
	if (level_id <= 3) {
		printf("cap.nonleaf.shaper_private_supported %" PRId32 "\n",
			lcap.nonleaf.shaper_private_supported);
		printf("cap.nonleaf.shaper_private_dual_rate_supported %" PRId32
			"\n", lcap.nonleaf.shaper_private_dual_rate_supported);
		printf("cap.nonleaf.shaper_private_rate_min %" PRIu64 "\n",
			lcap.nonleaf.shaper_private_rate_min);
		printf("cap.nonleaf.shaper_private_rate_max %" PRIu64 "\n",
			lcap.nonleaf.shaper_private_rate_max);
		printf("cap.nonleaf.shaper_shared_n_max %" PRIu32 "\n",
			lcap.nonleaf.shaper_shared_n_max);
		printf("cap.nonleaf.sched_n_children_max %" PRIu32 "\n",
			lcap.nonleaf.sched_n_children_max);
		printf("cap.nonleaf.sched_sp_n_priorities_max %" PRIu32 "\n",
			lcap.nonleaf.sched_sp_n_priorities_max);
		printf("cap.nonleaf.sched_wfq_n_children_per_group_max %" PRIu32
			"\n", lcap.nonleaf.sched_wfq_n_children_per_group_max);
		printf("cap.nonleaf.sched_wfq_n_groups_max %" PRIu32 "\n",
			lcap.nonleaf.sched_wfq_n_groups_max);
		printf("cap.nonleaf.sched_wfq_weight_max %" PRIu32 "\n",
			lcap.nonleaf.sched_wfq_weight_max);
		printf("cap.nonleaf.stats_mask %" PRIx64 "\n",
			lcap.nonleaf.stats_mask);
	} else {
		printf("cap.leaf.shaper_private_supported %" PRId32 "\n",
			lcap.leaf.shaper_private_supported);
		printf("cap.leaf.shaper_private_dual_rate_supported %" PRId32
			"\n", lcap.leaf.shaper_private_dual_rate_supported);
		printf("cap.leaf.shaper_private_rate_min %" PRIu64 "\n",
			lcap.leaf.shaper_private_rate_min);
		printf("cap.leaf.shaper_private_rate_max %" PRIu64 "\n",
			lcap.leaf.shaper_private_rate_max);
		printf("cap.leaf.shaper_shared_n_max %" PRIu32 "\n",
			lcap.leaf.shaper_shared_n_max);
		printf("cap.leaf.cman_head_drop_supported %" PRId32 "\n",
			lcap.leaf.cman_head_drop_supported);
		printf("cap.leaf.cman_wred_context_private_supported %"	PRId32
			"\n", lcap.leaf.cman_wred_context_private_supported);
		printf("cap.leaf.cman_wred_context_shared_n_max %" PRIu32 "\n",
			lcap.leaf.cman_wred_context_shared_n_max);
		printf("cap.leaf.stats_mask %" PRIx64 "\n",
			lcap.leaf.stats_mask);
	}
}

cmdline_parse_inst_t cmd_show_port_tm_level_cap = {
	.f = cmd_show_port_tm_level_cap_parsed,
	.data = NULL,
	.help_str = "Show Port TM Hierarhical level Capabilities",
	.tokens = {
		(void *)&cmd_show_port_tm_level_cap_show,
		(void *)&cmd_show_port_tm_level_cap_port,
		(void *)&cmd_show_port_tm_level_cap_tm,
		(void *)&cmd_show_port_tm_level_cap_level,
		(void *)&cmd_show_port_tm_level_cap_cap,
		(void *)&cmd_show_port_tm_level_cap_port_id,
		(void *)&cmd_show_port_tm_level_cap_level_id,
		NULL,
	},
};

/* *** Port TM Hierarchy Node Capability *** */
struct cmd_show_port_tm_node_cap_result {
	cmdline_fixed_string_t show;
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t tm;
	cmdline_fixed_string_t node;
	cmdline_fixed_string_t cap;
	uint16_t port_id;
	uint32_t node_id;
};

cmdline_parse_token_string_t cmd_show_port_tm_node_cap_show =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_node_cap_result,
		show, "show");
cmdline_parse_token_string_t cmd_show_port_tm_node_cap_port =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_node_cap_result,
		port, "port");
cmdline_parse_token_string_t cmd_show_port_tm_node_cap_tm =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_node_cap_result,
		tm, "tm");
cmdline_parse_token_string_t cmd_show_port_tm_node_cap_node =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_node_cap_result,
		node, "node");
cmdline_parse_token_string_t cmd_show_port_tm_node_cap_cap =
	TOKEN_STRING_INITIALIZER(struct cmd_show_port_tm_node_cap_result,
		cap, "cap");
cmdline_parse_token_num_t cmd_show_port_tm_node_cap_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_show_port_tm_node_cap_result,
		 port_id, UINT16);
cmdline_parse_token_num_t cmd_show_port_tm_node_cap_node_id =
	TOKEN_NUM_INITIALIZER(struct cmd_show_port_tm_node_cap_result,
		 node_id, UINT32);

static void cmd_show_port_tm_node_cap_parsed(void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_show_port_tm_node_cap_result *res = parsed_result;
	struct rte_tm_node_capabilities ncap;
	struct rte_tm_error error;
	uint32_t node_id = res->node_id;
	portid_t port_id = res->port_id;
	int ret, is_leaf = 0;

	if (port_id_is_invalid(port_id, ENABLED_WARN))
		return;

	/* Node id must be valid */
	ret = rte_tm_node_type_get(port_id, node_id, &is_leaf, &error);
	if (ret != 0) {
		print_err_msg(&error);
		return;
	}

	memset(&ncap, 0, sizeof(struct rte_tm_node_capabilities));
	ret = rte_tm_node_capabilities_get(port_id, node_id, &ncap, &error);
	if (ret != 0) {
		print_err_msg(&error);
		return;
	}
	printf("\n**   Port TM Hierarchy node %" PRIu32 " Capability **\n\n",
		node_id);
	printf("cap.shaper_private_supported %" PRId32 "\n",
		ncap.shaper_private_supported);
	printf("cap.shaper_private_dual_rate_supported %" PRId32 "\n",
		ncap.shaper_private_dual_rate_supported);
	printf("cap.shaper_private_rate_min %" PRIu64 "\n",
		ncap.shaper_private_rate_min);
	printf("cap.shaper_private_rate_max %" PRIu64 "\n",
		ncap.shaper_private_rate_max);
	printf("cap.shaper_shared_n_max %" PRIu32 "\n",
		ncap.shaper_shared_n_max);
	if (!is_leaf) {
		printf("cap.nonleaf.sched_n_children_max %" PRIu32 "\n",
			ncap.nonleaf.sched_n_children_max);
		printf("cap.nonleaf.sched_sp_n_priorities_max %" PRIu32 "\n",
			ncap.nonleaf.sched_sp_n_priorities_max);
		printf("cap.nonleaf.sched_wfq_n_children_per_group_max %" PRIu32
			"\n", ncap.nonleaf.sched_wfq_n_children_per_group_max);
		printf("cap.nonleaf.sched_wfq_n_groups_max %" PRIu32 "\n",
			ncap.nonleaf.sched_wfq_n_groups_max);
		printf("cap.nonleaf.sched_wfq_weight_max %" PRIu32 "\n",
			ncap.nonleaf.sched_wfq_weight_max);
	} else {
		printf("cap.leaf.cman_head_drop_supported %" PRId32 "\n",
			ncap.leaf.cman_head_drop_supported);
		printf("cap.leaf.cman_wred_context_private_supported %" PRId32
			"\n", ncap.leaf.cman_wred_context_private_supported);
		printf("cap.leaf.cman_wred_context_shared_n_max %" PRIu32 "\n",
			ncap.leaf.cman_wred_context_shared_n_max);
	}
	printf("cap.stats_mask %" PRIx64 "\n", ncap.stats_mask);
}

cmdline_parse_inst_t cmd_show_port_tm_node_cap = {
	.f = cmd_show_port_tm_node_cap_parsed,
	.data = NULL,
	.help_str = "Show Port TM Hierarchy node capabilities",
	.tokens = {
		(void *)&cmd_show_port_tm_node_cap_show,
		(void *)&cmd_show_port_tm_node_cap_port,
		(void *)&cmd_show_port_tm_node_cap_tm,
		(void *)&cmd_show_port_tm_node_cap_node,
		(void *)&cmd_show_port_tm_node_cap_cap,
		(void *)&cmd_show_port_tm_node_cap_port_id,
		(void *)&cmd_show_port_tm_node_cap_node_id,
		NULL,
	},
};

/* *** Show Port TM Node Statistics *** */
struct cmd_show_port_tm_node_stats_result {
	cmdline_fixed_string_t show;
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t tm;
	cmdline_fixed_string_t node;
	cmdline_fixed_string_t stats;
	uint16_t port_id;
	uint32_t node_id;
	uint32_t clear;
};

cmdline_parse_token_string_t cmd_show_port_tm_node_stats_show =
	TOKEN_STRING_INITIALIZER(
		struct cmd_show_port_tm_node_stats_result, show, "show");
cmdline_parse_token_string_t cmd_show_port_tm_node_stats_port =
	TOKEN_STRING_INITIALIZER(
		struct cmd_show_port_tm_node_stats_result, port, "port");
cmdline_parse_token_string_t cmd_show_port_tm_node_stats_tm =
	TOKEN_STRING_INITIALIZER(
		struct cmd_show_port_tm_node_stats_result, tm, "tm");
cmdline_parse_token_string_t cmd_show_port_tm_node_stats_node =
	TOKEN_STRING_INITIALIZER(
		struct cmd_show_port_tm_node_stats_result, node, "node");
cmdline_parse_token_string_t cmd_show_port_tm_node_stats_stats =
	TOKEN_STRING_INITIALIZER(
		struct cmd_show_port_tm_node_stats_result, stats, "stats");
cmdline_parse_token_num_t cmd_show_port_tm_node_stats_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_show_port_tm_node_stats_result,
			port_id, UINT16);
cmdline_parse_token_num_t cmd_show_port_tm_node_stats_node_id =
	TOKEN_NUM_INITIALIZER(
		struct cmd_show_port_tm_node_stats_result,
			node_id, UINT32);
cmdline_parse_token_num_t cmd_show_port_tm_node_stats_clear =
	TOKEN_NUM_INITIALIZER(
		struct cmd_show_port_tm_node_stats_result, clear, UINT32);

static void cmd_show_port_tm_node_stats_parsed(void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_show_port_tm_node_stats_result *res = parsed_result;
	struct rte_tm_node_stats stats;
	struct rte_tm_error error;
	uint64_t stats_mask = 0;
	uint32_t node_id = res->node_id;
	uint32_t clear = res->clear;
	portid_t port_id = res->port_id;
	int ret;

	if (port_id_is_invalid(port_id, ENABLED_WARN))
		return;

	/* Port status */
	if (!port_is_started(port_id)) {
		printf(" Port %u not started (error)\n", port_id);
		return;
	}

	memset(&stats, 0, sizeof(struct rte_tm_node_stats));
	ret = rte_tm_node_stats_read(port_id, node_id, &stats,
			&stats_mask, clear, &error);
	if (ret != 0) {
		print_err_msg(&error);
		return;
	}

	/* Display stats */
	if (stats_mask & RTE_TM_STATS_N_PKTS)
		printf("\tPkts scheduled from node: %" PRIu64 "\n",
			stats.n_pkts);
	if (stats_mask & RTE_TM_STATS_N_BYTES)
		printf("\tBytes scheduled from node: %" PRIu64 "\n",
			stats.n_bytes);
	if (stats_mask & RTE_TM_STATS_N_PKTS_GREEN_DROPPED)
		printf("\tPkts dropped (green): %" PRIu64 "\n",
			stats.leaf.n_pkts_dropped[RTE_TM_GREEN]);
	if (stats_mask & RTE_TM_STATS_N_PKTS_YELLOW_DROPPED)
		printf("\tPkts dropped (yellow): %" PRIu64 "\n",
			stats.leaf.n_pkts_dropped[RTE_TM_YELLOW]);
	if (stats_mask & RTE_TM_STATS_N_PKTS_RED_DROPPED)
		printf("\tPkts dropped (red): %" PRIu64 "\n",
			stats.leaf.n_pkts_dropped[RTE_TM_RED]);
	if (stats_mask & RTE_TM_STATS_N_BYTES_GREEN_DROPPED)
		printf("\tBytes dropped (green): %" PRIu64 "\n",
			stats.leaf.n_bytes_dropped[RTE_TM_GREEN]);
	if (stats_mask & RTE_TM_STATS_N_BYTES_YELLOW_DROPPED)
		printf("\tBytes dropped (yellow): %" PRIu64 "\n",
			stats.leaf.n_bytes_dropped[RTE_TM_YELLOW]);
	if (stats_mask & RTE_TM_STATS_N_BYTES_RED_DROPPED)
		printf("\tBytes dropped (red): %" PRIu64 "\n",
			stats.leaf.n_bytes_dropped[RTE_TM_RED]);
	if (stats_mask & RTE_TM_STATS_N_PKTS_QUEUED)
		printf("\tPkts queued: %" PRIu64 "\n",
			stats.leaf.n_pkts_queued);
	if (stats_mask & RTE_TM_STATS_N_BYTES_QUEUED)
		printf("\tBytes queued: %" PRIu64 "\n",
			stats.leaf.n_bytes_queued);
}

cmdline_parse_inst_t cmd_show_port_tm_node_stats = {
	.f = cmd_show_port_tm_node_stats_parsed,
	.data = NULL,
	.help_str = "Show port tm node stats",
	.tokens = {
		(void *)&cmd_show_port_tm_node_stats_show,
		(void *)&cmd_show_port_tm_node_stats_port,
		(void *)&cmd_show_port_tm_node_stats_tm,
		(void *)&cmd_show_port_tm_node_stats_node,
		(void *)&cmd_show_port_tm_node_stats_stats,
		(void *)&cmd_show_port_tm_node_stats_port_id,
		(void *)&cmd_show_port_tm_node_stats_node_id,
		(void *)&cmd_show_port_tm_node_stats_clear,
		NULL,
	},
};

/* *** Show Port TM Node Type *** */
struct cmd_show_port_tm_node_type_result {
	cmdline_fixed_string_t show;
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t tm;
	cmdline_fixed_string_t node;
	cmdline_fixed_string_t type;
	uint16_t port_id;
	uint32_t node_id;
};

cmdline_parse_token_string_t cmd_show_port_tm_node_type_show =
	TOKEN_STRING_INITIALIZER(
		struct cmd_show_port_tm_node_type_result, show, "show");
cmdline_parse_token_string_t cmd_show_port_tm_node_type_port =
	TOKEN_STRING_INITIALIZER(
		struct cmd_show_port_tm_node_type_result, port, "port");
cmdline_parse_token_string_t cmd_show_port_tm_node_type_tm =
	TOKEN_STRING_INITIALIZER(
		struct cmd_show_port_tm_node_type_result, tm, "tm");
cmdline_parse_token_string_t cmd_show_port_tm_node_type_node =
	TOKEN_STRING_INITIALIZER(
		struct cmd_show_port_tm_node_type_result, node, "node");
cmdline_parse_token_string_t cmd_show_port_tm_node_type_type =
	TOKEN_STRING_INITIALIZER(
		struct cmd_show_port_tm_node_type_result, type, "type");
cmdline_parse_token_num_t cmd_show_port_tm_node_type_port_id =
	TOKEN_NUM_INITIALIZER(
		struct cmd_show_port_tm_node_type_result,
			port_id, UINT16);
cmdline_parse_token_num_t cmd_show_port_tm_node_type_node_id =
	TOKEN_NUM_INITIALIZER(
		struct cmd_show_port_tm_node_type_result,
			node_id, UINT32);

static void cmd_show_port_tm_node_type_parsed(void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_show_port_tm_node_type_result *res = parsed_result;
	struct rte_tm_error error;
	uint32_t node_id = res->node_id;
	portid_t port_id = res->port_id;
	int ret, is_leaf = 0;

	if (port_id_is_invalid(port_id, ENABLED_WARN))
		return;

	ret = rte_tm_node_type_get(port_id, node_id, &is_leaf, &error);
	if (ret != 0) {
		print_err_msg(&error);
		return;
	}

	if (is_leaf == 1)
		printf("leaf node\n");
	else
		printf("nonleaf node\n");

}

cmdline_parse_inst_t cmd_show_port_tm_node_type = {
	.f = cmd_show_port_tm_node_type_parsed,
	.data = NULL,
	.help_str = "Show port tm node type",
	.tokens = {
		(void *)&cmd_show_port_tm_node_type_show,
		(void *)&cmd_show_port_tm_node_type_port,
		(void *)&cmd_show_port_tm_node_type_tm,
		(void *)&cmd_show_port_tm_node_type_node,
		(void *)&cmd_show_port_tm_node_type_type,
		(void *)&cmd_show_port_tm_node_type_port_id,
		(void *)&cmd_show_port_tm_node_type_node_id,
		NULL,
	},
};
