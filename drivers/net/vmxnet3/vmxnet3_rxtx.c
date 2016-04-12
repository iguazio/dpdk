/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2015 Intel Corporation. All rights reserved.
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <inttypes.h>

#include <rte_byteorder.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_log.h>
#include <rte_debug.h>
#include <rte_interrupts.h>
#include <rte_pci.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_prefetch.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_tcp.h>
#include <rte_sctp.h>
#include <rte_string_fns.h>
#include <rte_errno.h>

#include "base/vmxnet3_defs.h"
#include "vmxnet3_ring.h"

#include "vmxnet3_logs.h"
#include "vmxnet3_ethdev.h"

static const uint32_t rxprod_reg[2] = {VMXNET3_REG_RXPROD, VMXNET3_REG_RXPROD2};

static int vmxnet3_post_rx_bufs(vmxnet3_rx_queue_t*, uint8_t);
static void vmxnet3_tq_tx_complete(vmxnet3_tx_queue_t *);
#ifdef RTE_LIBRTE_VMXNET3_DEBUG_DRIVER_NOT_USED
static void vmxnet3_rxq_dump(struct vmxnet3_rx_queue *);
static void vmxnet3_txq_dump(struct vmxnet3_tx_queue *);
#endif

#ifdef RTE_LIBRTE_VMXNET3_DEBUG_DRIVER_NOT_USED
static void
vmxnet3_rxq_dump(struct vmxnet3_rx_queue *rxq)
{
	uint32_t avail = 0;

	if (rxq == NULL)
		return;

	PMD_RX_LOG(DEBUG,
		   "RXQ: cmd0 base : 0x%p cmd1 base : 0x%p comp ring base : 0x%p.",
		   rxq->cmd_ring[0].base, rxq->cmd_ring[1].base, rxq->comp_ring.base);
	PMD_RX_LOG(DEBUG,
		   "RXQ: cmd0 basePA : 0x%lx cmd1 basePA : 0x%lx comp ring basePA : 0x%lx.",
		   (unsigned long)rxq->cmd_ring[0].basePA,
		   (unsigned long)rxq->cmd_ring[1].basePA,
		   (unsigned long)rxq->comp_ring.basePA);

	avail = vmxnet3_cmd_ring_desc_avail(&rxq->cmd_ring[0]);
	PMD_RX_LOG(DEBUG,
		   "RXQ:cmd0: size=%u; free=%u; next2proc=%u; queued=%u",
		   (uint32_t)rxq->cmd_ring[0].size, avail,
		   rxq->comp_ring.next2proc,
		   rxq->cmd_ring[0].size - avail);

	avail = vmxnet3_cmd_ring_desc_avail(&rxq->cmd_ring[1]);
	PMD_RX_LOG(DEBUG, "RXQ:cmd1 size=%u; free=%u; next2proc=%u; queued=%u",
		   (uint32_t)rxq->cmd_ring[1].size, avail, rxq->comp_ring.next2proc,
		   rxq->cmd_ring[1].size - avail);

}

static void
vmxnet3_txq_dump(struct vmxnet3_tx_queue *txq)
{
	uint32_t avail = 0;

	if (txq == NULL)
		return;

	PMD_TX_LOG(DEBUG, "TXQ: cmd base : 0x%p comp ring base : 0x%p data ring base : 0x%p.",
		   txq->cmd_ring.base, txq->comp_ring.base, txq->data_ring.base);
	PMD_TX_LOG(DEBUG, "TXQ: cmd basePA : 0x%lx comp ring basePA : 0x%lx data ring basePA : 0x%lx.",
		   (unsigned long)txq->cmd_ring.basePA,
		   (unsigned long)txq->comp_ring.basePA,
		   (unsigned long)txq->data_ring.basePA);

	avail = vmxnet3_cmd_ring_desc_avail(&txq->cmd_ring);
	PMD_TX_LOG(DEBUG, "TXQ: size=%u; free=%u; next2proc=%u; queued=%u",
		   (uint32_t)txq->cmd_ring.size, avail,
		   txq->comp_ring.next2proc, txq->cmd_ring.size - avail);
}
#endif

static void
vmxnet3_cmd_ring_release_mbufs(vmxnet3_cmd_ring_t *ring)
{
	while (ring->next2comp != ring->next2fill) {
		/* No need to worry about tx desc ownership, device is quiesced by now. */
		vmxnet3_buf_info_t *buf_info = ring->buf_info + ring->next2comp;

		if (buf_info->m) {
			rte_pktmbuf_free(buf_info->m);
			buf_info->m = NULL;
			buf_info->bufPA = 0;
			buf_info->len = 0;
		}
		vmxnet3_cmd_ring_adv_next2comp(ring);
	}
}

static void
vmxnet3_cmd_ring_release(vmxnet3_cmd_ring_t *ring)
{
	vmxnet3_cmd_ring_release_mbufs(ring);
	rte_free(ring->buf_info);
	ring->buf_info = NULL;
}


void
vmxnet3_dev_tx_queue_release(void *txq)
{
	vmxnet3_tx_queue_t *tq = txq;

	if (tq != NULL) {
		/* Release the cmd_ring */
		vmxnet3_cmd_ring_release(&tq->cmd_ring);
	}
}

void
vmxnet3_dev_rx_queue_release(void *rxq)
{
	int i;
	vmxnet3_rx_queue_t *rq = rxq;

	if (rq != NULL) {
		/* Release both the cmd_rings */
		for (i = 0; i < VMXNET3_RX_CMDRING_SIZE; i++)
			vmxnet3_cmd_ring_release(&rq->cmd_ring[i]);
	}
}

static void
vmxnet3_dev_tx_queue_reset(void *txq)
{
	vmxnet3_tx_queue_t *tq = txq;
	struct vmxnet3_cmd_ring *ring = &tq->cmd_ring;
	struct vmxnet3_comp_ring *comp_ring = &tq->comp_ring;
	struct vmxnet3_data_ring *data_ring = &tq->data_ring;
	int size;

	if (tq != NULL) {
		/* Release the cmd_ring mbufs */
		vmxnet3_cmd_ring_release_mbufs(&tq->cmd_ring);
	}

	/* Tx vmxnet rings structure initialization*/
	ring->next2fill = 0;
	ring->next2comp = 0;
	ring->gen = VMXNET3_INIT_GEN;
	comp_ring->next2proc = 0;
	comp_ring->gen = VMXNET3_INIT_GEN;

	size = sizeof(struct Vmxnet3_TxDesc) * ring->size;
	size += sizeof(struct Vmxnet3_TxCompDesc) * comp_ring->size;
	size += sizeof(struct Vmxnet3_TxDataDesc) * data_ring->size;

	memset(ring->base, 0, size);
}

static void
vmxnet3_dev_rx_queue_reset(void *rxq)
{
	int i;
	vmxnet3_rx_queue_t *rq = rxq;
	struct vmxnet3_cmd_ring *ring0, *ring1;
	struct vmxnet3_comp_ring *comp_ring;
	int size;

	if (rq != NULL) {
		/* Release both the cmd_rings mbufs */
		for (i = 0; i < VMXNET3_RX_CMDRING_SIZE; i++)
			vmxnet3_cmd_ring_release_mbufs(&rq->cmd_ring[i]);
	}

	ring0 = &rq->cmd_ring[0];
	ring1 = &rq->cmd_ring[1];
	comp_ring = &rq->comp_ring;

	/* Rx vmxnet rings structure initialization */
	ring0->next2fill = 0;
	ring1->next2fill = 0;
	ring0->next2comp = 0;
	ring1->next2comp = 0;
	ring0->gen = VMXNET3_INIT_GEN;
	ring1->gen = VMXNET3_INIT_GEN;
	comp_ring->next2proc = 0;
	comp_ring->gen = VMXNET3_INIT_GEN;

	size = sizeof(struct Vmxnet3_RxDesc) * (ring0->size + ring1->size);
	size += sizeof(struct Vmxnet3_RxCompDesc) * comp_ring->size;

	memset(ring0->base, 0, size);
}

void
vmxnet3_dev_clear_queues(struct rte_eth_dev *dev)
{
	unsigned i;

	PMD_INIT_FUNC_TRACE();

	for (i = 0; i < dev->data->nb_tx_queues; i++) {
		struct vmxnet3_tx_queue *txq = dev->data->tx_queues[i];

		if (txq != NULL) {
			txq->stopped = TRUE;
			vmxnet3_dev_tx_queue_reset(txq);
		}
	}

	for (i = 0; i < dev->data->nb_rx_queues; i++) {
		struct vmxnet3_rx_queue *rxq = dev->data->rx_queues[i];

		if (rxq != NULL) {
			rxq->stopped = TRUE;
			vmxnet3_dev_rx_queue_reset(rxq);
		}
	}
}

static int
vmxnet3_unmap_pkt(uint16_t eop_idx, vmxnet3_tx_queue_t *txq)
{
	int completed = 0;
	struct rte_mbuf *mbuf;

	/* Release cmd_ring descriptor and free mbuf */
	RTE_ASSERT(txq->cmd_ring.base[eop_idx].txd.eop == 1);

	mbuf = txq->cmd_ring.buf_info[eop_idx].m;
	if (mbuf == NULL)
		rte_panic("EOP desc does not point to a valid mbuf");
	rte_pktmbuf_free(mbuf);

	txq->cmd_ring.buf_info[eop_idx].m = NULL;

	while (txq->cmd_ring.next2comp != eop_idx) {
		/* no out-of-order completion */
		RTE_ASSERT(txq->cmd_ring.base[txq->cmd_ring.next2comp].txd.cq == 0);
		vmxnet3_cmd_ring_adv_next2comp(&txq->cmd_ring);
		completed++;
	}

	/* Mark the txd for which tcd was generated as completed */
	vmxnet3_cmd_ring_adv_next2comp(&txq->cmd_ring);

	return completed + 1;
}

static void
vmxnet3_tq_tx_complete(vmxnet3_tx_queue_t *txq)
{
	int completed = 0;
	vmxnet3_comp_ring_t *comp_ring = &txq->comp_ring;
	struct Vmxnet3_TxCompDesc *tcd = (struct Vmxnet3_TxCompDesc *)
		(comp_ring->base + comp_ring->next2proc);

	while (tcd->gen == comp_ring->gen) {
		completed += vmxnet3_unmap_pkt(tcd->txdIdx, txq);

		vmxnet3_comp_ring_adv_next2proc(comp_ring);
		tcd = (struct Vmxnet3_TxCompDesc *)(comp_ring->base +
						    comp_ring->next2proc);
	}

	PMD_TX_LOG(DEBUG, "Processed %d tx comps & command descs.", completed);
}

uint16_t
vmxnet3_xmit_pkts(void *tx_queue, struct rte_mbuf **tx_pkts,
		  uint16_t nb_pkts)
{
	uint16_t nb_tx;
	vmxnet3_tx_queue_t *txq = tx_queue;
	struct vmxnet3_hw *hw = txq->hw;
	Vmxnet3_TxQueueCtrl *txq_ctrl = &txq->shared->ctrl;
	uint32_t deferred = rte_le_to_cpu_32(txq_ctrl->txNumDeferred);

	if (unlikely(txq->stopped)) {
		PMD_TX_LOG(DEBUG, "Tx queue is stopped.");
		return 0;
	}

	/* Free up the comp_descriptors aggressively */
	vmxnet3_tq_tx_complete(txq);

	nb_tx = 0;
	while (nb_tx < nb_pkts) {
		Vmxnet3_GenericDesc *gdesc;
		vmxnet3_buf_info_t *tbi;
		uint32_t first2fill, avail, dw2;
		struct rte_mbuf *txm = tx_pkts[nb_tx];
		struct rte_mbuf *m_seg = txm;
		int copy_size = 0;
		bool tso = (txm->ol_flags & PKT_TX_TCP_SEG) != 0;
		/* # of descriptors needed for a packet. */
		unsigned count = txm->nb_segs;

		avail = vmxnet3_cmd_ring_desc_avail(&txq->cmd_ring);
		if (count > avail) {
			/* Is command ring full? */
			if (unlikely(avail == 0)) {
				PMD_TX_LOG(DEBUG, "No free ring descriptors");
				txq->stats.tx_ring_full++;
				txq->stats.drop_total += (nb_pkts - nb_tx);
				break;
			}

			/* Command ring is not full but cannot handle the
			 * multi-segmented packet. Let's try the next packet
			 * in this case.
			 */
			PMD_TX_LOG(DEBUG, "Running out of ring descriptors "
				   "(avail %d needed %d)", avail, count);
			txq->stats.drop_total++;
			if (tso)
				txq->stats.drop_tso++;
			rte_pktmbuf_free(txm);
			nb_tx++;
			continue;
		}

		/* Drop non-TSO packet that is excessively fragmented */
		if (unlikely(!tso && count > VMXNET3_MAX_TXD_PER_PKT)) {
			PMD_TX_LOG(ERR, "Non-TSO packet cannot occupy more than %d tx "
				   "descriptors. Packet dropped.", VMXNET3_MAX_TXD_PER_PKT);
			txq->stats.drop_too_many_segs++;
			txq->stats.drop_total++;
			rte_pktmbuf_free(txm);
			nb_tx++;
			continue;
		}

		if (txm->nb_segs == 1 && rte_pktmbuf_pkt_len(txm) <= VMXNET3_HDR_COPY_SIZE) {
			struct Vmxnet3_TxDataDesc *tdd;

			tdd = txq->data_ring.base + txq->cmd_ring.next2fill;
			copy_size = rte_pktmbuf_pkt_len(txm);
			rte_memcpy(tdd->data, rte_pktmbuf_mtod(txm, char *), copy_size);
		}

		/* use the previous gen bit for the SOP desc */
		dw2 = (txq->cmd_ring.gen ^ 0x1) << VMXNET3_TXD_GEN_SHIFT;
		first2fill = txq->cmd_ring.next2fill;
		do {
			/* Remember the transmit buffer for cleanup */
			tbi = txq->cmd_ring.buf_info + txq->cmd_ring.next2fill;

			/* NB: the following assumes that VMXNET3 maximum
			 * transmit buffer size (16K) is greater than
			 * maximum size of mbuf segment size.
			 */
			gdesc = txq->cmd_ring.base + txq->cmd_ring.next2fill;
			if (copy_size)
				gdesc->txd.addr = rte_cpu_to_le_64(txq->data_ring.basePA +
								txq->cmd_ring.next2fill *
								sizeof(struct Vmxnet3_TxDataDesc));
			else
				gdesc->txd.addr = rte_mbuf_data_dma_addr(m_seg);

			gdesc->dword[2] = dw2 | m_seg->data_len;
			gdesc->dword[3] = 0;

			/* move to the next2fill descriptor */
			vmxnet3_cmd_ring_adv_next2fill(&txq->cmd_ring);

			/* use the right gen for non-SOP desc */
			dw2 = txq->cmd_ring.gen << VMXNET3_TXD_GEN_SHIFT;
		} while ((m_seg = m_seg->next) != NULL);

		/* set the last buf_info for the pkt */
		tbi->m = txm;
		/* Update the EOP descriptor */
		gdesc->dword[3] |= VMXNET3_TXD_EOP | VMXNET3_TXD_CQ;

		/* Add VLAN tag if present */
		gdesc = txq->cmd_ring.base + first2fill;
		if (txm->ol_flags & PKT_TX_VLAN_PKT) {
			gdesc->txd.ti = 1;
			gdesc->txd.tci = txm->vlan_tci;
		}

		if (tso) {
			uint16_t mss = txm->tso_segsz;

			RTE_ASSERT(mss > 0);

			gdesc->txd.hlen = txm->l2_len + txm->l3_len + txm->l4_len;
			gdesc->txd.om = VMXNET3_OM_TSO;
			gdesc->txd.msscof = mss;

			deferred += (rte_pktmbuf_pkt_len(txm) - gdesc->txd.hlen + mss - 1) / mss;
		} else if (txm->ol_flags & PKT_TX_L4_MASK) {
			gdesc->txd.om = VMXNET3_OM_CSUM;
			gdesc->txd.hlen = txm->l2_len + txm->l3_len;

			switch (txm->ol_flags & PKT_TX_L4_MASK) {
			case PKT_TX_TCP_CKSUM:
				gdesc->txd.msscof = gdesc->txd.hlen + offsetof(struct tcp_hdr, cksum);
				break;
			case PKT_TX_UDP_CKSUM:
				gdesc->txd.msscof = gdesc->txd.hlen + offsetof(struct udp_hdr, dgram_cksum);
				break;
			default:
				PMD_TX_LOG(WARNING, "requested cksum offload not supported %#llx",
					   txm->ol_flags & PKT_TX_L4_MASK);
				abort();
			}
			deferred++;
		} else {
			gdesc->txd.hlen = 0;
			gdesc->txd.om = VMXNET3_OM_NONE;
			gdesc->txd.msscof = 0;
			deferred++;
		}

		/* flip the GEN bit on the SOP */
		rte_compiler_barrier();
		gdesc->dword[2] ^= VMXNET3_TXD_GEN;

		txq_ctrl->txNumDeferred = rte_cpu_to_le_32(deferred);
		nb_tx++;
	}

	PMD_TX_LOG(DEBUG, "vmxnet3 txThreshold: %u", rte_le_to_cpu_32(txq_ctrl->txThreshold));

	if (deferred >= rte_le_to_cpu_32(txq_ctrl->txThreshold)) {
		txq_ctrl->txNumDeferred = 0;
		/* Notify vSwitch that packets are available. */
		VMXNET3_WRITE_BAR0_REG(hw, (VMXNET3_REG_TXPROD + txq->queue_id * VMXNET3_REG_ALIGN),
				       txq->cmd_ring.next2fill);
	}

	return nb_tx;
}

/*
 *  Allocates mbufs and clusters. Post rx descriptors with buffer details
 *  so that device can receive packets in those buffers.
 *	Ring layout:
 *      Among the two rings, 1st ring contains buffers of type 0 and type1.
 *      bufs_per_pkt is set such that for non-LRO cases all the buffers required
 *      by a frame will fit in 1st ring (1st buf of type0 and rest of type1).
 *      2nd ring contains buffers of type 1 alone. Second ring mostly be used
 *      only for LRO.
 *
 */
static int
vmxnet3_post_rx_bufs(vmxnet3_rx_queue_t *rxq, uint8_t ring_id)
{
	int err = 0;
	uint32_t i = 0, val = 0;
	struct vmxnet3_cmd_ring *ring = &rxq->cmd_ring[ring_id];

	if (ring_id == 0) {
		/* Usually: One HEAD type buf per packet
		 * val = (ring->next2fill % rxq->hw->bufs_per_pkt) ?
		 * VMXNET3_RXD_BTYPE_BODY : VMXNET3_RXD_BTYPE_HEAD;
		 */

		/* We use single packet buffer so all heads here */
		val = VMXNET3_RXD_BTYPE_HEAD;
	} else {
		/* All BODY type buffers for 2nd ring */
		val = VMXNET3_RXD_BTYPE_BODY;
	}

	while (vmxnet3_cmd_ring_desc_avail(ring) > 0) {
		struct Vmxnet3_RxDesc *rxd;
		struct rte_mbuf *mbuf;
		vmxnet3_buf_info_t *buf_info = &ring->buf_info[ring->next2fill];

		rxd = (struct Vmxnet3_RxDesc *)(ring->base + ring->next2fill);

		/* Allocate blank mbuf for the current Rx Descriptor */
		mbuf = rte_mbuf_raw_alloc(rxq->mp);
		if (unlikely(mbuf == NULL)) {
			PMD_RX_LOG(ERR, "Error allocating mbuf");
			rxq->stats.rx_buf_alloc_failure++;
			err = ENOMEM;
			break;
		}

		/*
		 * Load mbuf pointer into buf_info[ring_size]
		 * buf_info structure is equivalent to cookie for virtio-virtqueue
		 */
		buf_info->m = mbuf;
		buf_info->len = (uint16_t)(mbuf->buf_len -
					   RTE_PKTMBUF_HEADROOM);
		buf_info->bufPA =
			rte_mbuf_data_dma_addr_default(mbuf);

		/* Load Rx Descriptor with the buffer's GPA */
		rxd->addr = buf_info->bufPA;

		/* After this point rxd->addr MUST not be NULL */
		rxd->btype = val;
		rxd->len = buf_info->len;
		/* Flip gen bit at the end to change ownership */
		rxd->gen = ring->gen;

		vmxnet3_cmd_ring_adv_next2fill(ring);
		i++;
	}

	/* Return error only if no buffers are posted at present */
	if (vmxnet3_cmd_ring_desc_avail(ring) >= (ring->size - 1))
		return -err;
	else
		return i;
}


/* Receive side checksum and other offloads */
static void
vmxnet3_rx_offload(const Vmxnet3_RxCompDesc *rcd, struct rte_mbuf *rxm)
{
	/* Check for RSS */
	if (rcd->rssType != VMXNET3_RCD_RSS_TYPE_NONE) {
		rxm->ol_flags |= PKT_RX_RSS_HASH;
		rxm->hash.rss = rcd->rssHash;
	}

	/* Check packet type, checksum errors, etc. Only support IPv4 for now. */
	if (rcd->v4) {
		struct ether_hdr *eth = rte_pktmbuf_mtod(rxm, struct ether_hdr *);
		struct ipv4_hdr *ip = (struct ipv4_hdr *)(eth + 1);

		if (((ip->version_ihl & 0xf) << 2) > (int)sizeof(struct ipv4_hdr))
			rxm->packet_type = RTE_PTYPE_L3_IPV4_EXT;
		else
			rxm->packet_type = RTE_PTYPE_L3_IPV4;

		if (!rcd->cnc) {
			if (!rcd->ipc)
				rxm->ol_flags |= PKT_RX_IP_CKSUM_BAD;

			if ((rcd->tcp || rcd->udp) && !rcd->tuc)
				rxm->ol_flags |= PKT_RX_L4_CKSUM_BAD;
		}
	}
}

/*
 * Process the Rx Completion Ring of given vmxnet3_rx_queue
 * for nb_pkts burst and return the number of packets received
 */
uint16_t
vmxnet3_recv_pkts(void *rx_queue, struct rte_mbuf **rx_pkts, uint16_t nb_pkts)
{
	uint16_t nb_rx;
	uint32_t nb_rxd, idx;
	uint8_t ring_idx;
	vmxnet3_rx_queue_t *rxq;
	Vmxnet3_RxCompDesc *rcd;
	vmxnet3_buf_info_t *rbi;
	Vmxnet3_RxDesc *rxd;
	struct rte_mbuf *rxm = NULL;
	struct vmxnet3_hw *hw;

	nb_rx = 0;
	ring_idx = 0;
	nb_rxd = 0;
	idx = 0;

	rxq = rx_queue;
	hw = rxq->hw;

	rcd = &rxq->comp_ring.base[rxq->comp_ring.next2proc].rcd;

	if (unlikely(rxq->stopped)) {
		PMD_RX_LOG(DEBUG, "Rx queue is stopped.");
		return 0;
	}

	while (rcd->gen == rxq->comp_ring.gen) {
		if (nb_rx >= nb_pkts)
			break;

		idx = rcd->rxdIdx;
		ring_idx = (uint8_t)((rcd->rqID == rxq->qid1) ? 0 : 1);
		rxd = (Vmxnet3_RxDesc *)rxq->cmd_ring[ring_idx].base + idx;
		RTE_SET_USED(rxd); /* used only for assert when enabled */
		rbi = rxq->cmd_ring[ring_idx].buf_info + idx;

		PMD_RX_LOG(DEBUG, "rxd idx: %d ring idx: %d.", idx, ring_idx);

		RTE_ASSERT(rcd->len <= rxd->len);
		RTE_ASSERT(rbi->m);

		/* Get the packet buffer pointer from buf_info */
		rxm = rbi->m;

		/* Clear descriptor associated buf_info to be reused */
		rbi->m = NULL;
		rbi->bufPA = 0;

		/* Update the index that we received a packet */
		rxq->cmd_ring[ring_idx].next2comp = idx;

		/* For RCD with EOP set, check if there is frame error */
		if (unlikely(rcd->eop && rcd->err)) {
			rxq->stats.drop_total++;
			rxq->stats.drop_err++;

			if (!rcd->fcs) {
				rxq->stats.drop_fcs++;
				PMD_RX_LOG(ERR, "Recv packet dropped due to frame err.");
			}
			PMD_RX_LOG(ERR, "Error in received packet rcd#:%d rxd:%d",
				   (int)(rcd - (struct Vmxnet3_RxCompDesc *)
					 rxq->comp_ring.base), rcd->rxdIdx);
			rte_pktmbuf_free_seg(rxm);
			goto rcd_done;
		}


		/* Initialize newly received packet buffer */
		rxm->port = rxq->port_id;
		rxm->nb_segs = 1;
		rxm->next = NULL;
		rxm->pkt_len = (uint16_t)rcd->len;
		rxm->data_len = (uint16_t)rcd->len;
		rxm->data_off = RTE_PKTMBUF_HEADROOM;
		rxm->ol_flags = 0;
		rxm->vlan_tci = 0;

		/*
		 * If this is the first buffer of the received packet,
		 * set the pointer to the first mbuf of the packet
		 * Otherwise, update the total length and the number of segments
		 * of the current scattered packet, and update the pointer to
		 * the last mbuf of the current packet.
		 */
		if (rcd->sop) {
			RTE_ASSERT(rxd->btype == VMXNET3_RXD_BTYPE_HEAD);

			if (unlikely(rcd->len == 0)) {
				RTE_ASSERT(rcd->eop);

				PMD_RX_LOG(DEBUG,
					   "Rx buf was skipped. rxring[%d][%d])",
					   ring_idx, idx);
				rte_pktmbuf_free_seg(rxm);
				goto rcd_done;
			}

			rxq->start_seg = rxm;
			vmxnet3_rx_offload(rcd, rxm);
		} else {
			struct rte_mbuf *start = rxq->start_seg;

			RTE_ASSERT(rxd->btype == VMXNET3_RXD_BTYPE_BODY);

			start->pkt_len += rxm->data_len;
			start->nb_segs++;

			rxq->last_seg->next = rxm;
		}
		rxq->last_seg = rxm;

		if (rcd->eop) {
			struct rte_mbuf *start = rxq->start_seg;

			/* Check for hardware stripped VLAN tag */
			if (rcd->ts) {
				start->ol_flags |= (PKT_RX_VLAN_PKT | PKT_RX_VLAN_STRIPPED);
				start->vlan_tci = rte_le_to_cpu_16((uint16_t)rcd->tci);
			}

			rx_pkts[nb_rx++] = start;
			rxq->start_seg = NULL;
		}

rcd_done:
		rxq->cmd_ring[ring_idx].next2comp = idx;
		VMXNET3_INC_RING_IDX_ONLY(rxq->cmd_ring[ring_idx].next2comp, rxq->cmd_ring[ring_idx].size);

		/* It's time to allocate some new buf and renew descriptors */
		vmxnet3_post_rx_bufs(rxq, ring_idx);
		if (unlikely(rxq->shared->ctrl.updateRxProd)) {
			VMXNET3_WRITE_BAR0_REG(hw, rxprod_reg[ring_idx] + (rxq->queue_id * VMXNET3_REG_ALIGN),
					       rxq->cmd_ring[ring_idx].next2fill);
		}

		/* Advance to the next descriptor in comp_ring */
		vmxnet3_comp_ring_adv_next2proc(&rxq->comp_ring);

		rcd = &rxq->comp_ring.base[rxq->comp_ring.next2proc].rcd;
		nb_rxd++;
		if (nb_rxd > rxq->cmd_ring[0].size) {
			PMD_RX_LOG(ERR,
				   "Used up quota of receiving packets,"
				   " relinquish control.");
			break;
		}
	}

	return nb_rx;
}

/*
 * Create memzone for device rings. malloc can't be used as the physical address is
 * needed. If the memzone is already created, then this function returns a ptr
 * to the old one.
 */
static const struct rte_memzone *
ring_dma_zone_reserve(struct rte_eth_dev *dev, const char *ring_name,
		      uint16_t queue_id, uint32_t ring_size, int socket_id)
{
	char z_name[RTE_MEMZONE_NAMESIZE];
	const struct rte_memzone *mz;

	snprintf(z_name, sizeof(z_name), "%s_%s_%d_%d",
			dev->driver->pci_drv.name, ring_name,
			dev->data->port_id, queue_id);

	mz = rte_memzone_lookup(z_name);
	if (mz)
		return mz;

	return rte_memzone_reserve_aligned(z_name, ring_size,
			socket_id, 0, VMXNET3_RING_BA_ALIGN);
}

int
vmxnet3_dev_tx_queue_setup(struct rte_eth_dev *dev,
			   uint16_t queue_idx,
			   uint16_t nb_desc,
			   unsigned int socket_id,
			   __attribute__((unused)) const struct rte_eth_txconf *tx_conf)
{
	struct vmxnet3_hw *hw = dev->data->dev_private;
	const struct rte_memzone *mz;
	struct vmxnet3_tx_queue *txq;
	struct vmxnet3_cmd_ring *ring;
	struct vmxnet3_comp_ring *comp_ring;
	struct vmxnet3_data_ring *data_ring;
	int size;

	PMD_INIT_FUNC_TRACE();

	if ((tx_conf->txq_flags & ETH_TXQ_FLAGS_NOXSUMSCTP) !=
	    ETH_TXQ_FLAGS_NOXSUMSCTP) {
		PMD_INIT_LOG(ERR, "SCTP checksum offload not supported");
		return -EINVAL;
	}

	txq = rte_zmalloc("ethdev_tx_queue", sizeof(struct vmxnet3_tx_queue), RTE_CACHE_LINE_SIZE);
	if (txq == NULL) {
		PMD_INIT_LOG(ERR, "Can not allocate tx queue structure");
		return -ENOMEM;
	}

	txq->queue_id = queue_idx;
	txq->port_id = dev->data->port_id;
	txq->shared = &hw->tqd_start[queue_idx];
	txq->hw = hw;
	txq->qid = queue_idx;
	txq->stopped = TRUE;

	ring = &txq->cmd_ring;
	comp_ring = &txq->comp_ring;
	data_ring = &txq->data_ring;

	/* Tx vmxnet ring length should be between 512-4096 */
	if (nb_desc < VMXNET3_DEF_TX_RING_SIZE) {
		PMD_INIT_LOG(ERR, "VMXNET3 Tx Ring Size Min: %u",
			     VMXNET3_DEF_TX_RING_SIZE);
		return -EINVAL;
	} else if (nb_desc > VMXNET3_TX_RING_MAX_SIZE) {
		PMD_INIT_LOG(ERR, "VMXNET3 Tx Ring Size Max: %u",
			     VMXNET3_TX_RING_MAX_SIZE);
		return -EINVAL;
	} else {
		ring->size = nb_desc;
		ring->size &= ~VMXNET3_RING_SIZE_MASK;
	}
	comp_ring->size = data_ring->size = ring->size;

	/* Tx vmxnet rings structure initialization*/
	ring->next2fill = 0;
	ring->next2comp = 0;
	ring->gen = VMXNET3_INIT_GEN;
	comp_ring->next2proc = 0;
	comp_ring->gen = VMXNET3_INIT_GEN;

	size = sizeof(struct Vmxnet3_TxDesc) * ring->size;
	size += sizeof(struct Vmxnet3_TxCompDesc) * comp_ring->size;
	size += sizeof(struct Vmxnet3_TxDataDesc) * data_ring->size;

	mz = ring_dma_zone_reserve(dev, "txdesc", queue_idx, size, socket_id);
	if (mz == NULL) {
		PMD_INIT_LOG(ERR, "ERROR: Creating queue descriptors zone");
		return -ENOMEM;
	}
	memset(mz->addr, 0, mz->len);

	/* cmd_ring initialization */
	ring->base = mz->addr;
	ring->basePA = mz->phys_addr;

	/* comp_ring initialization */
	comp_ring->base = ring->base + ring->size;
	comp_ring->basePA = ring->basePA +
		(sizeof(struct Vmxnet3_TxDesc) * ring->size);

	/* data_ring initialization */
	data_ring->base = (Vmxnet3_TxDataDesc *)(comp_ring->base + comp_ring->size);
	data_ring->basePA = comp_ring->basePA +
			(sizeof(struct Vmxnet3_TxCompDesc) * comp_ring->size);

	/* cmd_ring0 buf_info allocation */
	ring->buf_info = rte_zmalloc("tx_ring_buf_info",
				     ring->size * sizeof(vmxnet3_buf_info_t), RTE_CACHE_LINE_SIZE);
	if (ring->buf_info == NULL) {
		PMD_INIT_LOG(ERR, "ERROR: Creating tx_buf_info structure");
		return -ENOMEM;
	}

	/* Update the data portion with txq */
	dev->data->tx_queues[queue_idx] = txq;

	return 0;
}

int
vmxnet3_dev_rx_queue_setup(struct rte_eth_dev *dev,
			   uint16_t queue_idx,
			   uint16_t nb_desc,
			   unsigned int socket_id,
			   __attribute__((unused)) const struct rte_eth_rxconf *rx_conf,
			   struct rte_mempool *mp)
{
	const struct rte_memzone *mz;
	struct vmxnet3_rx_queue *rxq;
	struct vmxnet3_hw     *hw = dev->data->dev_private;
	struct vmxnet3_cmd_ring *ring0, *ring1, *ring;
	struct vmxnet3_comp_ring *comp_ring;
	int size;
	uint8_t i;
	char mem_name[32];

	PMD_INIT_FUNC_TRACE();

	rxq = rte_zmalloc("ethdev_rx_queue", sizeof(struct vmxnet3_rx_queue), RTE_CACHE_LINE_SIZE);
	if (rxq == NULL) {
		PMD_INIT_LOG(ERR, "Can not allocate rx queue structure");
		return -ENOMEM;
	}

	rxq->mp = mp;
	rxq->queue_id = queue_idx;
	rxq->port_id = dev->data->port_id;
	rxq->shared = &hw->rqd_start[queue_idx];
	rxq->hw = hw;
	rxq->qid1 = queue_idx;
	rxq->qid2 = queue_idx + hw->num_rx_queues;
	rxq->stopped = TRUE;

	ring0 = &rxq->cmd_ring[0];
	ring1 = &rxq->cmd_ring[1];
	comp_ring = &rxq->comp_ring;

	/* Rx vmxnet rings length should be between 256-4096 */
	if (nb_desc < VMXNET3_DEF_RX_RING_SIZE) {
		PMD_INIT_LOG(ERR, "VMXNET3 Rx Ring Size Min: 256");
		return -EINVAL;
	} else if (nb_desc > VMXNET3_RX_RING_MAX_SIZE) {
		PMD_INIT_LOG(ERR, "VMXNET3 Rx Ring Size Max: 4096");
		return -EINVAL;
	} else {
		ring0->size = nb_desc;
		ring0->size &= ~VMXNET3_RING_SIZE_MASK;
		ring1->size = ring0->size;
	}

	comp_ring->size = ring0->size + ring1->size;

	/* Rx vmxnet rings structure initialization */
	ring0->next2fill = 0;
	ring1->next2fill = 0;
	ring0->next2comp = 0;
	ring1->next2comp = 0;
	ring0->gen = VMXNET3_INIT_GEN;
	ring1->gen = VMXNET3_INIT_GEN;
	comp_ring->next2proc = 0;
	comp_ring->gen = VMXNET3_INIT_GEN;

	size = sizeof(struct Vmxnet3_RxDesc) * (ring0->size + ring1->size);
	size += sizeof(struct Vmxnet3_RxCompDesc) * comp_ring->size;

	mz = ring_dma_zone_reserve(dev, "rxdesc", queue_idx, size, socket_id);
	if (mz == NULL) {
		PMD_INIT_LOG(ERR, "ERROR: Creating queue descriptors zone");
		return -ENOMEM;
	}
	memset(mz->addr, 0, mz->len);

	/* cmd_ring0 initialization */
	ring0->base = mz->addr;
	ring0->basePA = mz->phys_addr;

	/* cmd_ring1 initialization */
	ring1->base = ring0->base + ring0->size;
	ring1->basePA = ring0->basePA + sizeof(struct Vmxnet3_RxDesc) * ring0->size;

	/* comp_ring initialization */
	comp_ring->base = ring1->base + ring1->size;
	comp_ring->basePA = ring1->basePA + sizeof(struct Vmxnet3_RxDesc) *
		ring1->size;

	/* cmd_ring0-cmd_ring1 buf_info allocation */
	for (i = 0; i < VMXNET3_RX_CMDRING_SIZE; i++) {

		ring = &rxq->cmd_ring[i];
		ring->rid = i;
		snprintf(mem_name, sizeof(mem_name), "rx_ring_%d_buf_info", i);

		ring->buf_info = rte_zmalloc(mem_name, ring->size * sizeof(vmxnet3_buf_info_t), RTE_CACHE_LINE_SIZE);
		if (ring->buf_info == NULL) {
			PMD_INIT_LOG(ERR, "ERROR: Creating rx_buf_info structure");
			return -ENOMEM;
		}
	}

	/* Update the data portion with rxq */
	dev->data->rx_queues[queue_idx] = rxq;

	return 0;
}

/*
 * Initializes Receive Unit
 * Load mbufs in rx queue in advance
 */
int
vmxnet3_dev_rxtx_init(struct rte_eth_dev *dev)
{
	struct vmxnet3_hw *hw = dev->data->dev_private;

	int i, ret;
	uint8_t j;

	PMD_INIT_FUNC_TRACE();

	for (i = 0; i < hw->num_rx_queues; i++) {
		vmxnet3_rx_queue_t *rxq = dev->data->rx_queues[i];

		for (j = 0; j < VMXNET3_RX_CMDRING_SIZE; j++) {
			/* Passing 0 as alloc_num will allocate full ring */
			ret = vmxnet3_post_rx_bufs(rxq, j);
			if (ret <= 0) {
				PMD_INIT_LOG(ERR, "ERROR: Posting Rxq: %d buffers ring: %d", i, j);
				return -ret;
			}
			/* Updating device with the index:next2fill to fill the mbufs for coming packets */
			if (unlikely(rxq->shared->ctrl.updateRxProd)) {
				VMXNET3_WRITE_BAR0_REG(hw, rxprod_reg[j] + (rxq->queue_id * VMXNET3_REG_ALIGN),
						       rxq->cmd_ring[j].next2fill);
			}
		}
		rxq->stopped = FALSE;
		rxq->start_seg = NULL;
	}

	for (i = 0; i < dev->data->nb_tx_queues; i++) {
		struct vmxnet3_tx_queue *txq = dev->data->tx_queues[i];

		txq->stopped = FALSE;
	}

	return 0;
}

static uint8_t rss_intel_key[40] = {
	0x6D, 0x5A, 0x56, 0xDA, 0x25, 0x5B, 0x0E, 0xC2,
	0x41, 0x67, 0x25, 0x3D, 0x43, 0xA3, 0x8F, 0xB0,
	0xD0, 0xCA, 0x2B, 0xCB, 0xAE, 0x7B, 0x30, 0xB4,
	0x77, 0xCB, 0x2D, 0xA3, 0x80, 0x30, 0xF2, 0x0C,
	0x6A, 0x42, 0xB7, 0x3B, 0xBE, 0xAC, 0x01, 0xFA,
};

/*
 * Configure RSS feature
 */
int
vmxnet3_rss_configure(struct rte_eth_dev *dev)
{
	struct vmxnet3_hw *hw = dev->data->dev_private;
	struct VMXNET3_RSSConf *dev_rss_conf;
	struct rte_eth_rss_conf *port_rss_conf;
	uint64_t rss_hf;
	uint8_t i, j;

	PMD_INIT_FUNC_TRACE();

	dev_rss_conf = hw->rss_conf;
	port_rss_conf = &dev->data->dev_conf.rx_adv_conf.rss_conf;

	/* loading hashFunc */
	dev_rss_conf->hashFunc = VMXNET3_RSS_HASH_FUNC_TOEPLITZ;
	/* loading hashKeySize */
	dev_rss_conf->hashKeySize = VMXNET3_RSS_MAX_KEY_SIZE;
	/* loading indTableSize : Must not exceed VMXNET3_RSS_MAX_IND_TABLE_SIZE (128)*/
	dev_rss_conf->indTableSize = (uint16_t)(hw->num_rx_queues * 4);

	if (port_rss_conf->rss_key == NULL) {
		/* Default hash key */
		port_rss_conf->rss_key = rss_intel_key;
	}

	/* loading hashKey */
	memcpy(&dev_rss_conf->hashKey[0], port_rss_conf->rss_key, dev_rss_conf->hashKeySize);

	/* loading indTable */
	for (i = 0, j = 0; i < dev_rss_conf->indTableSize; i++, j++) {
		if (j == dev->data->nb_rx_queues)
			j = 0;
		dev_rss_conf->indTable[i] = j;
	}

	/* loading hashType */
	dev_rss_conf->hashType = 0;
	rss_hf = port_rss_conf->rss_hf & VMXNET3_RSS_OFFLOAD_ALL;
	if (rss_hf & ETH_RSS_IPV4)
		dev_rss_conf->hashType |= VMXNET3_RSS_HASH_TYPE_IPV4;
	if (rss_hf & ETH_RSS_NONFRAG_IPV4_TCP)
		dev_rss_conf->hashType |= VMXNET3_RSS_HASH_TYPE_TCP_IPV4;
	if (rss_hf & ETH_RSS_IPV6)
		dev_rss_conf->hashType |= VMXNET3_RSS_HASH_TYPE_IPV6;
	if (rss_hf & ETH_RSS_NONFRAG_IPV6_TCP)
		dev_rss_conf->hashType |= VMXNET3_RSS_HASH_TYPE_TCP_IPV6;

	return VMXNET3_SUCCESS;
}
