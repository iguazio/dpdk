/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <rte_string_fns.h>
#include <rte_sched.h>

#include "cfg_file.h"
#include "main.h"


/** when we resize a file structure, how many extra entries
 * for new sections do we add in */
#define CFG_ALLOC_SECTION_BATCH 8
/** when we resize a section structure, how many extra entries
 * for new entries do we add in */
#define CFG_ALLOC_ENTRY_BATCH 16

int
cfg_load_port(struct rte_cfgfile *cfg, struct rte_sched_port_params *port_params)
{
	const char *entry;
	int j;

	if (!cfg || !port_params)
		return -1;

	entry = rte_cfgfile_get_entry(cfg, "port", "frame overhead");
	if (entry)
		port_params->frame_overhead = (uint32_t)atoi(entry);

	entry = rte_cfgfile_get_entry(cfg, "port", "number of subports per port");
	if (entry)
		port_params->n_subports_per_port = (uint32_t)atoi(entry);

	entry = rte_cfgfile_get_entry(cfg, "port", "number of pipes per subport");
	if (entry)
		port_params->n_pipes_per_subport = (uint32_t)atoi(entry);

	entry = rte_cfgfile_get_entry(cfg, "port", "queue sizes");
	if (entry) {
		char *next;

		for(j = 0; j < RTE_SCHED_TRAFFIC_CLASSES_PER_PIPE; j++) {
			port_params->qsize[j] = (uint16_t)strtol(entry, &next, 10);
			if (next == NULL)
				break;
			entry = next;
		}
	}

#ifdef RTE_SCHED_RED
	for (j = 0; j < RTE_SCHED_TRAFFIC_CLASSES_PER_PIPE; j++) {
		char str[32];

		/* Parse WRED min thresholds */
		snprintf(str, sizeof(str), "tc %d wred min", j);
		entry = rte_cfgfile_get_entry(cfg, "red", str);
		if (entry) {
			char *next;
			int k;
			/* for each packet colour (green, yellow, red) */
			for (k = 0; k < e_RTE_METER_COLORS; k++) {
				port_params->red_params[j][k].min_th
					= (uint16_t)strtol(entry, &next, 10);
				if (next == NULL)
					break;
				entry = next;
			}
		}

		/* Parse WRED max thresholds */
		snprintf(str, sizeof(str), "tc %d wred max", j);
		entry = rte_cfgfile_get_entry(cfg, "red", str);
		if (entry) {
			char *next;
			int k;
			/* for each packet colour (green, yellow, red) */
			for (k = 0; k < e_RTE_METER_COLORS; k++) {
				port_params->red_params[j][k].max_th
					= (uint16_t)strtol(entry, &next, 10);
				if (next == NULL)
					break;
				entry = next;
			}
		}

		/* Parse WRED inverse mark probabilities */
		snprintf(str, sizeof(str), "tc %d wred inv prob", j);
		entry = rte_cfgfile_get_entry(cfg, "red", str);
		if (entry) {
			char *next;
			int k;
			/* for each packet colour (green, yellow, red) */
			for (k = 0; k < e_RTE_METER_COLORS; k++) {
				port_params->red_params[j][k].maxp_inv
					= (uint8_t)strtol(entry, &next, 10);

				if (next == NULL)
					break;
				entry = next;
			}
		}

		/* Parse WRED EWMA filter weights */
		snprintf(str, sizeof(str), "tc %d wred weight", j);
		entry = rte_cfgfile_get_entry(cfg, "red", str);
		if (entry) {
			char *next;
			int k;
			/* for each packet colour (green, yellow, red) */
			for (k = 0; k < e_RTE_METER_COLORS; k++) {
				port_params->red_params[j][k].wq_log2
					= (uint8_t)strtol(entry, &next, 10);
				if (next == NULL)
					break;
				entry = next;
			}
		}
	}
#endif /* RTE_SCHED_RED */

	return 0;
}

int
cfg_load_pipe(struct rte_cfgfile *cfg, struct rte_sched_pipe_params *pipe_params)
{
	int i, j;
	char *next;
	const char *entry;
	int profiles;

	if (!cfg || !pipe_params)
		return -1;

	profiles = rte_cfgfile_num_sections(cfg, "pipe profile", sizeof("pipe profile") - 1);
	port_params.n_pipe_profiles = profiles;

	for (j = 0; j < profiles; j++) {
		char pipe_name[32];
		snprintf(pipe_name, sizeof(pipe_name), "pipe profile %d", j);

		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tb rate");
		if (entry)
			pipe_params[j].tb_rate = (uint32_t)atoi(entry);

		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tb size");
		if (entry)
			pipe_params[j].tb_size = (uint32_t)atoi(entry);

		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tc period");
		if (entry)
			pipe_params[j].tc_period = (uint32_t)atoi(entry);

		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tc 0 rate");
		if (entry)
			pipe_params[j].tc_rate[0] = (uint32_t)atoi(entry);

		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tc 1 rate");
		if (entry)
			pipe_params[j].tc_rate[1] = (uint32_t)atoi(entry);

		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tc 2 rate");
		if (entry)
			pipe_params[j].tc_rate[2] = (uint32_t)atoi(entry);

		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tc 3 rate");
		if (entry)
			pipe_params[j].tc_rate[3] = (uint32_t)atoi(entry);

#ifdef RTE_SCHED_SUBPORT_TC_OV
		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tc 3 oversubscription weight");
		if (entry)
			pipe_params[j].tc_ov_weight = (uint8_t)atoi(entry);
#endif

		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tc 0 wrr weights");
		if (entry) {
			for(i = 0; i < RTE_SCHED_QUEUES_PER_TRAFFIC_CLASS; i++) {
				pipe_params[j].wrr_weights[RTE_SCHED_TRAFFIC_CLASSES_PER_PIPE*0 + i] =
					(uint8_t)strtol(entry, &next, 10);
				if (next == NULL)
					break;
				entry = next;
			}
		}
		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tc 1 wrr weights");
		if (entry) {
			for(i = 0; i < RTE_SCHED_QUEUES_PER_TRAFFIC_CLASS; i++) {
				pipe_params[j].wrr_weights[RTE_SCHED_TRAFFIC_CLASSES_PER_PIPE*1 + i] =
					(uint8_t)strtol(entry, &next, 10);
				if (next == NULL)
					break;
				entry = next;
			}
		}
		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tc 2 wrr weights");
		if (entry) {
			for(i = 0; i < RTE_SCHED_QUEUES_PER_TRAFFIC_CLASS; i++) {
				pipe_params[j].wrr_weights[RTE_SCHED_TRAFFIC_CLASSES_PER_PIPE*2 + i] =
					(uint8_t)strtol(entry, &next, 10);
				if (next == NULL)
					break;
				entry = next;
			}
		}
		entry = rte_cfgfile_get_entry(cfg, pipe_name, "tc 3 wrr weights");
		if (entry) {
			for(i = 0; i < RTE_SCHED_QUEUES_PER_TRAFFIC_CLASS; i++) {
				pipe_params[j].wrr_weights[RTE_SCHED_TRAFFIC_CLASSES_PER_PIPE*3 + i] =
					(uint8_t)strtol(entry, &next, 10);
				if (next == NULL)
					break;
				entry = next;
			}
		}
	}
	return 0;
}

int
cfg_load_subport(struct rte_cfgfile *cfg, struct rte_sched_subport_params *subport_params)
{
	const char *entry;
	int i, j, k;

	if (!cfg || !subport_params)
		return -1;

	memset(app_pipe_to_profile, -1, sizeof(app_pipe_to_profile));

	for (i = 0; i < MAX_SCHED_SUBPORTS; i++) {
		char sec_name[CFG_NAME_LEN];
		snprintf(sec_name, sizeof(sec_name), "subport %d", i);

		if (rte_cfgfile_has_section(cfg, sec_name)) {
			entry = rte_cfgfile_get_entry(cfg, sec_name, "tb rate");
			if (entry)
				subport_params[i].tb_rate = (uint32_t)atoi(entry);

			entry = rte_cfgfile_get_entry(cfg, sec_name, "tb size");
			if (entry)
				subport_params[i].tb_size = (uint32_t)atoi(entry);

			entry = rte_cfgfile_get_entry(cfg, sec_name, "tc period");
			if (entry)
				subport_params[i].tc_period = (uint32_t)atoi(entry);

			entry = rte_cfgfile_get_entry(cfg, sec_name, "tc 0 rate");
			if (entry)
				subport_params[i].tc_rate[0] = (uint32_t)atoi(entry);

			entry = rte_cfgfile_get_entry(cfg, sec_name, "tc 1 rate");
			if (entry)
				subport_params[i].tc_rate[1] = (uint32_t)atoi(entry);

			entry = rte_cfgfile_get_entry(cfg, sec_name, "tc 2 rate");
			if (entry)
				subport_params[i].tc_rate[2] = (uint32_t)atoi(entry);

			entry = rte_cfgfile_get_entry(cfg, sec_name, "tc 3 rate");
			if (entry)
				subport_params[i].tc_rate[3] = (uint32_t)atoi(entry);

			int n_entries = rte_cfgfile_section_num_entries(cfg, sec_name);
			struct rte_cfgfile_entry entries[n_entries];

			rte_cfgfile_section_entries(cfg, sec_name, entries, n_entries);

			for (j = 0; j < n_entries; j++) {
				if (strncmp("pipe", entries[j].name, sizeof("pipe") - 1) == 0) {
					int profile;
					char *tokens[2] = {NULL, NULL};
					int n_tokens;
					int begin, end;

					profile = atoi(entries[j].value);
					n_tokens = rte_strsplit(&entries[j].name[sizeof("pipe")],
							strnlen(entries[j].name, CFG_NAME_LEN), tokens, 2, '-');

					begin =  atoi(tokens[0]);
					if (n_tokens == 2)
						end = atoi(tokens[1]);
					else
						end = begin;

					if (end >= MAX_SCHED_PIPES || begin > end)
						return -1;

					for (k = begin; k <= end; k++) {
						char profile_name[CFG_NAME_LEN];

						snprintf(profile_name, sizeof(profile_name),
								"pipe profile %d", profile);
						if (rte_cfgfile_has_section(cfg, profile_name))
							app_pipe_to_profile[i][k] = profile;
						else
							rte_exit(EXIT_FAILURE, "Wrong pipe profile %s\n",
									entries[j].value);

					}
				}
			}
		}
	}

	return 0;
}
