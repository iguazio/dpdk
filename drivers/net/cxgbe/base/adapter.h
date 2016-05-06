/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2014-2016 Chelsio Communications.
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
 *     * Neither the name of Chelsio Communications nor the names of its
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

/* This file should not be included directly.  Include common.h instead. */

#ifndef __T4_ADAPTER_H__
#define __T4_ADAPTER_H__

#include <rte_mbuf.h>

#include "cxgbe_compat.h"
#include "t4_regs_values.h"

enum {
	MAX_ETH_QSETS = 64,           /* # of Ethernet Tx/Rx queue sets */
};

struct adapter;
struct sge_rspq;

enum {
	PORT_RSS_DONE = (1 << 0),
};

struct port_info {
	struct adapter *adapter;        /* adapter that this port belongs to */
	struct rte_eth_dev *eth_dev;    /* associated rte eth device */
	struct port_stats stats_base;   /* port statistics base */
	struct link_config link_cfg;    /* link configuration info */

	unsigned long flags;            /* port related flags */
	short int xact_addr_filt;       /* index of exact MAC address filter */

	u16    viid;                    /* associated virtual interface id */
	s8     mdio_addr;               /* address of the PHY */
	u8     port_type;               /* firmware port type */
	u8     mod_type;                /* firmware module type */
	u8     port_id;                 /* physical port ID */
	u8     tx_chan;                 /* associated channel */

	u8     n_rx_qsets;              /* # of rx qsets */
	u8     n_tx_qsets;              /* # of tx qsets */
	u8     first_qset;              /* index of first qset */

	u16    *rss;                    /* rss table */
	u8     rss_mode;                /* rss mode */
	u16    rss_size;                /* size of VI's RSS table slice */
};

/* Enable or disable autonegotiation.  If this is set to enable,
 * the forced link modes above are completely ignored.
 */
#define AUTONEG_DISABLE         0x00
#define AUTONEG_ENABLE          0x01

enum {                                 /* adapter flags */
	FULL_INIT_DONE     = (1 << 0),
	USING_MSI          = (1 << 1),
	USING_MSIX         = (1 << 2),
	FW_QUEUE_BOUND     = (1 << 3),
	FW_OK              = (1 << 4),
	CFG_QUEUES	   = (1 << 5),
	MASTER_PF          = (1 << 6),
};

struct rx_sw_desc {                /* SW state per Rx descriptor */
	void *buf;                 /* struct page or mbuf */
	dma_addr_t dma_addr;
};

struct sge_fl {                     /* SGE free-buffer queue state */
	/* RO fields */
	struct rx_sw_desc *sdesc;   /* address of SW Rx descriptor ring */

	dma_addr_t addr;            /* bus address of HW ring start */
	__be64 *desc;               /* address of HW Rx descriptor ring */

	void __iomem *bar2_addr;    /* address of BAR2 Queue registers */
	unsigned int bar2_qid;      /* Queue ID for BAR2 Queue registers */

	unsigned int cntxt_id;      /* SGE relative QID for the free list */
	unsigned int size;          /* capacity of free list */

	unsigned int avail;         /* # of available Rx buffers */
	unsigned int pend_cred;     /* new buffers since last FL DB ring */
	unsigned int cidx;          /* consumer index */
	unsigned int pidx;          /* producer index */

	unsigned long alloc_failed; /* # of times buffer allocation failed */
	unsigned long low;          /* # of times momentarily starving */
};

#define MAX_MBUF_FRAGS (16384 / 512 + 2)

/* A packet gather list */
struct pkt_gl {
	union {
		struct rte_mbuf *mbufs[MAX_MBUF_FRAGS];
	} /* UNNAMED */;
	void *va;                         /* virtual address of first byte */
	unsigned int nfrags;              /* # of fragments */
	unsigned int tot_len;             /* total length of fragments */
	bool usembufs;                    /* use mbufs for fragments */
};

typedef int (*rspq_handler_t)(struct sge_rspq *q, const __be64 *rsp,
			      const struct pkt_gl *gl);

struct sge_rspq {                   /* state for an SGE response queue */
	struct adapter *adapter;      /* adapter that this queue belongs to */
	struct rte_eth_dev *eth_dev;  /* associated rte eth device */
	struct rte_mempool  *mb_pool; /* associated mempool */

	dma_addr_t phys_addr;       /* physical address of the ring */
	__be64 *desc;               /* address of HW response ring */
	const __be64 *cur_desc;     /* current descriptor in queue */

	void __iomem *bar2_addr;    /* address of BAR2 Queue registers */
	unsigned int bar2_qid;      /* Queue ID for BAR2 Queue registers */

	unsigned int cidx;          /* consumer index */
	unsigned int gts_idx;	    /* last gts write sent */
	unsigned int iqe_len;       /* entry size */
	unsigned int size;          /* capacity of response queue */
	int offset;                 /* offset into current Rx buffer */

	u8 gen;                     /* current generation bit */
	u8 intr_params;             /* interrupt holdoff parameters */
	u8 next_intr_params;        /* holdoff params for next interrupt */
	u8 pktcnt_idx;              /* interrupt packet threshold */
	u8 port_id;		    /* associated port-id */
	u8 idx;                     /* queue index within its group */
	u16 cntxt_id;               /* SGE relative QID for the response Q */
	u16 abs_id;                 /* absolute SGE id for the response q */

	rspq_handler_t handler;     /* associated handler for this response q */
};

struct sge_eth_rx_stats {	/* Ethernet rx queue statistics */
	u64 pkts;		/* # of ethernet packets */
	u64 rx_bytes;		/* # of ethernet bytes */
	u64 rx_cso;		/* # of Rx checksum offloads */
	u64 vlan_ex;		/* # of Rx VLAN extractions */
	u64 rx_drops;		/* # of packets dropped due to no mem */
};

struct sge_eth_rxq {                /* a SW Ethernet Rx queue */
	struct sge_rspq rspq;
	struct sge_fl fl;
	struct sge_eth_rx_stats stats;
	bool usembufs;               /* one ingress packet per mbuf FL buffer */
} __rte_cache_aligned;

/*
 * Currently there are two types of coalesce WR. Type 0 needs 48 bytes per
 * packet (if one sgl is present) and type 1 needs 32 bytes. This means
 * that type 0 can fit a maximum of 10 packets per WR and type 1 can fit
 * 15 packets. We need to keep track of the mbuf pointers in a coalesce WR
 * to be able to free those mbufs when we get completions back from the FW.
 * Allocating the maximum number of pointers in every tx desc is a waste
 * of memory resources so we only store 2 pointers per tx desc which should
 * be enough since a tx desc can only fit 2 packets in the best case
 * scenario where a packet needs 32 bytes.
 */
#define ETH_COALESCE_PKT_NUM 15
#define ETH_COALESCE_PKT_PER_DESC 2

struct tx_eth_coal_desc {
	struct rte_mbuf *mbuf[ETH_COALESCE_PKT_PER_DESC];
	struct ulptx_sgl *sgl[ETH_COALESCE_PKT_PER_DESC];
	int idx;
};

struct tx_desc {
	__be64 flit[8];
};

struct tx_sw_desc {                /* SW state per Tx descriptor */
	struct rte_mbuf *mbuf;
	struct ulptx_sgl *sgl;
	struct tx_eth_coal_desc coalesce;
};

enum {
	EQ_STOPPED = (1 << 0),
};

struct eth_coalesce {
	unsigned char *ptr;
	unsigned char type;
	unsigned int idx;
	unsigned int len;
	unsigned int flits;
	unsigned int max;
};

struct sge_txq {
	struct tx_desc *desc;       /* address of HW Tx descriptor ring */
	struct tx_sw_desc *sdesc;   /* address of SW Tx descriptor ring */
	struct sge_qstat *stat;     /* queue status entry */
	struct eth_coalesce coalesce; /* coalesce info */

	uint64_t phys_addr;         /* physical address of the ring */

	void __iomem *bar2_addr;    /* address of BAR2 Queue registers */
	unsigned int bar2_qid;      /* Queue ID for BAR2 Queue registers */

	unsigned int cntxt_id;     /* SGE relative QID for the Tx Q */
	unsigned int in_use;       /* # of in-use Tx descriptors */
	unsigned int size;         /* # of descriptors */
	unsigned int cidx;         /* SW consumer index */
	unsigned int pidx;         /* producer index */
	unsigned int dbidx;	   /* last idx when db ring was done */
	unsigned int equeidx;	   /* last sent credit request */
	unsigned int last_pidx;	   /* last pidx recorded by tx monitor */
	unsigned int last_coal_idx;/* last coal-idx recorded by tx monitor */

	int db_disabled;            /* doorbell state */
	unsigned short db_pidx;     /* doorbell producer index */
	unsigned short db_pidx_inc; /* doorbell producer increment */
};

struct sge_eth_tx_stats {	/* Ethernet tx queue statistics */
	u64 pkts;		/* # of ethernet packets */
	u64 tx_bytes;		/* # of ethernet bytes */
	u64 tso;		/* # of TSO requests */
	u64 tx_cso;		/* # of Tx checksum offloads */
	u64 vlan_ins;		/* # of Tx VLAN insertions */
	u64 mapping_err;	/* # of I/O MMU packet mapping errors */
	u64 coal_wr;            /* # of coalesced wr */
	u64 coal_pkts;          /* # of coalesced packets */
};

struct sge_eth_txq {                   /* state for an SGE Ethernet Tx queue */
	struct sge_txq q;
	struct rte_eth_dev *eth_dev;   /* port that this queue belongs to */
	struct sge_eth_tx_stats stats; /* queue statistics */
	rte_spinlock_t txq_lock;

	unsigned int flags;            /* flags for state of the queue */
} __rte_cache_aligned;

struct sge {
	struct sge_eth_txq ethtxq[MAX_ETH_QSETS];
	struct sge_eth_rxq ethrxq[MAX_ETH_QSETS];
	struct sge_rspq fw_evtq __rte_cache_aligned;

	u16 max_ethqsets;           /* # of available Ethernet queue sets */
	u32 stat_len;               /* length of status page at ring end */
	u32 pktshift;               /* padding between CPL & packet data */

	/* response queue interrupt parameters */
	u16 timer_val[SGE_NTIMERS];
	u8  counter_val[SGE_NCOUNTERS];

	u32 fl_align;               /* response queue message alignment */
	u32 fl_pg_order;            /* large page allocation size */
	u32 fl_starve_thres;        /* Free List starvation threshold */
};

#define T4_OS_NEEDS_MBOX_LOCKING 1

/*
 * OS Lock/List primitives for those interfaces in the Common Code which
 * need this.
 */

struct mbox_entry {
	TAILQ_ENTRY(mbox_entry) next;
};

TAILQ_HEAD(mbox_list, mbox_entry);

struct adapter {
	struct rte_pci_device *pdev;       /* associated rte pci device */
	struct rte_eth_dev *eth_dev;       /* first port's rte eth device */
	struct adapter_params params;      /* adapter parameters */
	struct port_info port[MAX_NPORTS]; /* ports belonging to this adapter */
	struct sge sge;                    /* associated SGE */

	/* support for single-threading access to adapter mailbox registers */
	struct mbox_list mbox_list;
	rte_spinlock_t mbox_lock;

	u8 *regs;              /* pointer to registers region */
	u8 *bar2;              /* pointer to bar2 region */
	unsigned long flags;   /* adapter flags */
	unsigned int mbox;     /* associated mailbox */
	unsigned int pf;       /* associated physical function id */

	int use_unpacked_mode; /* unpacked rx mode state */
};

#define CXGBE_PCI_REG(reg) (*((volatile uint32_t *)(reg)))

static inline uint64_t cxgbe_read_addr64(volatile void *addr)
{
	uint64_t val = CXGBE_PCI_REG(addr);
	uint64_t val2 = CXGBE_PCI_REG(((volatile uint8_t *)(addr) + 4));

	val2 = (uint64_t)(val2 << 32);
	val += val2;
	return val;
}

static inline uint32_t cxgbe_read_addr(volatile void *addr)
{
	return CXGBE_PCI_REG(addr);
}

#define CXGBE_PCI_REG_ADDR(adap, reg) \
	((volatile uint32_t *)((char *)(adap)->regs + (reg)))

#define CXGBE_READ_REG(adap, reg) \
	cxgbe_read_addr(CXGBE_PCI_REG_ADDR((adap), (reg)))

#define CXGBE_READ_REG64(adap, reg) \
	cxgbe_read_addr64(CXGBE_PCI_REG_ADDR((adap), (reg)))

#define CXGBE_PCI_REG_WRITE(reg, value) ({ \
	CXGBE_PCI_REG((reg)) = (value); })

#define CXGBE_WRITE_REG(adap, reg, value) \
	CXGBE_PCI_REG_WRITE(CXGBE_PCI_REG_ADDR((adap), (reg)), (value))

static inline uint64_t cxgbe_write_addr64(volatile void *addr, uint64_t val)
{
	CXGBE_PCI_REG(addr) = val;
	CXGBE_PCI_REG(((volatile uint8_t *)(addr) + 4)) = (val >> 32);
	return val;
}

#define CXGBE_WRITE_REG64(adap, reg, value) \
	cxgbe_write_addr64(CXGBE_PCI_REG_ADDR((adap), (reg)), (value))

/**
 * t4_read_reg - read a HW register
 * @adapter: the adapter
 * @reg_addr: the register address
 *
 * Returns the 32-bit value of the given HW register.
 */
static inline u32 t4_read_reg(struct adapter *adapter, u32 reg_addr)
{
	u32 val = CXGBE_READ_REG(adapter, reg_addr);

	CXGBE_DEBUG_REG(adapter, "read register 0x%x value 0x%x\n", reg_addr,
			val);
	return val;
}

/**
 * t4_write_reg - write a HW register
 * @adapter: the adapter
 * @reg_addr: the register address
 * @val: the value to write
 *
 * Write a 32-bit value into the given HW register.
 */
static inline void t4_write_reg(struct adapter *adapter, u32 reg_addr, u32 val)
{
	CXGBE_DEBUG_REG(adapter, "setting register 0x%x to 0x%x\n", reg_addr,
			val);
	CXGBE_WRITE_REG(adapter, reg_addr, val);
}

/**
 * t4_read_reg64 - read a 64-bit HW register
 * @adapter: the adapter
 * @reg_addr: the register address
 *
 * Returns the 64-bit value of the given HW register.
 */
static inline u64 t4_read_reg64(struct adapter *adapter, u32 reg_addr)
{
	u64 val = CXGBE_READ_REG64(adapter, reg_addr);

	CXGBE_DEBUG_REG(adapter, "64-bit read register %#x value %#llx\n",
			reg_addr, (unsigned long long)val);
	return val;
}

/**
 * t4_write_reg64 - write a 64-bit HW register
 * @adapter: the adapter
 * @reg_addr: the register address
 * @val: the value to write
 *
 * Write a 64-bit value into the given HW register.
 */
static inline void t4_write_reg64(struct adapter *adapter, u32 reg_addr,
				  u64 val)
{
	CXGBE_DEBUG_REG(adapter, "setting register %#x to %#llx\n", reg_addr,
			(unsigned long long)val);

	CXGBE_WRITE_REG64(adapter, reg_addr, val);
}

#define PCI_STATUS              0x06    /* 16 bits */
#define PCI_STATUS_CAP_LIST     0x10    /* Support Capability List */
#define PCI_CAPABILITY_LIST     0x34
/* Offset of first capability list entry */
#define PCI_CAP_ID_EXP          0x10    /* PCI Express */
#define PCI_CAP_LIST_ID         0       /* Capability ID */
#define PCI_CAP_LIST_NEXT       1       /* Next capability in the list */
#define PCI_EXP_DEVCTL2         40      /* Device Control 2 */

/**
 * t4_os_pci_write_cfg4 - 32-bit write to PCI config space
 * @adapter: the adapter
 * @addr: the register address
 * @val: the value to write
 *
 * Write a 32-bit value into the given register in PCI config space.
 */
static inline void t4_os_pci_write_cfg4(struct adapter *adapter, size_t addr,
					off_t val)
{
	u32 val32 = val;

	if (rte_eal_pci_write_config(adapter->pdev, &val32, sizeof(val32),
				     addr) < 0)
		dev_err(adapter, "Can't write to PCI config space\n");
}

/**
 * t4_os_pci_read_cfg4 - read a 32-bit value from PCI config space
 * @adapter: the adapter
 * @addr: the register address
 * @val: where to store the value read
 *
 * Read a 32-bit value from the given register in PCI config space.
 */
static inline void t4_os_pci_read_cfg4(struct adapter *adapter, size_t addr,
				       u32 *val)
{
	if (rte_eal_pci_read_config(adapter->pdev, val, sizeof(*val),
				    addr) < 0)
		dev_err(adapter, "Can't read from PCI config space\n");
}

/**
 * t4_os_pci_write_cfg2 - 16-bit write to PCI config space
 * @adapter: the adapter
 * @addr: the register address
 * @val: the value to write
 *
 * Write a 16-bit value into the given register in PCI config space.
 */
static inline void t4_os_pci_write_cfg2(struct adapter *adapter, size_t addr,
					off_t val)
{
	u16 val16 = val;

	if (rte_eal_pci_write_config(adapter->pdev, &val16, sizeof(val16),
				     addr) < 0)
		dev_err(adapter, "Can't write to PCI config space\n");
}

/**
 * t4_os_pci_read_cfg2 - read a 16-bit value from PCI config space
 * @adapter: the adapter
 * @addr: the register address
 * @val: where to store the value read
 *
 * Read a 16-bit value from the given register in PCI config space.
 */
static inline void t4_os_pci_read_cfg2(struct adapter *adapter, size_t addr,
				       u16 *val)
{
	if (rte_eal_pci_read_config(adapter->pdev, val, sizeof(*val),
				    addr) < 0)
		dev_err(adapter, "Can't read from PCI config space\n");
}

/**
 * t4_os_pci_read_cfg - read a 8-bit value from PCI config space
 * @adapter: the adapter
 * @addr: the register address
 * @val: where to store the value read
 *
 * Read a 8-bit value from the given register in PCI config space.
 */
static inline void t4_os_pci_read_cfg(struct adapter *adapter, size_t addr,
				      u8 *val)
{
	if (rte_eal_pci_read_config(adapter->pdev, val, sizeof(*val),
				    addr) < 0)
		dev_err(adapter, "Can't read from PCI config space\n");
}

/**
 * t4_os_find_pci_capability - lookup a capability in the PCI capability list
 * @adapter: the adapter
 * @cap: the capability
 *
 * Return the address of the given capability within the PCI capability list.
 */
static inline int t4_os_find_pci_capability(struct adapter *adapter, int cap)
{
	u16 status;
	int ttl = 48;
	u8 pos = 0;
	u8 id = 0;

	t4_os_pci_read_cfg2(adapter, PCI_STATUS, &status);
	if (!(status & PCI_STATUS_CAP_LIST)) {
		dev_err(adapter, "PCIe capability reading failed\n");
		return -1;
	}

	t4_os_pci_read_cfg(adapter, PCI_CAPABILITY_LIST, &pos);
	while (ttl-- && pos >= 0x40) {
		pos &= ~3;
		t4_os_pci_read_cfg(adapter, (pos + PCI_CAP_LIST_ID), &id);

		if (id == 0xff)
			break;

		if (id == cap)
			return (int)pos;

		t4_os_pci_read_cfg(adapter, (pos + PCI_CAP_LIST_NEXT), &pos);
	}
	return 0;
}

/**
 * t4_os_set_hw_addr - store a port's MAC address in SW
 * @adapter: the adapter
 * @port_idx: the port index
 * @hw_addr: the Ethernet address
 *
 * Store the Ethernet address of the given port in SW.  Called by the
 * common code when it retrieves a port's Ethernet address from EEPROM.
 */
static inline void t4_os_set_hw_addr(struct adapter *adapter, int port_idx,
				     u8 hw_addr[])
{
	struct port_info *pi = &adapter->port[port_idx];

	ether_addr_copy((struct ether_addr *)hw_addr,
			&pi->eth_dev->data->mac_addrs[0]);
}

/**
 * t4_os_lock_init - initialize spinlock
 * @lock: the spinlock
 */
static inline void t4_os_lock_init(rte_spinlock_t *lock)
{
	rte_spinlock_init(lock);
}

/**
 * t4_os_lock - spin until lock is acquired
 * @lock: the spinlock
 */
static inline void t4_os_lock(rte_spinlock_t *lock)
{
	rte_spinlock_lock(lock);
}

/**
 * t4_os_unlock - unlock a spinlock
 * @lock: the spinlock
 */
static inline void t4_os_unlock(rte_spinlock_t *lock)
{
	rte_spinlock_unlock(lock);
}

/**
 * t4_os_trylock - try to get a lock
 * @lock: the spinlock
 */
static inline int t4_os_trylock(rte_spinlock_t *lock)
{
	return rte_spinlock_trylock(lock);
}

/**
 * t4_os_init_list_head - initialize
 * @head: head of list to initialize [to empty]
 */
static inline void t4_os_init_list_head(struct mbox_list *head)
{
	TAILQ_INIT(head);
}

static inline struct mbox_entry *t4_os_list_first_entry(struct mbox_list *head)
{
	return TAILQ_FIRST(head);
}

/**
 * t4_os_atomic_add_tail - Enqueue list element atomically onto list
 * @new: the entry to be addded to the queue
 * @head: current head of the linked list
 * @lock: lock to use to guarantee atomicity
 */
static inline void t4_os_atomic_add_tail(struct mbox_entry *entry,
					 struct mbox_list *head,
					 rte_spinlock_t *lock)
{
	t4_os_lock(lock);
	TAILQ_INSERT_TAIL(head, entry, next);
	t4_os_unlock(lock);
}

/**
 * t4_os_atomic_list_del - Dequeue list element atomically from list
 * @entry: the entry to be remove/dequeued from the list.
 * @lock: the spinlock
 */
static inline void t4_os_atomic_list_del(struct mbox_entry *entry,
					 struct mbox_list *head,
					 rte_spinlock_t *lock)
{
	t4_os_lock(lock);
	TAILQ_REMOVE(head, entry, next);
	t4_os_unlock(lock);
}

/**
 * adap2pinfo - return the port_info of a port
 * @adap: the adapter
 * @idx: the port index
 *
 * Return the port_info structure for the port of the given index.
 */
static inline struct port_info *adap2pinfo(struct adapter *adap, int idx)
{
	return &adap->port[idx];
}

void *t4_alloc_mem(size_t size);
void t4_free_mem(void *addr);
#define t4_os_alloc(_size)     t4_alloc_mem((_size))
#define t4_os_free(_ptr)       t4_free_mem((_ptr))

void t4_os_portmod_changed(const struct adapter *adap, int port_id);
void t4_os_link_changed(struct adapter *adap, int port_id, int link_stat);

void reclaim_completed_tx(struct sge_txq *q);
void t4_free_sge_resources(struct adapter *adap);
void t4_sge_tx_monitor_start(struct adapter *adap);
void t4_sge_tx_monitor_stop(struct adapter *adap);
int t4_eth_xmit(struct sge_eth_txq *txq, struct rte_mbuf *mbuf);
int t4_ethrx_handler(struct sge_rspq *q, const __be64 *rsp,
		     const struct pkt_gl *gl);
int t4_sge_init(struct adapter *adap);
int t4_sge_alloc_eth_txq(struct adapter *adap, struct sge_eth_txq *txq,
			 struct rte_eth_dev *eth_dev, uint16_t queue_id,
			 unsigned int iqid, int socket_id);
int t4_sge_alloc_rxq(struct adapter *adap, struct sge_rspq *rspq, bool fwevtq,
		     struct rte_eth_dev *eth_dev, int intr_idx,
		     struct sge_fl *fl, rspq_handler_t handler,
		     int cong, struct rte_mempool *mp, int queue_id,
		     int socket_id);
int t4_sge_eth_txq_start(struct sge_eth_txq *txq);
int t4_sge_eth_txq_stop(struct sge_eth_txq *txq);
void t4_sge_eth_txq_release(struct adapter *adap, struct sge_eth_txq *txq);
int t4_sge_eth_rxq_start(struct adapter *adap, struct sge_rspq *rq);
int t4_sge_eth_rxq_stop(struct adapter *adap, struct sge_rspq *rq);
void t4_sge_eth_rxq_release(struct adapter *adap, struct sge_eth_rxq *rxq);
void t4_sge_eth_clear_queues(struct port_info *pi);
int cxgb4_set_rspq_intr_params(struct sge_rspq *q, unsigned int us,
			       unsigned int cnt);
int cxgbe_poll(struct sge_rspq *q, struct rte_mbuf **rx_pkts,
	       unsigned int budget, unsigned int *work_done);
int cxgb4_write_rss(const struct port_info *pi, const u16 *queues);

#endif /* __T4_ADAPTER_H__ */
