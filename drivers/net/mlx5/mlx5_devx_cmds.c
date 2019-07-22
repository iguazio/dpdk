// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2018 Mellanox Technologies, Ltd */

#include <rte_flow_driver.h>
#include <rte_malloc.h>
#include <unistd.h>

#include "mlx5.h"
#include "mlx5_glue.h"
#include "mlx5_prm.h"

/**
 * Allocate flow counters via devx interface.
 *
 * @param[in] ctx
 *   ibv contexts returned from mlx5dv_open_device.
 * @param dcs
 *   Pointer to counters properties structure to be filled by the routine.
 * @param bulk_n_128
 *   Bulk counter numbers in 128 counters units.
 *
 * @return
 *   Pointer to counter object on success, a negative value otherwise and
 *   rte_errno is set.
 */
struct mlx5_devx_obj *
mlx5_devx_cmd_flow_counter_alloc(struct ibv_context *ctx, uint32_t bulk_n_128)
{
	struct mlx5_devx_obj *dcs = rte_zmalloc("dcs", sizeof(*dcs), 0);
	uint32_t in[MLX5_ST_SZ_DW(alloc_flow_counter_in)]   = {0};
	uint32_t out[MLX5_ST_SZ_DW(alloc_flow_counter_out)] = {0};

	if (!dcs) {
		rte_errno = ENOMEM;
		return NULL;
	}
	MLX5_SET(alloc_flow_counter_in, in, opcode,
		 MLX5_CMD_OP_ALLOC_FLOW_COUNTER);
	MLX5_SET(alloc_flow_counter_in, in, flow_counter_bulk, bulk_n_128);
	dcs->obj = mlx5_glue->devx_obj_create(ctx, in,
					      sizeof(in), out, sizeof(out));
	if (!dcs->obj) {
		DRV_LOG(ERR, "Can't allocate counters - error %d\n", errno);
		rte_errno = errno;
		rte_free(dcs);
		return NULL;
	}
	dcs->id = MLX5_GET(alloc_flow_counter_out, out, flow_counter_id);
	return dcs;
}

/**
 * Query flow counters values.
 *
 * @param[in] dcs
 *   devx object that was obtained from mlx5_devx_cmd_fc_alloc.
 * @param[in] clear
 *   Whether hardware should clear the counters after the query or not.
 * @param[in] n_counters
 *   0 in case of 1 counter to read, otherwise the counter number to read.
 *  @param pkts
 *   The number of packets that matched the flow.
 *  @param bytes
 *    The number of bytes that matched the flow.
 *  @param mkey
 *   The mkey key for batch query.
 *  @param addr
 *    The address in the mkey range for batch query.
 *  @param cmd_comp
 *   The completion object for asynchronous batch query.
 *  @param async_id
 *    The ID to be returned in the asynchronous batch query response.
 *
 * @return
 *   0 on success, a negative value otherwise.
 */
int
mlx5_devx_cmd_flow_counter_query(struct mlx5_devx_obj *dcs,
				 int clear, uint32_t n_counters,
				 uint64_t *pkts, uint64_t *bytes,
				 uint32_t mkey, void *addr,
				 struct mlx5dv_devx_cmd_comp *cmd_comp,
				 uint64_t async_id)
{
	int out_len = MLX5_ST_SZ_BYTES(query_flow_counter_out) +
			MLX5_ST_SZ_BYTES(traffic_counter);
	uint32_t out[out_len];
	uint32_t in[MLX5_ST_SZ_DW(query_flow_counter_in)] = {0};
	void *stats;
	int rc;

	MLX5_SET(query_flow_counter_in, in, opcode,
		 MLX5_CMD_OP_QUERY_FLOW_COUNTER);
	MLX5_SET(query_flow_counter_in, in, op_mod, 0);
	MLX5_SET(query_flow_counter_in, in, flow_counter_id, dcs->id);
	MLX5_SET(query_flow_counter_in, in, clear, !!clear);

	if (n_counters) {
		MLX5_SET(query_flow_counter_in, in, num_of_counters,
			 n_counters);
		MLX5_SET(query_flow_counter_in, in, dump_to_memory, 1);
		MLX5_SET(query_flow_counter_in, in, mkey, mkey);
		MLX5_SET64(query_flow_counter_in, in, address,
			   (uint64_t)(uintptr_t)addr);
	}
	if (!cmd_comp)
		rc = mlx5_glue->devx_obj_query(dcs->obj, in, sizeof(in), out,
					       out_len);
	else
		rc = mlx5_glue->devx_obj_query_async(dcs->obj, in, sizeof(in),
						     out_len, async_id,
						     cmd_comp);
	if (rc) {
		DRV_LOG(ERR, "Failed to query devx counters with rc %d\n ", rc);
		rte_errno = rc;
		return -rc;
	}
	if (!n_counters) {
		stats = MLX5_ADDR_OF(query_flow_counter_out,
				     out, flow_statistics);
		*pkts = MLX5_GET64(traffic_counter, stats, packets);
		*bytes = MLX5_GET64(traffic_counter, stats, octets);
	}
	return 0;
}

/**
 * Create a new mkey.
 *
 * @param[in] ctx
 *   ibv contexts returned from mlx5dv_open_device.
 * @param[in] attr
 *   Attributes of the requested mkey.
 *
 * @return
 *   Pointer to Devx mkey on success, a negative value otherwise and rte_errno
 *   is set.
 */
struct mlx5_devx_obj *
mlx5_devx_cmd_mkey_create(struct ibv_context *ctx,
			  struct mlx5_devx_mkey_attr *attr)
{
	uint32_t in[MLX5_ST_SZ_DW(create_mkey_in)] = {0};
	uint32_t out[MLX5_ST_SZ_DW(create_mkey_out)] = {0};
	void *mkc;
	struct mlx5_devx_obj *mkey = rte_zmalloc("mkey", sizeof(*mkey), 0);
	size_t pgsize;
	uint32_t translation_size;

	if (!mkey) {
		rte_errno = ENOMEM;
		return NULL;
	}
	pgsize = sysconf(_SC_PAGESIZE);
	translation_size = (RTE_ALIGN(attr->size, pgsize) * 8) / 16;
	MLX5_SET(create_mkey_in, in, opcode, MLX5_CMD_OP_CREATE_MKEY);
	MLX5_SET(create_mkey_in, in, translations_octword_actual_size,
		 translation_size);
	MLX5_SET(create_mkey_in, in, mkey_umem_id, attr->umem_id);
	mkc = MLX5_ADDR_OF(create_mkey_in, in, memory_key_mkey_entry);
	MLX5_SET(mkc, mkc, lw, 0x1);
	MLX5_SET(mkc, mkc, lr, 0x1);
	MLX5_SET(mkc, mkc, access_mode_1_0, MLX5_MKC_ACCESS_MODE_MTT);
	MLX5_SET(mkc, mkc, qpn, 0xffffff);
	MLX5_SET(mkc, mkc, pd, attr->pd);
	MLX5_SET(mkc, mkc, mkey_7_0, attr->umem_id & 0xFF);
	MLX5_SET(mkc, mkc, translations_octword_size, translation_size);
	MLX5_SET64(mkc, mkc, start_addr, attr->addr);
	MLX5_SET64(mkc, mkc, len, attr->size);
	MLX5_SET(mkc, mkc, log_page_size, rte_log2_u32(pgsize));
	mkey->obj = mlx5_glue->devx_obj_create(ctx, in, sizeof(in), out,
					       sizeof(out));
	if (!mkey->obj) {
		DRV_LOG(ERR, "Can't create mkey - error %d\n", errno);
		rte_errno = errno;
		rte_free(mkey);
		return NULL;
	}
	mkey->id = MLX5_GET(create_mkey_out, out, mkey_index);
	mkey->id = (mkey->id << 8) | (attr->umem_id & 0xFF);
	return mkey;
}

/**
 * Get status of devx command response.
 * Mainly used for asynchronous commands.
 *
 * @param[in] out
 *   The out response buffer.
 *
 * @return
 *   0 on success, non-zero value otherwise.
 */
int
mlx5_devx_get_out_command_status(void *out)
{
	int status;

	if (!out)
		return -EINVAL;
	status = MLX5_GET(query_flow_counter_out, out, status);
	if (status) {
		int syndrome = MLX5_GET(query_flow_counter_out, out, syndrome);

		DRV_LOG(ERR, "Bad devX status %x, syndrome = %x\n", status,
			syndrome);
	}
	return status;
}

/**
 * Destroy any object allocated by a Devx API.
 *
 * @param[in] obj
 *   Pointer to a general object.
 *
 * @return
 *   0 on success, a negative value otherwise.
 */
int
mlx5_devx_cmd_destroy(struct mlx5_devx_obj *obj)
{
	int ret;

	if (!obj)
		return 0;
	ret =  mlx5_glue->devx_obj_destroy(obj->obj);
	rte_free(obj);
	return ret;
}

/**
 * Query NIC vport context.
 * Fills minimal inline attribute.
 *
 * @param[in] ctx
 *   ibv contexts returned from mlx5dv_open_device.
 * @param[in] vport
 *   vport index
 * @param[out] attr
 *   Attributes device values.
 *
 * @return
 *   0 on success, a negative value otherwise.
 */
static int
mlx5_devx_cmd_query_nic_vport_context(struct ibv_context *ctx,
				      unsigned int vport,
				      struct mlx5_hca_attr *attr)
{
	uint32_t in[MLX5_ST_SZ_DW(query_nic_vport_context_in)] = {0};
	uint32_t out[MLX5_ST_SZ_DW(query_nic_vport_context_out)] = {0};
	void *vctx;
	int status, syndrome, rc;

	/* Query NIC vport context to determine inline mode. */
	MLX5_SET(query_nic_vport_context_in, in, opcode,
		 MLX5_CMD_OP_QUERY_NIC_VPORT_CONTEXT);
	MLX5_SET(query_nic_vport_context_in, in, vport_number, vport);
	if (vport)
		MLX5_SET(query_nic_vport_context_in, in, other_vport, 1);
	rc = mlx5_glue->devx_general_cmd(ctx,
					 in, sizeof(in),
					 out, sizeof(out));
	if (rc)
		goto error;
	status = MLX5_GET(query_nic_vport_context_out, out, status);
	syndrome = MLX5_GET(query_nic_vport_context_out, out, syndrome);
	if (status) {
		DRV_LOG(DEBUG, "Failed to query NIC vport context, "
			"status %x, syndrome = %x",
			status, syndrome);
		return -1;
	}
	vctx = MLX5_ADDR_OF(query_nic_vport_context_out, out,
			    nic_vport_context);
	attr->vport_inline_mode = MLX5_GET(nic_vport_context, vctx,
					   min_wqe_inline_mode);
	return 0;
error:
	rc = (rc > 0) ? -rc : rc;
	return rc;
}

/**
 * Query HCA attributes.
 * Using those attributes we can check on run time if the device
 * is having the required capabilities.
 *
 * @param[in] ctx
 *   ibv contexts returned from mlx5dv_open_device.
 * @param[out] attr
 *   Attributes device values.
 *
 * @return
 *   0 on success, a negative value otherwise.
 */
int
mlx5_devx_cmd_query_hca_attr(struct ibv_context *ctx,
			     struct mlx5_hca_attr *attr)
{
	uint32_t in[MLX5_ST_SZ_DW(query_hca_cap_in)] = {0};
	uint32_t out[MLX5_ST_SZ_DW(query_hca_cap_out)] = {0};
	void *hcattr;
	int status, syndrome, rc;

	MLX5_SET(query_hca_cap_in, in, opcode, MLX5_CMD_OP_QUERY_HCA_CAP);
	MLX5_SET(query_hca_cap_in, in, op_mod,
		 MLX5_GET_HCA_CAP_OP_MOD_GENERAL_DEVICE |
		 MLX5_HCA_CAP_OPMOD_GET_CUR);

	rc = mlx5_glue->devx_general_cmd(ctx,
					 in, sizeof(in), out, sizeof(out));
	if (rc)
		goto error;
	status = MLX5_GET(query_hca_cap_out, out, status);
	syndrome = MLX5_GET(query_hca_cap_out, out, syndrome);
	if (status) {
		DRV_LOG(DEBUG, "Failed to query devx HCA capabilities, "
			"status %x, syndrome = %x",
			status, syndrome);
		return -1;
	}
	hcattr = MLX5_ADDR_OF(query_hca_cap_out, out, capability);
	attr->flow_counter_bulk_alloc_bitmap =
			MLX5_GET(cmd_hca_cap, hcattr, flow_counter_bulk_alloc);
	attr->flow_counters_dump = MLX5_GET(cmd_hca_cap, hcattr,
					    flow_counters_dump);
	attr->eswitch_manager = MLX5_GET(cmd_hca_cap, hcattr, eswitch_manager);
	attr->eth_net_offloads = MLX5_GET(cmd_hca_cap, hcattr,
					  eth_net_offloads);
	attr->eth_virt = MLX5_GET(cmd_hca_cap, hcattr, eth_virt);
	if (!attr->eth_net_offloads)
		return 0;

	/* Query HCA offloads for Ethernet protocol. */
	memset(in, 0, sizeof(in));
	memset(out, 0, sizeof(out));
	MLX5_SET(query_hca_cap_in, in, opcode, MLX5_CMD_OP_QUERY_HCA_CAP);
	MLX5_SET(query_hca_cap_in, in, op_mod,
		 MLX5_GET_HCA_CAP_OP_MOD_ETHERNET_OFFLOAD_CAPS |
		 MLX5_HCA_CAP_OPMOD_GET_CUR);

	rc = mlx5_glue->devx_general_cmd(ctx,
					 in, sizeof(in),
					 out, sizeof(out));
	if (rc) {
		attr->eth_net_offloads = 0;
		goto error;
	}
	status = MLX5_GET(query_hca_cap_out, out, status);
	syndrome = MLX5_GET(query_hca_cap_out, out, syndrome);
	if (status) {
		DRV_LOG(DEBUG, "Failed to query devx HCA capabilities, "
			"status %x, syndrome = %x",
			status, syndrome);
		attr->eth_net_offloads = 0;
		return -1;
	}
	hcattr = MLX5_ADDR_OF(query_hca_cap_out, out, capability);
	attr->wqe_vlan_insert = MLX5_GET(per_protocol_networking_offload_caps,
					 hcattr, wqe_vlan_insert);
	attr->lro_cap = MLX5_GET(per_protocol_networking_offload_caps, hcattr,
				 lro_cap);
	attr->tunnel_lro_gre = MLX5_GET(per_protocol_networking_offload_caps,
					hcattr, tunnel_lro_gre);
	attr->tunnel_lro_vxlan = MLX5_GET(per_protocol_networking_offload_caps,
					  hcattr, tunnel_lro_vxlan);
	attr->lro_max_msg_sz_mode = MLX5_GET
					(per_protocol_networking_offload_caps,
					 hcattr, lro_max_msg_sz_mode);
	for (int i = 0 ; i < MLX5_LRO_NUM_SUPP_PERIODS ; i++) {
		attr->lro_timer_supported_periods[i] =
			MLX5_GET(per_protocol_networking_offload_caps, hcattr,
				 lro_timer_supported_periods[i]);
	}
	attr->wqe_inline_mode = MLX5_GET(per_protocol_networking_offload_caps,
					 hcattr, wqe_inline_mode);
	if (attr->wqe_inline_mode != MLX5_CAP_INLINE_MODE_VPORT_CONTEXT)
		return 0;
	if (attr->eth_virt) {
		rc = mlx5_devx_cmd_query_nic_vport_context(ctx, 0, attr);
		if (rc) {
			attr->eth_virt = 0;
			goto error;
		}
	}
	return 0;
error:
	rc = (rc > 0) ? -rc : rc;
	return rc;
}
