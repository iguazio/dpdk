/*
 * Copyright (c) 2016 QLogic Corporation.
 * All rights reserved.
 * www.qlogic.com
 *
 * See LICENSE.qede_pmd for copyright and licensing details.
 */

#ifndef __ECORE_INT_API_H__
#define __ECORE_INT_API_H__

#ifndef __EXTRACT__LINUX__
#define ECORE_SB_IDX		0x0002

#define RX_PI		0
#define TX_PI(tc)	(RX_PI + 1 + tc)

#ifndef ECORE_INT_MODE
#define ECORE_INT_MODE
enum ecore_int_mode {
	ECORE_INT_MODE_INTA,
	ECORE_INT_MODE_MSIX,
	ECORE_INT_MODE_MSI,
	ECORE_INT_MODE_POLL,
};
#endif

struct ecore_sb_info {
	struct status_block *sb_virt;
	dma_addr_t sb_phys;
	u32 sb_ack;		/* Last given ack */
	u16 igu_sb_id;
	void OSAL_IOMEM *igu_addr;
	u8 flags;
#define ECORE_SB_INFO_INIT	0x1
#define ECORE_SB_INFO_SETUP	0x2

#ifdef ECORE_CONFIG_DIRECT_HWFN
	struct ecore_hwfn *p_hwfn;
#endif
	struct ecore_dev *p_dev;
};

struct ecore_sb_cnt_info {
	int sb_cnt;
	int sb_iov_cnt;
	int sb_free_blk;
};

static OSAL_INLINE u16 ecore_sb_update_sb_idx(struct ecore_sb_info *sb_info)
{
	u32 prod = 0;
	u16 rc = 0;

	/* barrier(); status block is written to by the chip */
	/* FIXME: need some sort of barrier. */
	prod = OSAL_LE32_TO_CPU(sb_info->sb_virt->prod_index) &
	    STATUS_BLOCK_PROD_INDEX_MASK;
	if (sb_info->sb_ack != prod) {
		sb_info->sb_ack = prod;
		rc |= ECORE_SB_IDX;
	}

	OSAL_MMIOWB(sb_info->p_dev);
	return rc;
}

/**
 *
 * @brief This function creates an update command for interrupts that is
 *        written to the IGU.
 *
 * @param sb_info	- This is the structure allocated and
 *	   initialized per status block. Assumption is
 *	   that it was initialized using ecore_sb_init
 * @param int_cmd	- Enable/Disable/Nop
 * @param upd_flg	- whether igu consumer should be
 *	   updated.
 *
 * @return OSAL_INLINE void
 */
static OSAL_INLINE void ecore_sb_ack(struct ecore_sb_info *sb_info,
				     enum igu_int_cmd int_cmd, u8 upd_flg)
{
	struct igu_prod_cons_update igu_ack = { 0 };

	igu_ack.sb_id_and_flags =
	    ((sb_info->sb_ack << IGU_PROD_CONS_UPDATE_SB_INDEX_SHIFT) |
	     (upd_flg << IGU_PROD_CONS_UPDATE_UPDATE_FLAG_SHIFT) |
	     (int_cmd << IGU_PROD_CONS_UPDATE_ENABLE_INT_SHIFT) |
	     (IGU_SEG_ACCESS_REG << IGU_PROD_CONS_UPDATE_SEGMENT_ACCESS_SHIFT));

#ifdef ECORE_CONFIG_DIRECT_HWFN
	DIRECT_REG_WR(sb_info->p_hwfn, sb_info->igu_addr,
		      igu_ack.sb_id_and_flags);
#else
	DIRECT_REG_WR(OSAL_NULL, sb_info->igu_addr, igu_ack.sb_id_and_flags);
#endif
	/* Both segments (interrupts & acks) are written to same place address;
	 * Need to guarantee all commands will be received (in-order) by HW.
	 */
	OSAL_MMIOWB(sb_info->p_dev);
	OSAL_BARRIER(sb_info->p_dev);
}

#ifdef ECORE_CONFIG_DIRECT_HWFN
static OSAL_INLINE void __internal_ram_wr(struct ecore_hwfn *p_hwfn,
					  void OSAL_IOMEM *addr,
					  int size, u32 *data)
#else
static OSAL_INLINE void __internal_ram_wr(void *p_hwfn,
					  void OSAL_IOMEM *addr,
					  int size, u32 *data)
#endif
{
	unsigned int i;

	for (i = 0; i < size / sizeof(*data); i++)
		DIRECT_REG_WR(p_hwfn, &((u32 OSAL_IOMEM *)addr)[i], data[i]);
}

#ifdef ECORE_CONFIG_DIRECT_HWFN
static OSAL_INLINE void internal_ram_wr(struct ecore_hwfn *p_hwfn,
					void OSAL_IOMEM *addr,
					int size, u32 *data)
{
	__internal_ram_wr(p_hwfn, addr, size, data);
}
#else
static OSAL_INLINE void internal_ram_wr(void OSAL_IOMEM *addr,
					int size, u32 *data)
{
	__internal_ram_wr(OSAL_NULL, addr, size, data);
}
#endif
#endif

struct ecore_hwfn;
struct ecore_ptt;

enum ecore_coalescing_fsm {
	ECORE_COAL_RX_STATE_MACHINE,
	ECORE_COAL_TX_STATE_MACHINE
};

/**
 * @brief ecore_int_cau_conf_pi - configure cau for a given
 *        status block
 *
 * @param p_hwfn
 * @param p_ptt
 * @param igu_sb_id
 * @param pi_index
 * @param state
 * @param timeset
 */
void ecore_int_cau_conf_pi(struct ecore_hwfn *p_hwfn,
			   struct ecore_ptt *p_ptt,
			   u16 igu_sb_id,
			   u32 pi_index,
			   enum ecore_coalescing_fsm coalescing_fsm,
			   u8 timeset);

/**
 *
 * @brief ecore_int_igu_enable_int - enable device interrupts
 *
 * @param p_hwfn
 * @param p_ptt
 * @param int_mode - interrupt mode to use
 */
void ecore_int_igu_enable_int(struct ecore_hwfn *p_hwfn,
			      struct ecore_ptt *p_ptt,
			      enum ecore_int_mode int_mode);

/**
 *
 * @brief ecore_int_igu_disable_int - disable device interrupts
 *
 * @param p_hwfn
 * @param p_ptt
 */
void ecore_int_igu_disable_int(struct ecore_hwfn *p_hwfn,
			       struct ecore_ptt *p_ptt);

/**
 *
 * @brief ecore_int_igu_read_sisr_reg - Reads the single isr multiple dpc
 *        register from igu.
 *
 * @param p_hwfn
 *
 * @return u64
 */
u64 ecore_int_igu_read_sisr_reg(struct ecore_hwfn *p_hwfn);

#define ECORE_SP_SB_ID 0xffff
/**
 * @brief ecore_int_sb_init - Initializes the sb_info structure.
 *
 * once the structure is initialized it can be passed to sb related functions.
 *
 * @param p_hwfn
 * @param p_ptt
 * @param sb_info	points to an uninitialized (but
 *			allocated) sb_info structure
 * @param sb_virt_addr
 * @param sb_phy_addr
 * @param sb_id		the sb_id to be used (zero based in driver)
 *			should use ECORE_SP_SB_ID for SP Status block
 *
 * @return enum _ecore_status_t
 */
enum _ecore_status_t ecore_int_sb_init(struct ecore_hwfn *p_hwfn,
				       struct ecore_ptt *p_ptt,
				       struct ecore_sb_info *sb_info,
				       void *sb_virt_addr,
				       dma_addr_t sb_phy_addr, u16 sb_id);
/**
 * @brief ecore_int_sb_setup - Setup the sb.
 *
 * @param p_hwfn
 * @param p_ptt
 * @param sb_info	initialized sb_info structure
 */
void ecore_int_sb_setup(struct ecore_hwfn *p_hwfn,
			struct ecore_ptt *p_ptt, struct ecore_sb_info *sb_info);

/**
 * @brief ecore_int_sb_release - releases the sb_info structure.
 *
 * once the structure is released, it's memory can be freed
 *
 * @param p_hwfn
 * @param sb_info	points to an allocated sb_info structure
 * @param sb_id		the sb_id to be used (zero based in driver)
 *			should never be equal to ECORE_SP_SB_ID
 *			(SP Status block)
 *
 * @return enum _ecore_status_t
 */
enum _ecore_status_t ecore_int_sb_release(struct ecore_hwfn *p_hwfn,
					  struct ecore_sb_info *sb_info,
					  u16 sb_id);

/**
 * @brief ecore_int_sp_dpc - To be called when an interrupt is received on the
 *        default status block.
 *
 * @param p_hwfn - pointer to hwfn
 *
 */
void ecore_int_sp_dpc(osal_int_ptr_t hwfn_cookie);

/**
 * @brief ecore_int_get_num_sbs - get the number of status
 *        blocks configured for this funciton in the igu.
 *
 * @param p_hwfn
 * @param p_sb_cnt_info
 *
 * @return
 */
void ecore_int_get_num_sbs(struct ecore_hwfn *p_hwfn,
			   struct ecore_sb_cnt_info *p_sb_cnt_info);

/**
 * @brief ecore_int_disable_post_isr_release - performs the cleanup post ISR
 *        release. The API need to be called after releasing all slowpath IRQs
 *        of the device.
 *
 * @param p_dev
 *
 */
void ecore_int_disable_post_isr_release(struct ecore_dev *p_dev);

#endif
