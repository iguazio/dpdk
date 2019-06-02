/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(C) 2019 Marvell International Ltd.
 */

#ifndef __OTX2_RX_H__
#define __OTX2_RX_H__

/* Default mark value used when none is provided. */
#define OTX2_FLOW_ACTION_FLAG_DEFAULT	0xffff

#define PTYPE_WIDTH 12
#define PTYPE_NON_TUNNEL_ARRAY_SZ	BIT(PTYPE_WIDTH)
#define PTYPE_TUNNEL_ARRAY_SZ		BIT(PTYPE_WIDTH)
#define PTYPE_ARRAY_SZ			((PTYPE_NON_TUNNEL_ARRAY_SZ +\
					 PTYPE_TUNNEL_ARRAY_SZ) *\
					 sizeof(uint16_t))

#define NIX_RX_OFFLOAD_NONE            (0)
#define NIX_RX_OFFLOAD_RSS_F           BIT(0)
#define NIX_RX_OFFLOAD_PTYPE_F         BIT(1)
#define NIX_RX_OFFLOAD_CHECKSUM_F      BIT(2)
#define NIX_RX_OFFLOAD_VLAN_STRIP_F    BIT(3)
#define NIX_RX_OFFLOAD_MARK_UPDATE_F   BIT(4)
#define NIX_RX_OFFLOAD_TSTAMP_F        BIT(5)

#define NIX_TIMESYNC_RX_OFFSET		8

struct otx2_timesync_info {
	uint64_t	rx_tstamp;
	rte_iova_t	tx_tstamp_iova;
	uint64_t	*tx_tstamp;
	uint8_t		tx_ready;
	uint8_t		rx_ready;
} __rte_cache_aligned;

union mbuf_initializer {
	struct {
		uint16_t data_off;
		uint16_t refcnt;
		uint16_t nb_segs;
		uint16_t port;
	} fields;
	uint64_t value;
};

static __rte_always_inline void
otx2_nix_mbuf_to_tstamp(struct rte_mbuf *mbuf,
			struct otx2_timesync_info *tstamp, const uint16_t flag)
{
	if ((flag & NIX_RX_OFFLOAD_TSTAMP_F) &&
	    mbuf->packet_type == RTE_PTYPE_L2_ETHER_TIMESYNC &&
	    (mbuf->data_off == RTE_PKTMBUF_HEADROOM +
	     NIX_TIMESYNC_RX_OFFSET)) {
		uint64_t *tstamp_ptr;

		/* Deal with rx timestamp */
		tstamp_ptr = rte_pktmbuf_mtod_offset(mbuf, uint64_t *,
						     -NIX_TIMESYNC_RX_OFFSET);
		mbuf->timestamp = rte_be_to_cpu_64(*tstamp_ptr);
		tstamp->rx_tstamp = mbuf->timestamp;
		tstamp->rx_ready = 1;
		mbuf->ol_flags |= PKT_RX_IEEE1588_PTP | PKT_RX_IEEE1588_TMST
			| PKT_RX_TIMESTAMP;
	}
}

static __rte_always_inline uint64_t
nix_clear_data_off(uint64_t oldval)
{
	union mbuf_initializer mbuf_init = { .value = oldval };

	mbuf_init.fields.data_off = 0;
	return mbuf_init.value;
}

static __rte_always_inline struct rte_mbuf *
nix_get_mbuf_from_cqe(void *cq, const uint64_t data_off)
{
	rte_iova_t buff;

	/* Skip CQE, NIX_RX_PARSE_S and SG HDR(9 DWORDs) and peek buff addr */
	buff = *((rte_iova_t *)((uint64_t *)cq + 9));
	return (struct rte_mbuf *)(buff - data_off);
}


static __rte_always_inline uint32_t
nix_ptype_get(const void * const lookup_mem, const uint64_t in)
{
	const uint16_t * const ptype = lookup_mem;
	const uint16_t lg_lf_le = (in & 0xFFF000000000000) >> 48;
	const uint16_t tu_l2 = ptype[(in & 0x000FFF000000000) >> 36];
	const uint16_t il4_tu = ptype[PTYPE_NON_TUNNEL_ARRAY_SZ + lg_lf_le];

	return (il4_tu << PTYPE_WIDTH) | tu_l2;
}

static __rte_always_inline uint32_t
nix_rx_olflags_get(const void * const lookup_mem, const uint64_t in)
{
	const uint32_t * const ol_flags = (const uint32_t *)
			((const uint8_t *)lookup_mem + PTYPE_ARRAY_SZ);

	return ol_flags[(in & 0xfff00000) >> 20];
}

static inline uint64_t
nix_update_match_id(const uint16_t match_id, uint64_t ol_flags,
		    struct rte_mbuf *mbuf)
{
	/* There is no separate bit to check match_id
	 * is valid or not? and no flag to identify it is an
	 * RTE_FLOW_ACTION_TYPE_FLAG vs RTE_FLOW_ACTION_TYPE_MARK
	 * action. The former case addressed through 0 being invalid
	 * value and inc/dec match_id pair when MARK is activated.
	 * The later case addressed through defining
	 * OTX2_FLOW_MARK_DEFAULT as value for
	 * RTE_FLOW_ACTION_TYPE_MARK.
	 * This would translate to not use
	 * OTX2_FLOW_ACTION_FLAG_DEFAULT - 1 and
	 * OTX2_FLOW_ACTION_FLAG_DEFAULT for match_id.
	 * i.e valid mark_id's are from
	 * 0 to OTX2_FLOW_ACTION_FLAG_DEFAULT - 2
	 */
	if (likely(match_id)) {
		ol_flags |= PKT_RX_FDIR;
		if (match_id != OTX2_FLOW_ACTION_FLAG_DEFAULT) {
			ol_flags |= PKT_RX_FDIR_ID;
			mbuf->hash.fdir.hi = match_id - 1;
		}
	}

	return ol_flags;
}

static __rte_always_inline void
otx2_nix_cqe_to_mbuf(const struct nix_cqe_hdr_s *cq, const uint32_t tag,
		     struct rte_mbuf *mbuf, const void *lookup_mem,
		     const uint64_t val, const uint16_t flag)
{
	const struct nix_rx_parse_s *rx =
		 (const struct nix_rx_parse_s *)((const uint64_t *)cq + 1);
	const uint64_t w1 = *(const uint64_t *)rx;
	const uint16_t len = rx->pkt_lenm1 + 1;
	uint64_t ol_flags = 0;

	/* Mark mempool obj as "get" as it is alloc'ed by NIX */
	__mempool_check_cookies(mbuf->pool, (void **)&mbuf, 1, 1);

	if (flag & NIX_RX_OFFLOAD_PTYPE_F)
		mbuf->packet_type = nix_ptype_get(lookup_mem, w1);
	else
		mbuf->packet_type = 0;

	if (flag & NIX_RX_OFFLOAD_RSS_F) {
		mbuf->hash.rss = tag;
		ol_flags |= PKT_RX_RSS_HASH;
	}

	if (flag & NIX_RX_OFFLOAD_CHECKSUM_F)
		ol_flags |= nix_rx_olflags_get(lookup_mem, w1);

	if (flag & NIX_RX_OFFLOAD_VLAN_STRIP_F) {
		if (rx->vtag0_gone) {
			ol_flags |= PKT_RX_VLAN | PKT_RX_VLAN_STRIPPED;
			mbuf->vlan_tci = rx->vtag0_tci;
		}
		if (rx->vtag1_gone) {
			ol_flags |= PKT_RX_QINQ | PKT_RX_QINQ_STRIPPED;
			mbuf->vlan_tci_outer = rx->vtag1_tci;
		}
	}

	if (flag & NIX_RX_OFFLOAD_MARK_UPDATE_F)
		ol_flags = nix_update_match_id(rx->match_id, ol_flags, mbuf);

	mbuf->ol_flags = ol_flags;
	*(uint64_t *)(&mbuf->rearm_data) = val;
	mbuf->pkt_len = len;

	mbuf->data_len = len;
}

#define CKSUM_F NIX_RX_OFFLOAD_CHECKSUM_F
#define PTYPE_F NIX_RX_OFFLOAD_PTYPE_F
#define RSS_F	NIX_RX_OFFLOAD_RSS_F
#define RX_VLAN_F  NIX_RX_OFFLOAD_VLAN_STRIP_F
#define MARK_F  NIX_RX_OFFLOAD_MARK_UPDATE_F
#define TS_F	NIX_RX_OFFLOAD_TSTAMP_F

/* [TSMP] [MARK] [VLAN] [CKSUM] [PTYPE] [RSS] */
#define NIX_RX_FASTPATH_MODES						       \
R(no_offload,			0, 0, 0, 0, 0, 0, NIX_RX_OFFLOAD_NONE)	\
R(rss,				0, 0, 0, 0, 0, 1, RSS_F)		\
R(ptype,			0, 0, 0, 0, 1, 0, PTYPE_F)		\
R(ptype_rss,			0, 0, 0, 0, 1, 1, PTYPE_F | RSS_F)	\
R(cksum,			0, 0, 0, 1, 0, 0, CKSUM_F)		\
R(cksum_rss,			0, 0, 0, 1, 0, 1, CKSUM_F | RSS_F)	\
R(cksum_ptype,			0, 0, 0, 1, 1, 0, CKSUM_F | PTYPE_F)	\
R(cksum_ptype_rss,		0, 0, 0, 1, 1, 1, CKSUM_F | PTYPE_F | RSS_F)\
R(vlan,				0, 0, 1, 0, 0, 0, RX_VLAN_F)		\
R(vlan_rss,			0, 0, 1, 0, 0, 1, RX_VLAN_F | RSS_F)	\
R(vlan_ptype,			0, 0, 1, 0, 1, 0, RX_VLAN_F | PTYPE_F)	\
R(vlan_ptype_rss,		0, 0, 1, 0, 1, 1, RX_VLAN_F | PTYPE_F | RSS_F)\
R(vlan_cksum,			0, 0, 1, 1, 0, 0, RX_VLAN_F | CKSUM_F)	\
R(vlan_cksum_rss,		0, 0, 1, 1, 0, 1, RX_VLAN_F | CKSUM_F | RSS_F)\
R(vlan_cksum_ptype,		0, 0, 1, 1, 1, 0,			\
			RX_VLAN_F | CKSUM_F | PTYPE_F)			\
R(vlan_cksum_ptype_rss,		0, 0, 1, 1, 1, 1,			\
			RX_VLAN_F | CKSUM_F | PTYPE_F | RSS_F)		\
R(mark,				0, 1, 0, 0, 0, 0, MARK_F)		\
R(mark_rss,			0, 1, 0, 0, 0, 1, MARK_F | RSS_F)	\
R(mark_ptype,			0, 1, 0, 0, 1, 0, MARK_F | PTYPE_F)	\
R(mark_ptype_rss,		0, 1, 0, 0, 1, 1, MARK_F | PTYPE_F | RSS_F)\
R(mark_cksum,			0, 1, 0, 1, 0, 0, MARK_F | CKSUM_F)	\
R(mark_cksum_rss,		0, 1, 0, 1, 0, 1, MARK_F | CKSUM_F | RSS_F)\
R(mark_cksum_ptype,		0, 1, 0, 1, 1, 0, MARK_F | CKSUM_F | PTYPE_F)\
R(mark_cksum_ptype_rss,		0, 1, 0, 1, 1, 1,			\
			MARK_F | CKSUM_F | PTYPE_F | RSS_F)		\
R(mark_vlan,			0, 1, 1, 0, 0, 0, MARK_F | RX_VLAN_F)	\
R(mark_vlan_rss,		0, 1, 1, 0, 0, 1, MARK_F | RX_VLAN_F | RSS_F)\
R(mark_vlan_ptype,		0, 1, 1, 0, 1, 0,			\
			MARK_F | RX_VLAN_F | PTYPE_F)			\
R(mark_vlan_ptype_rss,		0, 1, 1, 0, 1, 1,			\
			MARK_F | RX_VLAN_F | PTYPE_F | RSS_F)		\
R(mark_vlan_cksum,		0, 1, 1, 1, 0, 0,			\
			MARK_F | RX_VLAN_F | CKSUM_F)			\
R(mark_vlan_cksum_rss,		0, 1, 1, 1, 0, 1,			\
			MARK_F | RX_VLAN_F | CKSUM_F | RSS_F)		\
R(mark_vlan_cksum_ptype,	0, 1, 1, 1, 1, 0,			\
			MARK_F | RX_VLAN_F | CKSUM_F | PTYPE_F)		\
R(mark_vlan_cksum_ptype_rss,	0, 1, 1, 1, 1, 1,			\
			MARK_F | RX_VLAN_F | CKSUM_F | PTYPE_F | RSS_F)	\
R(ts,				1, 0, 0, 0, 0, 0, TS_F)			\
R(ts_rss,			1, 0, 0, 0, 0, 1, TS_F | RSS_F)		\
R(ts_ptype,			1, 0, 0, 0, 1, 0, TS_F | PTYPE_F)	\
R(ts_ptype_rss,			1, 0, 0, 0, 1, 1, TS_F | PTYPE_F | RSS_F)\
R(ts_cksum,			1, 0, 0, 1, 0, 0, TS_F | CKSUM_F)	\
R(ts_cksum_rss,			1, 0, 0, 1, 0, 1, TS_F | CKSUM_F | RSS_F)\
R(ts_cksum_ptype,		1, 0, 0, 1, 1, 0, TS_F | CKSUM_F | PTYPE_F)\
R(ts_cksum_ptype_rss,		1, 0, 0, 1, 1, 1,			\
			TS_F | CKSUM_F | PTYPE_F | RSS_F)		\
R(ts_vlan,			1, 0, 1, 0, 0, 0, TS_F | RX_VLAN_F)	\
R(ts_vlan_rss,			1, 0, 1, 0, 0, 1, TS_F | RX_VLAN_F | RSS_F)\
R(ts_vlan_ptype,		1, 0, 1, 0, 1, 0, TS_F | RX_VLAN_F | PTYPE_F)\
R(ts_vlan_ptype_rss,		1, 0, 1, 0, 1, 1,			\
			TS_F | RX_VLAN_F | PTYPE_F | RSS_F)		\
R(ts_vlan_cksum,		1, 0, 1, 1, 0, 0,			\
			TS_F | RX_VLAN_F | CKSUM_F)			\
R(ts_vlan_cksum_rss,		1, 0, 1, 1, 0, 1,			\
			MARK_F | RX_VLAN_F | CKSUM_F | RSS_F)		\
R(ts_vlan_cksum_ptype,		1, 0, 1, 1, 1, 0,			\
			TS_F | RX_VLAN_F | CKSUM_F | PTYPE_F)		\
R(ts_vlan_cksum_ptype_rss,	1, 0, 1, 1, 1, 1,			\
			TS_F | RX_VLAN_F | CKSUM_F | PTYPE_F | RSS_F)	\
R(ts_mark,			1, 1, 0, 0, 0, 0, TS_F | MARK_F)	\
R(ts_mark_rss,			1, 1, 0, 0, 0, 1, TS_F | MARK_F | RSS_F)\
R(ts_mark_ptype,		1, 1, 0, 0, 1, 0, TS_F | MARK_F | PTYPE_F)\
R(ts_mark_ptype_rss,		1, 1, 0, 0, 1, 1,			\
			TS_F | MARK_F | PTYPE_F | RSS_F)		\
R(ts_mark_cksum,		1, 1, 0, 1, 0, 0, TS_F | MARK_F | CKSUM_F)\
R(ts_mark_cksum_rss,		1, 1, 0, 1, 0, 1,			\
			TS_F | MARK_F | CKSUM_F | RSS_F)\
R(ts_mark_cksum_ptype,		1, 1, 0, 1, 1, 0,			\
			TS_F | MARK_F | CKSUM_F | PTYPE_F)		\
R(ts_mark_cksum_ptype_rss,	1, 1, 0, 1, 1, 1,			\
			TS_F | MARK_F | CKSUM_F | PTYPE_F | RSS_F)	\
R(ts_mark_vlan,			1, 1, 1, 0, 0, 0, TS_F | MARK_F | RX_VLAN_F)\
R(ts_mark_vlan_rss,		1, 1, 1, 0, 0, 1,			\
			TS_F | MARK_F | RX_VLAN_F | RSS_F)\
R(ts_mark_vlan_ptype,		1, 1, 1, 0, 1, 0,			\
			TS_F | MARK_F | RX_VLAN_F | PTYPE_F)		\
R(ts_mark_vlan_ptype_rss,	1, 1, 1, 0, 1, 1,			\
			TS_F | MARK_F | RX_VLAN_F | PTYPE_F | RSS_F)	\
R(ts_mark_vlan_cksum_ptype,	1, 1, 1, 1, 1, 0,			\
			TS_F | MARK_F | RX_VLAN_F | CKSUM_F | PTYPE_F)	\
R(ts_mark_vlan_cksum_ptype_rss,	1, 1, 1, 1, 1, 1,			\
			TS_F | MARK_F | RX_VLAN_F | CKSUM_F | PTYPE_F | RSS_F)

#endif /* __OTX2_RX_H__ */
