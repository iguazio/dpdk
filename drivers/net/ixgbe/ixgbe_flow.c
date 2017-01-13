/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2016 Intel Corporation. All rights reserved.
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

#include <sys/queue.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <rte_byteorder.h>
#include <rte_common.h>
#include <rte_cycles.h>

#include <rte_interrupts.h>
#include <rte_log.h>
#include <rte_debug.h>
#include <rte_pci.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_eal.h>
#include <rte_alarm.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_atomic.h>
#include <rte_malloc.h>
#include <rte_random.h>
#include <rte_dev.h>
#include <rte_hash_crc.h>
#include <rte_flow.h>
#include <rte_flow_driver.h>

#include "ixgbe_logs.h"
#include "base/ixgbe_api.h"
#include "base/ixgbe_vf.h"
#include "base/ixgbe_common.h"
#include "ixgbe_ethdev.h"
#include "ixgbe_bypass.h"
#include "ixgbe_rxtx.h"
#include "base/ixgbe_type.h"
#include "base/ixgbe_phy.h"
#include "rte_pmd_ixgbe.h"

static int ixgbe_flow_flush(struct rte_eth_dev *dev,
		struct rte_flow_error *error);
static int
cons_parse_ntuple_filter(const struct rte_flow_attr *attr,
					const struct rte_flow_item pattern[],
					const struct rte_flow_action actions[],
					struct rte_eth_ntuple_filter *filter,
					struct rte_flow_error *error);
static int
ixgbe_parse_ntuple_filter(const struct rte_flow_attr *attr,
					const struct rte_flow_item pattern[],
					const struct rte_flow_action actions[],
					struct rte_eth_ntuple_filter *filter,
					struct rte_flow_error *error);
static int
cons_parse_ethertype_filter(const struct rte_flow_attr *attr,
			    const struct rte_flow_item *pattern,
			    const struct rte_flow_action *actions,
			    struct rte_eth_ethertype_filter *filter,
			    struct rte_flow_error *error);
static int
ixgbe_parse_ethertype_filter(const struct rte_flow_attr *attr,
				const struct rte_flow_item pattern[],
				const struct rte_flow_action actions[],
				struct rte_eth_ethertype_filter *filter,
				struct rte_flow_error *error);
static int
cons_parse_syn_filter(const struct rte_flow_attr *attr,
		const struct rte_flow_item pattern[],
		const struct rte_flow_action actions[],
		struct rte_eth_syn_filter *filter,
		struct rte_flow_error *error);
static int
ixgbe_parse_syn_filter(const struct rte_flow_attr *attr,
				const struct rte_flow_item pattern[],
				const struct rte_flow_action actions[],
				struct rte_eth_syn_filter *filter,
				struct rte_flow_error *error);
static int
cons_parse_l2_tn_filter(const struct rte_flow_attr *attr,
		const struct rte_flow_item pattern[],
		const struct rte_flow_action actions[],
		struct rte_eth_l2_tunnel_conf *filter,
		struct rte_flow_error *error);
static int
ixgbe_validate_l2_tn_filter(struct rte_eth_dev *dev,
			const struct rte_flow_attr *attr,
			const struct rte_flow_item pattern[],
			const struct rte_flow_action actions[],
			struct rte_eth_l2_tunnel_conf *rule,
			struct rte_flow_error *error);
static int
ixgbe_flow_validate(__rte_unused struct rte_eth_dev *dev,
		const struct rte_flow_attr *attr,
		const struct rte_flow_item pattern[],
		const struct rte_flow_action actions[],
		struct rte_flow_error *error);

const struct rte_flow_ops ixgbe_flow_ops = {
	ixgbe_flow_validate,
	NULL,
	NULL,
	ixgbe_flow_flush,
	NULL,
};

#define IXGBE_MIN_N_TUPLE_PRIO 1
#define IXGBE_MAX_N_TUPLE_PRIO 7
#define NEXT_ITEM_OF_PATTERN(item, pattern, index)\
	do {		\
		item = pattern + index;\
		while (item->type == RTE_FLOW_ITEM_TYPE_VOID) {\
		index++;				\
		item = pattern + index;		\
		}						\
	} while (0)

#define NEXT_ITEM_OF_ACTION(act, actions, index)\
	do {								\
		act = actions + index;					\
		while (act->type == RTE_FLOW_ACTION_TYPE_VOID) {\
		index++;					\
		act = actions + index;				\
		}							\
	} while (0)

/**
 * Please aware there's an asumption for all the parsers.
 * rte_flow_item is using big endian, rte_flow_attr and
 * rte_flow_action are using CPU order.
 * Because the pattern is used to describe the packets,
 * normally the packets should use network order.
 */

/**
 * Parse the rule to see if it is a n-tuple rule.
 * And get the n-tuple filter info BTW.
 * pattern:
 * The first not void item can be ETH or IPV4.
 * The second not void item must be IPV4 if the first one is ETH.
 * The third not void item must be UDP or TCP.
 * The next not void item must be END.
 * action:
 * The first not void action should be QUEUE.
 * The next not void action should be END.
 * pattern example:
 * ITEM		Spec			Mask
 * ETH		NULL			NULL
 * IPV4		src_addr 192.168.1.20	0xFFFFFFFF
 *		dst_addr 192.167.3.50	0xFFFFFFFF
 *		next_proto_id	17	0xFF
 * UDP/TCP	src_port	80	0xFFFF
 *		dst_port	80	0xFFFF
 * END
 * other members in mask and spec should set to 0x00.
 * item->last should be NULL.
 */
static int
cons_parse_ntuple_filter(const struct rte_flow_attr *attr,
			 const struct rte_flow_item pattern[],
			 const struct rte_flow_action actions[],
			 struct rte_eth_ntuple_filter *filter,
			 struct rte_flow_error *error)
{
	const struct rte_flow_item *item;
	const struct rte_flow_action *act;
	const struct rte_flow_item_ipv4 *ipv4_spec;
	const struct rte_flow_item_ipv4 *ipv4_mask;
	const struct rte_flow_item_tcp *tcp_spec;
	const struct rte_flow_item_tcp *tcp_mask;
	const struct rte_flow_item_udp *udp_spec;
	const struct rte_flow_item_udp *udp_mask;
	uint32_t index;

	if (!pattern) {
		rte_flow_error_set(error,
			EINVAL, RTE_FLOW_ERROR_TYPE_ITEM_NUM,
			NULL, "NULL pattern.");
		return -rte_errno;
	}

	if (!actions) {
		rte_flow_error_set(error, EINVAL,
				   RTE_FLOW_ERROR_TYPE_ACTION_NUM,
				   NULL, "NULL action.");
		return -rte_errno;
	}
	if (!attr) {
		rte_flow_error_set(error, EINVAL,
				   RTE_FLOW_ERROR_TYPE_ATTR,
				   NULL, "NULL attribute.");
		return -rte_errno;
	}

	/* parse pattern */
	index = 0;

	/* the first not void item can be MAC or IPv4 */
	NEXT_ITEM_OF_PATTERN(item, pattern, index);

	if (item->type != RTE_FLOW_ITEM_TYPE_ETH &&
	    item->type != RTE_FLOW_ITEM_TYPE_IPV4) {
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Not supported by ntuple filter");
		return -rte_errno;
	}
	/* Skip Ethernet */
	if (item->type == RTE_FLOW_ITEM_TYPE_ETH) {
		/*Not supported last point for range*/
		if (item->last) {
			rte_flow_error_set(error,
			  EINVAL,
			  RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
			  item, "Not supported last point for range");
			return -rte_errno;

		}
		/* if the first item is MAC, the content should be NULL */
		if (item->spec || item->mask) {
			rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by ntuple filter");
			return -rte_errno;
		}
		/* check if the next not void item is IPv4 */
		index++;
		NEXT_ITEM_OF_PATTERN(item, pattern, index);
		if (item->type != RTE_FLOW_ITEM_TYPE_IPV4) {
			rte_flow_error_set(error,
			  EINVAL, RTE_FLOW_ERROR_TYPE_ITEM,
			  item, "Not supported by ntuple filter");
			  return -rte_errno;
		}
	}

	/* get the IPv4 info */
	if (!item->spec || !item->mask) {
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Invalid ntuple mask");
		return -rte_errno;
	}
	/*Not supported last point for range*/
	if (item->last) {
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
			item, "Not supported last point for range");
		return -rte_errno;

	}

	ipv4_mask = (const struct rte_flow_item_ipv4 *)item->mask;
	/**
	 * Only support src & dst addresses, protocol,
	 * others should be masked.
	 */
	if (ipv4_mask->hdr.version_ihl ||
	    ipv4_mask->hdr.type_of_service ||
	    ipv4_mask->hdr.total_length ||
	    ipv4_mask->hdr.packet_id ||
	    ipv4_mask->hdr.fragment_offset ||
	    ipv4_mask->hdr.time_to_live ||
	    ipv4_mask->hdr.hdr_checksum) {
			rte_flow_error_set(error,
			EINVAL, RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Not supported by ntuple filter");
		return -rte_errno;
	}

	filter->dst_ip_mask = ipv4_mask->hdr.dst_addr;
	filter->src_ip_mask = ipv4_mask->hdr.src_addr;
	filter->proto_mask  = ipv4_mask->hdr.next_proto_id;

	ipv4_spec = (const struct rte_flow_item_ipv4 *)item->spec;
	filter->dst_ip = ipv4_spec->hdr.dst_addr;
	filter->src_ip = ipv4_spec->hdr.src_addr;
	filter->proto  = ipv4_spec->hdr.next_proto_id;

	/* check if the next not void item is TCP or UDP */
	index++;
	NEXT_ITEM_OF_PATTERN(item, pattern, index);
	if (item->type != RTE_FLOW_ITEM_TYPE_TCP &&
	    item->type != RTE_FLOW_ITEM_TYPE_UDP) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Not supported by ntuple filter");
		return -rte_errno;
	}

	/* get the TCP/UDP info */
	if (!item->spec || !item->mask) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Invalid ntuple mask");
		return -rte_errno;
	}

	/*Not supported last point for range*/
	if (item->last) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
			item, "Not supported last point for range");
		return -rte_errno;

	}

	if (item->type == RTE_FLOW_ITEM_TYPE_TCP) {
		tcp_mask = (const struct rte_flow_item_tcp *)item->mask;

		/**
		 * Only support src & dst ports, tcp flags,
		 * others should be masked.
		 */
		if (tcp_mask->hdr.sent_seq ||
		    tcp_mask->hdr.recv_ack ||
		    tcp_mask->hdr.data_off ||
		    tcp_mask->hdr.rx_win ||
		    tcp_mask->hdr.cksum ||
		    tcp_mask->hdr.tcp_urp) {
			memset(filter, 0,
				sizeof(struct rte_eth_ntuple_filter));
			rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by ntuple filter");
			return -rte_errno;
		}

		filter->dst_port_mask  = tcp_mask->hdr.dst_port;
		filter->src_port_mask  = tcp_mask->hdr.src_port;
		if (tcp_mask->hdr.tcp_flags == 0xFF) {
			filter->flags |= RTE_NTUPLE_FLAGS_TCP_FLAG;
		} else if (!tcp_mask->hdr.tcp_flags) {
			filter->flags &= ~RTE_NTUPLE_FLAGS_TCP_FLAG;
		} else {
			memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
			rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by ntuple filter");
			return -rte_errno;
		}

		tcp_spec = (const struct rte_flow_item_tcp *)item->spec;
		filter->dst_port  = tcp_spec->hdr.dst_port;
		filter->src_port  = tcp_spec->hdr.src_port;
		filter->tcp_flags = tcp_spec->hdr.tcp_flags;
	} else {
		udp_mask = (const struct rte_flow_item_udp *)item->mask;

		/**
		 * Only support src & dst ports,
		 * others should be masked.
		 */
		if (udp_mask->hdr.dgram_len ||
		    udp_mask->hdr.dgram_cksum) {
			memset(filter, 0,
				sizeof(struct rte_eth_ntuple_filter));
			rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by ntuple filter");
			return -rte_errno;
		}

		filter->dst_port_mask = udp_mask->hdr.dst_port;
		filter->src_port_mask = udp_mask->hdr.src_port;

		udp_spec = (const struct rte_flow_item_udp *)item->spec;
		filter->dst_port = udp_spec->hdr.dst_port;
		filter->src_port = udp_spec->hdr.src_port;
	}

	/* check if the next not void item is END */
	index++;
	NEXT_ITEM_OF_PATTERN(item, pattern, index);
	if (item->type != RTE_FLOW_ITEM_TYPE_END) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Not supported by ntuple filter");
		return -rte_errno;
	}

	/* parse action */
	index = 0;

	/**
	 * n-tuple only supports forwarding,
	 * check if the first not void action is QUEUE.
	 */
	NEXT_ITEM_OF_ACTION(act, actions, index);
	if (act->type != RTE_FLOW_ACTION_TYPE_QUEUE) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ACTION,
			item, "Not supported action.");
		return -rte_errno;
	}
	filter->queue =
		((const struct rte_flow_action_queue *)act->conf)->index;

	/* check if the next not void item is END */
	index++;
	NEXT_ITEM_OF_ACTION(act, actions, index);
	if (act->type != RTE_FLOW_ACTION_TYPE_END) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ACTION,
			act, "Not supported action.");
		return -rte_errno;
	}

	/* parse attr */
	/* must be input direction */
	if (!attr->ingress) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
				   RTE_FLOW_ERROR_TYPE_ATTR_INGRESS,
				   attr, "Only support ingress.");
		return -rte_errno;
	}

	/* not supported */
	if (attr->egress) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
				   RTE_FLOW_ERROR_TYPE_ATTR_EGRESS,
				   attr, "Not support egress.");
		return -rte_errno;
	}

	if (attr->priority > 0xFFFF) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
				   RTE_FLOW_ERROR_TYPE_ATTR_PRIORITY,
				   attr, "Error priority.");
		return -rte_errno;
	}
	filter->priority = (uint16_t)attr->priority;
	if (attr->priority < IXGBE_MIN_N_TUPLE_PRIO ||
	    attr->priority > IXGBE_MAX_N_TUPLE_PRIO)
	    filter->priority = 1;

	return 0;
}

/* a specific function for ixgbe because the flags is specific */
static int
ixgbe_parse_ntuple_filter(const struct rte_flow_attr *attr,
			  const struct rte_flow_item pattern[],
			  const struct rte_flow_action actions[],
			  struct rte_eth_ntuple_filter *filter,
			  struct rte_flow_error *error)
{
	int ret;

	ret = cons_parse_ntuple_filter(attr, pattern, actions, filter, error);

	if (ret)
		return ret;

	/* Ixgbe doesn't support tcp flags. */
	if (filter->flags & RTE_NTUPLE_FLAGS_TCP_FLAG) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
				   RTE_FLOW_ERROR_TYPE_ITEM,
				   NULL, "Not supported by ntuple filter");
		return -rte_errno;
	}

	/* Ixgbe doesn't support many priorities. */
	if (filter->priority < IXGBE_MIN_N_TUPLE_PRIO ||
	    filter->priority > IXGBE_MAX_N_TUPLE_PRIO) {
		memset(filter, 0, sizeof(struct rte_eth_ntuple_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			NULL, "Priority not supported by ntuple filter");
		return -rte_errno;
	}

	if (filter->queue >= IXGBE_MAX_RX_QUEUE_NUM ||
		filter->priority > IXGBE_5TUPLE_MAX_PRI ||
		filter->priority < IXGBE_5TUPLE_MIN_PRI)
		return -rte_errno;

	/* fixed value for ixgbe */
	filter->flags = RTE_5TUPLE_FLAGS;
	return 0;
}

/**
 * Parse the rule to see if it is a ethertype rule.
 * And get the ethertype filter info BTW.
 * pattern:
 * The first not void item can be ETH.
 * The next not void item must be END.
 * action:
 * The first not void action should be QUEUE.
 * The next not void action should be END.
 * pattern example:
 * ITEM		Spec			Mask
 * ETH		type	0x0807		0xFFFF
 * END
 * other members in mask and spec should set to 0x00.
 * item->last should be NULL.
 */
static int
cons_parse_ethertype_filter(const struct rte_flow_attr *attr,
			    const struct rte_flow_item *pattern,
			    const struct rte_flow_action *actions,
			    struct rte_eth_ethertype_filter *filter,
			    struct rte_flow_error *error)
{
	const struct rte_flow_item *item;
	const struct rte_flow_action *act;
	const struct rte_flow_item_eth *eth_spec;
	const struct rte_flow_item_eth *eth_mask;
	const struct rte_flow_action_queue *act_q;
	uint32_t index;

	if (!pattern) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM_NUM,
				NULL, "NULL pattern.");
		return -rte_errno;
	}

	if (!actions) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ACTION_NUM,
				NULL, "NULL action.");
		return -rte_errno;
	}

	if (!attr) {
		rte_flow_error_set(error, EINVAL,
				   RTE_FLOW_ERROR_TYPE_ATTR,
				   NULL, "NULL attribute.");
		return -rte_errno;
	}

	/* Parse pattern */
	index = 0;

	/* The first non-void item should be MAC. */
	item = pattern + index;
	while (item->type == RTE_FLOW_ITEM_TYPE_VOID) {
		index++;
		item = pattern + index;
	}
	if (item->type != RTE_FLOW_ITEM_TYPE_ETH) {
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Not supported by ethertype filter");
		return -rte_errno;
	}

	/*Not supported last point for range*/
	if (item->last) {
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
			item, "Not supported last point for range");
		return -rte_errno;
	}

	/* Get the MAC info. */
	if (!item->spec || !item->mask) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by ethertype filter");
		return -rte_errno;
	}

	eth_spec = (const struct rte_flow_item_eth *)item->spec;
	eth_mask = (const struct rte_flow_item_eth *)item->mask;

	/* Mask bits of source MAC address must be full of 0.
	 * Mask bits of destination MAC address must be full
	 * of 1 or full of 0.
	 */
	if (!is_zero_ether_addr(&eth_mask->src) ||
	    (!is_zero_ether_addr(&eth_mask->dst) &&
	     !is_broadcast_ether_addr(&eth_mask->dst))) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Invalid ether address mask");
		return -rte_errno;
	}

	if ((eth_mask->type & UINT16_MAX) != UINT16_MAX) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Invalid ethertype mask");
		return -rte_errno;
	}

	/* If mask bits of destination MAC address
	 * are full of 1, set RTE_ETHTYPE_FLAGS_MAC.
	 */
	if (is_broadcast_ether_addr(&eth_mask->dst)) {
		filter->mac_addr = eth_spec->dst;
		filter->flags |= RTE_ETHTYPE_FLAGS_MAC;
	} else {
		filter->flags &= ~RTE_ETHTYPE_FLAGS_MAC;
	}
	filter->ether_type = rte_be_to_cpu_16(eth_spec->type);

	/* Check if the next non-void item is END. */
	index++;
	item = pattern + index;
	while (item->type == RTE_FLOW_ITEM_TYPE_VOID) {
		index++;
		item = pattern + index;
	}
	if (item->type != RTE_FLOW_ITEM_TYPE_END) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by ethertype filter.");
		return -rte_errno;
	}

	/* Parse action */

	index = 0;
	/* Check if the first non-void action is QUEUE or DROP. */
	act = actions + index;
	while (act->type == RTE_FLOW_ACTION_TYPE_VOID) {
		index++;
		act = actions + index;
	}
	if (act->type != RTE_FLOW_ACTION_TYPE_QUEUE &&
	    act->type != RTE_FLOW_ACTION_TYPE_DROP) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ACTION,
				act, "Not supported action.");
		return -rte_errno;
	}

	if (act->type == RTE_FLOW_ACTION_TYPE_QUEUE) {
		act_q = (const struct rte_flow_action_queue *)act->conf;
		filter->queue = act_q->index;
	} else {
		filter->flags |= RTE_ETHTYPE_FLAGS_DROP;
	}

	/* Check if the next non-void item is END */
	index++;
	act = actions + index;
	while (act->type == RTE_FLOW_ACTION_TYPE_VOID) {
		index++;
		act = actions + index;
	}
	if (act->type != RTE_FLOW_ACTION_TYPE_END) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ACTION,
				act, "Not supported action.");
		return -rte_errno;
	}

	/* Parse attr */
	/* Must be input direction */
	if (!attr->ingress) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ATTR_INGRESS,
				attr, "Only support ingress.");
		return -rte_errno;
	}

	/* Not supported */
	if (attr->egress) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ATTR_EGRESS,
				attr, "Not support egress.");
		return -rte_errno;
	}

	/* Not supported */
	if (attr->priority) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ATTR_PRIORITY,
				attr, "Not support priority.");
		return -rte_errno;
	}

	/* Not supported */
	if (attr->group) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ATTR_GROUP,
				attr, "Not support group.");
		return -rte_errno;
	}

	return 0;
}

static int
ixgbe_parse_ethertype_filter(const struct rte_flow_attr *attr,
			     const struct rte_flow_item pattern[],
			     const struct rte_flow_action actions[],
			     struct rte_eth_ethertype_filter *filter,
			     struct rte_flow_error *error)
{
	int ret;

	ret = cons_parse_ethertype_filter(attr, pattern,
					actions, filter, error);

	if (ret)
		return ret;

	/* Ixgbe doesn't support MAC address. */
	if (filter->flags & RTE_ETHTYPE_FLAGS_MAC) {
		memset(filter, 0, sizeof(struct rte_eth_ethertype_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			NULL, "Not supported by ethertype filter");
		return -rte_errno;
	}

	if (filter->queue >= IXGBE_MAX_RX_QUEUE_NUM) {
		memset(filter, 0, sizeof(struct rte_eth_ethertype_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			NULL, "queue index much too big");
		return -rte_errno;
	}

	if (filter->ether_type == ETHER_TYPE_IPv4 ||
		filter->ether_type == ETHER_TYPE_IPv6) {
		memset(filter, 0, sizeof(struct rte_eth_ethertype_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			NULL, "IPv4/IPv6 not supported by ethertype filter");
		return -rte_errno;
	}

	if (filter->flags & RTE_ETHTYPE_FLAGS_MAC) {
		memset(filter, 0, sizeof(struct rte_eth_ethertype_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			NULL, "mac compare is unsupported");
		return -rte_errno;
	}

	if (filter->flags & RTE_ETHTYPE_FLAGS_DROP) {
		memset(filter, 0, sizeof(struct rte_eth_ethertype_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			NULL, "drop option is unsupported");
		return -rte_errno;
	}

	return 0;
}

/**
 * Parse the rule to see if it is a TCP SYN rule.
 * And get the TCP SYN filter info BTW.
 * pattern:
 * The first not void item must be ETH.
 * The second not void item must be IPV4 or IPV6.
 * The third not void item must be TCP.
 * The next not void item must be END.
 * action:
 * The first not void action should be QUEUE.
 * The next not void action should be END.
 * pattern example:
 * ITEM		Spec			Mask
 * ETH		NULL			NULL
 * IPV4/IPV6	NULL			NULL
 * TCP		tcp_flags	0x02	0xFF
 * END
 * other members in mask and spec should set to 0x00.
 * item->last should be NULL.
 */
static int
cons_parse_syn_filter(const struct rte_flow_attr *attr,
				const struct rte_flow_item pattern[],
				const struct rte_flow_action actions[],
				struct rte_eth_syn_filter *filter,
				struct rte_flow_error *error)
{
	const struct rte_flow_item *item;
	const struct rte_flow_action *act;
	const struct rte_flow_item_tcp *tcp_spec;
	const struct rte_flow_item_tcp *tcp_mask;
	const struct rte_flow_action_queue *act_q;
	uint32_t index;

	if (!pattern) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM_NUM,
				NULL, "NULL pattern.");
		return -rte_errno;
	}

	if (!actions) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ACTION_NUM,
				NULL, "NULL action.");
		return -rte_errno;
	}

	if (!attr) {
		rte_flow_error_set(error, EINVAL,
				   RTE_FLOW_ERROR_TYPE_ATTR,
				   NULL, "NULL attribute.");
		return -rte_errno;
	}

	/* parse pattern */
	index = 0;

	/* the first not void item should be MAC or IPv4 or IPv6 or TCP */
	NEXT_ITEM_OF_PATTERN(item, pattern, index);
	if (item->type != RTE_FLOW_ITEM_TYPE_ETH &&
	    item->type != RTE_FLOW_ITEM_TYPE_IPV4 &&
	    item->type != RTE_FLOW_ITEM_TYPE_IPV6 &&
	    item->type != RTE_FLOW_ITEM_TYPE_TCP) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by syn filter");
		return -rte_errno;
	}
		/*Not supported last point for range*/
	if (item->last) {
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
			item, "Not supported last point for range");
		return -rte_errno;
	}

	/* Skip Ethernet */
	if (item->type == RTE_FLOW_ITEM_TYPE_ETH) {
		/* if the item is MAC, the content should be NULL */
		if (item->spec || item->mask) {
			rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Invalid SYN address mask");
			return -rte_errno;
		}

		/* check if the next not void item is IPv4 or IPv6 */
		index++;
		NEXT_ITEM_OF_PATTERN(item, pattern, index);
		if (item->type != RTE_FLOW_ITEM_TYPE_IPV4 &&
		    item->type != RTE_FLOW_ITEM_TYPE_IPV6) {
			rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by syn filter");
			return -rte_errno;
		}
	}

	/* Skip IP */
	if (item->type == RTE_FLOW_ITEM_TYPE_IPV4 ||
	    item->type == RTE_FLOW_ITEM_TYPE_IPV6) {
		/* if the item is IP, the content should be NULL */
		if (item->spec || item->mask) {
			rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Invalid SYN mask");
			return -rte_errno;
		}

		/* check if the next not void item is TCP */
		index++;
		NEXT_ITEM_OF_PATTERN(item, pattern, index);
		if (item->type != RTE_FLOW_ITEM_TYPE_TCP) {
			rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by syn filter");
			return -rte_errno;
		}
	}

	/* Get the TCP info. Only support SYN. */
	if (!item->spec || !item->mask) {
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Invalid SYN mask");
		return -rte_errno;
	}
	/*Not supported last point for range*/
	if (item->last) {
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
			item, "Not supported last point for range");
		return -rte_errno;
	}

	tcp_spec = (const struct rte_flow_item_tcp *)item->spec;
	tcp_mask = (const struct rte_flow_item_tcp *)item->mask;
	if (!(tcp_spec->hdr.tcp_flags & TCP_SYN_FLAG) ||
	    tcp_mask->hdr.src_port ||
	    tcp_mask->hdr.dst_port ||
	    tcp_mask->hdr.sent_seq ||
	    tcp_mask->hdr.recv_ack ||
	    tcp_mask->hdr.data_off ||
	    tcp_mask->hdr.tcp_flags != TCP_SYN_FLAG ||
	    tcp_mask->hdr.rx_win ||
	    tcp_mask->hdr.cksum ||
	    tcp_mask->hdr.tcp_urp) {
		memset(filter, 0, sizeof(struct rte_eth_syn_filter));
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by syn filter");
		return -rte_errno;
	}

	/* check if the next not void item is END */
	index++;
	NEXT_ITEM_OF_PATTERN(item, pattern, index);
	if (item->type != RTE_FLOW_ITEM_TYPE_END) {
		memset(filter, 0, sizeof(struct rte_eth_syn_filter));
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item, "Not supported by syn filter");
		return -rte_errno;
	}

	/* parse action */
	index = 0;

	/* check if the first not void action is QUEUE. */
	NEXT_ITEM_OF_ACTION(act, actions, index);
	if (act->type != RTE_FLOW_ACTION_TYPE_QUEUE) {
		memset(filter, 0, sizeof(struct rte_eth_syn_filter));
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ACTION,
				act, "Not supported action.");
		return -rte_errno;
	}

	act_q = (const struct rte_flow_action_queue *)act->conf;
	filter->queue = act_q->index;
	if (filter->queue >= IXGBE_MAX_RX_QUEUE_NUM) {
		memset(filter, 0, sizeof(struct rte_eth_syn_filter));
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ACTION,
				act, "Not supported action.");
		return -rte_errno;
	}

	/* check if the next not void item is END */
	index++;
	NEXT_ITEM_OF_ACTION(act, actions, index);
	if (act->type != RTE_FLOW_ACTION_TYPE_END) {
		memset(filter, 0, sizeof(struct rte_eth_syn_filter));
		rte_flow_error_set(error, EINVAL,
				RTE_FLOW_ERROR_TYPE_ACTION,
				act, "Not supported action.");
		return -rte_errno;
	}

	/* parse attr */
	/* must be input direction */
	if (!attr->ingress) {
		memset(filter, 0, sizeof(struct rte_eth_syn_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ATTR_INGRESS,
			attr, "Only support ingress.");
		return -rte_errno;
	}

	/* not supported */
	if (attr->egress) {
		memset(filter, 0, sizeof(struct rte_eth_syn_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ATTR_EGRESS,
			attr, "Not support egress.");
		return -rte_errno;
	}

	/* Support 2 priorities, the lowest or highest. */
	if (!attr->priority) {
		filter->hig_pri = 0;
	} else if (attr->priority == (uint32_t)~0U) {
		filter->hig_pri = 1;
	} else {
		memset(filter, 0, sizeof(struct rte_eth_syn_filter));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ATTR_PRIORITY,
			attr, "Not support priority.");
		return -rte_errno;
	}

	return 0;
}

static int
ixgbe_parse_syn_filter(const struct rte_flow_attr *attr,
			     const struct rte_flow_item pattern[],
			     const struct rte_flow_action actions[],
			     struct rte_eth_syn_filter *filter,
			     struct rte_flow_error *error)
{
	int ret;

	ret = cons_parse_syn_filter(attr, pattern,
					actions, filter, error);

	if (ret)
		return ret;

	return 0;
}

/**
 * Parse the rule to see if it is a L2 tunnel rule.
 * And get the L2 tunnel filter info BTW.
 * Only support E-tag now.
 * pattern:
 * The first not void item can be E_TAG.
 * The next not void item must be END.
 * action:
 * The first not void action should be QUEUE.
 * The next not void action should be END.
 * pattern example:
 * ITEM		Spec			Mask
 * E_TAG	grp		0x1	0x3
		e_cid_base	0x309	0xFFF
 * END
 * other members in mask and spec should set to 0x00.
 * item->last should be NULL.
 */
static int
cons_parse_l2_tn_filter(const struct rte_flow_attr *attr,
			const struct rte_flow_item pattern[],
			const struct rte_flow_action actions[],
			struct rte_eth_l2_tunnel_conf *filter,
			struct rte_flow_error *error)
{
	const struct rte_flow_item *item;
	const struct rte_flow_item_e_tag *e_tag_spec;
	const struct rte_flow_item_e_tag *e_tag_mask;
	const struct rte_flow_action *act;
	const struct rte_flow_action_queue *act_q;
	uint32_t index;

	if (!pattern) {
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM_NUM,
			NULL, "NULL pattern.");
		return -rte_errno;
	}

	if (!actions) {
		rte_flow_error_set(error, EINVAL,
				   RTE_FLOW_ERROR_TYPE_ACTION_NUM,
				   NULL, "NULL action.");
		return -rte_errno;
	}

	if (!attr) {
		rte_flow_error_set(error, EINVAL,
				   RTE_FLOW_ERROR_TYPE_ATTR,
				   NULL, "NULL attribute.");
		return -rte_errno;
	}
	/* parse pattern */
	index = 0;

	/* The first not void item should be e-tag. */
	NEXT_ITEM_OF_PATTERN(item, pattern, index);
	if (item->type != RTE_FLOW_ITEM_TYPE_E_TAG) {
		memset(filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Not supported by L2 tunnel filter");
		return -rte_errno;
	}

	if (!item->spec || !item->mask) {
		memset(filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
		rte_flow_error_set(error, EINVAL, RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Not supported by L2 tunnel filter");
		return -rte_errno;
	}

	/*Not supported last point for range*/
	if (item->last) {
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
			item, "Not supported last point for range");
		return -rte_errno;
	}

	e_tag_spec = (const struct rte_flow_item_e_tag *)item->spec;
	e_tag_mask = (const struct rte_flow_item_e_tag *)item->mask;

	/* Only care about GRP and E cid base. */
	if (e_tag_mask->epcp_edei_in_ecid_b ||
	    e_tag_mask->in_ecid_e ||
	    e_tag_mask->ecid_e ||
	    e_tag_mask->rsvd_grp_ecid_b != rte_cpu_to_be_16(0x3FFF)) {
		memset(filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Not supported by L2 tunnel filter");
		return -rte_errno;
	}

	filter->l2_tunnel_type = RTE_L2_TUNNEL_TYPE_E_TAG;
	/**
	 * grp and e_cid_base are bit fields and only use 14 bits.
	 * e-tag id is taken as little endian by HW.
	 */
	filter->tunnel_id = rte_be_to_cpu_16(e_tag_spec->rsvd_grp_ecid_b);

	/* check if the next not void item is END */
	index++;
	NEXT_ITEM_OF_PATTERN(item, pattern, index);
	if (item->type != RTE_FLOW_ITEM_TYPE_END) {
		memset(filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item, "Not supported by L2 tunnel filter");
		return -rte_errno;
	}

	/* parse attr */
	/* must be input direction */
	if (!attr->ingress) {
		memset(filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ATTR_INGRESS,
			attr, "Only support ingress.");
		return -rte_errno;
	}

	/* not supported */
	if (attr->egress) {
		memset(filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ATTR_EGRESS,
			attr, "Not support egress.");
		return -rte_errno;
	}

	/* not supported */
	if (attr->priority) {
		memset(filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ATTR_PRIORITY,
			attr, "Not support priority.");
		return -rte_errno;
	}

	/* parse action */
	index = 0;

	/* check if the first not void action is QUEUE. */
	NEXT_ITEM_OF_ACTION(act, actions, index);
	if (act->type != RTE_FLOW_ACTION_TYPE_QUEUE) {
		memset(filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ACTION,
			act, "Not supported action.");
		return -rte_errno;
	}

	act_q = (const struct rte_flow_action_queue *)act->conf;
	filter->pool = act_q->index;

	/* check if the next not void item is END */
	index++;
	NEXT_ITEM_OF_ACTION(act, actions, index);
	if (act->type != RTE_FLOW_ACTION_TYPE_END) {
		memset(filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ACTION,
			act, "Not supported action.");
		return -rte_errno;
	}

	return 0;
}

static int
ixgbe_validate_l2_tn_filter(struct rte_eth_dev *dev,
			const struct rte_flow_attr *attr,
			const struct rte_flow_item pattern[],
			const struct rte_flow_action actions[],
			struct rte_eth_l2_tunnel_conf *l2_tn_filter,
			struct rte_flow_error *error)
{
	int ret = 0;
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	ret = cons_parse_l2_tn_filter(attr, pattern,
				actions, l2_tn_filter, error);

	if (hw->mac.type != ixgbe_mac_X550 &&
		hw->mac.type != ixgbe_mac_X550EM_x &&
		hw->mac.type != ixgbe_mac_X550EM_a) {
		memset(l2_tn_filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
		rte_flow_error_set(error, EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			NULL, "Not supported by L2 tunnel filter");
		return -rte_errno;
	}

	return ret;
}

/**
 * Check if the flow rule is supported by ixgbe.
 * It only checkes the format. Don't guarantee the rule can be programmed into
 * the HW. Because there can be no enough room for the rule.
 */
static int
ixgbe_flow_validate(__rte_unused struct rte_eth_dev *dev,
		const struct rte_flow_attr *attr,
		const struct rte_flow_item pattern[],
		const struct rte_flow_action actions[],
		struct rte_flow_error *error)
{
	struct rte_eth_ntuple_filter ntuple_filter;
	struct rte_eth_ethertype_filter ethertype_filter;
	struct rte_eth_syn_filter syn_filter;
	struct rte_eth_l2_tunnel_conf l2_tn_filter;
	int ret;

	memset(&ntuple_filter, 0, sizeof(struct rte_eth_ntuple_filter));
	ret = ixgbe_parse_ntuple_filter(attr, pattern,
				actions, &ntuple_filter, error);
	if (!ret)
		return 0;

	memset(&ethertype_filter, 0, sizeof(struct rte_eth_ethertype_filter));
	ret = ixgbe_parse_ethertype_filter(attr, pattern,
				actions, &ethertype_filter, error);
	if (!ret)
		return 0;

	memset(&syn_filter, 0, sizeof(struct rte_eth_syn_filter));
	ret = ixgbe_parse_syn_filter(attr, pattern,
				actions, &syn_filter, error);
	if (!ret)
		return 0;

	memset(&l2_tn_filter, 0, sizeof(struct rte_eth_l2_tunnel_conf));
	ret = ixgbe_validate_l2_tn_filter(dev, attr, pattern,
				actions, &l2_tn_filter, error);

	return ret;
}

/*  Destroy all flow rules associated with a port on ixgbe. */
static int
ixgbe_flow_flush(struct rte_eth_dev *dev,
		struct rte_flow_error *error)
{
	int ret = 0;

	ixgbe_clear_all_ntuple_filter(dev);
	ixgbe_clear_all_ethertype_filter(dev);
	ixgbe_clear_syn_filter(dev);

	ret = ixgbe_clear_all_fdir_filter(dev);
	if (ret < 0) {
		rte_flow_error_set(error, EINVAL, RTE_FLOW_ERROR_TYPE_HANDLE,
					NULL, "Failed to flush rule");
		return ret;
	}

	ret = ixgbe_clear_all_l2_tn_filter(dev);
	if (ret < 0) {
		rte_flow_error_set(error, EINVAL, RTE_FLOW_ERROR_TYPE_HANDLE,
					NULL, "Failed to flush rule");
		return ret;
	}

	return 0;
}
