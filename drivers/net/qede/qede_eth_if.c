/*
 * Copyright (c) 2016 QLogic Corporation.
 * All rights reserved.
 * www.qlogic.com
 *
 * See LICENSE.qede_pmd for copyright and licensing details.
 */

#include "qede_ethdev.h"

static int
qed_start_rxq(struct ecore_dev *edev,
	      uint8_t rss_num,
	      struct ecore_queue_start_common_params *p_params,
	      uint16_t bd_max_bytes,
	      dma_addr_t bd_chain_phys_addr,
	      dma_addr_t cqe_pbl_addr,
	      uint16_t cqe_pbl_size,
	      struct ecore_rxq_start_ret_params *ret_params)
{
	struct ecore_hwfn *p_hwfn;
	int rc, hwfn_index;

	hwfn_index = rss_num % edev->num_hwfns;
	p_hwfn = &edev->hwfns[hwfn_index];

	p_params->queue_id = p_params->queue_id / edev->num_hwfns;
	p_params->stats_id = p_params->vport_id;

	rc = ecore_eth_rx_queue_start(p_hwfn,
				      p_hwfn->hw_info.opaque_fid,
				      p_params,
				      bd_max_bytes,
				      bd_chain_phys_addr,
				      cqe_pbl_addr,
				      cqe_pbl_size,
				      ret_params);

	if (rc) {
		DP_ERR(edev, "Failed to start RXQ#%d\n", p_params->queue_id);
		return rc;
	}

	DP_VERBOSE(edev, ECORE_MSG_SPQ,
		   "Started RX-Q %d [rss_num %d] on V-PORT %d and SB %d\n",
		   p_params->queue_id, rss_num, p_params->vport_id,
		   p_params->sb);

	return 0;
}

static int
qed_stop_rxq(struct ecore_dev *edev, uint8_t rss_id, void *handle)
{
	int rc, hwfn_index;
	struct ecore_hwfn *p_hwfn;

	hwfn_index = rss_id % edev->num_hwfns;
	p_hwfn = &edev->hwfns[hwfn_index];

	rc = ecore_eth_rx_queue_stop(p_hwfn, handle, true, false);
	if (rc) {
		DP_ERR(edev, "Failed to stop RXQ#%02x\n", rss_id);
		return rc;
	}

	return 0;
}

static int
qed_start_txq(struct ecore_dev *edev,
	      uint8_t rss_num,
	      struct ecore_queue_start_common_params *p_params,
	      dma_addr_t pbl_addr,
	      uint16_t pbl_size,
	      struct ecore_txq_start_ret_params *ret_params)
{
	struct ecore_hwfn *p_hwfn;
	int rc, hwfn_index;

	hwfn_index = rss_num % edev->num_hwfns;
	p_hwfn = &edev->hwfns[hwfn_index];

	p_params->queue_id = p_params->queue_id / edev->num_hwfns;
	p_params->stats_id = p_params->vport_id;

	rc = ecore_eth_tx_queue_start(p_hwfn,
				      p_hwfn->hw_info.opaque_fid,
				      p_params, 0 /* tc */,
				      pbl_addr, pbl_size,
				      ret_params);

	if (rc) {
		DP_ERR(edev, "Failed to start TXQ#%d\n", p_params->queue_id);
		return rc;
	}

	DP_VERBOSE(edev, ECORE_MSG_SPQ,
		   "Started TX-Q %d [rss_num %d] on V-PORT %d and SB %d\n",
		   p_params->queue_id, rss_num, p_params->vport_id,
		   p_params->sb);

	return 0;
}

static int
qed_stop_txq(struct ecore_dev *edev, uint8_t rss_id, void *handle)
{
	struct ecore_hwfn *p_hwfn;
	int rc, hwfn_index;

	hwfn_index = rss_id % edev->num_hwfns;
	p_hwfn = &edev->hwfns[hwfn_index];

	rc = ecore_eth_tx_queue_stop(p_hwfn, handle);
	if (rc) {
		DP_ERR(edev, "Failed to stop TXQ#%02x\n", rss_id);
		return rc;
	}

	return 0;
}

static int
qed_fp_cqe_completion(struct ecore_dev *edev,
		      uint8_t rss_id, struct eth_slow_path_rx_cqe *cqe)
{
	return ecore_eth_cqe_completion(&edev->hwfns[rss_id % edev->num_hwfns],
					cqe);
}

static int qed_fastpath_stop(struct ecore_dev *edev)
{
	ecore_hw_stop_fastpath(edev);

	return 0;
}

static void qed_fastpath_start(struct ecore_dev *edev)
{
	struct ecore_hwfn *p_hwfn;
	int i;

	for_each_hwfn(edev, i) {
		p_hwfn = &edev->hwfns[i];
		ecore_hw_start_fastpath(p_hwfn);
	}
}

static void
qed_get_vport_stats(struct ecore_dev *edev, struct ecore_eth_stats *stats)
{
	ecore_get_vport_stats(edev, stats);
}

int qed_configure_filter_rx_mode(struct rte_eth_dev *eth_dev,
				 enum qed_filter_rx_mode_type type)
{
	struct qede_dev *qdev = QEDE_INIT_QDEV(eth_dev);
	struct ecore_dev *edev = QEDE_INIT_EDEV(qdev);
	struct ecore_filter_accept_flags flags;

	memset(&flags, 0, sizeof(flags));

	flags.update_rx_mode_config = 1;
	flags.update_tx_mode_config = 1;
	flags.rx_accept_filter = ECORE_ACCEPT_UCAST_MATCHED |
					ECORE_ACCEPT_MCAST_MATCHED |
					ECORE_ACCEPT_BCAST;

	flags.tx_accept_filter = ECORE_ACCEPT_UCAST_MATCHED |
				 ECORE_ACCEPT_MCAST_MATCHED |
				 ECORE_ACCEPT_BCAST;

	if (type == QED_FILTER_RX_MODE_TYPE_PROMISC) {
		flags.rx_accept_filter |= ECORE_ACCEPT_UCAST_UNMATCHED;
		if (IS_VF(edev)) {
			flags.tx_accept_filter |= ECORE_ACCEPT_UCAST_UNMATCHED;
			DP_INFO(edev, "Enabling Tx unmatched flag for VF\n");
		}
	} else if (type == QED_FILTER_RX_MODE_TYPE_MULTI_PROMISC) {
		flags.rx_accept_filter |= ECORE_ACCEPT_MCAST_UNMATCHED;
	} else if (type == (QED_FILTER_RX_MODE_TYPE_MULTI_PROMISC |
			    QED_FILTER_RX_MODE_TYPE_PROMISC)) {
		flags.rx_accept_filter |= ECORE_ACCEPT_UCAST_UNMATCHED |
					  ECORE_ACCEPT_MCAST_UNMATCHED;
	}

	return ecore_filter_accept_cmd(edev, 0, flags, false, false,
				       ECORE_SPQ_MODE_CB, NULL);
}

static const struct qed_eth_ops qed_eth_ops_pass = {
	INIT_STRUCT_FIELD(common, &qed_common_ops_pass),
	INIT_STRUCT_FIELD(fill_dev_info, &qed_fill_eth_dev_info),
	INIT_STRUCT_FIELD(q_rx_start, &qed_start_rxq),
	INIT_STRUCT_FIELD(q_tx_start, &qed_start_txq),
	INIT_STRUCT_FIELD(q_rx_stop, &qed_stop_rxq),
	INIT_STRUCT_FIELD(q_tx_stop, &qed_stop_txq),
	INIT_STRUCT_FIELD(eth_cqe_completion, &qed_fp_cqe_completion),
	INIT_STRUCT_FIELD(fastpath_stop, &qed_fastpath_stop),
	INIT_STRUCT_FIELD(fastpath_start, &qed_fastpath_start),
	INIT_STRUCT_FIELD(get_vport_stats, &qed_get_vport_stats),
};

const struct qed_eth_ops *qed_get_eth_ops(void)
{
	return &qed_eth_ops_pass;
}
