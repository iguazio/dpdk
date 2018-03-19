/*   SPDX-License-Identifier: BSD-3-Clause
 *   Copyright(c) 2018 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef _CCP_DEV_H_
#define _CCP_DEV_H_

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <rte_bus_pci.h>
#include <rte_atomic.h>
#include <rte_byteorder.h>
#include <rte_io.h>
#include <rte_pci.h>
#include <rte_spinlock.h>
#include <rte_crypto_sym.h>
#include <rte_cryptodev.h>

/**< CCP sspecific */
#define MAX_HW_QUEUES                   5

/**< CCP Register Mappings */
#define Q_MASK_REG                      0x000
#define TRNG_OUT_REG                    0x00c

/* CCP Version 5 Specifics */
#define CMD_QUEUE_MASK_OFFSET		0x00
#define	CMD_QUEUE_PRIO_OFFSET		0x04
#define CMD_REQID_CONFIG_OFFSET		0x08
#define	CMD_CMD_TIMEOUT_OFFSET		0x10
#define LSB_PUBLIC_MASK_LO_OFFSET	0x18
#define LSB_PUBLIC_MASK_HI_OFFSET	0x1C
#define LSB_PRIVATE_MASK_LO_OFFSET	0x20
#define LSB_PRIVATE_MASK_HI_OFFSET	0x24

#define CMD_Q_CONTROL_BASE		0x0000
#define CMD_Q_TAIL_LO_BASE		0x0004
#define CMD_Q_HEAD_LO_BASE		0x0008
#define CMD_Q_INT_ENABLE_BASE		0x000C
#define CMD_Q_INTERRUPT_STATUS_BASE	0x0010

#define CMD_Q_STATUS_BASE		0x0100
#define CMD_Q_INT_STATUS_BASE		0x0104

#define	CMD_CONFIG_0_OFFSET		0x6000
#define	CMD_TRNG_CTL_OFFSET		0x6008
#define	CMD_AES_MASK_OFFSET		0x6010
#define	CMD_CLK_GATE_CTL_OFFSET		0x603C

/* Address offset between two virtual queue registers */
#define CMD_Q_STATUS_INCR		0x1000

/* Bit masks */
#define CMD_Q_RUN			0x1
#define CMD_Q_SIZE			0x1F
#define CMD_Q_SHIFT			3
#define COMMANDS_PER_QUEUE		2048

#define QUEUE_SIZE_VAL                  ((ffs(COMMANDS_PER_QUEUE) - 2) & \
					 CMD_Q_SIZE)
#define Q_DESC_SIZE                     sizeof(struct ccp_desc)
#define Q_SIZE(n)                       (COMMANDS_PER_QUEUE*(n))

#define INT_COMPLETION                  0x1
#define INT_ERROR                       0x2
#define INT_QUEUE_STOPPED               0x4
#define ALL_INTERRUPTS                  (INT_COMPLETION| \
					 INT_ERROR| \
					 INT_QUEUE_STOPPED)

#define LSB_REGION_WIDTH                5
#define MAX_LSB_CNT                     8

#define LSB_SIZE                        16
#define LSB_ITEM_SIZE                   32
#define SLSB_MAP_SIZE                   (MAX_LSB_CNT * LSB_SIZE)

/* bitmap */
enum {
	BITS_PER_WORD = sizeof(unsigned long) * CHAR_BIT
};

#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

#define CCP_DIV_ROUND_UP(n, d)  (((n) + (d) - 1) / (d))
#define CCP_BITMAP_SIZE(nr) \
	CCP_DIV_ROUND_UP(nr, CHAR_BIT * sizeof(unsigned long))

#define CCP_BITMAP_FIRST_WORD_MASK(start) \
	(~0UL << ((start) & (BITS_PER_WORD - 1)))
#define CCP_BITMAP_LAST_WORD_MASK(nbits) \
	(~0UL >> (-(nbits) & (BITS_PER_WORD - 1)))

#define __ccp_round_mask(x, y) ((typeof(x))((y)-1))
#define ccp_round_down(x, y) ((x) & ~__ccp_round_mask(x, y))

/** CCP registers Write/Read */

static inline void ccp_pci_reg_write(void *base, int offset,
				     uint32_t value)
{
	volatile void *reg_addr = ((uint8_t *)base + offset);

	rte_write32((rte_cpu_to_le_32(value)), reg_addr);
}

static inline uint32_t ccp_pci_reg_read(void *base, int offset)
{
	volatile void *reg_addr = ((uint8_t *)base + offset);

	return rte_le_to_cpu_32(rte_read32(reg_addr));
}

#define CCP_READ_REG(hw_addr, reg_offset) \
	ccp_pci_reg_read(hw_addr, reg_offset)

#define CCP_WRITE_REG(hw_addr, reg_offset, value) \
	ccp_pci_reg_write(hw_addr, reg_offset, value)

TAILQ_HEAD(ccp_list, ccp_device);

extern struct ccp_list ccp_list;

/**
 * CCP device version
 */
enum ccp_device_version {
	CCP_VERSION_5A = 0,
	CCP_VERSION_5B,
};

/**
 * A structure describing a CCP command queue.
 */
struct ccp_queue {
	struct ccp_device *dev;
	char memz_name[RTE_MEMZONE_NAMESIZE];

	rte_atomic64_t free_slots;
	/**< available free slots updated from enq/deq calls */

	/* Queue identifier */
	uint64_t id;	/**< queue id */
	uint64_t qidx;	/**< queue index */
	uint64_t qsize;	/**< queue size */

	/* Queue address */
	struct ccp_desc *qbase_desc;
	void *qbase_addr;
	phys_addr_t qbase_phys_addr;
	/**< queue-page registers addr */
	void *reg_base;

	uint32_t qcontrol;
	/**< queue ctrl reg */

	int lsb;
	/**< lsb region assigned to queue */
	unsigned long lsbmask;
	/**< lsb regions queue can access */
	unsigned long lsbmap[CCP_BITMAP_SIZE(LSB_SIZE)];
	/**< all lsb resources which queue is using */
	uint32_t sb_key;
	/**< lsb assigned for queue */
	uint32_t sb_iv;
	/**< lsb assigned for iv */
	uint32_t sb_sha;
	/**< lsb assigned for sha ctx */
	uint32_t sb_hmac;
	/**< lsb assigned for hmac ctx */
} ____cacheline_aligned;

/**
 * A structure describing a CCP device.
 */
struct ccp_device {
	TAILQ_ENTRY(ccp_device) next;
	int id;
	/**< ccp dev id on platform */
	struct ccp_queue cmd_q[MAX_HW_QUEUES];
	/**< ccp queue */
	int cmd_q_count;
	/**< no. of ccp Queues */
	struct rte_pci_device pci;
	/**< ccp pci identifier */
	unsigned long lsbmap[CCP_BITMAP_SIZE(SLSB_MAP_SIZE)];
	/**< shared lsb mask of ccp */
	rte_spinlock_t lsb_lock;
	/**< protection for shared lsb region allocation */
	int qidx;
	/**< current queue index */
} __rte_cache_aligned;

/**
 * descriptor for version 5 CPP commands
 * 8 32-bit words:
 * word 0: function; engine; control bits
 * word 1: length of source data
 * word 2: low 32 bits of source pointer
 * word 3: upper 16 bits of source pointer; source memory type
 * word 4: low 32 bits of destination pointer
 * word 5: upper 16 bits of destination pointer; destination memory
 * type
 * word 6: low 32 bits of key pointer
 * word 7: upper 16 bits of key pointer; key memory type
 */
struct dword0 {
	uint32_t soc:1;
	uint32_t ioc:1;
	uint32_t rsvd1:1;
	uint32_t init:1;
	uint32_t eom:1;
	uint32_t function:15;
	uint32_t engine:4;
	uint32_t prot:1;
	uint32_t rsvd2:7;
};

struct dword3 {
	uint32_t src_hi:16;
	uint32_t src_mem:2;
	uint32_t lsb_cxt_id:8;
	uint32_t rsvd1:5;
	uint32_t fixed:1;
};

union dword4 {
	uint32_t dst_lo;	/* NON-SHA */
	uint32_t sha_len_lo;	/* SHA */
};

union dword5 {
	struct {
		uint32_t dst_hi:16;
		uint32_t dst_mem:2;
		uint32_t rsvd1:13;
		uint32_t fixed:1;
	}
	fields;
	uint32_t sha_len_hi;
};

struct dword7 {
	uint32_t key_hi:16;
	uint32_t key_mem:2;
	uint32_t rsvd1:14;
};

struct ccp_desc {
	struct dword0 dw0;
	uint32_t length;
	uint32_t src_lo;
	struct dword3 dw3;
	union dword4 dw4;
	union dword5 dw5;
	uint32_t key_lo;
	struct dword7 dw7;
};

static inline uint32_t
low32_value(unsigned long addr)
{
	return ((uint64_t)addr) & 0x0ffffffff;
}

static inline uint32_t
high32_value(unsigned long addr)
{
	return ((uint64_t)addr >> 32) & 0x00000ffff;
}

/**
 * Detect ccp platform and initialize all ccp devices
 *
 * @param ccp_id rte_pci_id list for supported CCP devices
 * @return no. of successfully initialized CCP devices
 */
int ccp_probe_devices(const struct rte_pci_id *ccp_id);

#endif /* _CCP_DEV_H_ */
