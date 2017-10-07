/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2017 Intel Corporation. All rights reserved.
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

#ifndef _GSO_COMMON_H_
#define _GSO_COMMON_H_

#include <stdint.h>

#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_tcp.h>

#define IS_FRAGMENTED(frag_off) (((frag_off) & IPV4_HDR_OFFSET_MASK) != 0 \
		|| ((frag_off) & IPV4_HDR_MF_FLAG) == IPV4_HDR_MF_FLAG)

#define TCP_HDR_PSH_MASK ((uint8_t)0x08)
#define TCP_HDR_FIN_MASK ((uint8_t)0x01)

#define IS_IPV4_TCP(flag) (((flag) & (PKT_TX_TCP_SEG | PKT_TX_IPV4)) == \
		(PKT_TX_TCP_SEG | PKT_TX_IPV4))

/**
 * Internal function which updates the TCP header of a packet, following
 * segmentation. This is required to update the header's 'sent' sequence
 * number, and also to clear 'PSH' and 'FIN' flags for non-tail segments.
 *
 * @param pkt
 *  The packet containing the TCP header.
 * @param l4_offset
 *  The offset of the TCP header from the start of the packet.
 * @param sent_seq
 *  The sent sequence number.
 * @param non-tail
 *  Indicates whether or not this is a tail segment.
 */
static inline void
update_tcp_header(struct rte_mbuf *pkt, uint16_t l4_offset, uint32_t sent_seq,
		uint8_t non_tail)
{
	struct tcp_hdr *tcp_hdr;

	tcp_hdr = (struct tcp_hdr *)(rte_pktmbuf_mtod(pkt, char *) +
			l4_offset);
	tcp_hdr->sent_seq = rte_cpu_to_be_32(sent_seq);
	if (likely(non_tail))
		tcp_hdr->tcp_flags &= (~(TCP_HDR_PSH_MASK |
					TCP_HDR_FIN_MASK));
}

/**
 * Internal function which updates the IPv4 header of a packet, following
 * segmentation. This is required to update the header's 'total_length' field,
 * to reflect the reduced length of the now-segmented packet. Furthermore, the
 * header's 'packet_id' field must be updated to reflect the new ID of the
 * now-segmented packet.
 *
 * @param pkt
 *  The packet containing the IPv4 header.
 * @param l3_offset
 *  The offset of the IPv4 header from the start of the packet.
 * @param id
 *  The new ID of the packet.
 */
static inline void
update_ipv4_header(struct rte_mbuf *pkt, uint16_t l3_offset, uint16_t id)
{
	struct ipv4_hdr *ipv4_hdr;

	ipv4_hdr = (struct ipv4_hdr *)(rte_pktmbuf_mtod(pkt, char *) +
			l3_offset);
	ipv4_hdr->total_length = rte_cpu_to_be_16(pkt->pkt_len - l3_offset);
	ipv4_hdr->packet_id = rte_cpu_to_be_16(id);
}

/**
 * Internal function which divides the input packet into small segments.
 * Each of the newly-created segments is organized as a two-segment MBUF,
 * where the first segment is a standard mbuf, which stores a copy of
 * packet header, and the second is an indirect mbuf which points to a
 * section of data in the input packet.
 *
 * @param pkt
 *  Packet to segment.
 * @param pkt_hdr_offset
 *  Packet header offset, measured in bytes.
 * @param pyld_unit_size
 *  The max payload length of a GSO segment.
 * @param direct_pool
 *  MBUF pool used for allocating direct buffers for output segments.
 * @param indirect_pool
 *  MBUF pool used for allocating indirect buffers for output segments.
 * @param pkts_out
 *  Pointer array used to keep the mbuf addresses of output segments. If
 *  the memory space in pkts_out is insufficient, gso_do_segment() fails
 *  and returns -EINVAL.
 * @param nb_pkts_out
 *  The max number of items that pkts_out can keep.
 *
 * @return
 *  - The number of segments created in the event of success.
 *  - Return -ENOMEM if run out of memory in MBUF pools.
 *  - Return -EINVAL for invalid parameters.
 */
int gso_do_segment(struct rte_mbuf *pkt,
		uint16_t pkt_hdr_offset,
		uint16_t pyld_unit_size,
		struct rte_mempool *direct_pool,
		struct rte_mempool *indirect_pool,
		struct rte_mbuf **pkts_out,
		uint16_t nb_pkts_out);
#endif
