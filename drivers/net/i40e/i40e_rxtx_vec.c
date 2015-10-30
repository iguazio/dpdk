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

#include <stdint.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>

#include "base/i40e_prototype.h"
#include "base/i40e_type.h"
#include "i40e_ethdev.h"
#include "i40e_rxtx.h"

#include <tmmintrin.h>

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

static inline void
i40e_rxq_rearm(struct i40e_rx_queue *rxq)
{
	int i;
	uint16_t rx_id;
	volatile union i40e_rx_desc *rxdp;
	struct i40e_rx_entry *rxep = &rxq->sw_ring[rxq->rxrearm_start];
	struct rte_mbuf *mb0, *mb1;
	__m128i hdr_room = _mm_set_epi64x(RTE_PKTMBUF_HEADROOM,
			RTE_PKTMBUF_HEADROOM);
	__m128i dma_addr0, dma_addr1;

	rxdp = rxq->rx_ring + rxq->rxrearm_start;

	/* Pull 'n' more MBUFs into the software ring */
	if (rte_mempool_get_bulk(rxq->mp,
				 (void *)rxep,
				 RTE_I40E_RXQ_REARM_THRESH) < 0) {
		if (rxq->rxrearm_nb + RTE_I40E_RXQ_REARM_THRESH >=
		    rxq->nb_rx_desc) {
			dma_addr0 = _mm_setzero_si128();
			for (i = 0; i < RTE_I40E_DESCS_PER_LOOP; i++) {
				rxep[i].mbuf = &rxq->fake_mbuf;
				_mm_store_si128((__m128i *)&rxdp[i].read,
						dma_addr0);
			}
		}
		rte_eth_devices[rxq->port_id].data->rx_mbuf_alloc_failed +=
			RTE_I40E_RXQ_REARM_THRESH;
		return;
	}

	/* Initialize the mbufs in vector, process 2 mbufs in one loop */
	for (i = 0; i < RTE_I40E_RXQ_REARM_THRESH; i += 2, rxep += 2) {
		__m128i vaddr0, vaddr1;
		uintptr_t p0, p1;

		mb0 = rxep[0].mbuf;
		mb1 = rxep[1].mbuf;

		 /* Flush mbuf with pkt template.
		 * Data to be rearmed is 6 bytes long.
		 * Though, RX will overwrite ol_flags that are coming next
		 * anyway. So overwrite whole 8 bytes with one load:
		 * 6 bytes of rearm_data plus first 2 bytes of ol_flags.
		 */
		p0 = (uintptr_t)&mb0->rearm_data;
		*(uint64_t *)p0 = rxq->mbuf_initializer;
		p1 = (uintptr_t)&mb1->rearm_data;
		*(uint64_t *)p1 = rxq->mbuf_initializer;

		/* load buf_addr(lo 64bit) and buf_physaddr(hi 64bit) */
		vaddr0 = _mm_loadu_si128((__m128i *)&mb0->buf_addr);
		vaddr1 = _mm_loadu_si128((__m128i *)&mb1->buf_addr);

		/* convert pa to dma_addr hdr/data */
		dma_addr0 = _mm_unpackhi_epi64(vaddr0, vaddr0);
		dma_addr1 = _mm_unpackhi_epi64(vaddr1, vaddr1);

		/* add headroom to pa values */
		dma_addr0 = _mm_add_epi64(dma_addr0, hdr_room);
		dma_addr1 = _mm_add_epi64(dma_addr1, hdr_room);

		/* flush desc with pa dma_addr */
		_mm_store_si128((__m128i *)&rxdp++->read, dma_addr0);
		_mm_store_si128((__m128i *)&rxdp++->read, dma_addr1);
	}

	rxq->rxrearm_start += RTE_I40E_RXQ_REARM_THRESH;
	if (rxq->rxrearm_start >= rxq->nb_rx_desc)
		rxq->rxrearm_start = 0;

	rxq->rxrearm_nb -= RTE_I40E_RXQ_REARM_THRESH;

	rx_id = (uint16_t)((rxq->rxrearm_start == 0) ?
			     (rxq->nb_rx_desc - 1) : (rxq->rxrearm_start - 1));

	/* Update the tail pointer on the NIC */
	I40E_PCI_REG_WRITE(rxq->qrx_tail, rx_id);
}

/* Handling the offload flags (olflags) field takes computation
 * time when receiving packets. Therefore we provide a flag to disable
 * the processing of the olflags field when they are not needed. This
 * gives improved performance, at the cost of losing the offload info
 * in the received packet
 */
#ifdef RTE_LIBRTE_I40E_RX_OLFLAGS_ENABLE

static inline void
desc_to_olflags_v(__m128i descs[4], struct rte_mbuf **rx_pkts)
{
	__m128i vlan0, vlan1, rss;
	union {
		uint16_t e[4];
		uint64_t dword;
	} vol;

	/* mask everything except rss and vlan flags
	*bit2 is for vlan tag, bits 13:12 for rss
	*/
	const __m128i rss_vlan_msk = _mm_set_epi16(
			0x0000, 0x0000, 0x0000, 0x0000,
			0x3004, 0x3004, 0x3004, 0x3004);

	/* map rss and vlan type to rss hash and vlan flag */
	const __m128i vlan_flags = _mm_set_epi8(0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, PKT_RX_VLAN_PKT,
			0, 0, 0, 0);

	const __m128i rss_flags = _mm_set_epi8(0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			PKT_RX_FDIR, 0, PKT_RX_RSS_HASH, 0);

	vlan0 = _mm_unpackhi_epi16(descs[0], descs[1]);
	vlan1 = _mm_unpackhi_epi16(descs[2], descs[3]);
	vlan0 = _mm_unpacklo_epi32(vlan0, vlan1);

	vlan1 = _mm_and_si128(vlan0, rss_vlan_msk);
	vlan0 = _mm_shuffle_epi8(vlan_flags, vlan1);

	rss = _mm_srli_epi16(vlan1, 12);
	rss = _mm_shuffle_epi8(rss_flags, rss);

	vlan0 = _mm_or_si128(vlan0, rss);
	vol.dword = _mm_cvtsi128_si64(vlan0);

	rx_pkts[0]->ol_flags = vol.e[0];
	rx_pkts[1]->ol_flags = vol.e[1];
	rx_pkts[2]->ol_flags = vol.e[2];
	rx_pkts[3]->ol_flags = vol.e[3];
}
#else
#define desc_to_olflags_v(desc, rx_pkts) do {} while (0)
#endif

#define PKTLEN_SHIFT     (6)
#define PKTLEN_MASK      (0x3FFF)
/* Handling the pkt len field is not aligned with 1byte, so shift is
 * needed to let it align
 */
static inline void
desc_pktlen_align(__m128i descs[4])
{
	__m128i pktlen0, pktlen1, zero;
	union {
		uint16_t e[4];
		uint64_t dword;
	} vol;

	/* mask everything except pktlen field*/
	const __m128i pktlen_msk = _mm_set_epi32(PKTLEN_MASK, PKTLEN_MASK,
						PKTLEN_MASK, PKTLEN_MASK);

	pktlen0 = _mm_unpackhi_epi32(descs[0], descs[2]);
	pktlen1 = _mm_unpackhi_epi32(descs[1], descs[3]);
	pktlen0 = _mm_unpackhi_epi32(pktlen0, pktlen1);

	zero = _mm_xor_si128(pktlen0, pktlen0);

	pktlen0 = _mm_srli_epi32(pktlen0, PKTLEN_SHIFT);
	pktlen0 = _mm_and_si128(pktlen0, pktlen_msk);

	pktlen0 = _mm_packs_epi32(pktlen0, zero);
	vol.dword = _mm_cvtsi128_si64(pktlen0);
	/* let the descriptor byte 15-14 store the pkt len */
	*((uint16_t *)&descs[0]+7) = vol.e[0];
	*((uint16_t *)&descs[1]+7) = vol.e[1];
	*((uint16_t *)&descs[2]+7) = vol.e[2];
	*((uint16_t *)&descs[3]+7) = vol.e[3];
}

 /* vPMD receive routine, now only accept (nb_pkts == RTE_I40E_VPMD_RX_BURST)
 * in one loop
 *
 * Notice:
 * - nb_pkts < RTE_I40E_DESCS_PER_LOOP, just return no packet
 * - nb_pkts > RTE_I40E_VPMD_RX_BURST, only scan RTE_I40E_VPMD_RX_BURST
 *   numbers of DD bits
 */
static inline uint16_t
_recv_raw_pkts_vec(struct i40e_rx_queue *rxq, struct rte_mbuf **rx_pkts,
		   uint16_t nb_pkts, uint8_t *split_packet)
{
	volatile union i40e_rx_desc *rxdp;
	struct i40e_rx_entry *sw_ring;
	uint16_t nb_pkts_recd;
	int pos;
	uint64_t var;
	__m128i shuf_msk;

	__m128i crc_adjust = _mm_set_epi16(
				0, 0, 0,    /* ignore non-length fields */
				-rxq->crc_len, /* sub crc on data_len */
				0,          /* ignore high-16bits of pkt_len */
				-rxq->crc_len, /* sub crc on pkt_len */
				0, 0            /* ignore pkt_type field */
			);
	__m128i dd_check, eop_check;

	/* nb_pkts shall be less equal than RTE_I40E_MAX_RX_BURST */
	nb_pkts = RTE_MIN(nb_pkts, RTE_I40E_MAX_RX_BURST);

	/* nb_pkts has to be floor-aligned to RTE_I40E_DESCS_PER_LOOP */
	nb_pkts = RTE_ALIGN_FLOOR(nb_pkts, RTE_I40E_DESCS_PER_LOOP);

	/* Just the act of getting into the function from the application is
	 * going to cost about 7 cycles
	 */
	rxdp = rxq->rx_ring + rxq->rx_tail;

	_mm_prefetch((const void *)rxdp, _MM_HINT_T0);

	/* See if we need to rearm the RX queue - gives the prefetch a bit
	 * of time to act
	 */
	if (rxq->rxrearm_nb > RTE_I40E_RXQ_REARM_THRESH)
		i40e_rxq_rearm(rxq);

	/* Before we start moving massive data around, check to see if
	 * there is actually a packet available
	 */
	if (!(rxdp->wb.qword1.status_error_len &
			rte_cpu_to_le_32(1 << I40E_RX_DESC_STATUS_DD_SHIFT)))
		return 0;

	/* 4 packets DD mask */
	dd_check = _mm_set_epi64x(0x0000000100000001LL, 0x0000000100000001LL);

	/* 4 packets EOP mask */
	eop_check = _mm_set_epi64x(0x0000000200000002LL, 0x0000000200000002LL);

	/* mask to shuffle from desc. to mbuf */
	shuf_msk = _mm_set_epi8(
		7, 6, 5, 4,  /* octet 4~7, 32bits rss */
		3, 2,        /* octet 2~3, low 16 bits vlan_macip */
		15, 14,      /* octet 15~14, 16 bits data_len */
		0xFF, 0xFF,  /* skip high 16 bits pkt_len, zero out */
		15, 14,      /* octet 15~14, low 16 bits pkt_len */
		0xFF, 0xFF,  /* pkt_type set as unknown */
		0xFF, 0xFF  /*pkt_type set as unknown */
		);

	/* Cache is empty -> need to scan the buffer rings, but first move
	 * the next 'n' mbufs into the cache
	 */
	sw_ring = &rxq->sw_ring[rxq->rx_tail];

	/* A. load 4 packet in one loop
	 * [A*. mask out 4 unused dirty field in desc]
	 * B. copy 4 mbuf point from swring to rx_pkts
	 * C. calc the number of DD bits among the 4 packets
	 * [C*. extract the end-of-packet bit, if requested]
	 * D. fill info. from desc to mbuf
	 */

	for (pos = 0, nb_pkts_recd = 0; pos < RTE_I40E_VPMD_RX_BURST;
			pos += RTE_I40E_DESCS_PER_LOOP,
			rxdp += RTE_I40E_DESCS_PER_LOOP) {
		__m128i descs[RTE_I40E_DESCS_PER_LOOP];
		__m128i pkt_mb1, pkt_mb2, pkt_mb3, pkt_mb4;
		__m128i zero, staterr, sterr_tmp1, sterr_tmp2;
		__m128i mbp1, mbp2; /* two mbuf pointer in one XMM reg. */

		/* B.1 load 1 mbuf point */
		mbp1 = _mm_loadu_si128((__m128i *)&sw_ring[pos]);
		/* Read desc statuses backwards to avoid race condition */
		/* A.1 load 4 pkts desc */
		descs[3] = _mm_loadu_si128((__m128i *)(rxdp + 3));

		/* B.2 copy 2 mbuf point into rx_pkts  */
		_mm_storeu_si128((__m128i *)&rx_pkts[pos], mbp1);

		/* B.1 load 1 mbuf point */
		mbp2 = _mm_loadu_si128((__m128i *)&sw_ring[pos+2]);

		descs[2] = _mm_loadu_si128((__m128i *)(rxdp + 2));
		/* B.1 load 2 mbuf point */
		descs[1] = _mm_loadu_si128((__m128i *)(rxdp + 1));
		descs[0] = _mm_loadu_si128((__m128i *)(rxdp));

		/* B.2 copy 2 mbuf point into rx_pkts  */
		_mm_storeu_si128((__m128i *)&rx_pkts[pos+2], mbp2);

		if (split_packet) {
			rte_prefetch0(&rx_pkts[pos]->cacheline1);
			rte_prefetch0(&rx_pkts[pos + 1]->cacheline1);
			rte_prefetch0(&rx_pkts[pos + 2]->cacheline1);
			rte_prefetch0(&rx_pkts[pos + 3]->cacheline1);
		}

		/*shift the pktlen field*/
		desc_pktlen_align(descs);

		/* avoid compiler reorder optimization */
		rte_compiler_barrier();

		/* D.1 pkt 3,4 convert format from desc to pktmbuf */
		pkt_mb4 = _mm_shuffle_epi8(descs[3], shuf_msk);
		pkt_mb3 = _mm_shuffle_epi8(descs[2], shuf_msk);

		/* C.1 4=>2 filter staterr info only */
		sterr_tmp2 = _mm_unpackhi_epi32(descs[3], descs[2]);
		/* C.1 4=>2 filter staterr info only */
		sterr_tmp1 = _mm_unpackhi_epi32(descs[1], descs[0]);

		desc_to_olflags_v(descs, &rx_pkts[pos]);

		/* D.2 pkt 3,4 set in_port/nb_seg and remove crc */
		pkt_mb4 = _mm_add_epi16(pkt_mb4, crc_adjust);
		pkt_mb3 = _mm_add_epi16(pkt_mb3, crc_adjust);

		/* D.1 pkt 1,2 convert format from desc to pktmbuf */
		pkt_mb2 = _mm_shuffle_epi8(descs[1], shuf_msk);
		pkt_mb1 = _mm_shuffle_epi8(descs[0], shuf_msk);

		/* C.2 get 4 pkts staterr value  */
		zero = _mm_xor_si128(dd_check, dd_check);
		staterr = _mm_unpacklo_epi32(sterr_tmp1, sterr_tmp2);

		/* D.3 copy final 3,4 data to rx_pkts */
		_mm_storeu_si128((void *)&rx_pkts[pos+3]->rx_descriptor_fields1,
				 pkt_mb4);
		_mm_storeu_si128((void *)&rx_pkts[pos+2]->rx_descriptor_fields1,
				 pkt_mb3);

		/* D.2 pkt 1,2 set in_port/nb_seg and remove crc */
		pkt_mb2 = _mm_add_epi16(pkt_mb2, crc_adjust);
		pkt_mb1 = _mm_add_epi16(pkt_mb1, crc_adjust);

		/* C* extract and record EOP bit */
		if (split_packet) {
			__m128i eop_shuf_mask = _mm_set_epi8(
					0xFF, 0xFF, 0xFF, 0xFF,
					0xFF, 0xFF, 0xFF, 0xFF,
					0xFF, 0xFF, 0xFF, 0xFF,
					0x04, 0x0C, 0x00, 0x08
					);

			/* and with mask to extract bits, flipping 1-0 */
			__m128i eop_bits = _mm_andnot_si128(staterr, eop_check);
			/* the staterr values are not in order, as the count
			 * count of dd bits doesn't care. However, for end of
			 * packet tracking, we do care, so shuffle. This also
			 * compresses the 32-bit values to 8-bit
			 */
			eop_bits = _mm_shuffle_epi8(eop_bits, eop_shuf_mask);
			/* store the resulting 32-bit value */
			*(int *)split_packet = _mm_cvtsi128_si32(eop_bits);
			split_packet += RTE_I40E_DESCS_PER_LOOP;

			/* zero-out next pointers */
			rx_pkts[pos]->next = NULL;
			rx_pkts[pos + 1]->next = NULL;
			rx_pkts[pos + 2]->next = NULL;
			rx_pkts[pos + 3]->next = NULL;
		}

		/* C.3 calc available number of desc */
		staterr = _mm_and_si128(staterr, dd_check);
		staterr = _mm_packs_epi32(staterr, zero);

		/* D.3 copy final 1,2 data to rx_pkts */
		_mm_storeu_si128((void *)&rx_pkts[pos+1]->rx_descriptor_fields1,
				 pkt_mb2);
		_mm_storeu_si128((void *)&rx_pkts[pos]->rx_descriptor_fields1,
				 pkt_mb1);
		/* C.4 calc avaialbe number of desc */
		var = __builtin_popcountll(_mm_cvtsi128_si64(staterr));
		nb_pkts_recd += var;
		if (likely(var != RTE_I40E_DESCS_PER_LOOP))
			break;
	}

	/* Update our internal tail pointer */
	rxq->rx_tail = (uint16_t)(rxq->rx_tail + nb_pkts_recd);
	rxq->rx_tail = (uint16_t)(rxq->rx_tail & (rxq->nb_rx_desc - 1));
	rxq->rxrearm_nb = (uint16_t)(rxq->rxrearm_nb + nb_pkts_recd);

	return nb_pkts_recd;
}

 /* vPMD receive routine, now only accept (nb_pkts == RTE_IXGBE_VPMD_RX_BURST)
 * in one loop
 *
 * Notice:
 * - nb_pkts < RTE_I40E_VPMD_RX_BURST, just return no packet
 * - nb_pkts > RTE_I40E_VPMD_RX_BURST, only scan RTE_IXGBE_VPMD_RX_BURST
 *   numbers of DD bit
 */
uint16_t
i40e_recv_pkts_vec(void *rx_queue, struct rte_mbuf **rx_pkts,
		   uint16_t nb_pkts)
{
	return _recv_raw_pkts_vec(rx_queue, rx_pkts, nb_pkts, NULL);
}

void __attribute__((cold))
i40e_rx_queue_release_mbufs_vec(struct i40e_rx_queue *rxq)
{
	const unsigned mask = rxq->nb_rx_desc - 1;
	unsigned i;

	if (rxq->sw_ring == NULL || rxq->rxrearm_nb >= rxq->nb_rx_desc)
		return;

	/* free all mbufs that are valid in the ring */
	for (i = rxq->rx_tail; i != rxq->rxrearm_start; i = (i + 1) & mask)
		rte_pktmbuf_free_seg(rxq->sw_ring[i].mbuf);
	rxq->rxrearm_nb = rxq->nb_rx_desc;

	/* set all entries to NULL */
	memset(rxq->sw_ring, 0, sizeof(rxq->sw_ring[0]) * rxq->nb_rx_desc);
}

int __attribute__((cold))
i40e_rxq_vec_setup(struct i40e_rx_queue *rxq)
{
	uintptr_t p;
	struct rte_mbuf mb_def = { .buf_addr = 0 }; /* zeroed mbuf */

	mb_def.nb_segs = 1;
	mb_def.data_off = RTE_PKTMBUF_HEADROOM;
	mb_def.port = rxq->port_id;
	rte_mbuf_refcnt_set(&mb_def, 1);

	/* prevent compiler reordering: rearm_data covers previous fields */
	rte_compiler_barrier();
	p = (uintptr_t)&mb_def.rearm_data;
	rxq->mbuf_initializer = *(uint64_t *)p;
	return 0;
}
