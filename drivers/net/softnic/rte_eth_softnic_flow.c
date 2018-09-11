/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */

#include "rte_eth_softnic_internals.h"
#include "rte_eth_softnic.h"

#define rte_ntohs rte_be_to_cpu_16
#define rte_ntohl rte_be_to_cpu_32

int
flow_attr_map_set(struct pmd_internals *softnic,
		uint32_t group_id,
		int ingress,
		const char *pipeline_name,
		uint32_t table_id)
{
	struct pipeline *pipeline;
	struct flow_attr_map *map;

	if (group_id >= SOFTNIC_FLOW_MAX_GROUPS ||
			pipeline_name == NULL)
		return -1;

	pipeline = softnic_pipeline_find(softnic, pipeline_name);
	if (pipeline == NULL ||
			table_id >= pipeline->n_tables)
		return -1;

	map = (ingress) ? &softnic->flow.ingress_map[group_id] :
		&softnic->flow.egress_map[group_id];
	strcpy(map->pipeline_name, pipeline_name);
	map->table_id = table_id;
	map->valid = 1;

	return 0;
}

struct flow_attr_map *
flow_attr_map_get(struct pmd_internals *softnic,
		uint32_t group_id,
		int ingress)
{
	if (group_id >= SOFTNIC_FLOW_MAX_GROUPS)
		return NULL;

	return (ingress) ? &softnic->flow.ingress_map[group_id] :
		&softnic->flow.egress_map[group_id];
}

static int
flow_pipeline_table_get(struct pmd_internals *softnic,
		const struct rte_flow_attr *attr,
		const char **pipeline_name,
		uint32_t *table_id,
		struct rte_flow_error *error)
{
	struct flow_attr_map *map;

	if (attr == NULL)
		return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ATTR,
				NULL,
				"Null attr");

	if (!attr->ingress && !attr->egress)
		return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ATTR_INGRESS,
				attr,
				"Ingress/egress not specified");

	if (attr->ingress && attr->egress)
		return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ATTR_INGRESS,
				attr,
				"Setting both ingress and egress is not allowed");

	map = flow_attr_map_get(softnic,
			attr->group,
			attr->ingress);
	if (map == NULL ||
			map->valid == 0)
		return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ATTR_GROUP,
				attr,
				"Invalid group ID");

	if (pipeline_name)
		*pipeline_name = map->pipeline_name;

	if (table_id)
		*table_id = map->table_id;

	return 0;
}

union flow_item {
	uint8_t raw[TABLE_RULE_MATCH_SIZE_MAX];
	struct rte_flow_item_eth eth;
	struct rte_flow_item_vlan vlan;
	struct rte_flow_item_ipv4 ipv4;
	struct rte_flow_item_ipv6 ipv6;
	struct rte_flow_item_icmp icmp;
	struct rte_flow_item_udp udp;
	struct rte_flow_item_tcp tcp;
	struct rte_flow_item_sctp sctp;
	struct rte_flow_item_vxlan vxlan;
	struct rte_flow_item_e_tag e_tag;
	struct rte_flow_item_nvgre nvgre;
	struct rte_flow_item_mpls mpls;
	struct rte_flow_item_gre gre;
	struct rte_flow_item_gtp gtp;
	struct rte_flow_item_esp esp;
	struct rte_flow_item_geneve geneve;
	struct rte_flow_item_vxlan_gpe vxlan_gpe;
	struct rte_flow_item_arp_eth_ipv4 arp_eth_ipv4;
	struct rte_flow_item_ipv6_ext ipv6_ext;
	struct rte_flow_item_icmp6 icmp6;
	struct rte_flow_item_icmp6_nd_ns icmp6_nd_ns;
	struct rte_flow_item_icmp6_nd_na icmp6_nd_na;
	struct rte_flow_item_icmp6_nd_opt icmp6_nd_opt;
	struct rte_flow_item_icmp6_nd_opt_sla_eth icmp6_nd_opt_sla_eth;
	struct rte_flow_item_icmp6_nd_opt_tla_eth icmp6_nd_opt_tla_eth;
};

static const union flow_item flow_item_raw_mask;

static int
flow_item_is_proto(enum rte_flow_item_type type,
	const void **mask,
	size_t *size)
{
	switch (type) {
	case RTE_FLOW_ITEM_TYPE_RAW:
		*mask = &flow_item_raw_mask;
		*size = sizeof(flow_item_raw_mask);
		return 1; /* TRUE */

	case RTE_FLOW_ITEM_TYPE_ETH:
		*mask = &rte_flow_item_eth_mask;
		*size = sizeof(struct rte_flow_item_eth);
		return 1; /* TRUE */

	case RTE_FLOW_ITEM_TYPE_VLAN:
		*mask = &rte_flow_item_vlan_mask;
		*size = sizeof(struct rte_flow_item_vlan);
		return 1;

	case RTE_FLOW_ITEM_TYPE_IPV4:
		*mask = &rte_flow_item_ipv4_mask;
		*size = sizeof(struct rte_flow_item_ipv4);
		return 1;

	case RTE_FLOW_ITEM_TYPE_IPV6:
		*mask = &rte_flow_item_ipv6_mask;
		*size = sizeof(struct rte_flow_item_ipv6);
		return 1;

	case RTE_FLOW_ITEM_TYPE_ICMP:
		*mask = &rte_flow_item_icmp_mask;
		*size = sizeof(struct rte_flow_item_icmp);
		return 1;

	case RTE_FLOW_ITEM_TYPE_UDP:
		*mask = &rte_flow_item_udp_mask;
		*size = sizeof(struct rte_flow_item_udp);
		return 1;

	case RTE_FLOW_ITEM_TYPE_TCP:
		*mask = &rte_flow_item_tcp_mask;
		*size = sizeof(struct rte_flow_item_tcp);
		return 1;

	case RTE_FLOW_ITEM_TYPE_SCTP:
		*mask = &rte_flow_item_sctp_mask;
		*size = sizeof(struct rte_flow_item_sctp);
		return 1;

	case RTE_FLOW_ITEM_TYPE_VXLAN:
		*mask = &rte_flow_item_vxlan_mask;
		*size = sizeof(struct rte_flow_item_vxlan);
		return 1;

	case RTE_FLOW_ITEM_TYPE_E_TAG:
		*mask = &rte_flow_item_e_tag_mask;
		*size = sizeof(struct rte_flow_item_e_tag);
		return 1;

	case RTE_FLOW_ITEM_TYPE_NVGRE:
		*mask = &rte_flow_item_nvgre_mask;
		*size = sizeof(struct rte_flow_item_nvgre);
		return 1;

	case RTE_FLOW_ITEM_TYPE_MPLS:
		*mask = &rte_flow_item_mpls_mask;
		*size = sizeof(struct rte_flow_item_mpls);
		return 1;

	case RTE_FLOW_ITEM_TYPE_GRE:
		*mask = &rte_flow_item_gre_mask;
		*size = sizeof(struct rte_flow_item_gre);
		return 1;

	case RTE_FLOW_ITEM_TYPE_GTP:
	case RTE_FLOW_ITEM_TYPE_GTPC:
	case RTE_FLOW_ITEM_TYPE_GTPU:
		*mask = &rte_flow_item_gtp_mask;
		*size = sizeof(struct rte_flow_item_gtp);
		return 1;

	case RTE_FLOW_ITEM_TYPE_ESP:
		*mask = &rte_flow_item_esp_mask;
		*size = sizeof(struct rte_flow_item_esp);
		return 1;

	case RTE_FLOW_ITEM_TYPE_GENEVE:
		*mask = &rte_flow_item_geneve_mask;
		*size = sizeof(struct rte_flow_item_geneve);
		return 1;

	case RTE_FLOW_ITEM_TYPE_VXLAN_GPE:
		*mask = &rte_flow_item_vxlan_gpe_mask;
		*size = sizeof(struct rte_flow_item_vxlan_gpe);
		return 1;

	case RTE_FLOW_ITEM_TYPE_ARP_ETH_IPV4:
		*mask = &rte_flow_item_arp_eth_ipv4_mask;
		*size = sizeof(struct rte_flow_item_arp_eth_ipv4);
		return 1;

	case RTE_FLOW_ITEM_TYPE_IPV6_EXT:
		*mask = &rte_flow_item_ipv6_ext_mask;
		*size = sizeof(struct rte_flow_item_ipv6_ext);
		return 1;

	case RTE_FLOW_ITEM_TYPE_ICMP6:
		*mask = &rte_flow_item_icmp6_mask;
		*size = sizeof(struct rte_flow_item_icmp6);
		return 1;

	case RTE_FLOW_ITEM_TYPE_ICMP6_ND_NS:
		*mask = &rte_flow_item_icmp6_nd_ns_mask;
		*size = sizeof(struct rte_flow_item_icmp6_nd_ns);
		return 1;

	case RTE_FLOW_ITEM_TYPE_ICMP6_ND_NA:
		*mask = &rte_flow_item_icmp6_nd_na_mask;
		*size = sizeof(struct rte_flow_item_icmp6_nd_na);
		return 1;

	case RTE_FLOW_ITEM_TYPE_ICMP6_ND_OPT:
		*mask = &rte_flow_item_icmp6_nd_opt_mask;
		*size = sizeof(struct rte_flow_item_icmp6_nd_opt);
		return 1;

	case RTE_FLOW_ITEM_TYPE_ICMP6_ND_OPT_SLA_ETH:
		*mask = &rte_flow_item_icmp6_nd_opt_sla_eth_mask;
		*size = sizeof(struct rte_flow_item_icmp6_nd_opt_sla_eth);
		return 1;

	case RTE_FLOW_ITEM_TYPE_ICMP6_ND_OPT_TLA_ETH:
		*mask = &rte_flow_item_icmp6_nd_opt_tla_eth_mask;
		*size = sizeof(struct rte_flow_item_icmp6_nd_opt_tla_eth);
		return 1;

	default: return 0; /* FALSE */
	}
}

static int
flow_item_proto_preprocess(const struct rte_flow_item *item,
	union flow_item *item_spec,
	union flow_item *item_mask,
	size_t *item_size,
	int *item_disabled,
	struct rte_flow_error *error)
{
	const void *mask_default;
	uint8_t *spec = (uint8_t *)item_spec;
	uint8_t *mask = (uint8_t *)item_mask;
	size_t size, i;

	if (!flow_item_is_proto(item->type, &mask_default, &size))
		return rte_flow_error_set(error,
			ENOTSUP,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item,
			"Item type not supported");

	/* spec */
	if (!item->spec) {
		/* If spec is NULL, then last and mask also have to be NULL. */
		if (item->last || item->mask)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"Invalid item (NULL spec with non-NULL last or mask)");

		memset(item_spec, 0, size);
		memset(item_mask, 0, size);
		*item_size = size;
		*item_disabled = 1; /* TRUE */
		return 0;
	}

	memcpy(spec, item->spec, size);
	*item_size = size;

	/* mask */
	if (item->mask)
		memcpy(mask, item->mask, size);
	else
		memcpy(mask, mask_default, size);

	/* disabled */
	for (i = 0; i < size; i++)
		if (mask[i])
			break;
	*item_disabled = (i == size) ? 1 : 0;

	/* Apply mask over spec. */
	for (i = 0; i < size; i++)
		spec[i] &= mask[i];

	/* last */
	if (item->last) {
		uint8_t last[size];

		/* init last */
		memcpy(last, item->last, size);
		for (i = 0; i < size; i++)
			last[i] &= mask[i];

		/* check for range */
		for (i = 0; i < size; i++)
			if (last[i] != spec[i])
				return rte_flow_error_set(error,
					ENOTSUP,
					RTE_FLOW_ERROR_TYPE_ITEM,
					item,
					"Range not supported");
	}

	return 0;
}

/***
 * Skip disabled protocol items and VOID items
 * until any of the mutually exclusive conditions
 * from the list below takes place:
 *    (A) A protocol present in the proto_mask
 *        is met (either ENABLED or DISABLED);
 *    (B) A protocol NOT present in the proto_mask is met in ENABLED state;
 *    (C) The END item is met.
 */
static int
flow_item_skip_disabled_protos(const struct rte_flow_item **item,
	uint64_t proto_mask,
	size_t *length,
	struct rte_flow_error *error)
{
	size_t len = 0;

	for ( ; (*item)->type != RTE_FLOW_ITEM_TYPE_END; (*item)++) {
		union flow_item spec, mask;
		size_t size;
		int disabled = 0, status;

		if ((*item)->type == RTE_FLOW_ITEM_TYPE_VOID)
			continue;

		status = flow_item_proto_preprocess(*item,
				&spec,
				&mask,
				&size,
				&disabled,
				error);
		if (status)
			return status;

		if ((proto_mask & (1LLU << (*item)->type)) ||
				!disabled)
			break;

		len += size;
	}

	if (length)
		*length = len;

	return 0;
}

#define FLOW_ITEM_PROTO_IP \
	((1LLU << RTE_FLOW_ITEM_TYPE_IPV4) | \
	 (1LLU << RTE_FLOW_ITEM_TYPE_IPV6))

static void
flow_item_skip_void(const struct rte_flow_item **item)
{
	for ( ; ; (*item)++)
		if ((*item)->type != RTE_FLOW_ITEM_TYPE_VOID)
			return;
}

#define IP_PROTOCOL_TCP 0x06
#define IP_PROTOCOL_UDP 0x11
#define IP_PROTOCOL_SCTP 0x84

static int
mask_to_depth(uint64_t mask,
		uint32_t *depth)
{
	uint64_t n;

	if (mask == UINT64_MAX) {
		if (depth)
			*depth = 64;

		return 0;
	}

	mask = ~mask;

	if (mask & (mask + 1))
		return -1;

	n = __builtin_popcountll(mask);
	if (depth)
		*depth = (uint32_t)(64 - n);

	return 0;
}

static int
ipv4_mask_to_depth(uint32_t mask,
		uint32_t *depth)
{
	uint32_t d;
	int status;

	status = mask_to_depth(mask | (UINT64_MAX << 32), &d);
	if (status)
		return status;

	d -= 32;
	if (depth)
		*depth = d;

	return 0;
}

static int
ipv6_mask_to_depth(uint8_t *mask,
	uint32_t *depth)
{
	uint64_t *m = (uint64_t *)mask;
	uint64_t m0 = rte_be_to_cpu_64(m[0]);
	uint64_t m1 = rte_be_to_cpu_64(m[1]);
	uint32_t d0, d1;
	int status;

	status = mask_to_depth(m0, &d0);
	if (status)
		return status;

	status = mask_to_depth(m1, &d1);
	if (status)
		return status;

	if (d0 < 64 && d1)
		return -1;

	if (depth)
		*depth = d0 + d1;

	return 0;
}

static int
port_mask_to_range(uint16_t port,
	uint16_t port_mask,
	uint16_t *port0,
	uint16_t *port1)
{
	int status;
	uint16_t p0, p1;

	status = mask_to_depth(port_mask | (UINT64_MAX << 16), NULL);
	if (status)
		return -1;

	p0 = port & port_mask;
	p1 = p0 | ~port_mask;

	if (port0)
		*port0 = p0;

	if (port1)
		*port1 = p1;

	return 0;
}

static int
flow_rule_match_acl_get(struct pmd_internals *softnic __rte_unused,
		struct pipeline *pipeline __rte_unused,
		struct softnic_table *table __rte_unused,
		const struct rte_flow_attr *attr,
		const struct rte_flow_item *item,
		struct softnic_table_rule_match *rule_match,
		struct rte_flow_error *error)
{
	union flow_item spec, mask;
	size_t size, length = 0;
	int disabled = 0, status;
	uint8_t ip_proto, ip_proto_mask;

	memset(rule_match, 0, sizeof(*rule_match));
	rule_match->match_type = TABLE_ACL;
	rule_match->match.acl.priority = attr->priority;

	/* VOID or disabled protos only, if any. */
	status = flow_item_skip_disabled_protos(&item,
			FLOW_ITEM_PROTO_IP, &length, error);
	if (status)
		return status;

	/* IP only. */
	status = flow_item_proto_preprocess(item, &spec, &mask,
			&size, &disabled, error);
	if (status)
		return status;

	switch (item->type) {
	case RTE_FLOW_ITEM_TYPE_IPV4:
	{
		uint32_t sa_depth, da_depth;

		status = ipv4_mask_to_depth(rte_ntohl(mask.ipv4.hdr.src_addr),
				&sa_depth);
		if (status)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Illegal IPv4 header source address mask");

		status = ipv4_mask_to_depth(rte_ntohl(mask.ipv4.hdr.dst_addr),
				&da_depth);
		if (status)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Illegal IPv4 header destination address mask");

		ip_proto = spec.ipv4.hdr.next_proto_id;
		ip_proto_mask = mask.ipv4.hdr.next_proto_id;

		rule_match->match.acl.ip_version = 1;
		rule_match->match.acl.ipv4.sa =
			rte_ntohl(spec.ipv4.hdr.src_addr);
		rule_match->match.acl.ipv4.da =
			rte_ntohl(spec.ipv4.hdr.dst_addr);
		rule_match->match.acl.sa_depth = sa_depth;
		rule_match->match.acl.da_depth = da_depth;
		rule_match->match.acl.proto = ip_proto;
		rule_match->match.acl.proto_mask = ip_proto_mask;
		break;
	} /* RTE_FLOW_ITEM_TYPE_IPV4 */

	case RTE_FLOW_ITEM_TYPE_IPV6:
	{
		uint32_t sa_depth, da_depth;

		status = ipv6_mask_to_depth(mask.ipv6.hdr.src_addr, &sa_depth);
		if (status)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Illegal IPv6 header source address mask");

		status = ipv6_mask_to_depth(mask.ipv6.hdr.dst_addr, &da_depth);
		if (status)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Illegal IPv6 header destination address mask");

		ip_proto = spec.ipv6.hdr.proto;
		ip_proto_mask = mask.ipv6.hdr.proto;

		rule_match->match.acl.ip_version = 0;
		memcpy(rule_match->match.acl.ipv6.sa,
			spec.ipv6.hdr.src_addr,
			sizeof(spec.ipv6.hdr.src_addr));
		memcpy(rule_match->match.acl.ipv6.da,
			spec.ipv6.hdr.dst_addr,
			sizeof(spec.ipv6.hdr.dst_addr));
		rule_match->match.acl.sa_depth = sa_depth;
		rule_match->match.acl.da_depth = da_depth;
		rule_match->match.acl.proto = ip_proto;
		rule_match->match.acl.proto_mask = ip_proto_mask;
		break;
	} /* RTE_FLOW_ITEM_TYPE_IPV6 */

	default:
		return rte_flow_error_set(error,
			ENOTSUP,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item,
			"ACL: IP protocol required");
	} /* switch */

	if (ip_proto_mask != UINT8_MAX)
		return rte_flow_error_set(error,
			EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item,
			"ACL: Illegal IP protocol mask");

	item++;

	/* VOID only, if any. */
	flow_item_skip_void(&item);

	/* TCP/UDP/SCTP only. */
	status = flow_item_proto_preprocess(item, &spec, &mask,
			&size, &disabled, error);
	if (status)
		return status;

	switch (item->type) {
	case RTE_FLOW_ITEM_TYPE_TCP:
	{
		uint16_t sp0, sp1, dp0, dp1;

		if (ip_proto != IP_PROTOCOL_TCP)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Item type is TCP, but IP protocol is not");

		status = port_mask_to_range(rte_ntohs(spec.tcp.hdr.src_port),
				rte_ntohs(mask.tcp.hdr.src_port),
				&sp0,
				&sp1);

		if (status)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Illegal TCP source port mask");

		status = port_mask_to_range(rte_ntohs(spec.tcp.hdr.dst_port),
				rte_ntohs(mask.tcp.hdr.dst_port),
				&dp0,
				&dp1);

		if (status)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Illegal TCP destination port mask");

		rule_match->match.acl.sp0 = sp0;
		rule_match->match.acl.sp1 = sp1;
		rule_match->match.acl.dp0 = dp0;
		rule_match->match.acl.dp1 = dp1;

		break;
	} /* RTE_FLOW_ITEM_TYPE_TCP */

	case RTE_FLOW_ITEM_TYPE_UDP:
	{
		uint16_t sp0, sp1, dp0, dp1;

		if (ip_proto != IP_PROTOCOL_UDP)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Item type is UDP, but IP protocol is not");

		status = port_mask_to_range(rte_ntohs(spec.udp.hdr.src_port),
			rte_ntohs(mask.udp.hdr.src_port),
			&sp0,
			&sp1);
		if (status)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Illegal UDP source port mask");

		status = port_mask_to_range(rte_ntohs(spec.udp.hdr.dst_port),
			rte_ntohs(mask.udp.hdr.dst_port),
			&dp0,
			&dp1);
		if (status)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Illegal UDP destination port mask");

		rule_match->match.acl.sp0 = sp0;
		rule_match->match.acl.sp1 = sp1;
		rule_match->match.acl.dp0 = dp0;
		rule_match->match.acl.dp1 = dp1;

		break;
	} /* RTE_FLOW_ITEM_TYPE_UDP */

	case RTE_FLOW_ITEM_TYPE_SCTP:
	{
		uint16_t sp0, sp1, dp0, dp1;

		if (ip_proto != IP_PROTOCOL_SCTP)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Item type is SCTP, but IP protocol is not");

		status = port_mask_to_range(rte_ntohs(spec.sctp.hdr.src_port),
			rte_ntohs(mask.sctp.hdr.src_port),
			&sp0,
			&sp1);

		if (status)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Illegal SCTP source port mask");

		status = port_mask_to_range(rte_ntohs(spec.sctp.hdr.dst_port),
			rte_ntohs(mask.sctp.hdr.dst_port),
			&dp0,
			&dp1);
		if (status)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"ACL: Illegal SCTP destination port mask");

		rule_match->match.acl.sp0 = sp0;
		rule_match->match.acl.sp1 = sp1;
		rule_match->match.acl.dp0 = dp0;
		rule_match->match.acl.dp1 = dp1;

		break;
	} /* RTE_FLOW_ITEM_TYPE_SCTP */

	default:
		return rte_flow_error_set(error,
			ENOTSUP,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item,
			"ACL: TCP/UDP/SCTP required");
	} /* switch */

	item++;

	/* VOID or disabled protos only, if any. */
	status = flow_item_skip_disabled_protos(&item, 0, NULL, error);
	if (status)
		return status;

	/* END only. */
	if (item->type != RTE_FLOW_ITEM_TYPE_END)
		return rte_flow_error_set(error,
			EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item,
			"ACL: Expecting END item");

	return 0;
}

/***
 * Both *tmask* and *fmask* are byte arrays of size *tsize* and *fsize*
 * respectively.
 * They are located within a larger buffer at offsets *toffset* and *foffset*
 * respectivelly. Both *tmask* and *fmask* represent bitmasks for the larger
 * buffer.
 * Question: are the two masks equivalent?
 *
 * Notes:
 * 1. Offset basically indicates that the first offset bytes in the buffer
 *    are "don't care", so offset is equivalent to pre-pending an "all-zeros"
 *    array of *offset* bytes to the *mask*.
 * 2. Each *mask* might contain a number of zero bytes at the beginning or
 *    at the end.
 * 3. Bytes in the larger buffer after the end of the *mask* are also considered
 *    "don't care", so they are equivalent to appending an "all-zeros" array of
 *    bytes to the *mask*.
 *
 * Example:
 * Buffer = [xx xx xx xx xx xx xx xx], buffer size = 8 bytes
 * tmask = [00 22 00 33 00], toffset = 2, tsize = 5
 *    => buffer mask = [00 00 00 22 00 33 00 00]
 * fmask = [22 00 33], foffset = 3, fsize = 3 =>
 *    => buffer mask = [00 00 00 22 00 33 00 00]
 * Therefore, the tmask and fmask from this example are equivalent.
 */
static int
hash_key_mask_is_same(uint8_t *tmask,
	size_t toffset,
	size_t tsize,
	uint8_t *fmask,
	size_t foffset,
	size_t fsize,
	size_t *toffset_plus,
	size_t *foffset_plus)
{
	size_t tpos; /* Position of first non-zero byte in the tmask buffer. */
	size_t fpos; /* Position of first non-zero byte in the fmask buffer. */

	/* Compute tpos and fpos. */
	for (tpos = 0; tmask[tpos] == 0; tpos++)
		;
	for (fpos = 0; fmask[fpos] == 0; fpos++)
		;

	if (toffset + tpos != foffset + fpos)
		return 0; /* FALSE */

	tsize -= tpos;
	fsize -= fpos;

	if (tsize < fsize) {
		size_t i;

		for (i = 0; i < tsize; i++)
			if (tmask[tpos + i] != fmask[fpos + i])
				return 0; /* FALSE */

		for ( ; i < fsize; i++)
			if (fmask[fpos + i])
				return 0; /* FALSE */
	} else {
		size_t i;

		for (i = 0; i < fsize; i++)
			if (tmask[tpos + i] != fmask[fpos + i])
				return 0; /* FALSE */

		for ( ; i < tsize; i++)
			if (tmask[tpos + i])
				return 0; /* FALSE */
	}

	if (toffset_plus)
		*toffset_plus = tpos;

	if (foffset_plus)
		*foffset_plus = fpos;

	return 1; /* TRUE */
}

static int
flow_rule_match_hash_get(struct pmd_internals *softnic __rte_unused,
	struct pipeline *pipeline __rte_unused,
	struct softnic_table *table,
	const struct rte_flow_attr *attr __rte_unused,
	const struct rte_flow_item *item,
	struct softnic_table_rule_match *rule_match,
	struct rte_flow_error *error)
{
	struct softnic_table_rule_match_hash key, key_mask;
	struct softnic_table_hash_params *params = &table->params.match.hash;
	size_t offset = 0, length = 0, tpos, fpos;
	int status;

	memset(&key, 0, sizeof(key));
	memset(&key_mask, 0, sizeof(key_mask));

	/* VOID or disabled protos only, if any. */
	status = flow_item_skip_disabled_protos(&item, 0, &offset, error);
	if (status)
		return status;

	if (item->type == RTE_FLOW_ITEM_TYPE_END)
		return rte_flow_error_set(error,
			EINVAL,
			RTE_FLOW_ERROR_TYPE_ITEM,
			item,
			"HASH: END detected too early");

	/* VOID or any protocols (enabled or disabled). */
	for ( ; item->type != RTE_FLOW_ITEM_TYPE_END; item++) {
		union flow_item spec, mask;
		size_t size;
		int disabled, status;

		if (item->type == RTE_FLOW_ITEM_TYPE_VOID)
			continue;

		status = flow_item_proto_preprocess(item,
			&spec,
			&mask,
			&size,
			&disabled,
			error);
		if (status)
			return status;

		if (length + size > sizeof(key)) {
			if (disabled)
				break;

			return rte_flow_error_set(error,
				ENOTSUP,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"HASH: Item too big");
		}

		memcpy(&key.key[length], &spec, size);
		memcpy(&key_mask.key[length], &mask, size);
		length += size;
	}

	if (item->type != RTE_FLOW_ITEM_TYPE_END) {
		/* VOID or disabled protos only, if any. */
		status = flow_item_skip_disabled_protos(&item, 0, NULL, error);
		if (status)
			return status;

		/* END only. */
		if (item->type != RTE_FLOW_ITEM_TYPE_END)
			return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				item,
				"HASH: Expecting END item");
	}

	/* Compare flow key mask against table key mask. */
	offset += sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM;

	if (!hash_key_mask_is_same(params->key_mask,
		params->key_offset,
		params->key_size,
		key_mask.key,
		offset,
		length,
		&tpos,
		&fpos))
		return rte_flow_error_set(error,
			EINVAL,
			RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
			NULL,
			"HASH: Item list is not observing the match format");

	/* Rule match. */
	memset(rule_match, 0, sizeof(*rule_match));
	rule_match->match_type = TABLE_HASH;
	memcpy(&rule_match->match.hash.key[tpos],
		&key.key[fpos],
		RTE_MIN(sizeof(rule_match->match.hash.key) - tpos,
			length - fpos));

	return 0;
}

static int
flow_rule_match_get(struct pmd_internals *softnic,
		struct pipeline *pipeline,
		struct softnic_table *table,
		const struct rte_flow_attr *attr,
		const struct rte_flow_item *item,
		struct softnic_table_rule_match *rule_match,
		struct rte_flow_error *error)
{
	switch (table->params.match_type) {
	case TABLE_ACL:
		return flow_rule_match_acl_get(softnic,
			pipeline,
			table,
			attr,
			item,
			rule_match,
			error);

		/* FALLTHROUGH */

	case TABLE_HASH:
		return flow_rule_match_hash_get(softnic,
			pipeline,
			table,
			attr,
			item,
			rule_match,
			error);

		/* FALLTHROUGH */

	default:
		return rte_flow_error_set(error,
			ENOTSUP,
			RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
			NULL,
			"Unsupported pipeline table match type");
	}
}

static int
flow_rule_action_get(struct pmd_internals *softnic,
	struct pipeline *pipeline,
	struct softnic_table *table,
	const struct rte_flow_attr *attr,
	const struct rte_flow_action *action,
	struct softnic_table_rule_action *rule_action,
	struct rte_flow_error *error __rte_unused)
{
	struct softnic_table_action_profile *profile;
	struct softnic_table_action_profile_params *params;
	int n_jump_queue_rss_drop = 0;
	int n_count = 0;

	profile = softnic_table_action_profile_find(softnic,
		table->params.action_profile_name);
	if (profile == NULL)
		return rte_flow_error_set(error,
			EINVAL,
			RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
			action,
			"JUMP: Table action profile");

	params = &profile->params;

	for ( ; action->type != RTE_FLOW_ACTION_TYPE_END; action++) {
		if (action->type == RTE_FLOW_ACTION_TYPE_VOID)
			continue;

		switch (action->type) {
		case RTE_FLOW_ACTION_TYPE_JUMP:
		{
			const struct rte_flow_action_jump *conf = action->conf;
			struct flow_attr_map *map;

			if (conf == NULL)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"JUMP: Null configuration");

			if (n_jump_queue_rss_drop)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"Only one termination action is"
					" allowed per flow");

			if ((params->action_mask &
				(1LLU << RTE_TABLE_ACTION_FWD)) == 0)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
					NULL,
					"JUMP action not enabled for this table");

			n_jump_queue_rss_drop = 1;

			map = flow_attr_map_get(softnic,
				conf->group,
				attr->ingress);
			if (map == NULL || map->valid == 0)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
					NULL,
					"JUMP: Invalid group mapping");

			if (strcmp(pipeline->name, map->pipeline_name) != 0)
				return rte_flow_error_set(error,
					ENOTSUP,
					RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
					NULL,
					"JUMP: Jump to table in different pipeline");

			/* RTE_TABLE_ACTION_FWD */
			rule_action->fwd.action = RTE_PIPELINE_ACTION_TABLE;
			rule_action->fwd.id = map->table_id;
			rule_action->action_mask |= 1 << RTE_TABLE_ACTION_FWD;
			break;
		} /* RTE_FLOW_ACTION_TYPE_JUMP */

		case RTE_FLOW_ACTION_TYPE_QUEUE:
		{
			char name[NAME_SIZE];
			struct rte_eth_dev *dev;
			const struct rte_flow_action_queue *conf = action->conf;
			uint32_t port_id;
			int status;

			if (conf == NULL)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"QUEUE: Null configuration");

			if (n_jump_queue_rss_drop)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"Only one termination action is allowed"
					" per flow");

			if ((params->action_mask &
				(1LLU << RTE_TABLE_ACTION_FWD)) == 0)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
					NULL,
					"QUEUE action not enabled for this table");

			n_jump_queue_rss_drop = 1;

			dev = ETHDEV(softnic);
			if (dev == NULL ||
				conf->index >= dev->data->nb_rx_queues)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"QUEUE: Invalid RX queue ID");

			sprintf(name, "RXQ%u", (uint32_t)conf->index);

			status = softnic_pipeline_port_out_find(softnic,
				pipeline->name,
				name,
				&port_id);
			if (status)
				return rte_flow_error_set(error,
					ENOTSUP,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"QUEUE: RX queue not accessible from this pipeline");

			/* RTE_TABLE_ACTION_FWD */
			rule_action->fwd.action = RTE_PIPELINE_ACTION_PORT;
			rule_action->fwd.id = port_id;
			rule_action->action_mask |= 1 << RTE_TABLE_ACTION_FWD;
			break;
		} /*RTE_FLOW_ACTION_TYPE_QUEUE */

		case RTE_FLOW_ACTION_TYPE_RSS:
		{
			const struct rte_flow_action_rss *conf = action->conf;
			uint32_t i;

			if (conf == NULL)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"RSS: Null configuration");

			if (!rte_is_power_of_2(conf->queue_num))
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION_CONF,
					conf,
					"RSS: Number of queues must be a power of 2");

			if (conf->queue_num > RTE_DIM(rule_action->lb.out))
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION_CONF,
					conf,
					"RSS: Number of queues too big");

			if (n_jump_queue_rss_drop)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"Only one termination action is allowed per flow");

			if (((params->action_mask &
				(1LLU << RTE_TABLE_ACTION_FWD)) == 0) ||
				((params->action_mask &
				(1LLU << RTE_TABLE_ACTION_LB)) == 0))
				return rte_flow_error_set(error,
					ENOTSUP,
					RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
					NULL,
					"RSS action not supported by this table");

			if (params->lb.out_offset !=
				pipeline->params.offset_port_id)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
					NULL,
					"RSS action not supported by this pipeline");

			n_jump_queue_rss_drop = 1;

			/* RTE_TABLE_ACTION_LB */
			for (i = 0; i < conf->queue_num; i++) {
				char name[NAME_SIZE];
				struct rte_eth_dev *dev;
				uint32_t port_id;
				int status;

				dev = ETHDEV(softnic);
				if (dev == NULL ||
					conf->queue[i] >=
						dev->data->nb_rx_queues)
					return rte_flow_error_set(error,
						EINVAL,
						RTE_FLOW_ERROR_TYPE_ACTION,
						action,
						"RSS: Invalid RX queue ID");

				sprintf(name, "RXQ%u",
					(uint32_t)conf->queue[i]);

				status = softnic_pipeline_port_out_find(softnic,
					pipeline->name,
					name,
					&port_id);
				if (status)
					return rte_flow_error_set(error,
						ENOTSUP,
						RTE_FLOW_ERROR_TYPE_ACTION,
						action,
						"RSS: RX queue not accessible from this pipeline");

				rule_action->lb.out[i] = port_id;
			}

			for ( ; i < RTE_DIM(rule_action->lb.out); i++)
				rule_action->lb.out[i] =
				rule_action->lb.out[i % conf->queue_num];

			rule_action->action_mask |= 1 << RTE_TABLE_ACTION_LB;

			/* RTE_TABLE_ACTION_FWD */
			rule_action->fwd.action = RTE_PIPELINE_ACTION_PORT_META;
			rule_action->action_mask |= 1 << RTE_TABLE_ACTION_FWD;
			break;
		} /* RTE_FLOW_ACTION_TYPE_RSS */

		case RTE_FLOW_ACTION_TYPE_DROP:
		{
			const void *conf = action->conf;

			if (conf != NULL)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"DROP: No configuration required");

			if (n_jump_queue_rss_drop)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"Only one termination action is allowed per flow");
			if ((params->action_mask &
				(1LLU << RTE_TABLE_ACTION_FWD)) == 0)
				return rte_flow_error_set(error,
					ENOTSUP,
					RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
					NULL,
					"DROP action not supported by this table");

			n_jump_queue_rss_drop = 1;

			/* RTE_TABLE_ACTION_FWD */
			rule_action->fwd.action = RTE_PIPELINE_ACTION_DROP;
			rule_action->action_mask |= 1 << RTE_TABLE_ACTION_FWD;
			break;
		} /* RTE_FLOW_ACTION_TYPE_DROP */

		case RTE_FLOW_ACTION_TYPE_COUNT:
		{
			const struct rte_flow_action_count *conf = action->conf;

			if (conf == NULL)
				return rte_flow_error_set(error,
					EINVAL,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"COUNT: Null configuration");

			if (conf->shared)
				return rte_flow_error_set(error,
					ENOTSUP,
					RTE_FLOW_ERROR_TYPE_ACTION_CONF,
					conf,
					"COUNT: Shared counters not supported");

			if (n_count)
				return rte_flow_error_set(error,
					ENOTSUP,
					RTE_FLOW_ERROR_TYPE_ACTION,
					action,
					"Only one COUNT action per flow");

			if ((params->action_mask &
				(1LLU << RTE_TABLE_ACTION_STATS)) == 0)
				return rte_flow_error_set(error,
					ENOTSUP,
					RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
					NULL,
					"COUNT action not supported by this table");

			n_count = 1;

			/* RTE_TABLE_ACTION_STATS */
			rule_action->stats.n_packets = 0;
			rule_action->stats.n_bytes = 0;
			rule_action->action_mask |= 1 << RTE_TABLE_ACTION_STATS;
			break;
		} /* RTE_FLOW_ACTION_TYPE_COUNT */

		default:
			return -ENOTSUP;
		}
	}

	if (n_jump_queue_rss_drop == 0)
		return rte_flow_error_set(error,
			EINVAL,
			RTE_FLOW_ERROR_TYPE_ACTION,
			action,
			"Flow does not have any terminating action");

	return 0;
}

static int
pmd_flow_validate(struct rte_eth_dev *dev,
		const struct rte_flow_attr *attr,
		const struct rte_flow_item item[],
		const struct rte_flow_action action[],
		struct rte_flow_error *error)
{
	struct softnic_table_rule_match rule_match;
	struct softnic_table_rule_action rule_action;

	struct pmd_internals *softnic = dev->data->dev_private;
	struct pipeline *pipeline;
	struct softnic_table *table;
	const char *pipeline_name = NULL;
	uint32_t table_id = 0;
	int status;

	/* Check input parameters. */
	if (attr == NULL)
		return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ATTR,
				NULL, "Null attr");

	if (item == NULL)
		return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ITEM,
				NULL,
				"Null item");

	if (action == NULL)
		return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_ACTION,
				NULL,
				"Null action");

	/* Identify the pipeline table to add this flow to. */
	status = flow_pipeline_table_get(softnic, attr, &pipeline_name,
					&table_id, error);
	if (status)
		return status;

	pipeline = softnic_pipeline_find(softnic, pipeline_name);
	if (pipeline == NULL)
		return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
				NULL,
				"Invalid pipeline name");

	if (table_id >= pipeline->n_tables)
		return rte_flow_error_set(error,
				EINVAL,
				RTE_FLOW_ERROR_TYPE_UNSPECIFIED,
				NULL,
				"Invalid pipeline table ID");

	table = &pipeline->table[table_id];

	/* Rule match. */
	memset(&rule_match, 0, sizeof(rule_match));
	status = flow_rule_match_get(softnic,
			pipeline,
			table,
			attr,
			item,
			&rule_match,
			error);
	if (status)
		return status;

	/* Rule action. */
	memset(&rule_action, 0, sizeof(rule_action));
	status = flow_rule_action_get(softnic,
		pipeline,
		table,
		attr,
		action,
		&rule_action,
		error);
	if (status)
		return status;

	return 0;
}

const struct rte_flow_ops pmd_flow_ops = {
	.validate = pmd_flow_validate,
};
