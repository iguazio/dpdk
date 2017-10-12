/*-
 *   BSD LICENSE
 *
 *   Copyright 2017 6WIND S.A.
 *   Copyright 2017 Mellanox
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
 *     * Neither the name of 6WIND S.A. nor the names of its
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

/**
 * @file
 * Flow API operations for mlx4 driver.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/queue.h>

/* Verbs headers do not support -pedantic. */
#ifdef PEDANTIC
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <infiniband/verbs.h>
#ifdef PEDANTIC
#pragma GCC diagnostic error "-Wpedantic"
#endif

#include <rte_errno.h>
#include <rte_eth_ctrl.h>
#include <rte_ethdev.h>
#include <rte_flow.h>
#include <rte_flow_driver.h>
#include <rte_malloc.h>

/* PMD headers. */
#include "mlx4.h"
#include "mlx4_flow.h"
#include "mlx4_rxtx.h"
#include "mlx4_utils.h"

/** Static initializer for a list of subsequent item types. */
#define NEXT_ITEM(...) \
	(const enum rte_flow_item_type []){ \
		__VA_ARGS__, RTE_FLOW_ITEM_TYPE_END, \
	}

/** Processor structure associated with a flow item. */
struct mlx4_flow_proc_item {
	/** Bit-masks corresponding to the possibilities for the item. */
	const void *mask;
	/**
	 * Default bit-masks to use when item->mask is not provided. When
	 * \default_mask is also NULL, the full supported bit-mask (\mask) is
	 * used instead.
	 */
	const void *default_mask;
	/** Bit-masks size in bytes. */
	const unsigned int mask_sz;
	/**
	 * Check support for a given item.
	 *
	 * @param item[in]
	 *   Item specification.
	 * @param mask[in]
	 *   Bit-masks covering supported fields to compare with spec,
	 *   last and mask in
	 *   \item.
	 * @param size
	 *   Bit-Mask size in bytes.
	 *
	 * @return
	 *   0 on success, negative value otherwise.
	 */
	int (*validate)(const struct rte_flow_item *item,
			const uint8_t *mask, unsigned int size);
	/**
	 * Conversion function from rte_flow to NIC specific flow.
	 *
	 * @param item
	 *   rte_flow item to convert.
	 * @param default_mask
	 *   Default bit-masks to use when item->mask is not provided.
	 * @param data
	 *   Internal structure to store the conversion.
	 *
	 * @return
	 *   0 on success, negative value otherwise.
	 */
	int (*convert)(const struct rte_flow_item *item,
		       const void *default_mask,
		       void *data);
	/** Size in bytes of the destination structure. */
	const unsigned int dst_sz;
	/** List of possible subsequent items. */
	const enum rte_flow_item_type *const next_item;
};

struct rte_flow_drop {
	struct ibv_qp *qp; /**< Verbs queue pair. */
	struct ibv_cq *cq; /**< Verbs completion queue. */
};

/**
 * Convert Ethernet item to Verbs specification.
 *
 * @param item[in]
 *   Item specification.
 * @param default_mask[in]
 *   Default bit-masks to use when item->mask is not provided.
 * @param data[in, out]
 *   User structure.
 */
static int
mlx4_flow_create_eth(const struct rte_flow_item *item,
		     const void *default_mask,
		     void *data)
{
	const struct rte_flow_item_eth *spec = item->spec;
	const struct rte_flow_item_eth *mask = item->mask;
	struct mlx4_flow *flow = (struct mlx4_flow *)data;
	struct ibv_flow_spec_eth *eth;
	const unsigned int eth_size = sizeof(struct ibv_flow_spec_eth);
	unsigned int i;

	++flow->ibv_attr->num_of_specs;
	flow->ibv_attr->priority = 2;
	eth = (void *)((uintptr_t)flow->ibv_attr + flow->offset);
	*eth = (struct ibv_flow_spec_eth) {
		.type = IBV_FLOW_SPEC_ETH,
		.size = eth_size,
	};
	if (!spec) {
		flow->ibv_attr->type = IBV_FLOW_ATTR_ALL_DEFAULT;
		return 0;
	}
	if (!mask)
		mask = default_mask;
	memcpy(eth->val.dst_mac, spec->dst.addr_bytes, ETHER_ADDR_LEN);
	memcpy(eth->val.src_mac, spec->src.addr_bytes, ETHER_ADDR_LEN);
	memcpy(eth->mask.dst_mac, mask->dst.addr_bytes, ETHER_ADDR_LEN);
	memcpy(eth->mask.src_mac, mask->src.addr_bytes, ETHER_ADDR_LEN);
	/* Remove unwanted bits from values. */
	for (i = 0; i < ETHER_ADDR_LEN; ++i) {
		eth->val.dst_mac[i] &= eth->mask.dst_mac[i];
		eth->val.src_mac[i] &= eth->mask.src_mac[i];
	}
	return 0;
}

/**
 * Convert VLAN item to Verbs specification.
 *
 * @param item[in]
 *   Item specification.
 * @param default_mask[in]
 *   Default bit-masks to use when item->mask is not provided.
 * @param data[in, out]
 *   User structure.
 */
static int
mlx4_flow_create_vlan(const struct rte_flow_item *item,
		      const void *default_mask,
		      void *data)
{
	const struct rte_flow_item_vlan *spec = item->spec;
	const struct rte_flow_item_vlan *mask = item->mask;
	struct mlx4_flow *flow = (struct mlx4_flow *)data;
	struct ibv_flow_spec_eth *eth;
	const unsigned int eth_size = sizeof(struct ibv_flow_spec_eth);

	eth = (void *)((uintptr_t)flow->ibv_attr + flow->offset - eth_size);
	if (!spec)
		return 0;
	if (!mask)
		mask = default_mask;
	eth->val.vlan_tag = spec->tci;
	eth->mask.vlan_tag = mask->tci;
	eth->val.vlan_tag &= eth->mask.vlan_tag;
	return 0;
}

/**
 * Convert IPv4 item to Verbs specification.
 *
 * @param item[in]
 *   Item specification.
 * @param default_mask[in]
 *   Default bit-masks to use when item->mask is not provided.
 * @param data[in, out]
 *   User structure.
 */
static int
mlx4_flow_create_ipv4(const struct rte_flow_item *item,
		      const void *default_mask,
		      void *data)
{
	const struct rte_flow_item_ipv4 *spec = item->spec;
	const struct rte_flow_item_ipv4 *mask = item->mask;
	struct mlx4_flow *flow = (struct mlx4_flow *)data;
	struct ibv_flow_spec_ipv4 *ipv4;
	unsigned int ipv4_size = sizeof(struct ibv_flow_spec_ipv4);

	++flow->ibv_attr->num_of_specs;
	flow->ibv_attr->priority = 1;
	ipv4 = (void *)((uintptr_t)flow->ibv_attr + flow->offset);
	*ipv4 = (struct ibv_flow_spec_ipv4) {
		.type = IBV_FLOW_SPEC_IPV4,
		.size = ipv4_size,
	};
	if (!spec)
		return 0;
	ipv4->val = (struct ibv_flow_ipv4_filter) {
		.src_ip = spec->hdr.src_addr,
		.dst_ip = spec->hdr.dst_addr,
	};
	if (!mask)
		mask = default_mask;
	ipv4->mask = (struct ibv_flow_ipv4_filter) {
		.src_ip = mask->hdr.src_addr,
		.dst_ip = mask->hdr.dst_addr,
	};
	/* Remove unwanted bits from values. */
	ipv4->val.src_ip &= ipv4->mask.src_ip;
	ipv4->val.dst_ip &= ipv4->mask.dst_ip;
	return 0;
}

/**
 * Convert UDP item to Verbs specification.
 *
 * @param item[in]
 *   Item specification.
 * @param default_mask[in]
 *   Default bit-masks to use when item->mask is not provided.
 * @param data[in, out]
 *   User structure.
 */
static int
mlx4_flow_create_udp(const struct rte_flow_item *item,
		     const void *default_mask,
		     void *data)
{
	const struct rte_flow_item_udp *spec = item->spec;
	const struct rte_flow_item_udp *mask = item->mask;
	struct mlx4_flow *flow = (struct mlx4_flow *)data;
	struct ibv_flow_spec_tcp_udp *udp;
	unsigned int udp_size = sizeof(struct ibv_flow_spec_tcp_udp);

	++flow->ibv_attr->num_of_specs;
	flow->ibv_attr->priority = 0;
	udp = (void *)((uintptr_t)flow->ibv_attr + flow->offset);
	*udp = (struct ibv_flow_spec_tcp_udp) {
		.type = IBV_FLOW_SPEC_UDP,
		.size = udp_size,
	};
	if (!spec)
		return 0;
	udp->val.dst_port = spec->hdr.dst_port;
	udp->val.src_port = spec->hdr.src_port;
	if (!mask)
		mask = default_mask;
	udp->mask.dst_port = mask->hdr.dst_port;
	udp->mask.src_port = mask->hdr.src_port;
	/* Remove unwanted bits from values. */
	udp->val.src_port &= udp->mask.src_port;
	udp->val.dst_port &= udp->mask.dst_port;
	return 0;
}

/**
 * Convert TCP item to Verbs specification.
 *
 * @param item[in]
 *   Item specification.
 * @param default_mask[in]
 *   Default bit-masks to use when item->mask is not provided.
 * @param data[in, out]
 *   User structure.
 */
static int
mlx4_flow_create_tcp(const struct rte_flow_item *item,
		     const void *default_mask,
		     void *data)
{
	const struct rte_flow_item_tcp *spec = item->spec;
	const struct rte_flow_item_tcp *mask = item->mask;
	struct mlx4_flow *flow = (struct mlx4_flow *)data;
	struct ibv_flow_spec_tcp_udp *tcp;
	unsigned int tcp_size = sizeof(struct ibv_flow_spec_tcp_udp);

	++flow->ibv_attr->num_of_specs;
	flow->ibv_attr->priority = 0;
	tcp = (void *)((uintptr_t)flow->ibv_attr + flow->offset);
	*tcp = (struct ibv_flow_spec_tcp_udp) {
		.type = IBV_FLOW_SPEC_TCP,
		.size = tcp_size,
	};
	if (!spec)
		return 0;
	tcp->val.dst_port = spec->hdr.dst_port;
	tcp->val.src_port = spec->hdr.src_port;
	if (!mask)
		mask = default_mask;
	tcp->mask.dst_port = mask->hdr.dst_port;
	tcp->mask.src_port = mask->hdr.src_port;
	/* Remove unwanted bits from values. */
	tcp->val.src_port &= tcp->mask.src_port;
	tcp->val.dst_port &= tcp->mask.dst_port;
	return 0;
}

/**
 * Check support for a given item.
 *
 * @param item[in]
 *   Item specification.
 * @param mask[in]
 *   Bit-masks covering supported fields to compare with spec, last and mask in
 *   \item.
 * @param size
 *   Bit-Mask size in bytes.
 *
 * @return
 *   0 on success, negative value otherwise.
 */
static int
mlx4_flow_item_validate(const struct rte_flow_item *item,
			const uint8_t *mask, unsigned int size)
{
	int ret = 0;

	if (!item->spec && (item->mask || item->last))
		return -1;
	if (item->spec && !item->mask) {
		unsigned int i;
		const uint8_t *spec = item->spec;

		for (i = 0; i < size; ++i)
			if ((spec[i] | mask[i]) != mask[i])
				return -1;
	}
	if (item->last && !item->mask) {
		unsigned int i;
		const uint8_t *spec = item->last;

		for (i = 0; i < size; ++i)
			if ((spec[i] | mask[i]) != mask[i])
				return -1;
	}
	if (item->spec && item->last) {
		uint8_t spec[size];
		uint8_t last[size];
		const uint8_t *apply = mask;
		unsigned int i;

		if (item->mask)
			apply = item->mask;
		for (i = 0; i < size; ++i) {
			spec[i] = ((const uint8_t *)item->spec)[i] & apply[i];
			last[i] = ((const uint8_t *)item->last)[i] & apply[i];
		}
		ret = memcmp(spec, last, size);
	}
	return ret;
}

static int
mlx4_flow_validate_eth(const struct rte_flow_item *item,
		       const uint8_t *mask, unsigned int size)
{
	if (item->mask) {
		const struct rte_flow_item_eth *mask = item->mask;

		if (mask->dst.addr_bytes[0] != 0xff ||
				mask->dst.addr_bytes[1] != 0xff ||
				mask->dst.addr_bytes[2] != 0xff ||
				mask->dst.addr_bytes[3] != 0xff ||
				mask->dst.addr_bytes[4] != 0xff ||
				mask->dst.addr_bytes[5] != 0xff)
			return -1;
	}
	return mlx4_flow_item_validate(item, mask, size);
}

static int
mlx4_flow_validate_vlan(const struct rte_flow_item *item,
			const uint8_t *mask, unsigned int size)
{
	if (item->mask) {
		const struct rte_flow_item_vlan *mask = item->mask;

		if (mask->tci != 0 &&
		    ntohs(mask->tci) != 0x0fff)
			return -1;
	}
	return mlx4_flow_item_validate(item, mask, size);
}

static int
mlx4_flow_validate_ipv4(const struct rte_flow_item *item,
			const uint8_t *mask, unsigned int size)
{
	if (item->mask) {
		const struct rte_flow_item_ipv4 *mask = item->mask;

		if (mask->hdr.src_addr != 0 &&
		    mask->hdr.src_addr != 0xffffffff)
			return -1;
		if (mask->hdr.dst_addr != 0 &&
		    mask->hdr.dst_addr != 0xffffffff)
			return -1;
	}
	return mlx4_flow_item_validate(item, mask, size);
}

static int
mlx4_flow_validate_udp(const struct rte_flow_item *item,
		       const uint8_t *mask, unsigned int size)
{
	if (item->mask) {
		const struct rte_flow_item_udp *mask = item->mask;

		if (mask->hdr.src_port != 0 &&
		    mask->hdr.src_port != 0xffff)
			return -1;
		if (mask->hdr.dst_port != 0 &&
		    mask->hdr.dst_port != 0xffff)
			return -1;
	}
	return mlx4_flow_item_validate(item, mask, size);
}

static int
mlx4_flow_validate_tcp(const struct rte_flow_item *item,
		       const uint8_t *mask, unsigned int size)
{
	if (item->mask) {
		const struct rte_flow_item_tcp *mask = item->mask;

		if (mask->hdr.src_port != 0 &&
		    mask->hdr.src_port != 0xffff)
			return -1;
		if (mask->hdr.dst_port != 0 &&
		    mask->hdr.dst_port != 0xffff)
			return -1;
	}
	return mlx4_flow_item_validate(item, mask, size);
}

/** Graph of supported items and associated actions. */
static const struct mlx4_flow_proc_item mlx4_flow_proc_item_list[] = {
	[RTE_FLOW_ITEM_TYPE_END] = {
		.next_item = NEXT_ITEM(RTE_FLOW_ITEM_TYPE_ETH),
	},
	[RTE_FLOW_ITEM_TYPE_ETH] = {
		.next_item = NEXT_ITEM(RTE_FLOW_ITEM_TYPE_VLAN,
				       RTE_FLOW_ITEM_TYPE_IPV4),
		.mask = &(const struct rte_flow_item_eth){
			.dst.addr_bytes = "\xff\xff\xff\xff\xff\xff",
			.src.addr_bytes = "\xff\xff\xff\xff\xff\xff",
		},
		.default_mask = &rte_flow_item_eth_mask,
		.mask_sz = sizeof(struct rte_flow_item_eth),
		.validate = mlx4_flow_validate_eth,
		.convert = mlx4_flow_create_eth,
		.dst_sz = sizeof(struct ibv_flow_spec_eth),
	},
	[RTE_FLOW_ITEM_TYPE_VLAN] = {
		.next_item = NEXT_ITEM(RTE_FLOW_ITEM_TYPE_IPV4),
		.mask = &(const struct rte_flow_item_vlan){
		/* rte_flow_item_vlan_mask is invalid for mlx4. */
#if RTE_BYTE_ORDER == RTE_BIG_ENDIAN
			.tci = 0x0fff,
#else
			.tci = 0xff0f,
#endif
		},
		.mask_sz = sizeof(struct rte_flow_item_vlan),
		.validate = mlx4_flow_validate_vlan,
		.convert = mlx4_flow_create_vlan,
		.dst_sz = 0,
	},
	[RTE_FLOW_ITEM_TYPE_IPV4] = {
		.next_item = NEXT_ITEM(RTE_FLOW_ITEM_TYPE_UDP,
				       RTE_FLOW_ITEM_TYPE_TCP),
		.mask = &(const struct rte_flow_item_ipv4){
			.hdr = {
				.src_addr = -1,
				.dst_addr = -1,
			},
		},
		.default_mask = &rte_flow_item_ipv4_mask,
		.mask_sz = sizeof(struct rte_flow_item_ipv4),
		.validate = mlx4_flow_validate_ipv4,
		.convert = mlx4_flow_create_ipv4,
		.dst_sz = sizeof(struct ibv_flow_spec_ipv4),
	},
	[RTE_FLOW_ITEM_TYPE_UDP] = {
		.mask = &(const struct rte_flow_item_udp){
			.hdr = {
				.src_port = -1,
				.dst_port = -1,
			},
		},
		.default_mask = &rte_flow_item_udp_mask,
		.mask_sz = sizeof(struct rte_flow_item_udp),
		.validate = mlx4_flow_validate_udp,
		.convert = mlx4_flow_create_udp,
		.dst_sz = sizeof(struct ibv_flow_spec_tcp_udp),
	},
	[RTE_FLOW_ITEM_TYPE_TCP] = {
		.mask = &(const struct rte_flow_item_tcp){
			.hdr = {
				.src_port = -1,
				.dst_port = -1,
			},
		},
		.default_mask = &rte_flow_item_tcp_mask,
		.mask_sz = sizeof(struct rte_flow_item_tcp),
		.validate = mlx4_flow_validate_tcp,
		.convert = mlx4_flow_create_tcp,
		.dst_sz = sizeof(struct ibv_flow_spec_tcp_udp),
	},
};

/**
 * Make sure a flow rule is supported and initialize associated structure.
 *
 * @param priv
 *   Pointer to private structure.
 * @param[in] attr
 *   Flow rule attributes.
 * @param[in] pattern
 *   Pattern specification (list terminated by the END pattern item).
 * @param[in] actions
 *   Associated actions (list terminated by the END action).
 * @param[out] error
 *   Perform verbose error reporting if not NULL.
 * @param[in, out] flow
 *   Flow structure to update.
 *
 * @return
 *   0 on success, a negative errno value otherwise and rte_errno is set.
 */
static int
mlx4_flow_prepare(struct priv *priv,
		  const struct rte_flow_attr *attr,
		  const struct rte_flow_item pattern[],
		  const struct rte_flow_action actions[],
		  struct rte_flow_error *error,
		  struct mlx4_flow *flow)
{
	const struct rte_flow_item *item;
	const struct rte_flow_action *action;
	const struct mlx4_flow_proc_item *proc = mlx4_flow_proc_item_list;
	struct mlx4_flow_target target = {
		.queue = 0,
		.drop = 0,
	};
	uint32_t priority_override = 0;

	if (attr->group) {
		rte_flow_error_set(error, ENOTSUP,
				   RTE_FLOW_ERROR_TYPE_ATTR_GROUP,
				   NULL,
				   "groups are not supported");
		return -rte_errno;
	}
	if (priv->isolated) {
		priority_override = attr->priority;
	} else if (attr->priority) {
		rte_flow_error_set(error, ENOTSUP,
				   RTE_FLOW_ERROR_TYPE_ATTR_PRIORITY,
				   NULL,
				   "priorities are not supported outside"
				   " isolated mode");
		return -rte_errno;
	}
	if (attr->priority > MLX4_FLOW_PRIORITY_LAST) {
		rte_flow_error_set(error, ENOTSUP,
				   RTE_FLOW_ERROR_TYPE_ATTR_PRIORITY,
				   NULL,
				   "maximum priority level is "
				   MLX4_STR_EXPAND(MLX4_FLOW_PRIORITY_LAST));
		return -rte_errno;
	}
	if (attr->egress) {
		rte_flow_error_set(error, ENOTSUP,
				   RTE_FLOW_ERROR_TYPE_ATTR_EGRESS,
				   NULL,
				   "egress is not supported");
		return -rte_errno;
	}
	if (!attr->ingress) {
		rte_flow_error_set(error, ENOTSUP,
				   RTE_FLOW_ERROR_TYPE_ATTR_INGRESS,
				   NULL,
				   "only ingress is supported");
		return -rte_errno;
	}
	/* Go over pattern. */
	for (item = pattern; item->type != RTE_FLOW_ITEM_TYPE_END; ++item) {
		const struct mlx4_flow_proc_item *next = NULL;
		unsigned int i;
		int err;

		if (item->type == RTE_FLOW_ITEM_TYPE_VOID)
			continue;
		/*
		 * The nic can support patterns with NULL eth spec only
		 * if eth is a single item in a rule.
		 */
		if (!item->spec && item->type == RTE_FLOW_ITEM_TYPE_ETH) {
			const struct rte_flow_item *next = item + 1;

			if (next->type != RTE_FLOW_ITEM_TYPE_END) {
				rte_flow_error_set(error, ENOTSUP,
						   RTE_FLOW_ERROR_TYPE_ITEM,
						   item,
						   "the rule requires"
						   " an Ethernet spec");
				return -rte_errno;
			}
		}
		for (i = 0;
		     proc->next_item &&
		     proc->next_item[i] != RTE_FLOW_ITEM_TYPE_END;
		     ++i) {
			if (proc->next_item[i] == item->type) {
				next = &mlx4_flow_proc_item_list[item->type];
				break;
			}
		}
		if (!next)
			goto exit_item_not_supported;
		proc = next;
		err = proc->validate(item, proc->mask, proc->mask_sz);
		if (err)
			goto exit_item_not_supported;
		if (flow->ibv_attr && proc->convert) {
			err = proc->convert(item,
					    (proc->default_mask ?
					     proc->default_mask :
					     proc->mask),
					    flow);
			if (err)
				goto exit_item_not_supported;
		}
		flow->offset += proc->dst_sz;
	}
	/* Use specified priority level when in isolated mode. */
	if (priv->isolated && flow->ibv_attr)
		flow->ibv_attr->priority = priority_override;
	/* Go over actions list. */
	for (action = actions;
	     action->type != RTE_FLOW_ACTION_TYPE_END;
	     ++action) {
		if (action->type == RTE_FLOW_ACTION_TYPE_VOID) {
			continue;
		} else if (action->type == RTE_FLOW_ACTION_TYPE_DROP) {
			target.drop = 1;
		} else if (action->type == RTE_FLOW_ACTION_TYPE_QUEUE) {
			const struct rte_flow_action_queue *queue =
				action->conf;

			if (!queue || (queue->index >
				       (priv->dev->data->nb_rx_queues - 1)))
				goto exit_action_not_supported;
			target.queue = 1;
		} else {
			goto exit_action_not_supported;
		}
	}
	if (!target.queue && !target.drop) {
		rte_flow_error_set(error, ENOTSUP, RTE_FLOW_ERROR_TYPE_HANDLE,
				   NULL, "no valid action");
		return -rte_errno;
	}
	return 0;
exit_item_not_supported:
	rte_flow_error_set(error, ENOTSUP, RTE_FLOW_ERROR_TYPE_ITEM,
			   item, "item not supported");
	return -rte_errno;
exit_action_not_supported:
	rte_flow_error_set(error, ENOTSUP, RTE_FLOW_ERROR_TYPE_ACTION,
			   action, "action not supported");
	return -rte_errno;
}

/**
 * Validate a flow supported by the NIC.
 *
 * @see rte_flow_validate()
 * @see rte_flow_ops
 */
static int
mlx4_flow_validate(struct rte_eth_dev *dev,
		   const struct rte_flow_attr *attr,
		   const struct rte_flow_item pattern[],
		   const struct rte_flow_action actions[],
		   struct rte_flow_error *error)
{
	struct priv *priv = dev->data->dev_private;
	struct mlx4_flow flow = { .offset = sizeof(struct ibv_flow_attr) };

	return mlx4_flow_prepare(priv, attr, pattern, actions, error, &flow);
}

/**
 * Destroy a drop queue.
 *
 * @param priv
 *   Pointer to private structure.
 */
static void
mlx4_flow_destroy_drop_queue(struct priv *priv)
{
	if (priv->flow_drop_queue) {
		struct rte_flow_drop *fdq = priv->flow_drop_queue;

		priv->flow_drop_queue = NULL;
		claim_zero(ibv_destroy_qp(fdq->qp));
		claim_zero(ibv_destroy_cq(fdq->cq));
		rte_free(fdq);
	}
}

/**
 * Create a single drop queue for all drop flows.
 *
 * @param priv
 *   Pointer to private structure.
 *
 * @return
 *   0 on success, negative value otherwise.
 */
static int
mlx4_flow_create_drop_queue(struct priv *priv)
{
	struct ibv_qp *qp;
	struct ibv_cq *cq;
	struct rte_flow_drop *fdq;

	fdq = rte_calloc(__func__, 1, sizeof(*fdq), 0);
	if (!fdq) {
		ERROR("Cannot allocate memory for drop struct");
		goto err;
	}
	cq = ibv_create_cq(priv->ctx, 1, NULL, NULL, 0);
	if (!cq) {
		ERROR("Cannot create drop CQ");
		goto err_create_cq;
	}
	qp = ibv_create_qp(priv->pd,
			   &(struct ibv_qp_init_attr){
				.send_cq = cq,
				.recv_cq = cq,
				.cap = {
					.max_recv_wr = 1,
					.max_recv_sge = 1,
				},
				.qp_type = IBV_QPT_RAW_PACKET,
			   });
	if (!qp) {
		ERROR("Cannot create drop QP");
		goto err_create_qp;
	}
	*fdq = (struct rte_flow_drop){
		.qp = qp,
		.cq = cq,
	};
	priv->flow_drop_queue = fdq;
	return 0;
err_create_qp:
	claim_zero(ibv_destroy_cq(cq));
err_create_cq:
	rte_free(fdq);
err:
	return -1;
}

/**
 * Complete flow rule creation.
 *
 * @param priv
 *   Pointer to private structure.
 * @param ibv_attr
 *   Verbs flow attributes.
 * @param target
 *   Rule target descriptor.
 * @param[out] error
 *   Perform verbose error reporting if not NULL.
 *
 * @return
 *   A flow if the rule could be created.
 */
static struct rte_flow *
mlx4_flow_create_target_queue(struct priv *priv,
			      struct ibv_flow_attr *ibv_attr,
			      struct mlx4_flow_target *target,
			      struct rte_flow_error *error)
{
	struct ibv_qp *qp;
	struct rte_flow *rte_flow;

	assert(priv->pd);
	assert(priv->ctx);
	rte_flow = rte_calloc(__func__, 1, sizeof(*rte_flow), 0);
	if (!rte_flow) {
		rte_flow_error_set(error, ENOMEM, RTE_FLOW_ERROR_TYPE_HANDLE,
				   NULL, "cannot allocate flow memory");
		return NULL;
	}
	if (target->drop) {
		qp = priv->flow_drop_queue ? priv->flow_drop_queue->qp : NULL;
	} else {
		struct rxq *rxq = priv->dev->data->rx_queues[target->queue_id];

		qp = rxq->qp;
		rte_flow->qp = qp;
	}
	rte_flow->ibv_attr = ibv_attr;
	if (!priv->started)
		return rte_flow;
	rte_flow->ibv_flow = ibv_create_flow(qp, rte_flow->ibv_attr);
	if (!rte_flow->ibv_flow) {
		rte_flow_error_set(error, ENOMEM, RTE_FLOW_ERROR_TYPE_HANDLE,
				   NULL, "flow rule creation failure");
		goto error;
	}
	return rte_flow;
error:
	rte_free(rte_flow);
	return NULL;
}

/**
 * Create a flow.
 *
 * @see rte_flow_create()
 * @see rte_flow_ops
 */
static struct rte_flow *
mlx4_flow_create(struct rte_eth_dev *dev,
		 const struct rte_flow_attr *attr,
		 const struct rte_flow_item pattern[],
		 const struct rte_flow_action actions[],
		 struct rte_flow_error *error)
{
	const struct rte_flow_action *action;
	struct priv *priv = dev->data->dev_private;
	struct rte_flow *rte_flow;
	struct mlx4_flow_target target;
	struct mlx4_flow flow = { .offset = sizeof(struct ibv_flow_attr), };
	int err;

	err = mlx4_flow_prepare(priv, attr, pattern, actions, error, &flow);
	if (err)
		return NULL;
	flow.ibv_attr = rte_malloc(__func__, flow.offset, 0);
	if (!flow.ibv_attr) {
		rte_flow_error_set(error, ENOMEM, RTE_FLOW_ERROR_TYPE_HANDLE,
				   NULL, "cannot allocate ibv_attr memory");
		return NULL;
	}
	flow.offset = sizeof(struct ibv_flow_attr);
	*flow.ibv_attr = (struct ibv_flow_attr){
		.comp_mask = 0,
		.type = IBV_FLOW_ATTR_NORMAL,
		.size = sizeof(struct ibv_flow_attr),
		.priority = attr->priority,
		.num_of_specs = 0,
		.port = priv->port,
		.flags = 0,
	};
	claim_zero(mlx4_flow_prepare(priv, attr, pattern, actions,
				     error, &flow));
	target = (struct mlx4_flow_target){
		.queue = 0,
		.drop = 0,
	};
	for (action = actions;
	     action->type != RTE_FLOW_ACTION_TYPE_END;
	     ++action) {
		if (action->type == RTE_FLOW_ACTION_TYPE_VOID) {
			continue;
		} else if (action->type == RTE_FLOW_ACTION_TYPE_QUEUE) {
			target.queue = 1;
			target.queue_id =
				((const struct rte_flow_action_queue *)
				 action->conf)->index;
		} else if (action->type == RTE_FLOW_ACTION_TYPE_DROP) {
			target.drop = 1;
		} else {
			rte_flow_error_set(error, ENOTSUP,
					   RTE_FLOW_ERROR_TYPE_ACTION,
					   action, "unsupported action");
			goto exit;
		}
	}
	rte_flow = mlx4_flow_create_target_queue(priv, flow.ibv_attr,
						 &target, error);
	if (rte_flow) {
		LIST_INSERT_HEAD(&priv->flows, rte_flow, next);
		DEBUG("Flow created %p", (void *)rte_flow);
		return rte_flow;
	}
exit:
	rte_free(flow.ibv_attr);
	return NULL;
}

/**
 * Configure isolated mode.
 *
 * @see rte_flow_isolate()
 * @see rte_flow_ops
 */
static int
mlx4_flow_isolate(struct rte_eth_dev *dev,
		  int enable,
		  struct rte_flow_error *error)
{
	struct priv *priv = dev->data->dev_private;

	if (!!enable == !!priv->isolated)
		return 0;
	priv->isolated = !!enable;
	if (enable) {
		mlx4_mac_addr_del(priv);
	} else if (mlx4_mac_addr_add(priv) < 0) {
		priv->isolated = 1;
		return rte_flow_error_set(error, rte_errno,
					  RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
					  NULL, "cannot leave isolated mode");
	}
	return 0;
}

/**
 * Destroy a flow.
 *
 * @see rte_flow_destroy()
 * @see rte_flow_ops
 */
static int
mlx4_flow_destroy(struct rte_eth_dev *dev,
		  struct rte_flow *flow,
		  struct rte_flow_error *error)
{
	(void)dev;
	(void)error;
	LIST_REMOVE(flow, next);
	if (flow->ibv_flow)
		claim_zero(ibv_destroy_flow(flow->ibv_flow));
	rte_free(flow->ibv_attr);
	DEBUG("Flow destroyed %p", (void *)flow);
	rte_free(flow);
	return 0;
}

/**
 * Destroy all flows.
 *
 * @see rte_flow_flush()
 * @see rte_flow_ops
 */
static int
mlx4_flow_flush(struct rte_eth_dev *dev,
		struct rte_flow_error *error)
{
	struct priv *priv = dev->data->dev_private;

	while (!LIST_EMPTY(&priv->flows)) {
		struct rte_flow *flow;

		flow = LIST_FIRST(&priv->flows);
		mlx4_flow_destroy(dev, flow, error);
	}
	return 0;
}

/**
 * Remove all flows.
 *
 * Called by dev_stop() to remove all flows.
 *
 * @param priv
 *   Pointer to private structure.
 */
void
mlx4_flow_stop(struct priv *priv)
{
	struct rte_flow *flow;

	for (flow = LIST_FIRST(&priv->flows);
	     flow;
	     flow = LIST_NEXT(flow, next)) {
		claim_zero(ibv_destroy_flow(flow->ibv_flow));
		flow->ibv_flow = NULL;
		DEBUG("Flow %p removed", (void *)flow);
	}
	mlx4_flow_destroy_drop_queue(priv);
}

/**
 * Add all flows.
 *
 * @param priv
 *   Pointer to private structure.
 *
 * @return
 *   0 on success, a errno value otherwise and rte_errno is set.
 */
int
mlx4_flow_start(struct priv *priv)
{
	int ret;
	struct ibv_qp *qp;
	struct rte_flow *flow;

	ret = mlx4_flow_create_drop_queue(priv);
	if (ret)
		return -1;
	for (flow = LIST_FIRST(&priv->flows);
	     flow;
	     flow = LIST_NEXT(flow, next)) {
		qp = flow->qp ? flow->qp : priv->flow_drop_queue->qp;
		flow->ibv_flow = ibv_create_flow(qp, flow->ibv_attr);
		if (!flow->ibv_flow) {
			DEBUG("Flow %p cannot be applied", (void *)flow);
			rte_errno = EINVAL;
			return rte_errno;
		}
		DEBUG("Flow %p applied", (void *)flow);
	}
	return 0;
}

static const struct rte_flow_ops mlx4_flow_ops = {
	.validate = mlx4_flow_validate,
	.create = mlx4_flow_create,
	.destroy = mlx4_flow_destroy,
	.flush = mlx4_flow_flush,
	.isolate = mlx4_flow_isolate,
};

/**
 * Manage filter operations.
 *
 * @param dev
 *   Pointer to Ethernet device structure.
 * @param filter_type
 *   Filter type.
 * @param filter_op
 *   Operation to perform.
 * @param arg
 *   Pointer to operation-specific structure.
 *
 * @return
 *   0 on success, negative errno value otherwise and rte_errno is set.
 */
int
mlx4_filter_ctrl(struct rte_eth_dev *dev,
		 enum rte_filter_type filter_type,
		 enum rte_filter_op filter_op,
		 void *arg)
{
	switch (filter_type) {
	case RTE_ETH_FILTER_GENERIC:
		if (filter_op != RTE_ETH_FILTER_GET)
			break;
		*(const void **)arg = &mlx4_flow_ops;
		return 0;
	default:
		ERROR("%p: filter type (%d) not supported",
		      (void *)dev, filter_type);
		break;
	}
	rte_errno = ENOTSUP;
	return -rte_errno;
}
