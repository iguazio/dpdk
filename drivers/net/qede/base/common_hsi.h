/*
 * Copyright (c) 2016 QLogic Corporation.
 * All rights reserved.
 * www.qlogic.com
 *
 * See LICENSE.qede_pmd for copyright and licensing details.
 */

#ifndef __COMMON_HSI__
#define __COMMON_HSI__
/********************************/
/* PROTOCOL COMMON FW CONSTANTS */
/********************************/

/* Temporarily here should be added to HSI automatically by resource allocation
 * tool.
 */
#define T_TEST_AGG_INT_TEMP    6
#define	M_TEST_AGG_INT_TEMP    8
#define	U_TEST_AGG_INT_TEMP    6
#define	X_TEST_AGG_INT_TEMP    14
#define	Y_TEST_AGG_INT_TEMP    4
#define	P_TEST_AGG_INT_TEMP    4

#define X_FINAL_CLEANUP_AGG_INT  1

#define EVENT_RING_PAGE_SIZE_BYTES          4096

#define NUM_OF_GLOBAL_QUEUES				128
#define COMMON_QUEUE_ENTRY_MAX_BYTE_SIZE	64

#define ISCSI_CDU_TASK_SEG_TYPE       0
#define FCOE_CDU_TASK_SEG_TYPE        0
#define RDMA_CDU_TASK_SEG_TYPE        1

#define FW_ASSERT_GENERAL_ATTN_IDX    32

#define MAX_PINNED_CCFC			32

#define EAGLE_ENG1_WORKAROUND_NIG_FLOWCTRL_MODE	3

/* Queue Zone sizes in bytes */
#define TSTORM_QZONE_SIZE    8	 /*tstorm_scsi_queue_zone*/
#define MSTORM_QZONE_SIZE    16  /*mstorm_eth_queue_zone. Used only for RX
				  *producer of VFs in backward compatibility
				  *mode.
				  */
#define USTORM_QZONE_SIZE    8	 /*ustorm_eth_queue_zone*/
#define XSTORM_QZONE_SIZE    8	 /*xstorm_eth_queue_zone*/
#define YSTORM_QZONE_SIZE    0
#define PSTORM_QZONE_SIZE    0

/*Log of mstorm default VF zone size.*/
#define MSTORM_VF_ZONE_DEFAULT_SIZE_LOG       7
/*Maximum number of RX queues that can be allocated to VF by default*/
#define ETH_MAX_NUM_RX_QUEUES_PER_VF_DEFAULT  16
/*Maximum number of RX queues that can be allocated to VF with doubled VF zone
 * size. Up to 96 VF supported in this mode
 */
#define ETH_MAX_NUM_RX_QUEUES_PER_VF_DOUBLE   48
/*Maximum number of RX queues that can be allocated to VF with 4 VF zone size.
 * Up to 48 VF supported in this mode
 */
#define ETH_MAX_NUM_RX_QUEUES_PER_VF_QUAD     112


/********************************/
/* CORE (LIGHT L2) FW CONSTANTS */
/********************************/

#define CORE_LL2_MAX_RAMROD_PER_CON				8
#define CORE_LL2_TX_BD_PAGE_SIZE_BYTES			4096
#define CORE_LL2_RX_BD_PAGE_SIZE_BYTES			4096
#define CORE_LL2_RX_CQE_PAGE_SIZE_BYTES			4096
#define CORE_LL2_RX_NUM_NEXT_PAGE_BDS			1

#define CORE_LL2_TX_MAX_BDS_PER_PACKET				12

#define CORE_SPQE_PAGE_SIZE_BYTES                       4096

#define MAX_NUM_LL2_RX_QUEUES					32
#define MAX_NUM_LL2_TX_STATS_COUNTERS			32


/****************************************************************************/
/* Include firmware version number only- do not add constants here to avoid */
/* redundunt compilations                                                   */
/****************************************************************************/


#define FW_MAJOR_VERSION		8
#define FW_MINOR_VERSION		10
#define FW_REVISION_VERSION		9
#define FW_ENGINEERING_VERSION	0

/***********************/
/* COMMON HW CONSTANTS */
/***********************/

/* PCI functions */
#define MAX_NUM_PORTS_K2	(4)
#define MAX_NUM_PORTS_BB	(2)
#define MAX_NUM_PORTS		(MAX_NUM_PORTS_K2)

#define MAX_NUM_PFS_K2	(16)
#define MAX_NUM_PFS_BB	(8)
#define MAX_NUM_PFS	(MAX_NUM_PFS_K2)
#define MAX_NUM_OF_PFS_IN_CHIP (16) /* On both engines */

#define MAX_NUM_VFS_K2	(192)
#define MAX_NUM_VFS_BB	(120)
#define MAX_NUM_VFS	(MAX_NUM_VFS_K2)

#define MAX_NUM_FUNCTIONS_BB	(MAX_NUM_PFS_BB + MAX_NUM_VFS_BB)
#define MAX_NUM_FUNCTIONS_K2	(MAX_NUM_PFS_K2 + MAX_NUM_VFS_K2)
#define MAX_NUM_FUNCTIONS	(MAX_NUM_PFS + MAX_NUM_VFS)

/* in both BB and K2, the VF number starts from 16. so for arrays containing all
 * possible PFs and VFs - we need a constant for this size
 */
#define MAX_FUNCTION_NUMBER_BB	(MAX_NUM_PFS + MAX_NUM_VFS_BB)
#define MAX_FUNCTION_NUMBER_K2	(MAX_NUM_PFS + MAX_NUM_VFS_K2)
#define MAX_FUNCTION_NUMBER	(MAX_NUM_PFS + MAX_NUM_VFS)

#define MAX_NUM_VPORTS_K2	(208)
#define MAX_NUM_VPORTS_BB	(160)
#define MAX_NUM_VPORTS		(MAX_NUM_VPORTS_K2)

#define MAX_NUM_L2_QUEUES_K2	(320)
#define MAX_NUM_L2_QUEUES_BB	(256)
#define MAX_NUM_L2_QUEUES	(MAX_NUM_L2_QUEUES_K2)

/* Traffic classes in network-facing blocks (PBF, BTB, NIG, BRB, PRS and QM) */
/* 4-Port K2. */
#define NUM_PHYS_TCS_4PORT_K2	(4)
#define NUM_OF_PHYS_TCS		(8)

#define NUM_TCS_4PORT_K2	(NUM_PHYS_TCS_4PORT_K2 + 1)
#define NUM_OF_TCS		(NUM_OF_PHYS_TCS + 1)

#define LB_TC			(NUM_OF_PHYS_TCS)

/* Num of possible traffic priority values */
#define NUM_OF_PRIO		(8)

#define MAX_NUM_VOQS_K2		(NUM_TCS_4PORT_K2 * MAX_NUM_PORTS_K2)
#define MAX_NUM_VOQS_BB		(NUM_OF_TCS * MAX_NUM_PORTS_BB)
#define MAX_NUM_VOQS		(MAX_NUM_VOQS_K2)
#define MAX_PHYS_VOQS		(NUM_OF_PHYS_TCS * MAX_NUM_PORTS_BB)

/* CIDs */
#define NUM_OF_CONNECTION_TYPES	(8)
#define NUM_OF_LCIDS		(320)
#define NUM_OF_LTIDS		(320)

/* Clock values */
#define MASTER_CLK_FREQ_E4		(375e6)
#define STORM_CLK_FREQ_E4		(1000e6)
#define CLK25M_CLK_FREQ_E4		(25e6)

/* Global PXP windows (GTT) */
#define NUM_OF_GTT			19
#define GTT_DWORD_SIZE_BITS	10
#define GTT_BYTE_SIZE_BITS	(GTT_DWORD_SIZE_BITS + 2)
#define GTT_DWORD_SIZE		(1 << GTT_DWORD_SIZE_BITS)

/* Tools Version */
#define TOOLS_VERSION 10
/*****************/
/* CDU CONSTANTS */
/*****************/

#define CDU_SEG_TYPE_OFFSET_REG_TYPE_SHIFT              (17)
#define CDU_SEG_TYPE_OFFSET_REG_OFFSET_MASK             (0x1ffff)

#define CDU_VF_FL_SEG_TYPE_OFFSET_REG_TYPE_SHIFT	(12)
#define CDU_VF_FL_SEG_TYPE_OFFSET_REG_OFFSET_MASK	(0xfff)


/*****************/
/* DQ CONSTANTS  */
/*****************/

/* DEMS */
#define DQ_DEMS_LEGACY			0
#define DQ_DEMS_TOE_MORE_TO_SEND			3
#define DQ_DEMS_TOE_LOCAL_ADV_WND			4
#define DQ_DEMS_ROCE_CQ_CONS				7

/* XCM agg val selection (HW) */
#define DQ_XCM_AGG_VAL_SEL_WORD2  0
#define DQ_XCM_AGG_VAL_SEL_WORD3  1
#define DQ_XCM_AGG_VAL_SEL_WORD4  2
#define DQ_XCM_AGG_VAL_SEL_WORD5  3
#define DQ_XCM_AGG_VAL_SEL_REG3   4
#define DQ_XCM_AGG_VAL_SEL_REG4   5
#define DQ_XCM_AGG_VAL_SEL_REG5   6
#define DQ_XCM_AGG_VAL_SEL_REG6   7

/* XCM agg val selection (FW) */
#define DQ_XCM_ETH_EDPM_NUM_BDS_CMD \
	DQ_XCM_AGG_VAL_SEL_WORD2
#define DQ_XCM_ETH_TX_BD_CONS_CMD \
	DQ_XCM_AGG_VAL_SEL_WORD3
#define DQ_XCM_CORE_TX_BD_CONS_CMD \
	DQ_XCM_AGG_VAL_SEL_WORD3
#define DQ_XCM_ETH_TX_BD_PROD_CMD \
	DQ_XCM_AGG_VAL_SEL_WORD4
#define DQ_XCM_CORE_TX_BD_PROD_CMD \
	DQ_XCM_AGG_VAL_SEL_WORD4
#define DQ_XCM_CORE_SPQ_PROD_CMD \
	DQ_XCM_AGG_VAL_SEL_WORD4
#define DQ_XCM_ETH_GO_TO_BD_CONS_CMD            DQ_XCM_AGG_VAL_SEL_WORD5
#define DQ_XCM_FCOE_SQ_CONS_CMD             DQ_XCM_AGG_VAL_SEL_WORD3
#define DQ_XCM_FCOE_SQ_PROD_CMD             DQ_XCM_AGG_VAL_SEL_WORD4
#define DQ_XCM_FCOE_X_FERQ_PROD_CMD         DQ_XCM_AGG_VAL_SEL_WORD5
#define DQ_XCM_ISCSI_SQ_CONS_CMD            DQ_XCM_AGG_VAL_SEL_WORD3
#define DQ_XCM_ISCSI_SQ_PROD_CMD            DQ_XCM_AGG_VAL_SEL_WORD4
#define DQ_XCM_ISCSI_MORE_TO_SEND_SEQ_CMD   DQ_XCM_AGG_VAL_SEL_REG3
#define DQ_XCM_ISCSI_EXP_STAT_SN_CMD        DQ_XCM_AGG_VAL_SEL_REG6
#define DQ_XCM_ROCE_SQ_PROD_CMD             DQ_XCM_AGG_VAL_SEL_WORD4
#define DQ_XCM_TOE_TX_BD_PROD_CMD           DQ_XCM_AGG_VAL_SEL_WORD4
#define DQ_XCM_TOE_MORE_TO_SEND_SEQ_CMD     DQ_XCM_AGG_VAL_SEL_REG3
#define DQ_XCM_TOE_LOCAL_ADV_WND_SEQ_CMD    DQ_XCM_AGG_VAL_SEL_REG4

/* UCM agg val selection (HW) */
#define DQ_UCM_AGG_VAL_SEL_WORD0  0
#define DQ_UCM_AGG_VAL_SEL_WORD1  1
#define DQ_UCM_AGG_VAL_SEL_WORD2  2
#define DQ_UCM_AGG_VAL_SEL_WORD3  3
#define DQ_UCM_AGG_VAL_SEL_REG0   4
#define DQ_UCM_AGG_VAL_SEL_REG1   5
#define DQ_UCM_AGG_VAL_SEL_REG2   6
#define DQ_UCM_AGG_VAL_SEL_REG3   7

/* UCM agg val selection (FW) */
#define DQ_UCM_ETH_PMD_TX_CONS_CMD			DQ_UCM_AGG_VAL_SEL_WORD2
#define DQ_UCM_ETH_PMD_RX_CONS_CMD			DQ_UCM_AGG_VAL_SEL_WORD3
#define DQ_UCM_ROCE_CQ_CONS_CMD				DQ_UCM_AGG_VAL_SEL_REG0
#define DQ_UCM_ROCE_CQ_PROD_CMD				DQ_UCM_AGG_VAL_SEL_REG2

/* TCM agg val selection (HW) */
#define DQ_TCM_AGG_VAL_SEL_WORD0  0
#define DQ_TCM_AGG_VAL_SEL_WORD1  1
#define DQ_TCM_AGG_VAL_SEL_WORD2  2
#define DQ_TCM_AGG_VAL_SEL_WORD3  3
#define DQ_TCM_AGG_VAL_SEL_REG1   4
#define DQ_TCM_AGG_VAL_SEL_REG2   5
#define DQ_TCM_AGG_VAL_SEL_REG6   6
#define DQ_TCM_AGG_VAL_SEL_REG9   7

/* TCM agg val selection (FW) */
#define DQ_TCM_L2B_BD_PROD_CMD				DQ_TCM_AGG_VAL_SEL_WORD1
#define DQ_TCM_ROCE_RQ_PROD_CMD				DQ_TCM_AGG_VAL_SEL_WORD0

/* XCM agg counter flag selection (HW) */
#define DQ_XCM_AGG_FLG_SHIFT_BIT14  0
#define DQ_XCM_AGG_FLG_SHIFT_BIT15  1
#define DQ_XCM_AGG_FLG_SHIFT_CF12   2
#define DQ_XCM_AGG_FLG_SHIFT_CF13   3
#define DQ_XCM_AGG_FLG_SHIFT_CF18   4
#define DQ_XCM_AGG_FLG_SHIFT_CF19   5
#define DQ_XCM_AGG_FLG_SHIFT_CF22   6
#define DQ_XCM_AGG_FLG_SHIFT_CF23   7

/* XCM agg counter flag selection (FW) */
#define DQ_XCM_ETH_DQ_CF_CMD		(1 << \
					DQ_XCM_AGG_FLG_SHIFT_CF18)
#define DQ_XCM_CORE_DQ_CF_CMD		(1 << \
					DQ_XCM_AGG_FLG_SHIFT_CF18)
#define DQ_XCM_ETH_TERMINATE_CMD	(1 << \
					DQ_XCM_AGG_FLG_SHIFT_CF19)
#define DQ_XCM_CORE_TERMINATE_CMD	(1 << \
					DQ_XCM_AGG_FLG_SHIFT_CF19)
#define DQ_XCM_ETH_SLOW_PATH_CMD	(1 << \
					DQ_XCM_AGG_FLG_SHIFT_CF22)
#define DQ_XCM_CORE_SLOW_PATH_CMD	(1 << \
					DQ_XCM_AGG_FLG_SHIFT_CF22)
#define DQ_XCM_ETH_TPH_EN_CMD		(1 << \
					DQ_XCM_AGG_FLG_SHIFT_CF23)
#define DQ_XCM_FCOE_SLOW_PATH_CMD           (1 << DQ_XCM_AGG_FLG_SHIFT_CF22)
#define DQ_XCM_ISCSI_DQ_FLUSH_CMD           (1 << DQ_XCM_AGG_FLG_SHIFT_CF19)
#define DQ_XCM_ISCSI_SLOW_PATH_CMD          (1 << DQ_XCM_AGG_FLG_SHIFT_CF22)
#define DQ_XCM_ISCSI_PROC_ONLY_CLEANUP_CMD  (1 << DQ_XCM_AGG_FLG_SHIFT_CF23)
#define DQ_XCM_TOE_DQ_FLUSH_CMD             (1 << DQ_XCM_AGG_FLG_SHIFT_CF19)
#define DQ_XCM_TOE_SLOW_PATH_CMD            (1 << DQ_XCM_AGG_FLG_SHIFT_CF22)

/* UCM agg counter flag selection (HW) */
#define DQ_UCM_AGG_FLG_SHIFT_CF0       0
#define DQ_UCM_AGG_FLG_SHIFT_CF1       1
#define DQ_UCM_AGG_FLG_SHIFT_CF3       2
#define DQ_UCM_AGG_FLG_SHIFT_CF4       3
#define DQ_UCM_AGG_FLG_SHIFT_CF5       4
#define DQ_UCM_AGG_FLG_SHIFT_CF6       5
#define DQ_UCM_AGG_FLG_SHIFT_RULE0EN   6
#define DQ_UCM_AGG_FLG_SHIFT_RULE1EN   7

/* UCM agg counter flag selection (FW) */
#define DQ_UCM_ETH_PMD_TX_ARM_CMD           (1 << DQ_UCM_AGG_FLG_SHIFT_CF4)
#define DQ_UCM_ETH_PMD_RX_ARM_CMD           (1 << DQ_UCM_AGG_FLG_SHIFT_CF5)
#define DQ_UCM_ROCE_CQ_ARM_SE_CF_CMD        (1 << DQ_UCM_AGG_FLG_SHIFT_CF4)
#define DQ_UCM_ROCE_CQ_ARM_CF_CMD           (1 << DQ_UCM_AGG_FLG_SHIFT_CF5)
#define DQ_UCM_TOE_TIMER_STOP_ALL_CMD       (1 << DQ_UCM_AGG_FLG_SHIFT_CF3)
#define DQ_UCM_TOE_SLOW_PATH_CF_CMD         (1 << DQ_UCM_AGG_FLG_SHIFT_CF4)
#define DQ_UCM_TOE_DQ_CF_CMD                (1 << DQ_UCM_AGG_FLG_SHIFT_CF5)

/* TCM agg counter flag selection (HW) */
#define DQ_TCM_AGG_FLG_SHIFT_CF0  0
#define DQ_TCM_AGG_FLG_SHIFT_CF1  1
#define DQ_TCM_AGG_FLG_SHIFT_CF2  2
#define DQ_TCM_AGG_FLG_SHIFT_CF3  3
#define DQ_TCM_AGG_FLG_SHIFT_CF4  4
#define DQ_TCM_AGG_FLG_SHIFT_CF5  5
#define DQ_TCM_AGG_FLG_SHIFT_CF6  6
#define DQ_TCM_AGG_FLG_SHIFT_CF7  7

/* TCM agg counter flag selection (FW) */
#define DQ_TCM_FCOE_FLUSH_Q0_CMD            (1 << DQ_TCM_AGG_FLG_SHIFT_CF1)
#define DQ_TCM_FCOE_DUMMY_TIMER_CMD         (1 << DQ_TCM_AGG_FLG_SHIFT_CF2)
#define DQ_TCM_FCOE_TIMER_STOP_ALL_CMD      (1 << DQ_TCM_AGG_FLG_SHIFT_CF3)
#define DQ_TCM_ISCSI_FLUSH_Q0_CMD           (1 << DQ_TCM_AGG_FLG_SHIFT_CF1)
#define DQ_TCM_ISCSI_TIMER_STOP_ALL_CMD     (1 << DQ_TCM_AGG_FLG_SHIFT_CF3)
#define DQ_TCM_TOE_FLUSH_Q0_CMD             (1 << DQ_TCM_AGG_FLG_SHIFT_CF1)
#define DQ_TCM_TOE_TIMER_STOP_ALL_CMD       (1 << DQ_TCM_AGG_FLG_SHIFT_CF3)
#define DQ_TCM_IWARP_POST_RQ_CF_CMD         (1 << DQ_TCM_AGG_FLG_SHIFT_CF1)

/* PWM address mapping */
#define DQ_PWM_OFFSET_DPM_BASE				0x0
#define DQ_PWM_OFFSET_DPM_END				0x27
#define DQ_PWM_OFFSET_XCM16_BASE			0x40
#define DQ_PWM_OFFSET_XCM32_BASE			0x44
#define DQ_PWM_OFFSET_UCM16_BASE			0x48
#define DQ_PWM_OFFSET_UCM32_BASE			0x4C
#define DQ_PWM_OFFSET_UCM16_4				0x50
#define DQ_PWM_OFFSET_TCM16_BASE			0x58
#define DQ_PWM_OFFSET_TCM32_BASE			0x5C
#define DQ_PWM_OFFSET_XCM_FLAGS				0x68
#define DQ_PWM_OFFSET_UCM_FLAGS				0x69
#define DQ_PWM_OFFSET_TCM_FLAGS				0x6B

#define DQ_PWM_OFFSET_XCM_RDMA_SQ_PROD		(DQ_PWM_OFFSET_XCM16_BASE + 2)
#define DQ_PWM_OFFSET_UCM_RDMA_CQ_CONS_32BIT	(DQ_PWM_OFFSET_UCM32_BASE)
#define DQ_PWM_OFFSET_UCM_RDMA_CQ_CONS_16BIT	(DQ_PWM_OFFSET_UCM16_4)
#define DQ_PWM_OFFSET_UCM_RDMA_INT_TIMEOUT	(DQ_PWM_OFFSET_UCM16_BASE + 2)
#define DQ_PWM_OFFSET_UCM_RDMA_ARM_FLAGS	(DQ_PWM_OFFSET_UCM_FLAGS)
#define DQ_PWM_OFFSET_TCM_ROCE_RQ_PROD		(DQ_PWM_OFFSET_TCM16_BASE + 1)
#define DQ_PWM_OFFSET_TCM_IWARP_RQ_PROD		(DQ_PWM_OFFSET_TCM16_BASE + 3)

#define DQ_REGION_SHIFT				        (12)

/* DPM */
#define	DQ_DPM_WQE_BUFF_SIZE			    (320)

/* Conn type ranges */
#define DQ_CONN_TYPE_RANGE_SHIFT			(4)

/*****************/
/* QM CONSTANTS  */
/*****************/

/* number of TX queues in the QM */
#define MAX_QM_TX_QUEUES_K2	512
#define MAX_QM_TX_QUEUES_BB	448
#define MAX_QM_TX_QUEUES	MAX_QM_TX_QUEUES_K2

/* number of Other queues in the QM */
#define MAX_QM_OTHER_QUEUES_BB	64
#define MAX_QM_OTHER_QUEUES_K2	128
#define MAX_QM_OTHER_QUEUES	MAX_QM_OTHER_QUEUES_K2

/* number of queues in a PF queue group */
#define QM_PF_QUEUE_GROUP_SIZE	8

/* the size of a single queue element in bytes */
#define QM_PQ_ELEMENT_SIZE			4

/* base number of Tx PQs in the CM PQ representation.
 * should be used when storing PQ IDs in CM PQ registers and context
 */
#define CM_TX_PQ_BASE	0x200

/* number of global Vport/QCN rate limiters */
#define MAX_QM_GLOBAL_RLS			256

/* QM registers data */
#define QM_LINE_CRD_REG_WIDTH		16
#define QM_LINE_CRD_REG_SIGN_BIT	(1 << (QM_LINE_CRD_REG_WIDTH - 1))
#define QM_BYTE_CRD_REG_WIDTH		24
#define QM_BYTE_CRD_REG_SIGN_BIT	(1 << (QM_BYTE_CRD_REG_WIDTH - 1))
#define QM_WFQ_CRD_REG_WIDTH		32
#define QM_WFQ_CRD_REG_SIGN_BIT		(1 << (QM_WFQ_CRD_REG_WIDTH - 1))
#define QM_RL_CRD_REG_WIDTH		32
#define QM_RL_CRD_REG_SIGN_BIT		(1 << (QM_RL_CRD_REG_WIDTH - 1))

/*****************/
/* CAU CONSTANTS */
/*****************/

#define CAU_FSM_ETH_RX  0
#define CAU_FSM_ETH_TX  1

/* Number of Protocol Indices per Status Block */
#define PIS_PER_SB    12

/* fsm is stopped or not valid for this sb */
#define CAU_HC_STOPPED_STATE	3
/* fsm is working without interrupt coalescing for this sb*/
#define CAU_HC_DISABLE_STATE	4
/* fsm is working with interrupt coalescing for this sb*/
#define CAU_HC_ENABLE_STATE	0


/*****************/
/* IGU CONSTANTS */
/*****************/

#define MAX_SB_PER_PATH_K2	(368)
#define MAX_SB_PER_PATH_BB	(288)
#define MAX_TOT_SB_PER_PATH \
	MAX_SB_PER_PATH_K2

#define MAX_SB_PER_PF_MIMD	129
#define MAX_SB_PER_PF_SIMD	64
#define MAX_SB_PER_VF		64

/* Memory addresses on the BAR for the IGU Sub Block */
#define IGU_MEM_BASE			0x0000

#define IGU_MEM_MSIX_BASE		0x0000
#define IGU_MEM_MSIX_UPPER		0x0101
#define IGU_MEM_MSIX_RESERVED_UPPER	0x01ff

#define IGU_MEM_PBA_MSIX_BASE		0x0200
#define IGU_MEM_PBA_MSIX_UPPER		0x0202
#define IGU_MEM_PBA_MSIX_RESERVED_UPPER	0x03ff

#define IGU_CMD_INT_ACK_BASE		0x0400
#define IGU_CMD_INT_ACK_UPPER		(IGU_CMD_INT_ACK_BASE +	\
					 MAX_TOT_SB_PER_PATH -	\
					 1)
#define IGU_CMD_INT_ACK_RESERVED_UPPER	0x05ff

#define IGU_CMD_ATTN_BIT_UPD_UPPER	0x05f0
#define IGU_CMD_ATTN_BIT_SET_UPPER	0x05f1
#define IGU_CMD_ATTN_BIT_CLR_UPPER	0x05f2

#define IGU_REG_SISR_MDPC_WMASK_UPPER		0x05f3
#define IGU_REG_SISR_MDPC_WMASK_LSB_UPPER	0x05f4
#define IGU_REG_SISR_MDPC_WMASK_MSB_UPPER	0x05f5
#define IGU_REG_SISR_MDPC_WOMASK_UPPER		0x05f6

#define IGU_CMD_PROD_UPD_BASE			0x0600
#define IGU_CMD_PROD_UPD_UPPER			(IGU_CMD_PROD_UPD_BASE +\
						 MAX_TOT_SB_PER_PATH - \
						 1)
#define IGU_CMD_PROD_UPD_RESERVED_UPPER		0x07ff

/*****************/
/* PXP CONSTANTS */
/*****************/

/* Bars for Blocks */
#define PXP_BAR_GRC                                         0
#define PXP_BAR_TSDM                                        0
#define PXP_BAR_USDM                                        0
#define PXP_BAR_XSDM                                        0
#define PXP_BAR_MSDM                                        0
#define PXP_BAR_YSDM                                        0
#define PXP_BAR_PSDM                                        0
#define PXP_BAR_IGU                                         0
#define PXP_BAR_DQ                                          1

/* PTT and GTT */
#define PXP_NUM_PF_WINDOWS		12
#define PXP_PER_PF_ENTRY_SIZE		8
#define PXP_NUM_GLOBAL_WINDOWS		243
#define PXP_GLOBAL_ENTRY_SIZE		4
#define PXP_ADMIN_WINDOW_ALLOWED_LENGTH	4
#define PXP_PF_WINDOW_ADMIN_START	0
#define PXP_PF_WINDOW_ADMIN_LENGTH	0x1000
#define PXP_PF_WINDOW_ADMIN_END		(PXP_PF_WINDOW_ADMIN_START + \
					 PXP_PF_WINDOW_ADMIN_LENGTH - 1)
#define PXP_PF_WINDOW_ADMIN_PER_PF_START	0
#define PXP_PF_WINDOW_ADMIN_PER_PF_LENGTH	(PXP_NUM_PF_WINDOWS * \
						 PXP_PER_PF_ENTRY_SIZE)
#define PXP_PF_WINDOW_ADMIN_PER_PF_END	(PXP_PF_WINDOW_ADMIN_PER_PF_START + \
					 PXP_PF_WINDOW_ADMIN_PER_PF_LENGTH - 1)
#define PXP_PF_WINDOW_ADMIN_GLOBAL_START	0x200
#define PXP_PF_WINDOW_ADMIN_GLOBAL_LENGTH	(PXP_NUM_GLOBAL_WINDOWS * \
						 PXP_GLOBAL_ENTRY_SIZE)
#define PXP_PF_WINDOW_ADMIN_GLOBAL_END \
		(PXP_PF_WINDOW_ADMIN_GLOBAL_START + \
		 PXP_PF_WINDOW_ADMIN_GLOBAL_LENGTH - 1)
#define PXP_PF_GLOBAL_PRETEND_ADDR	0x1f0
#define PXP_PF_ME_OPAQUE_MASK_ADDR	0xf4
#define PXP_PF_ME_OPAQUE_ADDR		0x1f8
#define PXP_PF_ME_CONCRETE_ADDR		0x1fc

#define PXP_EXTERNAL_BAR_PF_WINDOW_START	0x1000
#define PXP_EXTERNAL_BAR_PF_WINDOW_NUM		PXP_NUM_PF_WINDOWS
#define PXP_EXTERNAL_BAR_PF_WINDOW_SINGLE_SIZE	0x1000
#define PXP_EXTERNAL_BAR_PF_WINDOW_LENGTH \
	(PXP_EXTERNAL_BAR_PF_WINDOW_NUM * \
	 PXP_EXTERNAL_BAR_PF_WINDOW_SINGLE_SIZE)
#define PXP_EXTERNAL_BAR_PF_WINDOW_END \
	(PXP_EXTERNAL_BAR_PF_WINDOW_START + \
	 PXP_EXTERNAL_BAR_PF_WINDOW_LENGTH - 1)

#define PXP_EXTERNAL_BAR_GLOBAL_WINDOW_START \
	(PXP_EXTERNAL_BAR_PF_WINDOW_END + 1)
#define PXP_EXTERNAL_BAR_GLOBAL_WINDOW_NUM		PXP_NUM_GLOBAL_WINDOWS
#define PXP_EXTERNAL_BAR_GLOBAL_WINDOW_SINGLE_SIZE	0x1000
#define PXP_EXTERNAL_BAR_GLOBAL_WINDOW_LENGTH \
	(PXP_EXTERNAL_BAR_GLOBAL_WINDOW_NUM * \
	 PXP_EXTERNAL_BAR_GLOBAL_WINDOW_SINGLE_SIZE)
#define PXP_EXTERNAL_BAR_GLOBAL_WINDOW_END \
	(PXP_EXTERNAL_BAR_GLOBAL_WINDOW_START + \
	 PXP_EXTERNAL_BAR_GLOBAL_WINDOW_LENGTH - 1)

/* PF BAR */
/*#define PXP_BAR0_START_GRC 0x1000 */
/*#define PXP_BAR0_GRC_LENGTH 0xBFF000 */
#define PXP_BAR0_START_GRC                      0x0000
#define PXP_BAR0_GRC_LENGTH                     0x1C00000
#define PXP_BAR0_END_GRC                        \
	(PXP_BAR0_START_GRC + PXP_BAR0_GRC_LENGTH - 1)

#define PXP_BAR0_START_IGU                      0x1C00000
#define PXP_BAR0_IGU_LENGTH                     0x10000
#define PXP_BAR0_END_IGU                        \
	(PXP_BAR0_START_IGU + PXP_BAR0_IGU_LENGTH - 1)

#define PXP_BAR0_START_TSDM                     0x1C80000
#define PXP_BAR0_SDM_LENGTH                     0x40000
#define PXP_BAR0_SDM_RESERVED_LENGTH            0x40000
#define PXP_BAR0_END_TSDM                       \
	(PXP_BAR0_START_TSDM + PXP_BAR0_SDM_LENGTH - 1)

#define PXP_BAR0_START_MSDM                     0x1D00000
#define PXP_BAR0_END_MSDM                       \
	(PXP_BAR0_START_MSDM + PXP_BAR0_SDM_LENGTH - 1)

#define PXP_BAR0_START_USDM                     0x1D80000
#define PXP_BAR0_END_USDM                       \
	(PXP_BAR0_START_USDM + PXP_BAR0_SDM_LENGTH - 1)

#define PXP_BAR0_START_XSDM                     0x1E00000
#define PXP_BAR0_END_XSDM                       \
	(PXP_BAR0_START_XSDM + PXP_BAR0_SDM_LENGTH - 1)

#define PXP_BAR0_START_YSDM                     0x1E80000
#define PXP_BAR0_END_YSDM                       \
	(PXP_BAR0_START_YSDM + PXP_BAR0_SDM_LENGTH - 1)

#define PXP_BAR0_START_PSDM                     0x1F00000
#define PXP_BAR0_END_PSDM                       \
	(PXP_BAR0_START_PSDM + PXP_BAR0_SDM_LENGTH - 1)

#define PXP_BAR0_FIRST_INVALID_ADDRESS          \
	(PXP_BAR0_END_PSDM + 1)

#define PXP_ILT_PAGE_SIZE_NUM_BITS_MIN	12
#define PXP_ILT_BLOCK_FACTOR_MULTIPLIER	1024

/* ILT Records */
#define PXP_NUM_ILT_RECORDS_BB 7600
#define PXP_NUM_ILT_RECORDS_K2 11000
#define MAX_NUM_ILT_RECORDS MAX(PXP_NUM_ILT_RECORDS_BB, PXP_NUM_ILT_RECORDS_K2)


/* Host Interface */
#define PXP_QUEUES_ZONE_MAX_NUM	320




/*****************/
/* PRM CONSTANTS */
/*****************/
#define PRM_DMA_PAD_BYTES_NUM  2
/*****************/
/* SDMs CONSTANTS  */
/*****************/


#define SDM_OP_GEN_TRIG_NONE			0
#define SDM_OP_GEN_TRIG_WAKE_THREAD		1
#define SDM_OP_GEN_TRIG_AGG_INT			2
#define SDM_OP_GEN_TRIG_LOADER			4
#define SDM_OP_GEN_TRIG_INDICATE_ERROR	6
#define SDM_OP_GEN_TRIG_RELEASE_THREAD	7

/***********************************************************/
/* Completion types                                        */
/***********************************************************/

#define SDM_COMP_TYPE_NONE		0
#define SDM_COMP_TYPE_WAKE_THREAD	1
#define SDM_COMP_TYPE_AGG_INT		2
/* Send direct message to local CM and/or remote CMs. Destinations are defined
 * by vector in CompParams.
 */
#define SDM_COMP_TYPE_CM		3
#define SDM_COMP_TYPE_LOADER		4
/* Send direct message to PXP (like "internal write" command) to write to remote
 * Storm RAM via remote SDM
 */
#define SDM_COMP_TYPE_PXP		5
/* Indicate error per thread */
#define SDM_COMP_TYPE_INDICATE_ERROR	6
#define SDM_COMP_TYPE_RELEASE_THREAD	7
/* Write to local RAM as a completion */
#define SDM_COMP_TYPE_RAM		8


/******************/
/* PBF CONSTANTS  */
/******************/

/* Number of PBF command queue lines. Each line is 32B. */
#define PBF_MAX_CMD_LINES 3328

/* Number of BTB blocks. Each block is 256B. */
#define BTB_MAX_BLOCKS 1440

/*****************/
/* PRS CONSTANTS */
/*****************/

#define PRS_GFT_CAM_LINES_NO_MATCH  31
/* Async data KCQ CQE */
struct async_data {
	/* Context ID of the connection */
	__le32	cid;
	/* Task Id of the task (for error that happened on a a task) */
	__le16	itid;
	/* error code - relevant only if the opcode indicates its an error */
	u8	error_code;
	/* internal fw debug parameter */
	u8	fw_debug_param;
};

/*
 * Interrupt coalescing TimeSet
 */
struct coalescing_timeset {
	u8 value;
/* Interrupt coalescing TimeSet (timeout_ticks = TimeSet shl (TimerRes+1)) */
#define COALESCING_TIMESET_TIMESET_MASK  0x7F
#define COALESCING_TIMESET_TIMESET_SHIFT 0
/* Only if this flag is set, timeset will take effect */
#define COALESCING_TIMESET_VALID_MASK    0x1
#define COALESCING_TIMESET_VALID_SHIFT   7
};

struct common_queue_zone {
	__le16 ring_drv_data_consumer;
	__le16 reserved;
};

/*
 * ETH Rx producers data
 */
struct eth_rx_prod_data {
	__le16 bd_prod /* BD producer. */;
	__le16 cqe_prod /* CQE producer. */;
};

struct regpair {
	__le32 lo /* low word for reg-pair */;
	__le32 hi /* high word for reg-pair */;
};

/*
 * Event Ring VF-PF Channel data
 */
struct vf_pf_channel_eqe_data {
	struct regpair msg_addr /* VF-PF message address */;
};

struct iscsi_eqe_data {
	__le32 cid /* Context ID of the connection */;
	    /* Task Id of the task (for error that happened on a a task) */;
	__le16 conn_id;
/* error code - relevant only if the opcode indicates its an error */
	u8 error_code;
	u8 error_pdu_opcode_reserved;
/* The processed PDUs opcode on which happened the error - updated for specific
 * error codes, by default=0xFF
 */
#define ISCSI_EQE_DATA_ERROR_PDU_OPCODE_MASK        0x3F
#define ISCSI_EQE_DATA_ERROR_PDU_OPCODE_SHIFT       0
/* Indication for driver is the error_pdu_opcode field has valid value */
#define ISCSI_EQE_DATA_ERROR_PDU_OPCODE_VALID_MASK  0x1
#define ISCSI_EQE_DATA_ERROR_PDU_OPCODE_VALID_SHIFT 6
#define ISCSI_EQE_DATA_RESERVED0_MASK               0x1
#define ISCSI_EQE_DATA_RESERVED0_SHIFT              7
};

/*
 * Event Ring malicious VF data
 */
struct malicious_vf_eqe_data {
	u8 vfId /* Malicious VF ID */;
	u8 errId /* Malicious VF error */;
	__le16 reserved[3];
};

/*
 * Event Ring initial cleanup data
 */
struct initial_cleanup_eqe_data {
	u8 vfId /* VF ID */;
	u8 reserved[7];
};

/*
 * Event Data Union
 */
union event_ring_data {
	u8 bytes[8] /* Byte Array */;
	struct vf_pf_channel_eqe_data vf_pf_channel /* VF-PF Channel data */;
	struct iscsi_eqe_data iscsi_info /* Dedicated fields to iscsi data */;
	    /* Dedicated field for RoCE affiliated asynchronous error */;
	struct regpair roceHandle;
	struct malicious_vf_eqe_data malicious_vf /* Malicious VF data */;
	struct initial_cleanup_eqe_data vf_init_cleanup
	    /* VF Initial Cleanup data */;
/* Host handle for the Async Completions */
	struct regpair iwarp_handle;
};
/* Event Ring Entry */
struct event_ring_entry {
	u8 protocol_id /* Event Protocol ID */;
	u8 opcode /* Event Opcode */;
	__le16 reserved0 /* Reserved */;
	__le16 echo /* Echo value from ramrod data on the host */;
	u8 fw_return_code /* FW return code for SP ramrods */;
	u8 flags;
/* 0: synchronous EQE - a completion of SP message. 1: asynchronous EQE */
#define EVENT_RING_ENTRY_ASYNC_MASK      0x1
#define EVENT_RING_ENTRY_ASYNC_SHIFT     0
#define EVENT_RING_ENTRY_RESERVED1_MASK  0x7F
#define EVENT_RING_ENTRY_RESERVED1_SHIFT 1
	union event_ring_data	data;
};

/* Multi function mode */
enum mf_mode {
	ERROR_MODE /* Unsupported mode */,
	MF_OVLAN /* Multi function based on outer VLAN */,
	MF_NPAR /* Multi function based on MAC address (NIC partitioning) */,
	MAX_MF_MODE
};

/* Per-protocol connection types */
enum protocol_type {
	PROTOCOLID_ISCSI /* iSCSI */,
	PROTOCOLID_FCOE /* FCoE */,
	PROTOCOLID_ROCE /* RoCE */,
	PROTOCOLID_CORE /* Core (light L2, slow path core) */,
	PROTOCOLID_ETH /* Ethernet */,
	PROTOCOLID_IWARP /* iWARP */,
	PROTOCOLID_TOE /* TOE */,
	PROTOCOLID_PREROCE /* Pre (tapeout) RoCE */,
	PROTOCOLID_COMMON /* ProtocolCommon */,
	PROTOCOLID_TCP /* TCP */,
	MAX_PROTOCOL_TYPE
};

/*
 * Ustorm Queue Zone
 */
struct ustorm_eth_queue_zone {
/* Rx interrupt coalescing TimeSet */
	struct coalescing_timeset int_coalescing_timeset;
	u8 reserved[3];
};


struct ustorm_queue_zone {
	struct ustorm_eth_queue_zone eth;
	struct common_queue_zone common;
};

/* status block structure */
struct cau_pi_entry {
	__le32 prod;
/* A per protocol indexPROD value. */
#define CAU_PI_ENTRY_PROD_VAL_MASK    0xFFFF
#define CAU_PI_ENTRY_PROD_VAL_SHIFT   0
/* This value determines the TimeSet that the PI is associated with */
#define CAU_PI_ENTRY_PI_TIMESET_MASK  0x7F
#define CAU_PI_ENTRY_PI_TIMESET_SHIFT 16
/* Select the FSM within the SB */
#define CAU_PI_ENTRY_FSM_SEL_MASK     0x1
#define CAU_PI_ENTRY_FSM_SEL_SHIFT    23
/* Select the FSM within the SB */
#define CAU_PI_ENTRY_RESERVED_MASK    0xFF
#define CAU_PI_ENTRY_RESERVED_SHIFT   24
};

/* status block structure */
struct cau_sb_entry {
	__le32 data;
/* The SB PROD index which is sent to the IGU. */
#define CAU_SB_ENTRY_SB_PROD_MASK      0xFFFFFF
#define CAU_SB_ENTRY_SB_PROD_SHIFT     0
#define CAU_SB_ENTRY_STATE0_MASK       0xF /* RX state */
#define CAU_SB_ENTRY_STATE0_SHIFT      24
#define CAU_SB_ENTRY_STATE1_MASK       0xF /* TX state */
#define CAU_SB_ENTRY_STATE1_SHIFT      28
	__le32 params;
/* Indicates the RX TimeSet that this SB is associated with. */
#define CAU_SB_ENTRY_SB_TIMESET0_MASK  0x7F
#define CAU_SB_ENTRY_SB_TIMESET0_SHIFT 0
/* Indicates the TX TimeSet that this SB is associated with. */
#define CAU_SB_ENTRY_SB_TIMESET1_MASK  0x7F
#define CAU_SB_ENTRY_SB_TIMESET1_SHIFT 7
/* This value will determine the RX FSM timer resolution in ticks */
#define CAU_SB_ENTRY_TIMER_RES0_MASK   0x3
#define CAU_SB_ENTRY_TIMER_RES0_SHIFT  14
/* This value will determine the TX FSM timer resolution in ticks */
#define CAU_SB_ENTRY_TIMER_RES1_MASK   0x3
#define CAU_SB_ENTRY_TIMER_RES1_SHIFT  16
#define CAU_SB_ENTRY_VF_NUMBER_MASK    0xFF
#define CAU_SB_ENTRY_VF_NUMBER_SHIFT   18
#define CAU_SB_ENTRY_VF_VALID_MASK     0x1
#define CAU_SB_ENTRY_VF_VALID_SHIFT    26
#define CAU_SB_ENTRY_PF_NUMBER_MASK    0xF
#define CAU_SB_ENTRY_PF_NUMBER_SHIFT   27
/* If set then indicates that the TPH STAG is equal to the SB number. Otherwise
 * the STAG will be equal to all ones.
 */
#define CAU_SB_ENTRY_TPH_MASK          0x1
#define CAU_SB_ENTRY_TPH_SHIFT         31
};

/* core doorbell data */
struct core_db_data {
	u8 params;
/* destination of doorbell (use enum db_dest) */
#define CORE_DB_DATA_DEST_MASK         0x3
#define CORE_DB_DATA_DEST_SHIFT        0
/* aggregative command to CM (use enum db_agg_cmd_sel) */
#define CORE_DB_DATA_AGG_CMD_MASK      0x3
#define CORE_DB_DATA_AGG_CMD_SHIFT     2
#define CORE_DB_DATA_BYPASS_EN_MASK    0x1 /* enable QM bypass */
#define CORE_DB_DATA_BYPASS_EN_SHIFT   4
#define CORE_DB_DATA_RESERVED_MASK     0x1
#define CORE_DB_DATA_RESERVED_SHIFT    5
/* aggregative value selection */
#define CORE_DB_DATA_AGG_VAL_SEL_MASK  0x3
#define CORE_DB_DATA_AGG_VAL_SEL_SHIFT 6
/* bit for every DQ counter flags in CM context that DQ can increment */
	u8	agg_flags;
	__le16	spq_prod;
};

/* Enum of doorbell aggregative command selection */
enum db_agg_cmd_sel {
	DB_AGG_CMD_NOP /* No operation */,
	DB_AGG_CMD_SET /* Set the value */,
	DB_AGG_CMD_ADD /* Add the value */,
	DB_AGG_CMD_MAX /* Set max of current and new value */,
	MAX_DB_AGG_CMD_SEL
};

/* Enum of doorbell destination */
enum db_dest {
	DB_DEST_XCM /* TX doorbell to XCM */,
	DB_DEST_UCM /* RX doorbell to UCM */,
	DB_DEST_TCM /* RX doorbell to TCM */,
	DB_NUM_DESTINATIONS,
	MAX_DB_DEST
};


/*
 * Enum of doorbell DPM types
 */
enum db_dpm_type {
	DPM_LEGACY /* Legacy DPM- to Xstorm RAM */,
	DPM_ROCE /* RoCE DPM- to NIG */,
/* L2 DPM inline- to PBF, with packet data on doorbell */
	DPM_L2_INLINE,
	DPM_L2_BD /* L2 DPM with BD- to PBF, with TX BD data on doorbell */,
	MAX_DB_DPM_TYPE
};

/*
 * Structure for doorbell data, in L2 DPM mode, for the first doorbell in a DPM
 * burst
 */
struct db_l2_dpm_data {
	__le16 icid /* internal CID */;
	__le16 bd_prod /* bd producer value to update */;
	__le32 params;
/* Size in QWORD-s of the DPM burst */
#define DB_L2_DPM_DATA_SIZE_MASK       0x3F
#define DB_L2_DPM_DATA_SIZE_SHIFT      0
/* Type of DPM transaction (DPM_L2_INLINE or DPM_L2_BD) (use enum db_dpm_type)
 */
#define DB_L2_DPM_DATA_DPM_TYPE_MASK   0x3
#define DB_L2_DPM_DATA_DPM_TYPE_SHIFT  6
#define DB_L2_DPM_DATA_NUM_BDS_MASK    0xFF /* number of BD-s */
#define DB_L2_DPM_DATA_NUM_BDS_SHIFT   8
/* size of the packet to be transmitted in bytes */
#define DB_L2_DPM_DATA_PKT_SIZE_MASK   0x7FF
#define DB_L2_DPM_DATA_PKT_SIZE_SHIFT  16
#define DB_L2_DPM_DATA_RESERVED0_MASK  0x1
#define DB_L2_DPM_DATA_RESERVED0_SHIFT 27
/* In DPM_L2_BD mode: the number of SGE-s */
#define DB_L2_DPM_DATA_SGE_NUM_MASK    0x7
#define DB_L2_DPM_DATA_SGE_NUM_SHIFT   28
#define DB_L2_DPM_DATA_RESERVED1_MASK  0x1
#define DB_L2_DPM_DATA_RESERVED1_SHIFT 31
};

/*
 * Structure for SGE in a DPM doorbell of type DPM_L2_BD
 */
struct db_l2_dpm_sge {
	struct regpair addr /* Single continuous buffer */;
	__le16 nbytes /* Number of bytes in this BD. */;
	__le16 bitfields;
/* The TPH STAG index value */
#define DB_L2_DPM_SGE_TPH_ST_INDEX_MASK  0x1FF
#define DB_L2_DPM_SGE_TPH_ST_INDEX_SHIFT 0
#define DB_L2_DPM_SGE_RESERVED0_MASK     0x3
#define DB_L2_DPM_SGE_RESERVED0_SHIFT    9
/* Indicate if ST hint is requested or not */
#define DB_L2_DPM_SGE_ST_VALID_MASK      0x1
#define DB_L2_DPM_SGE_ST_VALID_SHIFT     11
#define DB_L2_DPM_SGE_RESERVED1_MASK     0xF
#define DB_L2_DPM_SGE_RESERVED1_SHIFT    12
	__le32 reserved2;
};

/* Structure for doorbell address, in legacy mode */
struct db_legacy_addr {
	__le32 addr;
#define DB_LEGACY_ADDR_RESERVED0_MASK  0x3
#define DB_LEGACY_ADDR_RESERVED0_SHIFT 0
/* doorbell extraction mode specifier- 0 if not used */
#define DB_LEGACY_ADDR_DEMS_MASK       0x7
#define DB_LEGACY_ADDR_DEMS_SHIFT      2
#define DB_LEGACY_ADDR_ICID_MASK       0x7FFFFFF /* internal CID */
#define DB_LEGACY_ADDR_ICID_SHIFT      5
};

/*
 * Structure for doorbell address, in PWM mode
 */
struct db_pwm_addr {
	__le32 addr;
#define DB_PWM_ADDR_RESERVED0_MASK  0x7
#define DB_PWM_ADDR_RESERVED0_SHIFT 0
/* Offset in PWM address space */
#define DB_PWM_ADDR_OFFSET_MASK     0x7F
#define DB_PWM_ADDR_OFFSET_SHIFT    3
#define DB_PWM_ADDR_WID_MASK        0x3 /* Window ID */
#define DB_PWM_ADDR_WID_SHIFT       10
#define DB_PWM_ADDR_DPI_MASK        0xFFFF /* Doorbell page ID */
#define DB_PWM_ADDR_DPI_SHIFT       12
#define DB_PWM_ADDR_RESERVED1_MASK  0xF
#define DB_PWM_ADDR_RESERVED1_SHIFT 28
};

/*
 * Parameters to RoCE firmware, passed in EDPM doorbell
 */
struct db_roce_dpm_params {
	__le32 params;
/* Size in QWORD-s of the DPM burst */
#define DB_ROCE_DPM_PARAMS_SIZE_MASK            0x3F
#define DB_ROCE_DPM_PARAMS_SIZE_SHIFT           0
/* Type of DPM transacation (DPM_ROCE) (use enum db_dpm_type) */
#define DB_ROCE_DPM_PARAMS_DPM_TYPE_MASK        0x3
#define DB_ROCE_DPM_PARAMS_DPM_TYPE_SHIFT       6
/* opcode for ROCE operation */
#define DB_ROCE_DPM_PARAMS_OPCODE_MASK          0xFF
#define DB_ROCE_DPM_PARAMS_OPCODE_SHIFT         8
/* the size of the WQE payload in bytes */
#define DB_ROCE_DPM_PARAMS_WQE_SIZE_MASK        0x7FF
#define DB_ROCE_DPM_PARAMS_WQE_SIZE_SHIFT       16
#define DB_ROCE_DPM_PARAMS_RESERVED0_MASK       0x1
#define DB_ROCE_DPM_PARAMS_RESERVED0_SHIFT      27
/* RoCE completion flag */
#define DB_ROCE_DPM_PARAMS_COMPLETION_FLG_MASK  0x1
#define DB_ROCE_DPM_PARAMS_COMPLETION_FLG_SHIFT 28
#define DB_ROCE_DPM_PARAMS_S_FLG_MASK           0x1 /* RoCE S flag */
#define DB_ROCE_DPM_PARAMS_S_FLG_SHIFT          29
#define DB_ROCE_DPM_PARAMS_RESERVED1_MASK       0x3
#define DB_ROCE_DPM_PARAMS_RESERVED1_SHIFT      30
};

/*
 * Structure for doorbell data, in ROCE DPM mode, for the first doorbell in a
 * DPM burst
 */
struct db_roce_dpm_data {
	__le16 icid /* internal CID */;
	__le16 prod_val /* aggregated value to update */;
/* parameters passed to RoCE firmware */
	struct db_roce_dpm_params params;
};

/* Igu interrupt command */
enum igu_int_cmd {
	IGU_INT_ENABLE	= 0,
	IGU_INT_DISABLE = 1,
	IGU_INT_NOP	= 2,
	IGU_INT_NOP2	= 3,
	MAX_IGU_INT_CMD
};

/* IGU producer or consumer update command */
struct igu_prod_cons_update {
	__le32 sb_id_and_flags;
#define IGU_PROD_CONS_UPDATE_SB_INDEX_MASK        0xFFFFFF
#define IGU_PROD_CONS_UPDATE_SB_INDEX_SHIFT       0
#define IGU_PROD_CONS_UPDATE_UPDATE_FLAG_MASK     0x1
#define IGU_PROD_CONS_UPDATE_UPDATE_FLAG_SHIFT    24
/* interrupt enable/disable/nop (use enum igu_int_cmd) */
#define IGU_PROD_CONS_UPDATE_ENABLE_INT_MASK      0x3
#define IGU_PROD_CONS_UPDATE_ENABLE_INT_SHIFT     25
/*  (use enum igu_seg_access) */
#define IGU_PROD_CONS_UPDATE_SEGMENT_ACCESS_MASK  0x1
#define IGU_PROD_CONS_UPDATE_SEGMENT_ACCESS_SHIFT 27
#define IGU_PROD_CONS_UPDATE_TIMER_MASK_MASK      0x1
#define IGU_PROD_CONS_UPDATE_TIMER_MASK_SHIFT     28
#define IGU_PROD_CONS_UPDATE_RESERVED0_MASK       0x3
#define IGU_PROD_CONS_UPDATE_RESERVED0_SHIFT      29
/* must always be set cleared (use enum command_type_bit) */
#define IGU_PROD_CONS_UPDATE_COMMAND_TYPE_MASK    0x1
#define IGU_PROD_CONS_UPDATE_COMMAND_TYPE_SHIFT   31
	__le32 reserved1;
};

/* Igu segments access for default status block only */
enum igu_seg_access {
	IGU_SEG_ACCESS_REG	= 0,
	IGU_SEG_ACCESS_ATTN	= 1,
	MAX_IGU_SEG_ACCESS
};


/*
 * Enumeration for L3 type field of parsing_and_err_flags_union. L3Type:
 * 0 - unknown (not ip) ,1 - Ipv4, 2 - Ipv6 (this field can be filled according
 * to the last-ethertype)
 */
enum l3_type {
	e_l3Type_unknown,
	e_l3Type_ipv4,
	e_l3Type_ipv6,
	MAX_L3_TYPE
};


/*
 * Enumeration for l4Protocol field of parsing_and_err_flags_union. L4-protocol
 * 0 - none, 1 - TCP, 2- UDP. if the packet is IPv4 fragment, and its not the
 * first fragment, the protocol-type should be set to none.
 */
enum l4_protocol {
	e_l4Protocol_none,
	e_l4Protocol_tcp,
	e_l4Protocol_udp,
	MAX_L4_PROTOCOL
};


/*
 * Parsing and error flags field.
 */
struct parsing_and_err_flags {
	__le16 flags;
/* L3Type: 0 - unknown (not ip) ,1 - Ipv4, 2 - Ipv6 (this field can be filled
 * according to the last-ethertype) (use enum l3_type)
 */
#define PARSING_AND_ERR_FLAGS_L3TYPE_MASK                      0x3
#define PARSING_AND_ERR_FLAGS_L3TYPE_SHIFT                     0
/* L4-protocol 0 - none, 1 - TCP, 2- UDP. if the packet is IPv4 fragment, and
 * its not the first fragment, the protocol-type should be set to none.
 * (use enum l4_protocol)
 */
#define PARSING_AND_ERR_FLAGS_L4PROTOCOL_MASK                  0x3
#define PARSING_AND_ERR_FLAGS_L4PROTOCOL_SHIFT                 2
/* Set if the packet is IPv4 fragment. */
#define PARSING_AND_ERR_FLAGS_IPV4FRAG_MASK                    0x1
#define PARSING_AND_ERR_FLAGS_IPV4FRAG_SHIFT                   4
/* Set if VLAN tag exists. Invalid if tunnel type are IP GRE or IP GENEVE. */
#define PARSING_AND_ERR_FLAGS_TAG8021QEXIST_MASK               0x1
#define PARSING_AND_ERR_FLAGS_TAG8021QEXIST_SHIFT              5
/* Set if L4 checksum was calculated. */
#define PARSING_AND_ERR_FLAGS_L4CHKSMWASCALCULATED_MASK        0x1
#define PARSING_AND_ERR_FLAGS_L4CHKSMWASCALCULATED_SHIFT       6
/* Set for PTP packet. */
#define PARSING_AND_ERR_FLAGS_TIMESYNCPKT_MASK                 0x1
#define PARSING_AND_ERR_FLAGS_TIMESYNCPKT_SHIFT                7
/* Set if PTP timestamp recorded. */
#define PARSING_AND_ERR_FLAGS_TIMESTAMPRECORDED_MASK           0x1
#define PARSING_AND_ERR_FLAGS_TIMESTAMPRECORDED_SHIFT          8
/* Set if either version-mismatch or hdr-len-error or ipv4-cksm is set or ipv6
 * ver mismatch
 */
#define PARSING_AND_ERR_FLAGS_IPHDRERROR_MASK                  0x1
#define PARSING_AND_ERR_FLAGS_IPHDRERROR_SHIFT                 9
/* Set if L4 checksum validation failed. Valid only if L4 checksum was
 * calculated.
 */
#define PARSING_AND_ERR_FLAGS_L4CHKSMERROR_MASK                0x1
#define PARSING_AND_ERR_FLAGS_L4CHKSMERROR_SHIFT               10
/* Set if GRE/VXLAN/GENEVE tunnel detected. */
#define PARSING_AND_ERR_FLAGS_TUNNELEXIST_MASK                 0x1
#define PARSING_AND_ERR_FLAGS_TUNNELEXIST_SHIFT                11
/* Set if VLAN tag exists in tunnel header. */
#define PARSING_AND_ERR_FLAGS_TUNNEL8021QTAGEXIST_MASK         0x1
#define PARSING_AND_ERR_FLAGS_TUNNEL8021QTAGEXIST_SHIFT        12
/* Set if either tunnel-ipv4-version-mismatch or tunnel-ipv4-hdr-len-error or
 * tunnel-ipv4-cksm is set or tunneling ipv6 ver mismatch
 */
#define PARSING_AND_ERR_FLAGS_TUNNELIPHDRERROR_MASK            0x1
#define PARSING_AND_ERR_FLAGS_TUNNELIPHDRERROR_SHIFT           13
/* Set if GRE or VXLAN/GENEVE UDP checksum was calculated. */
#define PARSING_AND_ERR_FLAGS_TUNNELL4CHKSMWASCALCULATED_MASK  0x1
#define PARSING_AND_ERR_FLAGS_TUNNELL4CHKSMWASCALCULATED_SHIFT 14
/* Set if tunnel L4 checksum validation failed. Valid only if tunnel L4 checksum
 * was calculated.
 */
#define PARSING_AND_ERR_FLAGS_TUNNELL4CHKSMERROR_MASK          0x1
#define PARSING_AND_ERR_FLAGS_TUNNELL4CHKSMERROR_SHIFT         15
};


/*
 * Pb context
 */
struct pb_context {
	__le32 crc[4];
};

/* Concrete Function ID. */
struct pxp_concrete_fid {
	__le16 fid;
#define PXP_CONCRETE_FID_PFID_MASK     0xF /* Parent PFID */
#define PXP_CONCRETE_FID_PFID_SHIFT    0
#define PXP_CONCRETE_FID_PORT_MASK     0x3 /* port number */
#define PXP_CONCRETE_FID_PORT_SHIFT    4
#define PXP_CONCRETE_FID_PATH_MASK     0x1 /* path number */
#define PXP_CONCRETE_FID_PATH_SHIFT    6
#define PXP_CONCRETE_FID_VFVALID_MASK  0x1
#define PXP_CONCRETE_FID_VFVALID_SHIFT 7
#define PXP_CONCRETE_FID_VFID_MASK     0xFF
#define PXP_CONCRETE_FID_VFID_SHIFT    8
};

struct pxp_pretend_concrete_fid {
	__le16 fid;
#define PXP_PRETEND_CONCRETE_FID_PFID_MASK      0xF
#define PXP_PRETEND_CONCRETE_FID_PFID_SHIFT     0
#define PXP_PRETEND_CONCRETE_FID_RESERVED_MASK  0x7
#define PXP_PRETEND_CONCRETE_FID_RESERVED_SHIFT 4
#define PXP_PRETEND_CONCRETE_FID_VFVALID_MASK   0x1
#define PXP_PRETEND_CONCRETE_FID_VFVALID_SHIFT  7
#define PXP_PRETEND_CONCRETE_FID_VFID_MASK      0xFF
#define PXP_PRETEND_CONCRETE_FID_VFID_SHIFT     8
};

union pxp_pretend_fid {
	struct pxp_pretend_concrete_fid concrete_fid;
	__le16				opaque_fid;
};

/* Pxp Pretend Command Register. */
struct pxp_pretend_cmd {
	union pxp_pretend_fid	fid;
	__le16			control;
#define PXP_PRETEND_CMD_PATH_MASK              0x1
#define PXP_PRETEND_CMD_PATH_SHIFT             0
#define PXP_PRETEND_CMD_USE_PORT_MASK          0x1
#define PXP_PRETEND_CMD_USE_PORT_SHIFT         1
#define PXP_PRETEND_CMD_PORT_MASK              0x3
#define PXP_PRETEND_CMD_PORT_SHIFT             2
#define PXP_PRETEND_CMD_RESERVED0_MASK         0xF
#define PXP_PRETEND_CMD_RESERVED0_SHIFT        4
#define PXP_PRETEND_CMD_RESERVED1_MASK         0xF
#define PXP_PRETEND_CMD_RESERVED1_SHIFT        8
#define PXP_PRETEND_CMD_PRETEND_PATH_MASK      0x1
#define PXP_PRETEND_CMD_PRETEND_PATH_SHIFT     12
#define PXP_PRETEND_CMD_PRETEND_PORT_MASK      0x1
#define PXP_PRETEND_CMD_PRETEND_PORT_SHIFT     13
#define PXP_PRETEND_CMD_PRETEND_FUNCTION_MASK  0x1
#define PXP_PRETEND_CMD_PRETEND_FUNCTION_SHIFT 14
#define PXP_PRETEND_CMD_IS_CONCRETE_MASK       0x1
#define PXP_PRETEND_CMD_IS_CONCRETE_SHIFT      15
};

/* PTT Record in PXP Admin Window. */
struct pxp_ptt_entry {
	__le32			offset;
#define PXP_PTT_ENTRY_OFFSET_MASK     0x7FFFFF
#define PXP_PTT_ENTRY_OFFSET_SHIFT    0
#define PXP_PTT_ENTRY_RESERVED0_MASK  0x1FF
#define PXP_PTT_ENTRY_RESERVED0_SHIFT 23
	struct pxp_pretend_cmd	pretend;
};


/*
 * VF Zone A Permission Register.
 */
struct pxp_vf_zone_a_permission {
	__le32 control;
#define PXP_VF_ZONE_A_PERMISSION_VFID_MASK       0xFF
#define PXP_VF_ZONE_A_PERMISSION_VFID_SHIFT      0
#define PXP_VF_ZONE_A_PERMISSION_VALID_MASK      0x1
#define PXP_VF_ZONE_A_PERMISSION_VALID_SHIFT     8
#define PXP_VF_ZONE_A_PERMISSION_RESERVED0_MASK  0x7F
#define PXP_VF_ZONE_A_PERMISSION_RESERVED0_SHIFT 9
#define PXP_VF_ZONE_A_PERMISSION_RESERVED1_MASK  0xFFFF
#define PXP_VF_ZONE_A_PERMISSION_RESERVED1_SHIFT 16
};


/*
 * Rdif context
 */
struct rdif_task_context {
	__le32 initialRefTag;
	__le16 appTagValue;
	__le16 appTagMask;
	u8 flags0;
#define RDIF_TASK_CONTEXT_IGNOREAPPTAG_MASK            0x1
#define RDIF_TASK_CONTEXT_IGNOREAPPTAG_SHIFT           0
#define RDIF_TASK_CONTEXT_INITIALREFTAGVALID_MASK      0x1
#define RDIF_TASK_CONTEXT_INITIALREFTAGVALID_SHIFT     1
/* 0 = IP checksum, 1 = CRC */
#define RDIF_TASK_CONTEXT_HOSTGUARDTYPE_MASK           0x1
#define RDIF_TASK_CONTEXT_HOSTGUARDTYPE_SHIFT          2
#define RDIF_TASK_CONTEXT_SETERRORWITHEOP_MASK         0x1
#define RDIF_TASK_CONTEXT_SETERRORWITHEOP_SHIFT        3
/* 1/2/3 - Protection Type */
#define RDIF_TASK_CONTEXT_PROTECTIONTYPE_MASK          0x3
#define RDIF_TASK_CONTEXT_PROTECTIONTYPE_SHIFT         4
/* 0=0x0000, 1=0xffff */
#define RDIF_TASK_CONTEXT_CRC_SEED_MASK                0x1
#define RDIF_TASK_CONTEXT_CRC_SEED_SHIFT               6
/* Keep reference tag constant */
#define RDIF_TASK_CONTEXT_KEEPREFTAGCONST_MASK         0x1
#define RDIF_TASK_CONTEXT_KEEPREFTAGCONST_SHIFT        7
	u8 partialDifData[7];
	__le16 partialCrcValue;
	__le16 partialChecksumValue;
	__le32 offsetInIO;
	__le16 flags1;
#define RDIF_TASK_CONTEXT_VALIDATEGUARD_MASK           0x1
#define RDIF_TASK_CONTEXT_VALIDATEGUARD_SHIFT          0
#define RDIF_TASK_CONTEXT_VALIDATEAPPTAG_MASK          0x1
#define RDIF_TASK_CONTEXT_VALIDATEAPPTAG_SHIFT         1
#define RDIF_TASK_CONTEXT_VALIDATEREFTAG_MASK          0x1
#define RDIF_TASK_CONTEXT_VALIDATEREFTAG_SHIFT         2
#define RDIF_TASK_CONTEXT_FORWARDGUARD_MASK            0x1
#define RDIF_TASK_CONTEXT_FORWARDGUARD_SHIFT           3
#define RDIF_TASK_CONTEXT_FORWARDAPPTAG_MASK           0x1
#define RDIF_TASK_CONTEXT_FORWARDAPPTAG_SHIFT          4
#define RDIF_TASK_CONTEXT_FORWARDREFTAG_MASK           0x1
#define RDIF_TASK_CONTEXT_FORWARDREFTAG_SHIFT          5
/* 0=512B, 1=1KB, 2=2KB, 3=4KB, 4=8KB */
#define RDIF_TASK_CONTEXT_INTERVALSIZE_MASK            0x7
#define RDIF_TASK_CONTEXT_INTERVALSIZE_SHIFT           6
/* 0=None, 1=DIF, 2=DIX */
#define RDIF_TASK_CONTEXT_HOSTINTERFACE_MASK           0x3
#define RDIF_TASK_CONTEXT_HOSTINTERFACE_SHIFT          9
/* DIF tag right at the beginning of DIF interval */
#define RDIF_TASK_CONTEXT_DIFBEFOREDATA_MASK           0x1
#define RDIF_TASK_CONTEXT_DIFBEFOREDATA_SHIFT          11
#define RDIF_TASK_CONTEXT_RESERVED0_MASK               0x1
#define RDIF_TASK_CONTEXT_RESERVED0_SHIFT              12
/* 0=None, 1=DIF */
#define RDIF_TASK_CONTEXT_NETWORKINTERFACE_MASK        0x1
#define RDIF_TASK_CONTEXT_NETWORKINTERFACE_SHIFT       13
/* Forward application tag with mask */
#define RDIF_TASK_CONTEXT_FORWARDAPPTAGWITHMASK_MASK   0x1
#define RDIF_TASK_CONTEXT_FORWARDAPPTAGWITHMASK_SHIFT  14
/* Forward reference tag with mask */
#define RDIF_TASK_CONTEXT_FORWARDREFTAGWITHMASK_MASK   0x1
#define RDIF_TASK_CONTEXT_FORWARDREFTAGWITHMASK_SHIFT  15
	__le16 state;
#define RDIF_TASK_CONTEXT_RECEIVEDDIFBYTESLEFT_MASK    0xF
#define RDIF_TASK_CONTEXT_RECEIVEDDIFBYTESLEFT_SHIFT   0
#define RDIF_TASK_CONTEXT_TRANSMITEDDIFBYTESLEFT_MASK  0xF
#define RDIF_TASK_CONTEXT_TRANSMITEDDIFBYTESLEFT_SHIFT 4
#define RDIF_TASK_CONTEXT_ERRORINIO_MASK               0x1
#define RDIF_TASK_CONTEXT_ERRORINIO_SHIFT              8
#define RDIF_TASK_CONTEXT_CHECKSUMOVERFLOW_MASK        0x1
#define RDIF_TASK_CONTEXT_CHECKSUMOVERFLOW_SHIFT       9
/* mask for refernce tag handling */
#define RDIF_TASK_CONTEXT_REFTAGMASK_MASK              0xF
#define RDIF_TASK_CONTEXT_REFTAGMASK_SHIFT             10
#define RDIF_TASK_CONTEXT_RESERVED1_MASK               0x3
#define RDIF_TASK_CONTEXT_RESERVED1_SHIFT              14
	__le32 reserved2;
};

/* RSS hash type */
enum rss_hash_type {
	RSS_HASH_TYPE_DEFAULT	= 0,
	RSS_HASH_TYPE_IPV4	= 1,
	RSS_HASH_TYPE_TCP_IPV4	= 2,
	RSS_HASH_TYPE_IPV6	= 3,
	RSS_HASH_TYPE_TCP_IPV6	= 4,
	RSS_HASH_TYPE_UDP_IPV4	= 5,
	RSS_HASH_TYPE_UDP_IPV6	= 6,
	MAX_RSS_HASH_TYPE
};

/* status block structure */
struct status_block {
	__le16	pi_array[PIS_PER_SB];
	__le32	sb_num;
#define STATUS_BLOCK_SB_NUM_MASK      0x1FF
#define STATUS_BLOCK_SB_NUM_SHIFT     0
#define STATUS_BLOCK_ZERO_PAD_MASK    0x7F
#define STATUS_BLOCK_ZERO_PAD_SHIFT   9
#define STATUS_BLOCK_ZERO_PAD2_MASK   0xFFFF
#define STATUS_BLOCK_ZERO_PAD2_SHIFT  16
	__le32 prod_index;
#define STATUS_BLOCK_PROD_INDEX_MASK  0xFFFFFF
#define STATUS_BLOCK_PROD_INDEX_SHIFT 0
#define STATUS_BLOCK_ZERO_PAD3_MASK   0xFF
#define STATUS_BLOCK_ZERO_PAD3_SHIFT  24
};


/* VF BAR */
#define PXP_VF_BAR0 0

#define PXP_VF_BAR0_START_GRC		0x3E00
#define PXP_VF_BAR0_GRC_LENGTH		0x200
#define PXP_VF_BAR0_END_GRC \
(PXP_VF_BAR0_START_GRC + PXP_VF_BAR0_GRC_LENGTH - 1)

#define PXP_VF_BAR0_START_IGU		0
#define PXP_VF_BAR0_IGU_LENGTH		0x3000
#define PXP_VF_BAR0_END_IGU \
(PXP_VF_BAR0_START_IGU + PXP_VF_BAR0_IGU_LENGTH - 1)

#define PXP_VF_BAR0_START_DQ		0x3000
#define PXP_VF_BAR0_DQ_LENGTH		0x200
#define PXP_VF_BAR0_DQ_OPAQUE_OFFSET    0
#define PXP_VF_BAR0_ME_OPAQUE_ADDRESS \
(PXP_VF_BAR0_START_DQ + PXP_VF_BAR0_DQ_OPAQUE_OFFSET)
#define PXP_VF_BAR0_ME_CONCRETE_ADDRESS \
(PXP_VF_BAR0_ME_OPAQUE_ADDRESS + 4)
#define PXP_VF_BAR0_END_DQ \
(PXP_VF_BAR0_START_DQ + PXP_VF_BAR0_DQ_LENGTH - 1)

#define PXP_VF_BAR0_START_TSDM_ZONE_B   0x3200
#define PXP_VF_BAR0_SDM_LENGTH_ZONE_B   0x200
#define PXP_VF_BAR0_END_TSDM_ZONE_B \
(PXP_VF_BAR0_START_TSDM_ZONE_B + PXP_VF_BAR0_SDM_LENGTH_ZONE_B - 1)

#define PXP_VF_BAR0_START_MSDM_ZONE_B   0x3400
#define PXP_VF_BAR0_END_MSDM_ZONE_B \
(PXP_VF_BAR0_START_MSDM_ZONE_B + PXP_VF_BAR0_SDM_LENGTH_ZONE_B - 1)

#define PXP_VF_BAR0_START_USDM_ZONE_B   0x3600
#define PXP_VF_BAR0_END_USDM_ZONE_B \
(PXP_VF_BAR0_START_USDM_ZONE_B + PXP_VF_BAR0_SDM_LENGTH_ZONE_B - 1)

#define PXP_VF_BAR0_START_XSDM_ZONE_B   0x3800
#define PXP_VF_BAR0_END_XSDM_ZONE_B \
(PXP_VF_BAR0_START_XSDM_ZONE_B + PXP_VF_BAR0_SDM_LENGTH_ZONE_B - 1)

#define PXP_VF_BAR0_START_YSDM_ZONE_B   0x3a00
#define PXP_VF_BAR0_END_YSDM_ZONE_B \
(PXP_VF_BAR0_START_YSDM_ZONE_B + PXP_VF_BAR0_SDM_LENGTH_ZONE_B - 1)

#define PXP_VF_BAR0_START_PSDM_ZONE_B   0x3c00
#define PXP_VF_BAR0_END_PSDM_ZONE_B \
(PXP_VF_BAR0_START_PSDM_ZONE_B + PXP_VF_BAR0_SDM_LENGTH_ZONE_B - 1)

#define PXP_VF_BAR0_START_SDM_ZONE_A    0x4000
#define PXP_VF_BAR0_END_SDM_ZONE_A      0x10000

#define PXP_VF_BAR0_GRC_WINDOW_LENGTH   32

/*
 * Tdif context
 */
struct tdif_task_context {
	__le32 initialRefTag;
	__le16 appTagValue;
	__le16 appTagMask;
	__le16 partialCrcValueB;
	__le16 partialChecksumValueB;
	__le16 stateB;
#define TDIF_TASK_CONTEXT_RECEIVEDDIFBYTESLEFTB_MASK    0xF
#define TDIF_TASK_CONTEXT_RECEIVEDDIFBYTESLEFTB_SHIFT   0
#define TDIF_TASK_CONTEXT_TRANSMITEDDIFBYTESLEFTB_MASK  0xF
#define TDIF_TASK_CONTEXT_TRANSMITEDDIFBYTESLEFTB_SHIFT 4
#define TDIF_TASK_CONTEXT_ERRORINIOB_MASK               0x1
#define TDIF_TASK_CONTEXT_ERRORINIOB_SHIFT              8
#define TDIF_TASK_CONTEXT_CHECKSUMOVERFLOW_MASK         0x1
#define TDIF_TASK_CONTEXT_CHECKSUMOVERFLOW_SHIFT        9
#define TDIF_TASK_CONTEXT_RESERVED0_MASK                0x3F
#define TDIF_TASK_CONTEXT_RESERVED0_SHIFT               10
	u8 reserved1;
	u8 flags0;
#define TDIF_TASK_CONTEXT_IGNOREAPPTAG_MASK             0x1
#define TDIF_TASK_CONTEXT_IGNOREAPPTAG_SHIFT            0
#define TDIF_TASK_CONTEXT_INITIALREFTAGVALID_MASK       0x1
#define TDIF_TASK_CONTEXT_INITIALREFTAGVALID_SHIFT      1
/* 0 = IP checksum, 1 = CRC */
#define TDIF_TASK_CONTEXT_HOSTGUARDTYPE_MASK            0x1
#define TDIF_TASK_CONTEXT_HOSTGUARDTYPE_SHIFT           2
#define TDIF_TASK_CONTEXT_SETERRORWITHEOP_MASK          0x1
#define TDIF_TASK_CONTEXT_SETERRORWITHEOP_SHIFT         3
/* 1/2/3 - Protection Type */
#define TDIF_TASK_CONTEXT_PROTECTIONTYPE_MASK           0x3
#define TDIF_TASK_CONTEXT_PROTECTIONTYPE_SHIFT          4
/* 0=0x0000, 1=0xffff */
#define TDIF_TASK_CONTEXT_CRC_SEED_MASK                 0x1
#define TDIF_TASK_CONTEXT_CRC_SEED_SHIFT                6
#define TDIF_TASK_CONTEXT_RESERVED2_MASK                0x1
#define TDIF_TASK_CONTEXT_RESERVED2_SHIFT               7
	__le32 flags1;
#define TDIF_TASK_CONTEXT_VALIDATEGUARD_MASK            0x1
#define TDIF_TASK_CONTEXT_VALIDATEGUARD_SHIFT           0
#define TDIF_TASK_CONTEXT_VALIDATEAPPTAG_MASK           0x1
#define TDIF_TASK_CONTEXT_VALIDATEAPPTAG_SHIFT          1
#define TDIF_TASK_CONTEXT_VALIDATEREFTAG_MASK           0x1
#define TDIF_TASK_CONTEXT_VALIDATEREFTAG_SHIFT          2
#define TDIF_TASK_CONTEXT_FORWARDGUARD_MASK             0x1
#define TDIF_TASK_CONTEXT_FORWARDGUARD_SHIFT            3
#define TDIF_TASK_CONTEXT_FORWARDAPPTAG_MASK            0x1
#define TDIF_TASK_CONTEXT_FORWARDAPPTAG_SHIFT           4
#define TDIF_TASK_CONTEXT_FORWARDREFTAG_MASK            0x1
#define TDIF_TASK_CONTEXT_FORWARDREFTAG_SHIFT           5
/* 0=512B, 1=1KB, 2=2KB, 3=4KB, 4=8KB */
#define TDIF_TASK_CONTEXT_INTERVALSIZE_MASK             0x7
#define TDIF_TASK_CONTEXT_INTERVALSIZE_SHIFT            6
/* 0=None, 1=DIF, 2=DIX */
#define TDIF_TASK_CONTEXT_HOSTINTERFACE_MASK            0x3
#define TDIF_TASK_CONTEXT_HOSTINTERFACE_SHIFT           9
/* DIF tag right at the beginning of DIF interval */
#define TDIF_TASK_CONTEXT_DIFBEFOREDATA_MASK            0x1
#define TDIF_TASK_CONTEXT_DIFBEFOREDATA_SHIFT           11
/* reserved */
#define TDIF_TASK_CONTEXT_RESERVED3_MASK                0x1
#define TDIF_TASK_CONTEXT_RESERVED3_SHIFT               12
/* 0=None, 1=DIF */
#define TDIF_TASK_CONTEXT_NETWORKINTERFACE_MASK         0x1
#define TDIF_TASK_CONTEXT_NETWORKINTERFACE_SHIFT        13
#define TDIF_TASK_CONTEXT_RECEIVEDDIFBYTESLEFTA_MASK    0xF
#define TDIF_TASK_CONTEXT_RECEIVEDDIFBYTESLEFTA_SHIFT   14
#define TDIF_TASK_CONTEXT_TRANSMITEDDIFBYTESLEFTA_MASK  0xF
#define TDIF_TASK_CONTEXT_TRANSMITEDDIFBYTESLEFTA_SHIFT 18
#define TDIF_TASK_CONTEXT_ERRORINIOA_MASK               0x1
#define TDIF_TASK_CONTEXT_ERRORINIOA_SHIFT              22
#define TDIF_TASK_CONTEXT_CHECKSUMOVERFLOWA_MASK        0x1
#define TDIF_TASK_CONTEXT_CHECKSUMOVERFLOWA_SHIFT       23
/* mask for refernce tag handling */
#define TDIF_TASK_CONTEXT_REFTAGMASK_MASK               0xF
#define TDIF_TASK_CONTEXT_REFTAGMASK_SHIFT              24
/* Forward application tag with mask */
#define TDIF_TASK_CONTEXT_FORWARDAPPTAGWITHMASK_MASK    0x1
#define TDIF_TASK_CONTEXT_FORWARDAPPTAGWITHMASK_SHIFT   28
/* Forward reference tag with mask */
#define TDIF_TASK_CONTEXT_FORWARDREFTAGWITHMASK_MASK    0x1
#define TDIF_TASK_CONTEXT_FORWARDREFTAGWITHMASK_SHIFT   29
/* Keep reference tag constant */
#define TDIF_TASK_CONTEXT_KEEPREFTAGCONST_MASK          0x1
#define TDIF_TASK_CONTEXT_KEEPREFTAGCONST_SHIFT         30
#define TDIF_TASK_CONTEXT_RESERVED4_MASK                0x1
#define TDIF_TASK_CONTEXT_RESERVED4_SHIFT               31
	__le32 offsetInIOB;
	__le16 partialCrcValueA;
	__le16 partialChecksumValueA;
	__le32 offsetInIOA;
	u8 partialDifDataA[8];
	u8 partialDifDataB[8];
};


/*
 * Timers context
 */
struct timers_context {
	__le32 logical_client_0;
/* Expiration time of logical client 0 */
#define TIMERS_CONTEXT_EXPIRATIONTIMELC0_MASK     0xFFFFFFF
#define TIMERS_CONTEXT_EXPIRATIONTIMELC0_SHIFT    0
/* Valid bit of logical client 0 */
#define TIMERS_CONTEXT_VALIDLC0_MASK              0x1
#define TIMERS_CONTEXT_VALIDLC0_SHIFT             28
/* Active bit of logical client 0 */
#define TIMERS_CONTEXT_ACTIVELC0_MASK             0x1
#define TIMERS_CONTEXT_ACTIVELC0_SHIFT            29
#define TIMERS_CONTEXT_RESERVED0_MASK             0x3
#define TIMERS_CONTEXT_RESERVED0_SHIFT            30
	__le32 logical_client_1;
/* Expiration time of logical client 1 */
#define TIMERS_CONTEXT_EXPIRATIONTIMELC1_MASK     0xFFFFFFF
#define TIMERS_CONTEXT_EXPIRATIONTIMELC1_SHIFT    0
/* Valid bit of logical client 1 */
#define TIMERS_CONTEXT_VALIDLC1_MASK              0x1
#define TIMERS_CONTEXT_VALIDLC1_SHIFT             28
/* Active bit of logical client 1 */
#define TIMERS_CONTEXT_ACTIVELC1_MASK             0x1
#define TIMERS_CONTEXT_ACTIVELC1_SHIFT            29
#define TIMERS_CONTEXT_RESERVED1_MASK             0x3
#define TIMERS_CONTEXT_RESERVED1_SHIFT            30
	__le32 logical_client_2;
/* Expiration time of logical client 2 */
#define TIMERS_CONTEXT_EXPIRATIONTIMELC2_MASK     0xFFFFFFF
#define TIMERS_CONTEXT_EXPIRATIONTIMELC2_SHIFT    0
/* Valid bit of logical client 2 */
#define TIMERS_CONTEXT_VALIDLC2_MASK              0x1
#define TIMERS_CONTEXT_VALIDLC2_SHIFT             28
/* Active bit of logical client 2 */
#define TIMERS_CONTEXT_ACTIVELC2_MASK             0x1
#define TIMERS_CONTEXT_ACTIVELC2_SHIFT            29
#define TIMERS_CONTEXT_RESERVED2_MASK             0x3
#define TIMERS_CONTEXT_RESERVED2_SHIFT            30
	__le32 host_expiration_fields;
/* Expiration time on host (closest one) */
#define TIMERS_CONTEXT_HOSTEXPRIRATIONVALUE_MASK  0xFFFFFFF
#define TIMERS_CONTEXT_HOSTEXPRIRATIONVALUE_SHIFT 0
/* Valid bit of host expiration */
#define TIMERS_CONTEXT_HOSTEXPRIRATIONVALID_MASK  0x1
#define TIMERS_CONTEXT_HOSTEXPRIRATIONVALID_SHIFT 28
#define TIMERS_CONTEXT_RESERVED3_MASK             0x7
#define TIMERS_CONTEXT_RESERVED3_SHIFT            29
};


/*
 * Enum for next_protocol field of tunnel_parsing_flags
 */
enum tunnel_next_protocol {
	e_unknown = 0,
	e_l2 = 1,
	e_ipv4 = 2,
	e_ipv6 = 3,
	MAX_TUNNEL_NEXT_PROTOCOL
};

#endif /* __COMMON_HSI__ */
