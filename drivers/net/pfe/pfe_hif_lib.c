/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019 NXP
 */

#include "pfe_logs.h"
#include "pfe_mod.h"

unsigned int emac_txq_cnt;

/*
 * @pfe_hal_lib.c
 * Common functions used by HIF client drivers
 */

/*HIF shared memory Global variable */
struct hif_shm ghif_shm;

/*This function sends indication to HIF driver
 *
 * @param[in] hif	hif context
 */
static void
hif_lib_indicate_hif(struct pfe_hif *hif, int req, int data1, int
		     data2)
{
	hif_process_client_req(hif, req, data1, data2);
}

void
hif_lib_indicate_client(struct hif_client_s *client, int event_type,
			int qno)
{
	if (!client || event_type >= HIF_EVENT_MAX ||
	    qno >= HIF_CLIENT_QUEUES_MAX)
		return;

	if (!test_and_set_bit(qno, &client->queue_mask[event_type]))
		client->event_handler(client->priv, event_type, qno);
}

/*This function releases Rx queue descriptors memory and pre-filled buffers
 *
 * @param[in] client	hif_client context
 */
static void
hif_lib_client_release_rx_buffers(struct hif_client_s *client)
{
	struct rte_mempool *pool;
	struct rte_pktmbuf_pool_private *mb_priv;
	struct rx_queue_desc *desc;
	unsigned int qno, ii;
	void *buf;

	pool = client->pfe->hif.shm->pool;
	mb_priv = rte_mempool_get_priv(pool);
	for (qno = 0; qno < client->rx_qn; qno++) {
		desc = client->rx_q[qno].base;

		for (ii = 0; ii < client->rx_q[qno].size; ii++) {
			buf = (void *)desc->data;
			if (buf) {
			/* Data pointor to mbuf pointor calculation:
			 * "Data - User private data - headroom - mbufsize"
			 * Actual data pointor given to HIF BDs was
			 * "mbuf->data_offset - PFE_PKT_HEADER_SZ"
			 */
				buf = buf + PFE_PKT_HEADER_SZ
					- sizeof(struct rte_mbuf)
					- RTE_PKTMBUF_HEADROOM
					- mb_priv->mbuf_priv_size;
				rte_pktmbuf_free((struct rte_mbuf *)buf);
				desc->ctrl = 0;
			}
			desc++;
		}
	}
	rte_free(client->rx_qbase);
}

/*This function allocates memory for the rxq descriptors and pre-fill rx queues
 * with buffers.
 * @param[in] client	client context
 * @param[in] q_size	size of the rxQ, all queues are of same size
 */
static int
hif_lib_client_init_rx_buffers(struct hif_client_s *client,
					  int q_size)
{
	struct rx_queue_desc *desc;
	struct hif_client_rx_queue *queue;
	unsigned int ii, qno;

	/*Allocate memory for the client queues */
	client->rx_qbase = rte_malloc(NULL, client->rx_qn * q_size *
			sizeof(struct rx_queue_desc), RTE_CACHE_LINE_SIZE);
	if (!client->rx_qbase)
		goto err;

	for (qno = 0; qno < client->rx_qn; qno++) {
		queue = &client->rx_q[qno];

		queue->base = client->rx_qbase + qno * q_size * sizeof(struct
				rx_queue_desc);
		queue->size = q_size;
		queue->read_idx = 0;
		queue->write_idx = 0;
		queue->queue_id = 0;
		queue->port_id = client->port_id;
		queue->priv = client->priv;
		PFE_PMD_DEBUG("rx queue: %d, base: %p, size: %d\n", qno,
			      queue->base, queue->size);
	}

	for (qno = 0; qno < client->rx_qn; qno++) {
		queue = &client->rx_q[qno];
		desc = queue->base;

		for (ii = 0; ii < queue->size; ii++) {
			desc->ctrl = CL_DESC_OWN;
			desc++;
		}
	}

	return 0;

err:
	return 1;
}


static void
hif_lib_client_cleanup_tx_queue(struct hif_client_tx_queue *queue)
{
	/*
	 * Check if there are any pending packets. Client must flush the tx
	 * queues before unregistering, by calling by calling
	 * hif_lib_tx_get_next_complete()
	 *
	 * Hif no longer calls since we are no longer registered
	 */
	if (queue->tx_pending)
		PFE_PMD_ERR("pending transmit packet");
}

static void
hif_lib_client_release_tx_buffers(struct hif_client_s *client)
{
	unsigned int qno;

	for (qno = 0; qno < client->tx_qn; qno++)
		hif_lib_client_cleanup_tx_queue(&client->tx_q[qno]);

	rte_free(client->tx_qbase);
}

static int
hif_lib_client_init_tx_buffers(struct hif_client_s *client, int
						q_size)
{
	struct hif_client_tx_queue *queue;
	unsigned int qno;

	client->tx_qbase = rte_malloc(NULL, client->tx_qn * q_size *
			sizeof(struct tx_queue_desc), RTE_CACHE_LINE_SIZE);
	if (!client->tx_qbase)
		return 1;

	for (qno = 0; qno < client->tx_qn; qno++) {
		queue = &client->tx_q[qno];

		queue->base = client->tx_qbase + qno * q_size * sizeof(struct
				tx_queue_desc);
		queue->size = q_size;
		queue->read_idx = 0;
		queue->write_idx = 0;
		queue->tx_pending = 0;
		queue->nocpy_flag = 0;
		queue->prev_tmu_tx_pkts = 0;
		queue->done_tmu_tx_pkts = 0;
		queue->priv = client->priv;
		queue->queue_id = 0;
		queue->port_id = client->port_id;

		PFE_PMD_DEBUG("tx queue: %d, base: %p, size: %d", qno,
			 queue->base, queue->size);
	}

	return 0;
}

static int
hif_lib_event_dummy(__rte_unused void *priv,
		__rte_unused int event_type, __rte_unused int qno)
{
	return 0;
}

int
hif_lib_client_register(struct hif_client_s *client)
{
	struct hif_shm *hif_shm;
	struct hif_client_shm *client_shm;
	int err, i;

	PMD_INIT_FUNC_TRACE();

	/*Allocate memory before spin_lock*/
	if (hif_lib_client_init_rx_buffers(client, client->rx_qsize)) {
		err = -ENOMEM;
		goto err_rx;
	}

	if (hif_lib_client_init_tx_buffers(client, client->tx_qsize)) {
		err = -ENOMEM;
		goto err_tx;
	}

	rte_spinlock_lock(&client->pfe->hif.lock);
	if (!(client->pfe) || client->id >= HIF_CLIENTS_MAX ||
	    client->pfe->hif_client[client->id]) {
		err = -EINVAL;
		goto err;
	}

	hif_shm = client->pfe->hif.shm;

	if (!client->event_handler)
		client->event_handler = hif_lib_event_dummy;

	/*Initialize client specific shared memory */
	client_shm = (struct hif_client_shm *)&hif_shm->client[client->id];
	client_shm->rx_qbase = (unsigned long)client->rx_qbase;
	client_shm->rx_qsize = client->rx_qsize;
	client_shm->tx_qbase = (unsigned long)client->tx_qbase;
	client_shm->tx_qsize = client->tx_qsize;
	client_shm->ctrl = (client->tx_qn << CLIENT_CTRL_TX_Q_CNT_OFST) |
				(client->rx_qn << CLIENT_CTRL_RX_Q_CNT_OFST);

	for (i = 0; i < HIF_EVENT_MAX; i++) {
		client->queue_mask[i] = 0;  /*
					     * By default all events are
					     * unmasked
					     */
	}

	/*Indicate to HIF driver*/
	hif_lib_indicate_hif(&client->pfe->hif, REQUEST_CL_REGISTER,
			client->id, 0);

	PFE_PMD_DEBUG("client: %p, client_id: %d, tx_qsize: %d, rx_qsize: %d",
		      client, client->id, client->tx_qsize, client->rx_qsize);

	client->cpu_id = -1;

	client->pfe->hif_client[client->id] = client;
	rte_spinlock_unlock(&client->pfe->hif.lock);

	return 0;

err:
	rte_spinlock_unlock(&client->pfe->hif.lock);
	hif_lib_client_release_tx_buffers(client);

err_tx:
	hif_lib_client_release_rx_buffers(client);

err_rx:
	return err;
}

int
hif_lib_client_unregister(struct hif_client_s *client)
{
	struct pfe *pfe = client->pfe;
	u32 client_id = client->id;

	PFE_PMD_INFO("client: %p, client_id: %d, txQ_depth: %d, rxQ_depth: %d",
		     client, client->id, client->tx_qsize, client->rx_qsize);

	rte_spinlock_lock(&pfe->hif.lock);
	hif_lib_indicate_hif(&pfe->hif, REQUEST_CL_UNREGISTER, client->id, 0);

	hif_lib_client_release_tx_buffers(client);
	hif_lib_client_release_rx_buffers(client);
	pfe->hif_client[client_id] = NULL;
	rte_spinlock_unlock(&pfe->hif.lock);

	return 0;
}

int
hif_lib_event_handler_start(struct hif_client_s *client, int event,
				int qno)
{
	struct hif_client_rx_queue *queue = &client->rx_q[qno];
	struct rx_queue_desc *desc = queue->base + queue->read_idx;

	if (event >= HIF_EVENT_MAX || qno >= HIF_CLIENT_QUEUES_MAX) {
		PFE_PMD_WARN("Unsupported event : %d  queue number : %d",
				event, qno);
		return -1;
	}

	test_and_clear_bit(qno, &client->queue_mask[event]);

	switch (event) {
	case EVENT_RX_PKT_IND:
		if (!(desc->ctrl & CL_DESC_OWN))
			hif_lib_indicate_client(client,
						EVENT_RX_PKT_IND, qno);
		break;

	case EVENT_HIGH_RX_WM:
	case EVENT_TXDONE_IND:
	default:
		break;
	}

	return 0;
}

void *
hif_lib_tx_get_next_complete(struct hif_client_s *client, int qno,
				   unsigned int *flags, __rte_unused  int count)
{
	struct hif_client_tx_queue *queue = &client->tx_q[qno];
	struct tx_queue_desc *desc = queue->base + queue->read_idx;

	PFE_DP_LOG(DEBUG, "qno : %d rd_indx: %d pending:%d",
		   qno, queue->read_idx, queue->tx_pending);

	if (!queue->tx_pending)
		return NULL;

	if (queue->nocpy_flag && !queue->done_tmu_tx_pkts) {
		u32 tmu_tx_pkts = 0;

		if (queue->prev_tmu_tx_pkts > tmu_tx_pkts)
			queue->done_tmu_tx_pkts = UINT_MAX -
				queue->prev_tmu_tx_pkts + tmu_tx_pkts;
		else
			queue->done_tmu_tx_pkts = tmu_tx_pkts -
						queue->prev_tmu_tx_pkts;

		queue->prev_tmu_tx_pkts  = tmu_tx_pkts;

		if (!queue->done_tmu_tx_pkts)
			return NULL;
	}

	if (desc->ctrl & CL_DESC_OWN)
		return NULL;

	queue->read_idx = (queue->read_idx + 1) & (queue->size - 1);
	queue->tx_pending--;

	*flags = CL_DESC_GET_FLAGS(desc->ctrl);

	if (queue->done_tmu_tx_pkts && (*flags & HIF_LAST_BUFFER))
		queue->done_tmu_tx_pkts--;

	return desc->data;
}

int
pfe_hif_lib_init(struct pfe *pfe)
{
	PMD_INIT_FUNC_TRACE();

	emac_txq_cnt = EMAC_TXQ_CNT;
	pfe->hif.shm = &ghif_shm;

	return 0;
}

void
pfe_hif_lib_exit(__rte_unused struct pfe *pfe)
{
	PMD_INIT_FUNC_TRACE();
}
