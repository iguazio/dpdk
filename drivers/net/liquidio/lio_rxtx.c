/*
 *   BSD LICENSE
 *
 *   Copyright(c) 2017 Cavium, Inc.. All rights reserved.
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
 *     * Neither the name of Cavium, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER(S) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_malloc.h>

#include "lio_logs.h"
#include "lio_struct.h"
#include "lio_ethdev.h"
#include "lio_rxtx.h"

#define LIO_MAX_SG 12

static void
lio_droq_compute_max_packet_bufs(struct lio_droq *droq)
{
	uint32_t count = 0;

	do {
		count += droq->buffer_size;
	} while (count < LIO_MAX_RX_PKTLEN);
}

static void
lio_droq_reset_indices(struct lio_droq *droq)
{
	droq->read_idx	= 0;
	droq->write_idx	= 0;
	droq->refill_idx = 0;
	droq->refill_count = 0;
	rte_atomic64_set(&droq->pkts_pending, 0);
}

static void
lio_droq_destroy_ring_buffers(struct lio_droq *droq)
{
	uint32_t i;

	for (i = 0; i < droq->max_count; i++) {
		if (droq->recv_buf_list[i].buffer) {
			rte_pktmbuf_free((struct rte_mbuf *)
					 droq->recv_buf_list[i].buffer);
			droq->recv_buf_list[i].buffer = NULL;
		}
	}

	lio_droq_reset_indices(droq);
}

static void *
lio_recv_buffer_alloc(struct lio_device *lio_dev, int q_no)
{
	struct lio_droq *droq = lio_dev->droq[q_no];
	struct rte_mempool *mpool = droq->mpool;
	struct rte_mbuf *m;

	m = rte_pktmbuf_alloc(mpool);
	if (m == NULL) {
		lio_dev_err(lio_dev, "Cannot allocate\n");
		return NULL;
	}

	rte_mbuf_refcnt_set(m, 1);
	m->next = NULL;
	m->data_off = RTE_PKTMBUF_HEADROOM;
	m->nb_segs = 1;
	m->pool = mpool;

	return m;
}

static int
lio_droq_setup_ring_buffers(struct lio_device *lio_dev,
			    struct lio_droq *droq)
{
	struct lio_droq_desc *desc_ring = droq->desc_ring;
	uint32_t i;
	void *buf;

	for (i = 0; i < droq->max_count; i++) {
		buf = lio_recv_buffer_alloc(lio_dev, droq->q_no);
		if (buf == NULL) {
			lio_dev_err(lio_dev, "buffer alloc failed\n");
			lio_droq_destroy_ring_buffers(droq);
			return -ENOMEM;
		}

		droq->recv_buf_list[i].buffer = buf;
		droq->info_list[i].length = 0;

		/* map ring buffers into memory */
		desc_ring[i].info_ptr = lio_map_ring_info(droq, i);
		desc_ring[i].buffer_ptr =
			lio_map_ring(droq->recv_buf_list[i].buffer);
	}

	lio_droq_reset_indices(droq);

	lio_droq_compute_max_packet_bufs(droq);

	return 0;
}

static void
lio_dma_zone_free(struct lio_device *lio_dev, const struct rte_memzone *mz)
{
	const struct rte_memzone *mz_tmp;
	int ret = 0;

	if (mz == NULL) {
		lio_dev_err(lio_dev, "Memzone NULL\n");
		return;
	}

	mz_tmp = rte_memzone_lookup(mz->name);
	if (mz_tmp == NULL) {
		lio_dev_err(lio_dev, "Memzone %s Not Found\n", mz->name);
		return;
	}

	ret = rte_memzone_free(mz);
	if (ret)
		lio_dev_err(lio_dev, "Memzone free Failed ret %d\n", ret);
}

/**
 *  Frees the space for descriptor ring for the droq.
 *
 *  @param lio_dev	- pointer to the lio device structure
 *  @param q_no		- droq no.
 */
static void
lio_delete_droq(struct lio_device *lio_dev, uint32_t q_no)
{
	struct lio_droq *droq = lio_dev->droq[q_no];

	lio_dev_dbg(lio_dev, "OQ[%d]\n", q_no);

	lio_droq_destroy_ring_buffers(droq);
	rte_free(droq->recv_buf_list);
	droq->recv_buf_list = NULL;
	lio_dma_zone_free(lio_dev, droq->info_mz);
	lio_dma_zone_free(lio_dev, droq->desc_ring_mz);

	memset(droq, 0, LIO_DROQ_SIZE);
}

static void *
lio_alloc_info_buffer(struct lio_device *lio_dev,
		      struct lio_droq *droq, unsigned int socket_id)
{
	droq->info_mz = rte_eth_dma_zone_reserve(lio_dev->eth_dev,
						 "info_list", droq->q_no,
						 (droq->max_count *
							LIO_DROQ_INFO_SIZE),
						 RTE_CACHE_LINE_SIZE,
						 socket_id);

	if (droq->info_mz == NULL)
		return NULL;

	droq->info_list_dma = droq->info_mz->phys_addr;
	droq->info_alloc_size = droq->info_mz->len;
	droq->info_base_addr = (size_t)droq->info_mz->addr;

	return droq->info_mz->addr;
}

/**
 *  Allocates space for the descriptor ring for the droq and
 *  sets the base addr, num desc etc in Octeon registers.
 *
 * @param lio_dev	- pointer to the lio device structure
 * @param q_no		- droq no.
 * @param app_ctx	- pointer to application context
 * @return Success: 0	Failure: -1
 */
static int
lio_init_droq(struct lio_device *lio_dev, uint32_t q_no,
	      uint32_t num_descs, uint32_t desc_size,
	      struct rte_mempool *mpool, unsigned int socket_id)
{
	uint32_t c_refill_threshold;
	uint32_t desc_ring_size;
	struct lio_droq *droq;

	lio_dev_dbg(lio_dev, "OQ[%d]\n", q_no);

	droq = lio_dev->droq[q_no];
	droq->lio_dev = lio_dev;
	droq->q_no = q_no;
	droq->mpool = mpool;

	c_refill_threshold = LIO_OQ_REFILL_THRESHOLD_CFG(lio_dev);

	droq->max_count = num_descs;
	droq->buffer_size = desc_size;

	desc_ring_size = droq->max_count * LIO_DROQ_DESC_SIZE;
	droq->desc_ring_mz = rte_eth_dma_zone_reserve(lio_dev->eth_dev,
						      "droq", q_no,
						      desc_ring_size,
						      RTE_CACHE_LINE_SIZE,
						      socket_id);

	if (droq->desc_ring_mz == NULL) {
		lio_dev_err(lio_dev,
			    "Output queue %d ring alloc failed\n", q_no);
		return -1;
	}

	droq->desc_ring_dma = droq->desc_ring_mz->phys_addr;
	droq->desc_ring = (struct lio_droq_desc *)droq->desc_ring_mz->addr;

	lio_dev_dbg(lio_dev, "droq[%d]: desc_ring: virt: 0x%p, dma: %lx\n",
		    q_no, droq->desc_ring, (unsigned long)droq->desc_ring_dma);
	lio_dev_dbg(lio_dev, "droq[%d]: num_desc: %d\n", q_no,
		    droq->max_count);

	droq->info_list = lio_alloc_info_buffer(lio_dev, droq, socket_id);
	if (droq->info_list == NULL) {
		lio_dev_err(lio_dev, "Cannot allocate memory for info list.\n");
		goto init_droq_fail;
	}

	droq->recv_buf_list = rte_zmalloc_socket("recv_buf_list",
						 (droq->max_count *
							LIO_DROQ_RECVBUF_SIZE),
						 RTE_CACHE_LINE_SIZE,
						 socket_id);
	if (droq->recv_buf_list == NULL) {
		lio_dev_err(lio_dev,
			    "Output queue recv buf list alloc failed\n");
		goto init_droq_fail;
	}

	if (lio_droq_setup_ring_buffers(lio_dev, droq))
		goto init_droq_fail;

	droq->refill_threshold = c_refill_threshold;

	rte_spinlock_init(&droq->lock);

	lio_dev->fn_list.setup_oq_regs(lio_dev, q_no);

	lio_dev->io_qmask.oq |= (1ULL << q_no);

	return 0;

init_droq_fail:
	lio_delete_droq(lio_dev, q_no);

	return -1;
}

int
lio_setup_droq(struct lio_device *lio_dev, int oq_no, int num_descs,
	       int desc_size, struct rte_mempool *mpool, unsigned int socket_id)
{
	struct lio_droq *droq;

	PMD_INIT_FUNC_TRACE();

	if (lio_dev->droq[oq_no]) {
		lio_dev_dbg(lio_dev, "Droq %d in use\n", oq_no);
		return 0;
	}

	/* Allocate the DS for the new droq. */
	droq = rte_zmalloc_socket("ethdev RX queue", sizeof(*droq),
				  RTE_CACHE_LINE_SIZE, socket_id);
	if (droq == NULL)
		return -ENOMEM;

	lio_dev->droq[oq_no] = droq;

	/* Initialize the Droq */
	if (lio_init_droq(lio_dev, oq_no, num_descs, desc_size, mpool,
			  socket_id)) {
		lio_dev_err(lio_dev, "Droq[%u] Initialization Failed\n", oq_no);
		rte_free(lio_dev->droq[oq_no]);
		lio_dev->droq[oq_no] = NULL;
		return -ENOMEM;
	}

	lio_dev->num_oqs++;

	lio_dev_dbg(lio_dev, "Total number of OQ: %d\n", lio_dev->num_oqs);

	/* Send credit for octeon output queues. credits are always
	 * sent after the output queue is enabled.
	 */
	rte_write32(lio_dev->droq[oq_no]->max_count,
		    lio_dev->droq[oq_no]->pkts_credit_reg);
	rte_wmb();

	return 0;
}

static inline uint32_t
lio_droq_get_bufcount(uint32_t buf_size, uint32_t total_len)
{
	uint32_t buf_cnt = 0;

	while (total_len > (buf_size * buf_cnt))
		buf_cnt++;

	return buf_cnt;
}

/* If we were not able to refill all buffers, try to move around
 * the buffers that were not dispatched.
 */
static inline uint32_t
lio_droq_refill_pullup_descs(struct lio_droq *droq,
			     struct lio_droq_desc *desc_ring)
{
	uint32_t refill_index = droq->refill_idx;
	uint32_t desc_refilled = 0;

	while (refill_index != droq->read_idx) {
		if (droq->recv_buf_list[refill_index].buffer) {
			droq->recv_buf_list[droq->refill_idx].buffer =
				droq->recv_buf_list[refill_index].buffer;
			desc_ring[droq->refill_idx].buffer_ptr =
				desc_ring[refill_index].buffer_ptr;
			droq->recv_buf_list[refill_index].buffer = NULL;
			desc_ring[refill_index].buffer_ptr = 0;
			do {
				droq->refill_idx = lio_incr_index(
							droq->refill_idx, 1,
							droq->max_count);
				desc_refilled++;
				droq->refill_count--;
			} while (droq->recv_buf_list[droq->refill_idx].buffer);
		}
		refill_index = lio_incr_index(refill_index, 1,
					      droq->max_count);
	}	/* while */

	return desc_refilled;
}

/* lio_droq_refill
 *
 * @param lio_dev	- pointer to the lio device structure
 * @param droq		- droq in which descriptors require new buffers.
 *
 * Description:
 *  Called during normal DROQ processing in interrupt mode or by the poll
 *  thread to refill the descriptors from which buffers were dispatched
 *  to upper layers. Attempts to allocate new buffers. If that fails, moves
 *  up buffers (that were not dispatched) to form a contiguous ring.
 *
 * Returns:
 *  No of descriptors refilled.
 *
 * Locks:
 * This routine is called with droq->lock held.
 */
static uint32_t
lio_droq_refill(struct lio_device *lio_dev, struct lio_droq *droq)
{
	struct lio_droq_desc *desc_ring;
	uint32_t desc_refilled = 0;
	void *buf = NULL;

	desc_ring = droq->desc_ring;

	while (droq->refill_count && (desc_refilled < droq->max_count)) {
		/* If a valid buffer exists (happens if there is no dispatch),
		 * reuse the buffer, else allocate.
		 */
		if (droq->recv_buf_list[droq->refill_idx].buffer == NULL) {
			buf = lio_recv_buffer_alloc(lio_dev, droq->q_no);
			/* If a buffer could not be allocated, no point in
			 * continuing
			 */
			if (buf == NULL)
				break;

			droq->recv_buf_list[droq->refill_idx].buffer = buf;
		}

		desc_ring[droq->refill_idx].buffer_ptr =
		    lio_map_ring(droq->recv_buf_list[droq->refill_idx].buffer);
		/* Reset any previous values in the length field. */
		droq->info_list[droq->refill_idx].length = 0;

		droq->refill_idx = lio_incr_index(droq->refill_idx, 1,
						  droq->max_count);
		desc_refilled++;
		droq->refill_count--;
	}

	if (droq->refill_count)
		desc_refilled += lio_droq_refill_pullup_descs(droq, desc_ring);

	/* if droq->refill_count
	 * The refill count would not change in pass two. We only moved buffers
	 * to close the gap in the ring, but we would still have the same no. of
	 * buffers to refill.
	 */
	return desc_refilled;
}

static int
lio_droq_fast_process_packet(struct lio_device *lio_dev,
			     struct lio_droq *droq,
			     struct rte_mbuf **rx_pkts)
{
	struct rte_mbuf *nicbuf = NULL;
	struct lio_droq_info *info;
	uint32_t total_len = 0;
	int data_total_len = 0;
	uint32_t pkt_len = 0;
	union octeon_rh *rh;
	int data_pkts = 0;

	info = &droq->info_list[droq->read_idx];
	lio_swap_8B_data((uint64_t *)info, 2);

	if (!info->length)
		return -1;

	/* Len of resp hdr in included in the received data len. */
	info->length -= OCTEON_RH_SIZE;
	rh = &info->rh;

	total_len += (uint32_t)info->length;

	if (lio_opcode_slow_path(rh)) {
		uint32_t buf_cnt;

		buf_cnt = lio_droq_get_bufcount(droq->buffer_size,
						(uint32_t)info->length);
		droq->read_idx = lio_incr_index(droq->read_idx, buf_cnt,
						droq->max_count);
		droq->refill_count += buf_cnt;
	} else {
		if (info->length <= droq->buffer_size) {
			if (rh->r_dh.has_hash)
				pkt_len = (uint32_t)(info->length - 8);
			else
				pkt_len = (uint32_t)info->length;

			nicbuf = droq->recv_buf_list[droq->read_idx].buffer;
			droq->recv_buf_list[droq->read_idx].buffer = NULL;
			droq->read_idx = lio_incr_index(
						droq->read_idx, 1,
						droq->max_count);
			droq->refill_count++;

			if (likely(nicbuf != NULL)) {
				nicbuf->data_off = RTE_PKTMBUF_HEADROOM;
				nicbuf->nb_segs = 1;
				nicbuf->next = NULL;
				/* We don't have a way to pass flags yet */
				nicbuf->ol_flags = 0;
				if (rh->r_dh.has_hash) {
					uint64_t *hash_ptr;

					nicbuf->ol_flags |= PKT_RX_RSS_HASH;
					hash_ptr = rte_pktmbuf_mtod(nicbuf,
								    uint64_t *);
					lio_swap_8B_data(hash_ptr, 1);
					nicbuf->hash.rss = (uint32_t)*hash_ptr;
					nicbuf->data_off += 8;
				}

				nicbuf->pkt_len = pkt_len;
				nicbuf->data_len = pkt_len;
				nicbuf->port = lio_dev->port_id;
				/* Store the mbuf */
				rx_pkts[data_pkts++] = nicbuf;
				data_total_len += pkt_len;
			}

			/* Prefetch buffer pointers when on a cache line
			 * boundary
			 */
			if ((droq->read_idx & 3) == 0) {
				rte_prefetch0(
				    &droq->recv_buf_list[droq->read_idx]);
				rte_prefetch0(
				    &droq->info_list[droq->read_idx]);
			}
		} else {
			struct rte_mbuf *first_buf = NULL;
			struct rte_mbuf *last_buf = NULL;

			while (pkt_len < info->length) {
				int cpy_len = 0;

				cpy_len = ((pkt_len + droq->buffer_size) >
						info->length)
						? ((uint32_t)info->length -
							pkt_len)
						: droq->buffer_size;

				nicbuf =
				    droq->recv_buf_list[droq->read_idx].buffer;
				droq->recv_buf_list[droq->read_idx].buffer =
				    NULL;

				if (likely(nicbuf != NULL)) {
					/* Note the first seg */
					if (!pkt_len)
						first_buf = nicbuf;

					nicbuf->data_off = RTE_PKTMBUF_HEADROOM;
					nicbuf->nb_segs = 1;
					nicbuf->next = NULL;
					nicbuf->port = lio_dev->port_id;
					/* We don't have a way to pass
					 * flags yet
					 */
					nicbuf->ol_flags = 0;
					if ((!pkt_len) && (rh->r_dh.has_hash)) {
						uint64_t *hash_ptr;

						nicbuf->ol_flags |=
						    PKT_RX_RSS_HASH;
						hash_ptr = rte_pktmbuf_mtod(
						    nicbuf, uint64_t *);
						lio_swap_8B_data(hash_ptr, 1);
						nicbuf->hash.rss =
						    (uint32_t)*hash_ptr;
						nicbuf->data_off += 8;
						nicbuf->pkt_len = cpy_len - 8;
						nicbuf->data_len = cpy_len - 8;
					} else {
						nicbuf->pkt_len = cpy_len;
						nicbuf->data_len = cpy_len;
					}

					if (pkt_len)
						first_buf->nb_segs++;

					if (last_buf)
						last_buf->next = nicbuf;

					last_buf = nicbuf;
				} else {
					PMD_RX_LOG(lio_dev, ERR, "no buf\n");
				}

				pkt_len += cpy_len;
				droq->read_idx = lio_incr_index(
							droq->read_idx,
							1, droq->max_count);
				droq->refill_count++;

				/* Prefetch buffer pointers when on a
				 * cache line boundary
				 */
				if ((droq->read_idx & 3) == 0) {
					rte_prefetch0(&droq->recv_buf_list
							      [droq->read_idx]);

					rte_prefetch0(
					    &droq->info_list[droq->read_idx]);
				}
			}
			rx_pkts[data_pkts++] = first_buf;
			if (rh->r_dh.has_hash)
				data_total_len += (pkt_len - 8);
			else
				data_total_len += pkt_len;
		}

		/* Inform upper layer about packet checksum verification */
		struct rte_mbuf *m = rx_pkts[data_pkts - 1];

		if (rh->r_dh.csum_verified & LIO_IP_CSUM_VERIFIED)
			m->ol_flags |= PKT_RX_IP_CKSUM_GOOD;

		if (rh->r_dh.csum_verified & LIO_L4_CSUM_VERIFIED)
			m->ol_flags |= PKT_RX_L4_CKSUM_GOOD;
	}

	if (droq->refill_count >= droq->refill_threshold) {
		int desc_refilled = lio_droq_refill(lio_dev, droq);

		/* Flush the droq descriptor data to memory to be sure
		 * that when we update the credits the data in memory is
		 * accurate.
		 */
		rte_wmb();
		rte_write32(desc_refilled, droq->pkts_credit_reg);
		/* make sure mmio write completes */
		rte_wmb();
	}

	info->length = 0;
	info->rh.rh64 = 0;

	return data_pkts;
}

static uint32_t
lio_droq_fast_process_packets(struct lio_device *lio_dev,
			      struct lio_droq *droq,
			      struct rte_mbuf **rx_pkts,
			      uint32_t pkts_to_process)
{
	int ret, data_pkts = 0;
	uint32_t pkt;

	for (pkt = 0; pkt < pkts_to_process; pkt++) {
		ret = lio_droq_fast_process_packet(lio_dev, droq,
						   &rx_pkts[data_pkts]);
		if (ret < 0) {
			lio_dev_err(lio_dev, "Port[%d] DROQ[%d] idx: %d len:0, pkt_cnt: %d\n",
				    lio_dev->port_id, droq->q_no,
				    droq->read_idx, pkts_to_process);
			break;
		}
		data_pkts += ret;
	}

	rte_atomic64_sub(&droq->pkts_pending, pkt);

	return data_pkts;
}

static inline uint32_t
lio_droq_check_hw_for_pkts(struct lio_droq *droq)
{
	uint32_t last_count;
	uint32_t pkt_count;

	pkt_count = rte_read32(droq->pkts_sent_reg);

	last_count = pkt_count - droq->pkt_count;
	droq->pkt_count = pkt_count;

	if (last_count)
		rte_atomic64_add(&droq->pkts_pending, last_count);

	return last_count;
}

uint16_t
lio_dev_recv_pkts(void *rx_queue,
		  struct rte_mbuf **rx_pkts,
		  uint16_t budget)
{
	struct lio_droq *droq = rx_queue;
	struct lio_device *lio_dev = droq->lio_dev;
	uint32_t pkts_processed = 0;
	uint32_t pkt_count = 0;

	lio_droq_check_hw_for_pkts(droq);

	pkt_count = rte_atomic64_read(&droq->pkts_pending);
	if (!pkt_count)
		return 0;

	if (pkt_count > budget)
		pkt_count = budget;

	/* Grab the lock */
	rte_spinlock_lock(&droq->lock);
	pkts_processed = lio_droq_fast_process_packets(lio_dev,
						       droq, rx_pkts,
						       pkt_count);

	if (droq->pkt_count) {
		rte_write32(droq->pkt_count, droq->pkts_sent_reg);
		droq->pkt_count = 0;
	}

	/* Release the spin lock */
	rte_spinlock_unlock(&droq->lock);

	return pkts_processed;
}

void
lio_delete_droq_queue(struct lio_device *lio_dev,
		      int oq_no)
{
	lio_delete_droq(lio_dev, oq_no);
	lio_dev->num_oqs--;
	rte_free(lio_dev->droq[oq_no]);
	lio_dev->droq[oq_no] = NULL;
}

/**
 *  lio_init_instr_queue()
 *  @param lio_dev	- pointer to the lio device structure.
 *  @param txpciq	- queue to be initialized.
 *
 *  Called at driver init time for each input queue. iq_conf has the
 *  configuration parameters for the queue.
 *
 *  @return  Success: 0	Failure: -1
 */
static int
lio_init_instr_queue(struct lio_device *lio_dev,
		     union octeon_txpciq txpciq,
		     uint32_t num_descs, unsigned int socket_id)
{
	uint32_t iq_no = (uint32_t)txpciq.s.q_no;
	struct lio_instr_queue *iq;
	uint32_t instr_type;
	uint32_t q_size;

	instr_type = LIO_IQ_INSTR_TYPE(lio_dev);

	q_size = instr_type * num_descs;
	iq = lio_dev->instr_queue[iq_no];
	iq->iq_mz = rte_eth_dma_zone_reserve(lio_dev->eth_dev,
					     "instr_queue", iq_no, q_size,
					     RTE_CACHE_LINE_SIZE,
					     socket_id);
	if (iq->iq_mz == NULL) {
		lio_dev_err(lio_dev, "Cannot allocate memory for instr queue %d\n",
			    iq_no);
		return -1;
	}

	iq->base_addr_dma = iq->iq_mz->phys_addr;
	iq->base_addr = (uint8_t *)iq->iq_mz->addr;

	iq->max_count = num_descs;

	/* Initialize a list to holds requests that have been posted to Octeon
	 * but has yet to be fetched by octeon
	 */
	iq->request_list = rte_zmalloc_socket("request_list",
					      sizeof(*iq->request_list) *
							num_descs,
					      RTE_CACHE_LINE_SIZE,
					      socket_id);
	if (iq->request_list == NULL) {
		lio_dev_err(lio_dev, "Alloc failed for IQ[%d] nr free list\n",
			    iq_no);
		lio_dma_zone_free(lio_dev, iq->iq_mz);
		return -1;
	}

	lio_dev_dbg(lio_dev, "IQ[%d]: base: %p basedma: %lx count: %d\n",
		    iq_no, iq->base_addr, (unsigned long)iq->base_addr_dma,
		    iq->max_count);

	iq->lio_dev = lio_dev;
	iq->txpciq.txpciq64 = txpciq.txpciq64;
	iq->fill_cnt = 0;
	iq->host_write_index = 0;
	iq->lio_read_index = 0;
	iq->flush_index = 0;

	rte_atomic64_set(&iq->instr_pending, 0);

	/* Initialize the spinlock for this instruction queue */
	rte_spinlock_init(&iq->lock);
	rte_spinlock_init(&iq->post_lock);

	rte_atomic64_clear(&iq->iq_flush_running);

	lio_dev->io_qmask.iq |= (1ULL << iq_no);

	/* Set the 32B/64B mode for each input queue */
	lio_dev->io_qmask.iq64B |= ((instr_type == 64) << iq_no);
	iq->iqcmd_64B = (instr_type == 64);

	lio_dev->fn_list.setup_iq_regs(lio_dev, iq_no);

	return 0;
}

int
lio_setup_instr_queue0(struct lio_device *lio_dev)
{
	union octeon_txpciq txpciq;
	uint32_t num_descs = 0;
	uint32_t iq_no = 0;

	num_descs = LIO_NUM_DEF_TX_DESCS_CFG(lio_dev);

	lio_dev->num_iqs = 0;

	lio_dev->instr_queue[0] = rte_zmalloc(NULL,
					sizeof(struct lio_instr_queue), 0);
	if (lio_dev->instr_queue[0] == NULL)
		return -ENOMEM;

	lio_dev->instr_queue[0]->q_index = 0;
	lio_dev->instr_queue[0]->app_ctx = (void *)(size_t)0;
	txpciq.txpciq64 = 0;
	txpciq.s.q_no = iq_no;
	txpciq.s.pkind = lio_dev->pfvf_hsword.pkind;
	txpciq.s.use_qpg = 0;
	txpciq.s.qpg = 0;
	if (lio_init_instr_queue(lio_dev, txpciq, num_descs, SOCKET_ID_ANY)) {
		rte_free(lio_dev->instr_queue[0]);
		lio_dev->instr_queue[0] = NULL;
		return -1;
	}

	lio_dev->num_iqs++;

	return 0;
}

/**
 *  lio_delete_instr_queue()
 *  @param lio_dev	- pointer to the lio device structure.
 *  @param iq_no	- queue to be deleted.
 *
 *  Called at driver unload time for each input queue. Deletes all
 *  allocated resources for the input queue.
 */
static void
lio_delete_instr_queue(struct lio_device *lio_dev, uint32_t iq_no)
{
	struct lio_instr_queue *iq = lio_dev->instr_queue[iq_no];

	rte_free(iq->request_list);
	iq->request_list = NULL;
	lio_dma_zone_free(lio_dev, iq->iq_mz);
}

void
lio_free_instr_queue0(struct lio_device *lio_dev)
{
	lio_delete_instr_queue(lio_dev, 0);
	rte_free(lio_dev->instr_queue[0]);
	lio_dev->instr_queue[0] = NULL;
	lio_dev->num_iqs--;
}

/* Return 0 on success, -1 on failure */
int
lio_setup_iq(struct lio_device *lio_dev, int q_index,
	     union octeon_txpciq txpciq, uint32_t num_descs, void *app_ctx,
	     unsigned int socket_id)
{
	uint32_t iq_no = (uint32_t)txpciq.s.q_no;

	if (lio_dev->instr_queue[iq_no]) {
		lio_dev_dbg(lio_dev, "IQ is in use. Cannot create the IQ: %d again\n",
			    iq_no);
		lio_dev->instr_queue[iq_no]->txpciq.txpciq64 = txpciq.txpciq64;
		lio_dev->instr_queue[iq_no]->app_ctx = app_ctx;
		return 0;
	}

	lio_dev->instr_queue[iq_no] = rte_zmalloc_socket("ethdev TX queue",
						sizeof(struct lio_instr_queue),
						RTE_CACHE_LINE_SIZE, socket_id);
	if (lio_dev->instr_queue[iq_no] == NULL)
		return -1;

	lio_dev->instr_queue[iq_no]->q_index = q_index;
	lio_dev->instr_queue[iq_no]->app_ctx = app_ctx;

	if (lio_init_instr_queue(lio_dev, txpciq, num_descs, socket_id))
		goto release_lio_iq;

	lio_dev->num_iqs++;
	if (lio_dev->fn_list.enable_io_queues(lio_dev))
		goto delete_lio_iq;

	return 0;

delete_lio_iq:
	lio_delete_instr_queue(lio_dev, iq_no);
	lio_dev->num_iqs--;
release_lio_iq:
	rte_free(lio_dev->instr_queue[iq_no]);
	lio_dev->instr_queue[iq_no] = NULL;

	return -1;
}

static inline void
lio_ring_doorbell(struct lio_device *lio_dev,
		  struct lio_instr_queue *iq)
{
	if (rte_atomic64_read(&lio_dev->status) == LIO_DEV_RUNNING) {
		rte_write32(iq->fill_cnt, iq->doorbell_reg);
		/* make sure doorbell write goes through */
		rte_wmb();
		iq->fill_cnt = 0;
	}
}

static inline void
copy_cmd_into_iq(struct lio_instr_queue *iq, uint8_t *cmd)
{
	uint8_t *iqptr, cmdsize;

	cmdsize = ((iq->iqcmd_64B) ? 64 : 32);
	iqptr = iq->base_addr + (cmdsize * iq->host_write_index);

	rte_memcpy(iqptr, cmd, cmdsize);
}

static inline struct lio_iq_post_status
post_command2(struct lio_instr_queue *iq, uint8_t *cmd)
{
	struct lio_iq_post_status st;

	st.status = LIO_IQ_SEND_OK;

	/* This ensures that the read index does not wrap around to the same
	 * position if queue gets full before Octeon could fetch any instr.
	 */
	if (rte_atomic64_read(&iq->instr_pending) >=
			(int32_t)(iq->max_count - 1)) {
		st.status = LIO_IQ_SEND_FAILED;
		st.index = -1;
		return st;
	}

	if (rte_atomic64_read(&iq->instr_pending) >=
			(int32_t)(iq->max_count - 2))
		st.status = LIO_IQ_SEND_STOP;

	copy_cmd_into_iq(iq, cmd);

	/* "index" is returned, host_write_index is modified. */
	st.index = iq->host_write_index;
	iq->host_write_index = lio_incr_index(iq->host_write_index, 1,
					      iq->max_count);
	iq->fill_cnt++;

	/* Flush the command into memory. We need to be sure the data is in
	 * memory before indicating that the instruction is pending.
	 */
	rte_wmb();

	rte_atomic64_inc(&iq->instr_pending);

	return st;
}

static inline void
lio_add_to_request_list(struct lio_instr_queue *iq,
			int idx, void *buf, int reqtype)
{
	iq->request_list[idx].buf = buf;
	iq->request_list[idx].reqtype = reqtype;
}

static int
lio_send_command(struct lio_device *lio_dev, uint32_t iq_no, void *cmd,
		 void *buf, uint32_t datasize __rte_unused, uint32_t reqtype)
{
	struct lio_instr_queue *iq = lio_dev->instr_queue[iq_no];
	struct lio_iq_post_status st;

	rte_spinlock_lock(&iq->post_lock);

	st = post_command2(iq, cmd);

	if (st.status != LIO_IQ_SEND_FAILED) {
		lio_add_to_request_list(iq, st.index, buf, reqtype);
		lio_ring_doorbell(lio_dev, iq);
	}

	rte_spinlock_unlock(&iq->post_lock);

	return st.status;
}

void
lio_prepare_soft_command(struct lio_device *lio_dev,
			 struct lio_soft_command *sc, uint8_t opcode,
			 uint8_t subcode, uint32_t irh_ossp, uint64_t ossp0,
			 uint64_t ossp1)
{
	struct octeon_instr_pki_ih3 *pki_ih3;
	struct octeon_instr_ih3 *ih3;
	struct octeon_instr_irh *irh;
	struct octeon_instr_rdp *rdp;

	RTE_ASSERT(opcode <= 15);
	RTE_ASSERT(subcode <= 127);

	ih3	  = (struct octeon_instr_ih3 *)&sc->cmd.cmd3.ih3;

	ih3->pkind = lio_dev->instr_queue[sc->iq_no]->txpciq.s.pkind;

	pki_ih3 = (struct octeon_instr_pki_ih3 *)&sc->cmd.cmd3.pki_ih3;

	pki_ih3->w	= 1;
	pki_ih3->raw	= 1;
	pki_ih3->utag	= 1;
	pki_ih3->uqpg	= lio_dev->instr_queue[sc->iq_no]->txpciq.s.use_qpg;
	pki_ih3->utt	= 1;

	pki_ih3->tag	= LIO_CONTROL;
	pki_ih3->tagtype = OCTEON_ATOMIC_TAG;
	pki_ih3->qpg	= lio_dev->instr_queue[sc->iq_no]->txpciq.s.qpg;
	pki_ih3->pm	= 0x7;
	pki_ih3->sl	= 8;

	if (sc->datasize)
		ih3->dlengsz = sc->datasize;

	irh		= (struct octeon_instr_irh *)&sc->cmd.cmd3.irh;
	irh->opcode	= opcode;
	irh->subcode	= subcode;

	/* opcode/subcode specific parameters (ossp) */
	irh->ossp = irh_ossp;
	sc->cmd.cmd3.ossp[0] = ossp0;
	sc->cmd.cmd3.ossp[1] = ossp1;

	if (sc->rdatasize) {
		rdp = (struct octeon_instr_rdp *)&sc->cmd.cmd3.rdp;
		rdp->pcie_port = lio_dev->pcie_port;
		rdp->rlen      = sc->rdatasize;
		irh->rflag = 1;
		/* PKI IH3 */
		ih3->fsz    = OCTEON_SOFT_CMD_RESP_IH3;
	} else {
		irh->rflag = 0;
		/* PKI IH3 */
		ih3->fsz    = OCTEON_PCI_CMD_O3;
	}
}

int
lio_send_soft_command(struct lio_device *lio_dev,
		      struct lio_soft_command *sc)
{
	struct octeon_instr_ih3 *ih3;
	struct octeon_instr_irh *irh;
	uint32_t len = 0;

	ih3 = (struct octeon_instr_ih3 *)&sc->cmd.cmd3.ih3;
	if (ih3->dlengsz) {
		RTE_ASSERT(sc->dmadptr);
		sc->cmd.cmd3.dptr = sc->dmadptr;
	}

	irh = (struct octeon_instr_irh *)&sc->cmd.cmd3.irh;
	if (irh->rflag) {
		RTE_ASSERT(sc->dmarptr);
		RTE_ASSERT(sc->status_word != NULL);
		*sc->status_word = LIO_COMPLETION_WORD_INIT;
		sc->cmd.cmd3.rptr = sc->dmarptr;
	}

	len = (uint32_t)ih3->dlengsz;

	if (sc->wait_time)
		sc->timeout = lio_uptime + sc->wait_time;

	return lio_send_command(lio_dev, sc->iq_no, &sc->cmd, sc, len,
				LIO_REQTYPE_SOFT_COMMAND);
}

int
lio_setup_sc_buffer_pool(struct lio_device *lio_dev)
{
	char sc_pool_name[RTE_MEMPOOL_NAMESIZE];
	uint16_t buf_size;

	buf_size = LIO_SOFT_COMMAND_BUFFER_SIZE + RTE_PKTMBUF_HEADROOM;
	snprintf(sc_pool_name, sizeof(sc_pool_name),
		 "lio_sc_pool_%u", lio_dev->port_id);
	lio_dev->sc_buf_pool = rte_pktmbuf_pool_create(sc_pool_name,
						LIO_MAX_SOFT_COMMAND_BUFFERS,
						0, 0, buf_size, SOCKET_ID_ANY);
	return 0;
}

void
lio_free_sc_buffer_pool(struct lio_device *lio_dev)
{
	rte_mempool_free(lio_dev->sc_buf_pool);
}

struct lio_soft_command *
lio_alloc_soft_command(struct lio_device *lio_dev, uint32_t datasize,
		       uint32_t rdatasize, uint32_t ctxsize)
{
	uint32_t offset = sizeof(struct lio_soft_command);
	struct lio_soft_command *sc;
	struct rte_mbuf *m;
	uint64_t dma_addr;

	RTE_ASSERT((offset + datasize + rdatasize + ctxsize) <=
		   LIO_SOFT_COMMAND_BUFFER_SIZE);

	m = rte_pktmbuf_alloc(lio_dev->sc_buf_pool);
	if (m == NULL) {
		lio_dev_err(lio_dev, "Cannot allocate mbuf for sc\n");
		return NULL;
	}

	/* set rte_mbuf data size and there is only 1 segment */
	m->pkt_len = LIO_SOFT_COMMAND_BUFFER_SIZE;
	m->data_len = LIO_SOFT_COMMAND_BUFFER_SIZE;

	/* use rte_mbuf buffer for soft command */
	sc = rte_pktmbuf_mtod(m, struct lio_soft_command *);
	memset(sc, 0, LIO_SOFT_COMMAND_BUFFER_SIZE);
	sc->size = LIO_SOFT_COMMAND_BUFFER_SIZE;
	sc->dma_addr = rte_mbuf_data_dma_addr(m);
	sc->mbuf = m;

	dma_addr = sc->dma_addr;

	if (ctxsize) {
		sc->ctxptr = (uint8_t *)sc + offset;
		sc->ctxsize = ctxsize;
	}

	/* Start data at 128 byte boundary */
	offset = (offset + ctxsize + 127) & 0xffffff80;

	if (datasize) {
		sc->virtdptr = (uint8_t *)sc + offset;
		sc->dmadptr = dma_addr + offset;
		sc->datasize = datasize;
	}

	/* Start rdata at 128 byte boundary */
	offset = (offset + datasize + 127) & 0xffffff80;

	if (rdatasize) {
		RTE_ASSERT(rdatasize >= 16);
		sc->virtrptr = (uint8_t *)sc + offset;
		sc->dmarptr = dma_addr + offset;
		sc->rdatasize = rdatasize;
		sc->status_word = (uint64_t *)((uint8_t *)(sc->virtrptr) +
					       rdatasize - 8);
	}

	return sc;
}

void
lio_free_soft_command(struct lio_soft_command *sc)
{
	rte_pktmbuf_free(sc->mbuf);
}

void
lio_setup_response_list(struct lio_device *lio_dev)
{
	STAILQ_INIT(&lio_dev->response_list.head);
	rte_spinlock_init(&lio_dev->response_list.lock);
	rte_atomic64_set(&lio_dev->response_list.pending_req_count, 0);
}

int
lio_process_ordered_list(struct lio_device *lio_dev)
{
	int resp_to_process = LIO_MAX_ORD_REQS_TO_PROCESS;
	struct lio_response_list *ordered_sc_list;
	struct lio_soft_command *sc;
	int request_complete = 0;
	uint64_t status64;
	uint32_t status;

	ordered_sc_list = &lio_dev->response_list;

	do {
		rte_spinlock_lock(&ordered_sc_list->lock);

		if (STAILQ_EMPTY(&ordered_sc_list->head)) {
			/* ordered_sc_list is empty; there is
			 * nothing to process
			 */
			rte_spinlock_unlock(&ordered_sc_list->lock);
			return -1;
		}

		sc = LIO_STQUEUE_FIRST_ENTRY(&ordered_sc_list->head,
					     struct lio_soft_command, node);

		status = LIO_REQUEST_PENDING;

		/* check if octeon has finished DMA'ing a response
		 * to where rptr is pointing to
		 */
		status64 = *sc->status_word;

		if (status64 != LIO_COMPLETION_WORD_INIT) {
			/* This logic ensures that all 64b have been written.
			 * 1. check byte 0 for non-FF
			 * 2. if non-FF, then swap result from BE to host order
			 * 3. check byte 7 (swapped to 0) for non-FF
			 * 4. if non-FF, use the low 32-bit status code
			 * 5. if either byte 0 or byte 7 is FF, don't use status
			 */
			if ((status64 & 0xff) != 0xff) {
				lio_swap_8B_data(&status64, 1);
				if (((status64 & 0xff) != 0xff)) {
					/* retrieve 16-bit firmware status */
					status = (uint32_t)(status64 &
							    0xffffULL);
					if (status) {
						status =
						LIO_FIRMWARE_STATUS_CODE(
									status);
					} else {
						/* i.e. no error */
						status = LIO_REQUEST_DONE;
					}
				}
			}
		} else if ((sc->timeout && lio_check_timeout(lio_uptime,
							     sc->timeout))) {
			lio_dev_err(lio_dev,
				    "cmd failed, timeout (%ld, %ld)\n",
				    (long)lio_uptime, (long)sc->timeout);
			status = LIO_REQUEST_TIMEOUT;
		}

		if (status != LIO_REQUEST_PENDING) {
			/* we have received a response or we have timed out.
			 * remove node from linked list
			 */
			STAILQ_REMOVE(&ordered_sc_list->head,
				      &sc->node, lio_stailq_node, entries);
			rte_atomic64_dec(
			    &lio_dev->response_list.pending_req_count);
			rte_spinlock_unlock(&ordered_sc_list->lock);

			if (sc->callback)
				sc->callback(status, sc->callback_arg);

			request_complete++;
		} else {
			/* no response yet */
			request_complete = 0;
			rte_spinlock_unlock(&ordered_sc_list->lock);
		}

		/* If we hit the Max Ordered requests to process every loop,
		 * we quit and let this function be invoked the next time
		 * the poll thread runs to process the remaining requests.
		 * This function can take up the entire CPU if there is
		 * no upper limit to the requests processed.
		 */
		if (request_complete >= resp_to_process)
			break;
	} while (request_complete);

	return 0;
}

static inline struct lio_stailq_node *
list_delete_first_node(struct lio_stailq_head *head)
{
	struct lio_stailq_node *node;

	if (STAILQ_EMPTY(head))
		node = NULL;
	else
		node = STAILQ_FIRST(head);

	if (node)
		STAILQ_REMOVE(head, node, lio_stailq_node, entries);

	return node;
}

static void
lio_delete_sglist(struct lio_instr_queue *txq)
{
	struct lio_device *lio_dev = txq->lio_dev;
	int iq_no = txq->q_index;
	struct lio_gather *g;

	if (lio_dev->glist_head == NULL)
		return;

	do {
		g = (struct lio_gather *)list_delete_first_node(
						&lio_dev->glist_head[iq_no]);
		if (g) {
			if (g->sg)
				rte_free(
				    (void *)((unsigned long)g->sg - g->adjust));
			rte_free(g);
		}
	} while (g);
}

/**
 * \brief Setup gather lists
 * @param lio per-network private data
 */
int
lio_setup_sglists(struct lio_device *lio_dev, int iq_no,
		  int fw_mapped_iq, int num_descs, unsigned int socket_id)
{
	struct lio_gather *g;
	int i;

	rte_spinlock_init(&lio_dev->glist_lock[iq_no]);

	STAILQ_INIT(&lio_dev->glist_head[iq_no]);

	for (i = 0; i < num_descs; i++) {
		g = rte_zmalloc_socket(NULL, sizeof(*g), RTE_CACHE_LINE_SIZE,
				       socket_id);
		if (g == NULL) {
			lio_dev_err(lio_dev,
				    "lio_gather memory allocation failed for qno %d\n",
				    iq_no);
			break;
		}

		g->sg_size =
		    ((ROUNDUP4(LIO_MAX_SG) >> 2) * LIO_SG_ENTRY_SIZE);

		g->sg = rte_zmalloc_socket(NULL, g->sg_size + 8,
					   RTE_CACHE_LINE_SIZE, socket_id);
		if (g->sg == NULL) {
			lio_dev_err(lio_dev,
				    "sg list memory allocation failed for qno %d\n",
				    iq_no);
			rte_free(g);
			break;
		}

		/* The gather component should be aligned on 64-bit boundary */
		if (((unsigned long)g->sg) & 7) {
			g->adjust = 8 - (((unsigned long)g->sg) & 7);
			g->sg =
			    (struct lio_sg_entry *)((unsigned long)g->sg +
						       g->adjust);
		}

		STAILQ_INSERT_TAIL(&lio_dev->glist_head[iq_no], &g->list,
				   entries);
	}

	if (i != num_descs) {
		lio_delete_sglist(lio_dev->instr_queue[fw_mapped_iq]);
		return -ENOMEM;
	}

	return 0;
}

void
lio_delete_instruction_queue(struct lio_device *lio_dev, int iq_no)
{
	lio_delete_instr_queue(lio_dev, iq_no);
	rte_free(lio_dev->instr_queue[iq_no]);
	lio_dev->instr_queue[iq_no] = NULL;
	lio_dev->num_iqs--;
}

/** Send data packet to the device
 *  @param lio_dev - lio device pointer
 *  @param ndata   - control structure with queueing, and buffer information
 *
 *  @returns IQ_FAILED if it failed to add to the input queue. IQ_STOP if it the
 *  queue should be stopped, and LIO_IQ_SEND_OK if it sent okay.
 */
static inline int
lio_send_data_pkt(struct lio_device *lio_dev, struct lio_data_pkt *ndata)
{
	return lio_send_command(lio_dev, ndata->q_no, &ndata->cmd,
				ndata->buf, ndata->datasize, ndata->reqtype);
}

uint16_t
lio_dev_xmit_pkts(void *tx_queue, struct rte_mbuf **pkts, uint16_t nb_pkts)
{
	struct lio_instr_queue *txq = tx_queue;
	union lio_cmd_setup cmdsetup;
	struct lio_device *lio_dev;
	struct lio_data_pkt ndata;
	int i, processed = 0;
	struct rte_mbuf *m;
	uint32_t tag = 0;
	int status = 0;
	int iq_no;

	lio_dev = txq->lio_dev;
	iq_no = txq->txpciq.s.q_no;

	if (!lio_dev->linfo.link.s.link_up) {
		PMD_TX_LOG(lio_dev, ERR, "Transmit failed link_status : %d\n",
			   lio_dev->linfo.link.s.link_up);
		goto xmit_failed;
	}

	for (i = 0; i < nb_pkts; i++) {
		uint32_t pkt_len = 0;

		m = pkts[i];

		/* Prepare the attributes for the data to be passed to BASE. */
		memset(&ndata, 0, sizeof(struct lio_data_pkt));

		ndata.buf = m;

		ndata.q_no = iq_no;

		cmdsetup.cmd_setup64 = 0;
		cmdsetup.s.iq_no = iq_no;

		/* check checksum offload flags to form cmd */
		if (m->ol_flags & PKT_TX_IP_CKSUM)
			cmdsetup.s.ip_csum = 1;

		if ((m->ol_flags & PKT_TX_TCP_CKSUM) ||
				(m->ol_flags & PKT_TX_UDP_CKSUM))
			cmdsetup.s.transport_csum = 1;

		if (m->nb_segs == 1) {
			pkt_len = rte_pktmbuf_data_len(m);
			cmdsetup.s.u.datasize = pkt_len;
			lio_prepare_pci_cmd(lio_dev, &ndata.cmd,
					    &cmdsetup, tag);
			ndata.cmd.cmd3.dptr = rte_mbuf_data_dma_addr(m);
			ndata.reqtype = LIO_REQTYPE_NORESP_NET;
		}

		ndata.datasize = pkt_len;

		status = lio_send_data_pkt(lio_dev, &ndata);

		if (unlikely(status == LIO_IQ_SEND_FAILED)) {
			PMD_TX_LOG(lio_dev, ERR, "send failed\n");
			break;
		}

		if (unlikely(status == LIO_IQ_SEND_STOP))
			PMD_TX_LOG(lio_dev, DEBUG, "iq full\n");

		processed++;
	}

xmit_failed:

	return processed;
}

