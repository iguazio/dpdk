/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Ericsson AB
 */

#ifndef _DSW_EVDEV_H_
#define _DSW_EVDEV_H_

#include <rte_event_ring.h>
#include <rte_eventdev.h>

#define DSW_PMD_NAME RTE_STR(event_dsw)

/* Code changes are required to allow more ports. */
#define DSW_MAX_PORTS (64)
#define DSW_MAX_PORT_DEQUEUE_DEPTH (128)
#define DSW_MAX_PORT_ENQUEUE_DEPTH (128)
#define DSW_MAX_PORT_OUT_BUFFER (32)

#define DSW_MAX_QUEUES (16)

#define DSW_MAX_EVENTS (16384)

/* Code changes are required to allow more flows than 32k. */
#define DSW_MAX_FLOWS_BITS (15)
#define DSW_MAX_FLOWS (1<<(DSW_MAX_FLOWS_BITS))
#define DSW_MAX_FLOWS_MASK (DSW_MAX_FLOWS-1)

/* Eventdev RTE_SCHED_TYPE_PARALLEL doesn't have a concept of flows,
 * but the 'dsw' scheduler (more or less) randomly assign flow id to
 * events on parallel queues, to be able to reuse some of the
 * migration mechanism and scheduling logic from
 * RTE_SCHED_TYPE_ATOMIC. By moving one of the parallel "flows" from a
 * particular port, the likely-hood of events being scheduled to this
 * port is reduced, and thus a kind of statistical load balancing is
 * achieved.
 */
#define DSW_PARALLEL_FLOWS (1024)

/* 'Background tasks' are polling the control rings for *
 *  migration-related messages, or flush the output buffer (so
 *  buffered events doesn't linger too long). Shouldn't be too low,
 *  since the system won't benefit from the 'batching' effects from
 *  the output buffer, and shouldn't be too high, since it will make
 *  buffered events linger too long in case the port goes idle.
 */
#define DSW_MAX_PORT_OPS_PER_BG_TASK (128)

/* Avoid making small 'loans' from the central in-flight event credit
 * pool, to improve efficiency.
 */
#define DSW_MIN_CREDIT_LOAN (64)
#define DSW_PORT_MAX_CREDITS (2*DSW_MIN_CREDIT_LOAN)
#define DSW_PORT_MIN_CREDITS (DSW_MIN_CREDIT_LOAN)

/* The rings are dimensioned so that all in-flight events can reside
 * on any one of the port rings, to avoid the trouble of having to
 * care about the case where there's no room on the destination port's
 * input ring.
 */
#define DSW_IN_RING_SIZE (DSW_MAX_EVENTS)

#define DSW_MAX_LOAD (INT16_MAX)
#define DSW_LOAD_FROM_PERCENT(x) ((int16_t)(((x)*DSW_MAX_LOAD)/100))
#define DSW_LOAD_TO_PERCENT(x) ((100*x)/DSW_MAX_LOAD)

/* The thought behind keeping the load update interval shorter than
 * the migration interval is that the load from newly migrated flows
 * should 'show up' on the load measurement before new migrations are
 * considered. This is to avoid having too many flows, from too many
 * source ports, to be migrated too quickly to a lightly loaded port -
 * in particular since this might cause the system to oscillate.
 */
#define DSW_LOAD_UPDATE_INTERVAL (DSW_MIGRATION_INTERVAL/4)
#define DSW_OLD_LOAD_WEIGHT (1)

#define DSW_MIGRATION_INTERVAL (1000)

struct dsw_port {
	uint16_t id;

	/* Keeping a pointer here to avoid container_of() calls, which
	 * are expensive since they are very frequent and will result
	 * in an integer multiplication (since the port id is an index
	 * into the dsw_evdev port array).
	 */
	struct dsw_evdev *dsw;

	uint16_t dequeue_depth;
	uint16_t enqueue_depth;

	int32_t inflight_credits;

	int32_t new_event_threshold;

	uint16_t pending_releases;

	uint16_t next_parallel_flow_id;

	uint16_t ops_since_bg_task;

	uint64_t last_bg;

	/* For port load measurement. */
	uint64_t next_load_update;
	uint64_t load_update_interval;
	uint64_t measurement_start;
	uint64_t busy_start;
	uint64_t busy_cycles;
	uint64_t total_busy_cycles;

	uint16_t out_buffer_len[DSW_MAX_PORTS];
	struct rte_event out_buffer[DSW_MAX_PORTS][DSW_MAX_PORT_OUT_BUFFER];

	struct rte_event_ring *in_ring __rte_cache_aligned;

	/* Estimate of current port load. */
	rte_atomic16_t load __rte_cache_aligned;
} __rte_cache_aligned;

struct dsw_queue {
	uint8_t schedule_type;
	uint8_t serving_ports[DSW_MAX_PORTS];
	uint16_t num_serving_ports;

	uint8_t flow_to_port_map[DSW_MAX_FLOWS] __rte_cache_aligned;
};

struct dsw_evdev {
	struct rte_eventdev_data *data;

	struct dsw_port ports[DSW_MAX_PORTS];
	uint16_t num_ports;
	struct dsw_queue queues[DSW_MAX_QUEUES];
	uint8_t num_queues;
	int32_t max_inflight;

	rte_atomic32_t credits_on_loan __rte_cache_aligned;
};

uint16_t dsw_event_enqueue(void *port, const struct rte_event *event);
uint16_t dsw_event_enqueue_burst(void *port,
				 const struct rte_event events[],
				 uint16_t events_len);
uint16_t dsw_event_enqueue_new_burst(void *port,
				     const struct rte_event events[],
				     uint16_t events_len);
uint16_t dsw_event_enqueue_forward_burst(void *port,
					 const struct rte_event events[],
					 uint16_t events_len);

uint16_t dsw_event_dequeue(void *port, struct rte_event *ev, uint64_t wait);
uint16_t dsw_event_dequeue_burst(void *port, struct rte_event *events,
				 uint16_t num, uint64_t wait);

static inline struct dsw_evdev *
dsw_pmd_priv(const struct rte_eventdev *eventdev)
{
	return eventdev->data->dev_private;
}

#define DSW_LOG_DP(level, fmt, args...)					\
	RTE_LOG_DP(level, EVENTDEV, "[%s] %s() line %u: " fmt,		\
		   DSW_PMD_NAME,					\
		   __func__, __LINE__, ## args)

#define DSW_LOG_DP_PORT(level, port_id, fmt, args...)		\
	DSW_LOG_DP(level, "<Port %d> " fmt, port_id, ## args)

#endif
