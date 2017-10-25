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
 * Interrupts handling for mlx4 driver.
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

/* Verbs headers do not support -pedantic. */
#ifdef PEDANTIC
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <infiniband/verbs.h>
#ifdef PEDANTIC
#pragma GCC diagnostic error "-Wpedantic"
#endif

#include <rte_alarm.h>
#include <rte_errno.h>
#include <rte_ethdev.h>
#include <rte_io.h>
#include <rte_interrupts.h>

#include "mlx4.h"
#include "mlx4_rxtx.h"
#include "mlx4_utils.h"

static int mlx4_link_status_check(struct priv *priv);

/**
 * Clean up Rx interrupts handler.
 *
 * @param priv
 *   Pointer to private structure.
 */
static void
mlx4_rx_intr_vec_disable(struct priv *priv)
{
	struct rte_intr_handle *intr_handle = &priv->intr_handle;

	rte_intr_free_epoll_fd(intr_handle);
	free(intr_handle->intr_vec);
	intr_handle->nb_efd = 0;
	intr_handle->intr_vec = NULL;
}

/**
 * Allocate queue vector and fill epoll fd list for Rx interrupts.
 *
 * @param priv
 *   Pointer to private structure.
 *
 * @return
 *   0 on success, negative errno value otherwise and rte_errno is set.
 */
static int
mlx4_rx_intr_vec_enable(struct priv *priv)
{
	unsigned int i;
	unsigned int rxqs_n = priv->dev->data->nb_rx_queues;
	unsigned int n = RTE_MIN(rxqs_n, (uint32_t)RTE_MAX_RXTX_INTR_VEC_ID);
	unsigned int count = 0;
	struct rte_intr_handle *intr_handle = &priv->intr_handle;

	mlx4_rx_intr_vec_disable(priv);
	intr_handle->intr_vec = malloc(sizeof(intr_handle->intr_vec[rxqs_n]));
	if (intr_handle->intr_vec == NULL) {
		rte_errno = ENOMEM;
		ERROR("failed to allocate memory for interrupt vector,"
		      " Rx interrupts will not be supported");
		return -rte_errno;
	}
	for (i = 0; i != n; ++i) {
		struct rxq *rxq = priv->dev->data->rx_queues[i];

		/* Skip queues that cannot request interrupts. */
		if (!rxq || !rxq->channel) {
			/* Use invalid intr_vec[] index to disable entry. */
			intr_handle->intr_vec[i] =
				RTE_INTR_VEC_RXTX_OFFSET +
				RTE_MAX_RXTX_INTR_VEC_ID;
			continue;
		}
		if (count >= RTE_MAX_RXTX_INTR_VEC_ID) {
			rte_errno = E2BIG;
			ERROR("too many Rx queues for interrupt vector size"
			      " (%d), Rx interrupts cannot be enabled",
			      RTE_MAX_RXTX_INTR_VEC_ID);
			mlx4_rx_intr_vec_disable(priv);
			return -rte_errno;
		}
		intr_handle->intr_vec[i] = RTE_INTR_VEC_RXTX_OFFSET + count;
		intr_handle->efds[count] = rxq->channel->fd;
		count++;
	}
	if (!count)
		mlx4_rx_intr_vec_disable(priv);
	else
		intr_handle->nb_efd = count;
	return 0;
}

/**
 * Process scheduled link status check.
 *
 * If LSC interrupts are requested, process related callback.
 *
 * @param priv
 *   Pointer to private structure.
 */
static void
mlx4_link_status_alarm(struct priv *priv)
{
	const struct rte_intr_conf *const intr_conf =
		&priv->dev->data->dev_conf.intr_conf;

	assert(priv->intr_alarm == 1);
	priv->intr_alarm = 0;
	if (intr_conf->lsc && !mlx4_link_status_check(priv))
		_rte_eth_dev_callback_process(priv->dev,
					      RTE_ETH_EVENT_INTR_LSC,
					      NULL, NULL);
}

/**
 * Check link status.
 *
 * In case of inconsistency, another check is scheduled.
 *
 * @param priv
 *   Pointer to private structure.
 *
 * @return
 *   0 on success (link status is consistent), negative errno value
 *   otherwise and rte_errno is set.
 */
static int
mlx4_link_status_check(struct priv *priv)
{
	struct rte_eth_link *link = &priv->dev->data->dev_link;
	int ret = mlx4_link_update(priv->dev, 0);

	if (ret)
		return ret;
	if ((!link->link_speed && link->link_status) ||
	    (link->link_speed && !link->link_status)) {
		if (!priv->intr_alarm) {
			/* Inconsistent status, check again later. */
			ret = rte_eal_alarm_set(MLX4_INTR_ALARM_TIMEOUT,
						(void (*)(void *))
						mlx4_link_status_alarm,
						priv);
			if (ret)
				return ret;
			priv->intr_alarm = 1;
		}
		rte_errno = EINPROGRESS;
		return -rte_errno;
	}
	return 0;
}

/**
 * Handle interrupts from the NIC.
 *
 * @param priv
 *   Pointer to private structure.
 */
static void
mlx4_interrupt_handler(struct priv *priv)
{
	enum { LSC, RMV, };
	static const enum rte_eth_event_type type[] = {
		[LSC] = RTE_ETH_EVENT_INTR_LSC,
		[RMV] = RTE_ETH_EVENT_INTR_RMV,
	};
	uint32_t caught[RTE_DIM(type)] = { 0 };
	struct ibv_async_event event;
	const struct rte_intr_conf *const intr_conf =
		&priv->dev->data->dev_conf.intr_conf;
	unsigned int i;

	/* Read all message and acknowledge them. */
	while (!ibv_get_async_event(priv->ctx, &event)) {
		switch (event.event_type) {
		case IBV_EVENT_PORT_ACTIVE:
		case IBV_EVENT_PORT_ERR:
			if (intr_conf->lsc && !mlx4_link_status_check(priv))
				++caught[LSC];
			break;
		case IBV_EVENT_DEVICE_FATAL:
			if (intr_conf->rmv)
				++caught[RMV];
			break;
		default:
			DEBUG("event type %d on physical port %d not handled",
			      event.event_type, event.element.port_num);
		}
		ibv_ack_async_event(&event);
	}
	for (i = 0; i != RTE_DIM(caught); ++i)
		if (caught[i])
			_rte_eth_dev_callback_process(priv->dev, type[i],
						      NULL, NULL);
}

/**
 * MLX4 CQ notification .
 *
 * @param rxq
 *   Pointer to receive queue structure.
 * @param solicited
 *   Is request solicited or not.
 */
static void
mlx4_arm_cq(struct rxq *rxq, int solicited)
{
	struct mlx4_cq *cq = &rxq->mcq;
	uint64_t doorbell;
	uint32_t sn = cq->arm_sn & MLX4_CQ_DB_GEQ_N_MASK;
	uint32_t ci = cq->cons_index & MLX4_CQ_DB_CI_MASK;
	uint32_t cmd = solicited ? MLX4_CQ_DB_REQ_NOT_SOL : MLX4_CQ_DB_REQ_NOT;

	*cq->arm_db = rte_cpu_to_be_32(sn << 28 | cmd | ci);
	/*
	 * Make sure that the doorbell record in host memory is
	 * written before ringing the doorbell via PCI MMIO.
	 */
	rte_wmb();
	doorbell = sn << 28 | cmd | cq->cqn;
	doorbell <<= 32;
	doorbell |= ci;
	rte_write64(rte_cpu_to_be_64(doorbell), cq->cq_db_reg);
}

/**
 * Uninstall interrupt handler.
 *
 * @param priv
 *   Pointer to private structure.
 *
 * @return
 *   0 on success, negative errno value otherwise and rte_errno is set.
 */
int
mlx4_intr_uninstall(struct priv *priv)
{
	int err = rte_errno; /* Make sure rte_errno remains unchanged. */

	if (priv->intr_handle.fd != -1) {
		rte_intr_callback_unregister(&priv->intr_handle,
					     (void (*)(void *))
					     mlx4_interrupt_handler,
					     priv);
		priv->intr_handle.fd = -1;
	}
	rte_eal_alarm_cancel((void (*)(void *))mlx4_link_status_alarm, priv);
	priv->intr_alarm = 0;
	mlx4_rx_intr_vec_disable(priv);
	rte_errno = err;
	return 0;
}

/**
 * Install interrupt handler.
 *
 * @param priv
 *   Pointer to private structure.
 *
 * @return
 *   0 on success, negative errno value otherwise and rte_errno is set.
 */
int
mlx4_intr_install(struct priv *priv)
{
	const struct rte_intr_conf *const intr_conf =
		&priv->dev->data->dev_conf.intr_conf;
	int rc;

	mlx4_intr_uninstall(priv);
	if (intr_conf->rxq && mlx4_rx_intr_vec_enable(priv) < 0)
		goto error;
	if (intr_conf->lsc | intr_conf->rmv) {
		priv->intr_handle.fd = priv->ctx->async_fd;
		rc = rte_intr_callback_register(&priv->intr_handle,
						(void (*)(void *))
						mlx4_interrupt_handler,
						priv);
		if (rc < 0) {
			rte_errno = -rc;
			goto error;
		}
	}
	return 0;
error:
	mlx4_intr_uninstall(priv);
	return -rte_errno;
}

/**
 * DPDK callback for Rx queue interrupt disable.
 *
 * @param dev
 *   Pointer to Ethernet device structure.
 * @param idx
 *   Rx queue index.
 *
 * @return
 *   0 on success, negative errno value otherwise and rte_errno is set.
 */
int
mlx4_rx_intr_disable(struct rte_eth_dev *dev, uint16_t idx)
{
	struct rxq *rxq = dev->data->rx_queues[idx];
	struct ibv_cq *ev_cq;
	void *ev_ctx;
	int ret;

	if (!rxq || !rxq->channel) {
		ret = EINVAL;
	} else {
		ret = ibv_get_cq_event(rxq->cq->channel, &ev_cq, &ev_ctx);
		if (ret || ev_cq != rxq->cq)
			ret = EINVAL;
	}
	if (ret) {
		rte_errno = ret;
		WARN("unable to disable interrupt on rx queue %d",
		     idx);
	} else {
		rxq->mcq.arm_sn++;
		ibv_ack_cq_events(rxq->cq, 1);
	}
	return -ret;
}

/**
 * DPDK callback for Rx queue interrupt enable.
 *
 * @param dev
 *   Pointer to Ethernet device structure.
 * @param idx
 *   Rx queue index.
 *
 * @return
 *   0 on success, negative errno value otherwise and rte_errno is set.
 */
int
mlx4_rx_intr_enable(struct rte_eth_dev *dev, uint16_t idx)
{
	struct rxq *rxq = dev->data->rx_queues[idx];
	int ret = 0;

	if (!rxq || !rxq->channel) {
		ret = EINVAL;
		rte_errno = ret;
		WARN("unable to arm interrupt on rx queue %d", idx);
	} else {
		mlx4_arm_cq(rxq, 0);
	}
	return -ret;
}
