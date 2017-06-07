/*
 * Copyright (c) 2016 QLogic Corporation.
 * All rights reserved.
 * www.qlogic.com
 *
 * See LICENSE.qede_pmd for copyright and licensing details.
 */

#ifndef __ECORE_HSI_COMMON__
#define __ECORE_HSI_COMMON__
/********************************/
/* Add include to common target */
/********************************/
#include "common_hsi.h"


/*
 * opcodes for the event ring
 */
enum common_event_opcode {
	COMMON_EVENT_PF_START,
	COMMON_EVENT_PF_STOP,
	COMMON_EVENT_VF_START,
	COMMON_EVENT_VF_STOP,
	COMMON_EVENT_VF_PF_CHANNEL,
	COMMON_EVENT_VF_FLR,
	COMMON_EVENT_PF_UPDATE,
	COMMON_EVENT_MALICIOUS_VF,
	COMMON_EVENT_RL_UPDATE,
	COMMON_EVENT_EMPTY,
	MAX_COMMON_EVENT_OPCODE
};


/*
 * Common Ramrod Command IDs
 */
enum common_ramrod_cmd_id {
	COMMON_RAMROD_UNUSED,
	COMMON_RAMROD_PF_START /* PF Function Start Ramrod */,
	COMMON_RAMROD_PF_STOP /* PF Function Stop Ramrod */,
	COMMON_RAMROD_VF_START /* VF Function Start */,
	COMMON_RAMROD_VF_STOP /* VF Function Stop Ramrod */,
	COMMON_RAMROD_PF_UPDATE /* PF update Ramrod */,
	COMMON_RAMROD_RL_UPDATE /* QCN/DCQCN RL update Ramrod */,
	COMMON_RAMROD_EMPTY /* Empty Ramrod */,
	MAX_COMMON_RAMROD_CMD_ID
};


/*
 * The core storm context for the Ystorm
 */
struct ystorm_core_conn_st_ctx {
	__le32 reserved[4];
};

/*
 * The core storm context for the Pstorm
 */
struct pstorm_core_conn_st_ctx {
	__le32 reserved[4];
};

/*
 * Core Slowpath Connection storm context of Xstorm
 */
struct xstorm_core_conn_st_ctx {
	__le32 spq_base_lo /* SPQ Ring Base Address low dword */;
	__le32 spq_base_hi /* SPQ Ring Base Address high dword */;
/* Consolidation Ring Base Address */
	struct regpair consolid_base_addr;
	__le16 spq_cons /* SPQ Ring Consumer */;
	__le16 consolid_cons /* Consolidation Ring Consumer */;
	__le32 reserved0[55] /* Pad to 15 cycles */;
};

struct e4_xstorm_core_conn_ag_ctx {
	u8 reserved0 /* cdu_validation */;
	u8 core_state /* state */;
	u8 flags0;
/* exist_in_qm0 */
#define E4_XSTORM_CORE_CONN_AG_CTX_EXIST_IN_QM0_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_EXIST_IN_QM0_SHIFT        0
/* exist_in_qm1 */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED1_MASK            0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED1_SHIFT           1
/* exist_in_qm2 */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED2_MASK            0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED2_SHIFT           2
/* exist_in_qm3 */
#define E4_XSTORM_CORE_CONN_AG_CTX_EXIST_IN_QM3_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_EXIST_IN_QM3_SHIFT        3
/* bit4 */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED3_MASK            0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED3_SHIFT           4
/* cf_array_active */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED4_MASK            0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED4_SHIFT           5
/* bit6 */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED5_MASK            0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED5_SHIFT           6
/* bit7 */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED6_MASK            0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED6_SHIFT           7
	u8 flags1;
/* bit8 */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED7_MASK            0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED7_SHIFT           0
/* bit9 */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED8_MASK            0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED8_SHIFT           1
/* bit10 */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED9_MASK            0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED9_SHIFT           2
/* bit11 */
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT11_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT11_SHIFT               3
/* bit12 */
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT12_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT12_SHIFT               4
/* bit13 */
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT13_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT13_SHIFT               5
/* bit14 */
#define E4_XSTORM_CORE_CONN_AG_CTX_TX_RULE_ACTIVE_MASK       0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_TX_RULE_ACTIVE_SHIFT      6
/* bit15 */
#define E4_XSTORM_CORE_CONN_AG_CTX_DQ_CF_ACTIVE_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_DQ_CF_ACTIVE_SHIFT        7
	u8 flags2;
/* timer0cf */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF0_MASK                  0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF0_SHIFT                 0
/* timer1cf */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF1_MASK                  0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF1_SHIFT                 2
/* timer2cf */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF2_MASK                  0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF2_SHIFT                 4
/* timer_stop_all */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF3_MASK                  0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF3_SHIFT                 6
	u8 flags3;
#define E4_XSTORM_CORE_CONN_AG_CTX_CF4_MASK                  0x3 /* cf4 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF4_SHIFT                 0
#define E4_XSTORM_CORE_CONN_AG_CTX_CF5_MASK                  0x3 /* cf5 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF5_SHIFT                 2
#define E4_XSTORM_CORE_CONN_AG_CTX_CF6_MASK                  0x3 /* cf6 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF6_SHIFT                 4
#define E4_XSTORM_CORE_CONN_AG_CTX_CF7_MASK                  0x3 /* cf7 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF7_SHIFT                 6
	u8 flags4;
#define E4_XSTORM_CORE_CONN_AG_CTX_CF8_MASK                  0x3 /* cf8 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF8_SHIFT                 0
#define E4_XSTORM_CORE_CONN_AG_CTX_CF9_MASK                  0x3 /* cf9 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF9_SHIFT                 2
/* cf10 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF10_MASK                 0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF10_SHIFT                4
/* cf11 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF11_MASK                 0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF11_SHIFT                6
	u8 flags5;
/* cf12 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF12_MASK                 0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF12_SHIFT                0
/* cf13 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF13_MASK                 0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF13_SHIFT                2
/* cf14 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF14_MASK                 0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF14_SHIFT                4
/* cf15 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF15_MASK                 0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF15_SHIFT                6
	u8 flags6;
/* cf16 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CONSOLID_PROD_CF_MASK     0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CONSOLID_PROD_CF_SHIFT    0
/* cf_array_cf */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF17_MASK                 0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF17_SHIFT                2
/* cf18 */
#define E4_XSTORM_CORE_CONN_AG_CTX_DQ_CF_MASK                0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_DQ_CF_SHIFT               4
/* cf19 */
#define E4_XSTORM_CORE_CONN_AG_CTX_TERMINATE_CF_MASK         0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_TERMINATE_CF_SHIFT        6
	u8 flags7;
/* cf20 */
#define E4_XSTORM_CORE_CONN_AG_CTX_FLUSH_Q0_MASK             0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_FLUSH_Q0_SHIFT            0
/* cf21 */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED10_MASK           0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED10_SHIFT          2
/* cf22 */
#define E4_XSTORM_CORE_CONN_AG_CTX_SLOW_PATH_MASK            0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_SLOW_PATH_SHIFT           4
/* cf0en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF0EN_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF0EN_SHIFT               6
/* cf1en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF1EN_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF1EN_SHIFT               7
	u8 flags8;
/* cf2en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF2EN_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF2EN_SHIFT               0
/* cf3en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF3EN_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF3EN_SHIFT               1
/* cf4en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF4EN_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF4EN_SHIFT               2
/* cf5en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF5EN_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF5EN_SHIFT               3
/* cf6en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF6EN_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF6EN_SHIFT               4
/* cf7en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF7EN_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF7EN_SHIFT               5
/* cf8en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF8EN_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF8EN_SHIFT               6
/* cf9en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF9EN_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF9EN_SHIFT               7
	u8 flags9;
/* cf10en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF10EN_MASK               0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF10EN_SHIFT              0
/* cf11en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF11EN_MASK               0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF11EN_SHIFT              1
/* cf12en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF12EN_MASK               0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF12EN_SHIFT              2
/* cf13en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF13EN_MASK               0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF13EN_SHIFT              3
/* cf14en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF14EN_MASK               0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF14EN_SHIFT              4
/* cf15en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF15EN_MASK               0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF15EN_SHIFT              5
/* cf16en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CONSOLID_PROD_CF_EN_MASK  0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CONSOLID_PROD_CF_EN_SHIFT 6
/* cf_array_cf_en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF17EN_MASK               0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF17EN_SHIFT              7
	u8 flags10;
/* cf18en */
#define E4_XSTORM_CORE_CONN_AG_CTX_DQ_CF_EN_MASK             0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_DQ_CF_EN_SHIFT            0
/* cf19en */
#define E4_XSTORM_CORE_CONN_AG_CTX_TERMINATE_CF_EN_MASK      0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_TERMINATE_CF_EN_SHIFT     1
/* cf20en */
#define E4_XSTORM_CORE_CONN_AG_CTX_FLUSH_Q0_EN_MASK          0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_FLUSH_Q0_EN_SHIFT         2
/* cf21en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED11_MASK           0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED11_SHIFT          3
/* cf22en */
#define E4_XSTORM_CORE_CONN_AG_CTX_SLOW_PATH_EN_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_SLOW_PATH_EN_SHIFT        4
/* cf23en */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF23EN_MASK               0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_CF23EN_SHIFT              5
/* rule0en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED12_MASK           0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED12_SHIFT          6
/* rule1en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED13_MASK           0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED13_SHIFT          7
	u8 flags11;
/* rule2en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED14_MASK           0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED14_SHIFT          0
/* rule3en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED15_MASK           0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RESERVED15_SHIFT          1
/* rule4en */
#define E4_XSTORM_CORE_CONN_AG_CTX_TX_DEC_RULE_EN_MASK       0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_TX_DEC_RULE_EN_SHIFT      2
/* rule5en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE5EN_MASK              0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE5EN_SHIFT             3
/* rule6en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE6EN_MASK              0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE6EN_SHIFT             4
/* rule7en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE7EN_MASK              0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE7EN_SHIFT             5
/* rule8en */
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED1_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED1_SHIFT        6
/* rule9en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE9EN_MASK              0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE9EN_SHIFT             7
	u8 flags12;
/* rule10en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE10EN_MASK             0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE10EN_SHIFT            0
/* rule11en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE11EN_MASK             0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE11EN_SHIFT            1
/* rule12en */
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED2_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED2_SHIFT        2
/* rule13en */
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED3_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED3_SHIFT        3
/* rule14en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE14EN_MASK             0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE14EN_SHIFT            4
/* rule15en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE15EN_MASK             0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE15EN_SHIFT            5
/* rule16en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE16EN_MASK             0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE16EN_SHIFT            6
/* rule17en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE17EN_MASK             0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE17EN_SHIFT            7
	u8 flags13;
/* rule18en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE18EN_MASK             0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE18EN_SHIFT            0
/* rule19en */
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE19EN_MASK             0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_RULE19EN_SHIFT            1
/* rule20en */
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED4_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED4_SHIFT        2
/* rule21en */
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED5_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED5_SHIFT        3
/* rule22en */
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED6_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED6_SHIFT        4
/* rule23en */
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED7_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED7_SHIFT        5
/* rule24en */
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED8_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED8_SHIFT        6
/* rule25en */
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED9_MASK         0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_A0_RESERVED9_SHIFT        7
	u8 flags14;
/* bit16 */
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT16_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT16_SHIFT               0
/* bit17 */
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT17_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT17_SHIFT               1
/* bit18 */
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT18_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT18_SHIFT               2
/* bit19 */
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT19_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT19_SHIFT               3
/* bit20 */
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT20_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT20_SHIFT               4
/* bit21 */
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT21_MASK                0x1
#define E4_XSTORM_CORE_CONN_AG_CTX_BIT21_SHIFT               5
/* cf23 */
#define E4_XSTORM_CORE_CONN_AG_CTX_CF23_MASK                 0x3
#define E4_XSTORM_CORE_CONN_AG_CTX_CF23_SHIFT                6
	u8 byte2 /* byte2 */;
	__le16 physical_q0 /* physical_q0 */;
	__le16 consolid_prod /* physical_q1 */;
	__le16 reserved16 /* physical_q2 */;
	__le16 tx_bd_cons /* word3 */;
	__le16 tx_bd_or_spq_prod /* word4 */;
	__le16 word5 /* word5 */;
	__le16 conn_dpi /* conn_dpi */;
	u8 byte3 /* byte3 */;
	u8 byte4 /* byte4 */;
	u8 byte5 /* byte5 */;
	u8 byte6 /* byte6 */;
	__le32 reg0 /* reg0 */;
	__le32 reg1 /* reg1 */;
	__le32 reg2 /* reg2 */;
	__le32 reg3 /* reg3 */;
	__le32 reg4 /* reg4 */;
	__le32 reg5 /* cf_array0 */;
	__le32 reg6 /* cf_array1 */;
	__le16 word7 /* word7 */;
	__le16 word8 /* word8 */;
	__le16 word9 /* word9 */;
	__le16 word10 /* word10 */;
	__le32 reg7 /* reg7 */;
	__le32 reg8 /* reg8 */;
	__le32 reg9 /* reg9 */;
	u8 byte7 /* byte7 */;
	u8 byte8 /* byte8 */;
	u8 byte9 /* byte9 */;
	u8 byte10 /* byte10 */;
	u8 byte11 /* byte11 */;
	u8 byte12 /* byte12 */;
	u8 byte13 /* byte13 */;
	u8 byte14 /* byte14 */;
	u8 byte15 /* byte15 */;
	u8 e5_reserved /* e5_reserved */;
	__le16 word11 /* word11 */;
	__le32 reg10 /* reg10 */;
	__le32 reg11 /* reg11 */;
	__le32 reg12 /* reg12 */;
	__le32 reg13 /* reg13 */;
	__le32 reg14 /* reg14 */;
	__le32 reg15 /* reg15 */;
	__le32 reg16 /* reg16 */;
	__le32 reg17 /* reg17 */;
	__le32 reg18 /* reg18 */;
	__le32 reg19 /* reg19 */;
	__le16 word12 /* word12 */;
	__le16 word13 /* word13 */;
	__le16 word14 /* word14 */;
	__le16 word15 /* word15 */;
};

struct e4_tstorm_core_conn_ag_ctx {
	u8 byte0 /* cdu_validation */;
	u8 byte1 /* state */;
	u8 flags0;
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT0_MASK     0x1 /* exist_in_qm0 */
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT0_SHIFT    0
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT1_MASK     0x1 /* exist_in_qm1 */
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT1_SHIFT    1
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT2_MASK     0x1 /* bit2 */
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT2_SHIFT    2
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT3_MASK     0x1 /* bit3 */
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT3_SHIFT    3
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT4_MASK     0x1 /* bit4 */
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT4_SHIFT    4
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT5_MASK     0x1 /* bit5 */
#define E4_TSTORM_CORE_CONN_AG_CTX_BIT5_SHIFT    5
#define E4_TSTORM_CORE_CONN_AG_CTX_CF0_MASK      0x3 /* timer0cf */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF0_SHIFT     6
	u8 flags1;
#define E4_TSTORM_CORE_CONN_AG_CTX_CF1_MASK      0x3 /* timer1cf */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF1_SHIFT     0
#define E4_TSTORM_CORE_CONN_AG_CTX_CF2_MASK      0x3 /* timer2cf */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF2_SHIFT     2
#define E4_TSTORM_CORE_CONN_AG_CTX_CF3_MASK      0x3 /* timer_stop_all */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF3_SHIFT     4
#define E4_TSTORM_CORE_CONN_AG_CTX_CF4_MASK      0x3 /* cf4 */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF4_SHIFT     6
	u8 flags2;
#define E4_TSTORM_CORE_CONN_AG_CTX_CF5_MASK      0x3 /* cf5 */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF5_SHIFT     0
#define E4_TSTORM_CORE_CONN_AG_CTX_CF6_MASK      0x3 /* cf6 */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF6_SHIFT     2
#define E4_TSTORM_CORE_CONN_AG_CTX_CF7_MASK      0x3 /* cf7 */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF7_SHIFT     4
#define E4_TSTORM_CORE_CONN_AG_CTX_CF8_MASK      0x3 /* cf8 */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF8_SHIFT     6
	u8 flags3;
#define E4_TSTORM_CORE_CONN_AG_CTX_CF9_MASK      0x3 /* cf9 */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF9_SHIFT     0
#define E4_TSTORM_CORE_CONN_AG_CTX_CF10_MASK     0x3 /* cf10 */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF10_SHIFT    2
#define E4_TSTORM_CORE_CONN_AG_CTX_CF0EN_MASK    0x1 /* cf0en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF0EN_SHIFT   4
#define E4_TSTORM_CORE_CONN_AG_CTX_CF1EN_MASK    0x1 /* cf1en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF1EN_SHIFT   5
#define E4_TSTORM_CORE_CONN_AG_CTX_CF2EN_MASK    0x1 /* cf2en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF2EN_SHIFT   6
#define E4_TSTORM_CORE_CONN_AG_CTX_CF3EN_MASK    0x1 /* cf3en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF3EN_SHIFT   7
	u8 flags4;
#define E4_TSTORM_CORE_CONN_AG_CTX_CF4EN_MASK    0x1 /* cf4en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF4EN_SHIFT   0
#define E4_TSTORM_CORE_CONN_AG_CTX_CF5EN_MASK    0x1 /* cf5en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF5EN_SHIFT   1
#define E4_TSTORM_CORE_CONN_AG_CTX_CF6EN_MASK    0x1 /* cf6en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF6EN_SHIFT   2
#define E4_TSTORM_CORE_CONN_AG_CTX_CF7EN_MASK    0x1 /* cf7en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF7EN_SHIFT   3
#define E4_TSTORM_CORE_CONN_AG_CTX_CF8EN_MASK    0x1 /* cf8en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF8EN_SHIFT   4
#define E4_TSTORM_CORE_CONN_AG_CTX_CF9EN_MASK    0x1 /* cf9en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF9EN_SHIFT   5
#define E4_TSTORM_CORE_CONN_AG_CTX_CF10EN_MASK   0x1 /* cf10en */
#define E4_TSTORM_CORE_CONN_AG_CTX_CF10EN_SHIFT  6
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE0EN_MASK  0x1 /* rule0en */
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE0EN_SHIFT 7
	u8 flags5;
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE1EN_MASK  0x1 /* rule1en */
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE1EN_SHIFT 0
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE2EN_MASK  0x1 /* rule2en */
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE2EN_SHIFT 1
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE3EN_MASK  0x1 /* rule3en */
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE3EN_SHIFT 2
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE4EN_MASK  0x1 /* rule4en */
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE4EN_SHIFT 3
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE5EN_MASK  0x1 /* rule5en */
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE5EN_SHIFT 4
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE6EN_MASK  0x1 /* rule6en */
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE6EN_SHIFT 5
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE7EN_MASK  0x1 /* rule7en */
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE7EN_SHIFT 6
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE8EN_MASK  0x1 /* rule8en */
#define E4_TSTORM_CORE_CONN_AG_CTX_RULE8EN_SHIFT 7
	__le32 reg0 /* reg0 */;
	__le32 reg1 /* reg1 */;
	__le32 reg2 /* reg2 */;
	__le32 reg3 /* reg3 */;
	__le32 reg4 /* reg4 */;
	__le32 reg5 /* reg5 */;
	__le32 reg6 /* reg6 */;
	__le32 reg7 /* reg7 */;
	__le32 reg8 /* reg8 */;
	u8 byte2 /* byte2 */;
	u8 byte3 /* byte3 */;
	__le16 word0 /* word0 */;
	u8 byte4 /* byte4 */;
	u8 byte5 /* byte5 */;
	__le16 word1 /* word1 */;
	__le16 word2 /* conn_dpi */;
	__le16 word3 /* word3 */;
	__le32 reg9 /* reg9 */;
	__le32 reg10 /* reg10 */;
};

struct e4_ustorm_core_conn_ag_ctx {
	u8 reserved /* cdu_validation */;
	u8 byte1 /* state */;
	u8 flags0;
#define E4_USTORM_CORE_CONN_AG_CTX_BIT0_MASK     0x1 /* exist_in_qm0 */
#define E4_USTORM_CORE_CONN_AG_CTX_BIT0_SHIFT    0
#define E4_USTORM_CORE_CONN_AG_CTX_BIT1_MASK     0x1 /* exist_in_qm1 */
#define E4_USTORM_CORE_CONN_AG_CTX_BIT1_SHIFT    1
#define E4_USTORM_CORE_CONN_AG_CTX_CF0_MASK      0x3 /* timer0cf */
#define E4_USTORM_CORE_CONN_AG_CTX_CF0_SHIFT     2
#define E4_USTORM_CORE_CONN_AG_CTX_CF1_MASK      0x3 /* timer1cf */
#define E4_USTORM_CORE_CONN_AG_CTX_CF1_SHIFT     4
#define E4_USTORM_CORE_CONN_AG_CTX_CF2_MASK      0x3 /* timer2cf */
#define E4_USTORM_CORE_CONN_AG_CTX_CF2_SHIFT     6
	u8 flags1;
#define E4_USTORM_CORE_CONN_AG_CTX_CF3_MASK      0x3 /* timer_stop_all */
#define E4_USTORM_CORE_CONN_AG_CTX_CF3_SHIFT     0
#define E4_USTORM_CORE_CONN_AG_CTX_CF4_MASK      0x3 /* cf4 */
#define E4_USTORM_CORE_CONN_AG_CTX_CF4_SHIFT     2
#define E4_USTORM_CORE_CONN_AG_CTX_CF5_MASK      0x3 /* cf5 */
#define E4_USTORM_CORE_CONN_AG_CTX_CF5_SHIFT     4
#define E4_USTORM_CORE_CONN_AG_CTX_CF6_MASK      0x3 /* cf6 */
#define E4_USTORM_CORE_CONN_AG_CTX_CF6_SHIFT     6
	u8 flags2;
#define E4_USTORM_CORE_CONN_AG_CTX_CF0EN_MASK    0x1 /* cf0en */
#define E4_USTORM_CORE_CONN_AG_CTX_CF0EN_SHIFT   0
#define E4_USTORM_CORE_CONN_AG_CTX_CF1EN_MASK    0x1 /* cf1en */
#define E4_USTORM_CORE_CONN_AG_CTX_CF1EN_SHIFT   1
#define E4_USTORM_CORE_CONN_AG_CTX_CF2EN_MASK    0x1 /* cf2en */
#define E4_USTORM_CORE_CONN_AG_CTX_CF2EN_SHIFT   2
#define E4_USTORM_CORE_CONN_AG_CTX_CF3EN_MASK    0x1 /* cf3en */
#define E4_USTORM_CORE_CONN_AG_CTX_CF3EN_SHIFT   3
#define E4_USTORM_CORE_CONN_AG_CTX_CF4EN_MASK    0x1 /* cf4en */
#define E4_USTORM_CORE_CONN_AG_CTX_CF4EN_SHIFT   4
#define E4_USTORM_CORE_CONN_AG_CTX_CF5EN_MASK    0x1 /* cf5en */
#define E4_USTORM_CORE_CONN_AG_CTX_CF5EN_SHIFT   5
#define E4_USTORM_CORE_CONN_AG_CTX_CF6EN_MASK    0x1 /* cf6en */
#define E4_USTORM_CORE_CONN_AG_CTX_CF6EN_SHIFT   6
#define E4_USTORM_CORE_CONN_AG_CTX_RULE0EN_MASK  0x1 /* rule0en */
#define E4_USTORM_CORE_CONN_AG_CTX_RULE0EN_SHIFT 7
	u8 flags3;
#define E4_USTORM_CORE_CONN_AG_CTX_RULE1EN_MASK  0x1 /* rule1en */
#define E4_USTORM_CORE_CONN_AG_CTX_RULE1EN_SHIFT 0
#define E4_USTORM_CORE_CONN_AG_CTX_RULE2EN_MASK  0x1 /* rule2en */
#define E4_USTORM_CORE_CONN_AG_CTX_RULE2EN_SHIFT 1
#define E4_USTORM_CORE_CONN_AG_CTX_RULE3EN_MASK  0x1 /* rule3en */
#define E4_USTORM_CORE_CONN_AG_CTX_RULE3EN_SHIFT 2
#define E4_USTORM_CORE_CONN_AG_CTX_RULE4EN_MASK  0x1 /* rule4en */
#define E4_USTORM_CORE_CONN_AG_CTX_RULE4EN_SHIFT 3
#define E4_USTORM_CORE_CONN_AG_CTX_RULE5EN_MASK  0x1 /* rule5en */
#define E4_USTORM_CORE_CONN_AG_CTX_RULE5EN_SHIFT 4
#define E4_USTORM_CORE_CONN_AG_CTX_RULE6EN_MASK  0x1 /* rule6en */
#define E4_USTORM_CORE_CONN_AG_CTX_RULE6EN_SHIFT 5
#define E4_USTORM_CORE_CONN_AG_CTX_RULE7EN_MASK  0x1 /* rule7en */
#define E4_USTORM_CORE_CONN_AG_CTX_RULE7EN_SHIFT 6
#define E4_USTORM_CORE_CONN_AG_CTX_RULE8EN_MASK  0x1 /* rule8en */
#define E4_USTORM_CORE_CONN_AG_CTX_RULE8EN_SHIFT 7
	u8 byte2 /* byte2 */;
	u8 byte3 /* byte3 */;
	__le16 word0 /* conn_dpi */;
	__le16 word1 /* word1 */;
	__le32 rx_producers /* reg0 */;
	__le32 reg1 /* reg1 */;
	__le32 reg2 /* reg2 */;
	__le32 reg3 /* reg3 */;
	__le16 word2 /* word2 */;
	__le16 word3 /* word3 */;
};

/*
 * The core storm context for the Mstorm
 */
struct mstorm_core_conn_st_ctx {
	__le32 reserved[24];
};

/*
 * The core storm context for the Ustorm
 */
struct ustorm_core_conn_st_ctx {
	__le32 reserved[4];
};

/*
 * core connection context
 */
struct core_conn_context {
/* ystorm storm context */
	struct ystorm_core_conn_st_ctx ystorm_st_context;
	struct regpair ystorm_st_padding[2] /* padding */;
/* pstorm storm context */
	struct pstorm_core_conn_st_ctx pstorm_st_context;
	struct regpair pstorm_st_padding[2] /* padding */;
/* xstorm storm context */
	struct xstorm_core_conn_st_ctx xstorm_st_context;
/* xstorm aggregative context */
	struct e4_xstorm_core_conn_ag_ctx xstorm_ag_context;
/* tstorm aggregative context */
	struct e4_tstorm_core_conn_ag_ctx tstorm_ag_context;
/* ustorm aggregative context */
	struct e4_ustorm_core_conn_ag_ctx ustorm_ag_context;
/* mstorm storm context */
	struct mstorm_core_conn_st_ctx mstorm_st_context;
/* ustorm storm context */
	struct ustorm_core_conn_st_ctx ustorm_st_context;
	struct regpair ustorm_st_padding[2] /* padding */;
};


/*
 * How ll2 should deal with packet upon errors
 */
enum core_error_handle {
	LL2_DROP_PACKET /* If error occurs drop packet */,
	LL2_DO_NOTHING /* If error occurs do nothing */,
	LL2_ASSERT /* If error occurs assert */,
	MAX_CORE_ERROR_HANDLE
};


/*
 * opcodes for the event ring
 */
enum core_event_opcode {
	CORE_EVENT_TX_QUEUE_START,
	CORE_EVENT_TX_QUEUE_STOP,
	CORE_EVENT_RX_QUEUE_START,
	CORE_EVENT_RX_QUEUE_STOP,
	CORE_EVENT_RX_QUEUE_FLUSH,
	MAX_CORE_EVENT_OPCODE
};


/*
 * The L4 pseudo checksum mode for Core
 */
enum core_l4_pseudo_checksum_mode {
/* Pseudo Checksum on packet is calculated with the correct packet length. */
	CORE_L4_PSEUDO_CSUM_CORRECT_LENGTH,
/* Pseudo Checksum on packet is calculated with zero length. */
	CORE_L4_PSEUDO_CSUM_ZERO_LENGTH,
	MAX_CORE_L4_PSEUDO_CHECKSUM_MODE
};


/*
 * Light-L2 RX Producers in Tstorm RAM
 */
struct core_ll2_port_stats {
	struct regpair gsi_invalid_hdr;
	struct regpair gsi_invalid_pkt_length;
	struct regpair gsi_unsupported_pkt_typ;
	struct regpair gsi_crcchksm_error;
};


/*
 * Ethernet TX Per Queue Stats
 */
struct core_ll2_pstorm_per_queue_stat {
/* number of total bytes sent without errors */
	struct regpair sent_ucast_bytes;
/* number of total bytes sent without errors */
	struct regpair sent_mcast_bytes;
/* number of total bytes sent without errors */
	struct regpair sent_bcast_bytes;
/* number of total packets sent without errors */
	struct regpair sent_ucast_pkts;
/* number of total packets sent without errors */
	struct regpair sent_mcast_pkts;
/* number of total packets sent without errors */
	struct regpair sent_bcast_pkts;
};


/*
 * Light-L2 RX Producers in Tstorm RAM
 */
struct core_ll2_rx_prod {
	__le16 bd_prod /* BD Producer */;
	__le16 cqe_prod /* CQE Producer */;
	__le32 reserved;
};


struct core_ll2_tstorm_per_queue_stat {
/* Number of packets discarded because they are bigger than MTU */
	struct regpair packet_too_big_discard;
/* Number of packets discarded due to lack of host buffers */
	struct regpair no_buff_discard;
};


struct core_ll2_ustorm_per_queue_stat {
	struct regpair rcv_ucast_bytes;
	struct regpair rcv_mcast_bytes;
	struct regpair rcv_bcast_bytes;
	struct regpair rcv_ucast_pkts;
	struct regpair rcv_mcast_pkts;
	struct regpair rcv_bcast_pkts;
};


/*
 * Core Ramrod Command IDs (light L2)
 */
enum core_ramrod_cmd_id {
	CORE_RAMROD_UNUSED,
	CORE_RAMROD_RX_QUEUE_START /* RX Queue Start Ramrod */,
	CORE_RAMROD_TX_QUEUE_START /* TX Queue Start Ramrod */,
	CORE_RAMROD_RX_QUEUE_STOP /* RX Queue Stop Ramrod */,
	CORE_RAMROD_TX_QUEUE_STOP /* TX Queue Stop Ramrod */,
	CORE_RAMROD_RX_QUEUE_FLUSH /* RX Flush queue Ramrod */,
	MAX_CORE_RAMROD_CMD_ID
};


/*
 * Core RX CQE Type for Light L2
 */
enum core_roce_flavor_type {
	CORE_ROCE,
	CORE_RROCE,
	MAX_CORE_ROCE_FLAVOR_TYPE
};


/*
 * Specifies how ll2 should deal with packets errors: packet_too_big and no_buff
 */
struct core_rx_action_on_error {
	u8 error_type;
/* ll2 how to handle error packet_too_big (use enum core_error_handle) */
#define CORE_RX_ACTION_ON_ERROR_PACKET_TOO_BIG_MASK  0x3
#define CORE_RX_ACTION_ON_ERROR_PACKET_TOO_BIG_SHIFT 0
/* ll2 how to handle error with no_buff  (use enum core_error_handle) */
#define CORE_RX_ACTION_ON_ERROR_NO_BUFF_MASK         0x3
#define CORE_RX_ACTION_ON_ERROR_NO_BUFF_SHIFT        2
#define CORE_RX_ACTION_ON_ERROR_RESERVED_MASK        0xF
#define CORE_RX_ACTION_ON_ERROR_RESERVED_SHIFT       4
};


/*
 * Core RX BD for Light L2
 */
struct core_rx_bd {
	struct regpair addr;
	__le16 reserved[4];
};


/*
 * Core RX CM offload BD for Light L2
 */
struct core_rx_bd_with_buff_len {
	struct regpair addr;
	__le16 buff_length;
	__le16 reserved[3];
};

/*
 * Core RX CM offload BD for Light L2
 */
union core_rx_bd_union {
	struct core_rx_bd rx_bd /* Core Rx Bd static buffer size */;
/* Core Rx Bd with dynamic buffer length */
	struct core_rx_bd_with_buff_len rx_bd_with_len;
};



/*
 * Opaque Data for Light L2 RX CQE .
 */
struct core_rx_cqe_opaque_data {
	__le32 data[2] /* Opaque CQE Data */;
};


/*
 * Core RX CQE Type for Light L2
 */
enum core_rx_cqe_type {
	CORE_RX_CQE_ILLIGAL_TYPE /* Bad RX Cqe type */,
	CORE_RX_CQE_TYPE_REGULAR /* Regular Core RX CQE */,
	CORE_RX_CQE_TYPE_GSI_OFFLOAD /* Fp Gsi offload RX CQE */,
	CORE_RX_CQE_TYPE_SLOW_PATH /* Slow path Core RX CQE */,
	MAX_CORE_RX_CQE_TYPE
};


/*
 * Core RX CQE for Light L2 .
 */
struct core_rx_fast_path_cqe {
	u8 type /* CQE type */;
/* Offset (in bytes) of the packet from start of the buffer */
	u8 placement_offset;
/* Parsing and error flags from the parser */
	struct parsing_and_err_flags parse_flags;
	__le16 packet_length /* Total packet length (from the parser) */;
	__le16 vlan /* 802.1q VLAN tag */;
	struct core_rx_cqe_opaque_data opaque_data /* Opaque Data */;
/* bit- map: each bit represents a specific error. errors indications are
 * provided by the cracker. see spec for detailed description
 */
	struct parsing_err_flags err_flags;
	__le16 reserved0;
	__le32 reserved1[3];
};

/*
 * Core Rx CM offload CQE .
 */
struct core_rx_gsi_offload_cqe {
	u8 type /* CQE type */;
	u8 data_length_error /* set if gsi data is bigger than buff */;
/* Parsing and error flags from the parser */
	struct parsing_and_err_flags parse_flags;
	__le16 data_length /* Total packet length (from the parser) */;
	__le16 vlan /* 802.1q VLAN tag */;
	__le32 src_mac_addrhi /* hi 4 bytes source mac address */;
	__le16 src_mac_addrlo /* lo 2 bytes of source mac address */;
/* These are the lower 16 bit of QP id in RoCE BTH header */
	__le16 qp_id;
	__le32 gid_dst[4] /* Gid destination address */;
};

/*
 * Core RX CQE for Light L2 .
 */
struct core_rx_slow_path_cqe {
	u8 type /* CQE type */;
	u8 ramrod_cmd_id;
	__le16 echo;
	struct core_rx_cqe_opaque_data opaque_data /* Opaque Data */;
	__le32 reserved1[5];
};

/*
 * Core RX CM offload BD for Light L2
 */
union core_rx_cqe_union {
	struct core_rx_fast_path_cqe rx_cqe_fp /* Fast path CQE */;
	struct core_rx_gsi_offload_cqe rx_cqe_gsi /* GSI offload CQE */;
	struct core_rx_slow_path_cqe rx_cqe_sp /* Slow path CQE */;
};





/*
 * Ramrod data for rx queue start ramrod
 */
struct core_rx_start_ramrod_data {
	struct regpair bd_base /* bd address of the first bd page */;
	struct regpair cqe_pbl_addr /* Base address on host of CQE PBL */;
	__le16 mtu /* Maximum transmission unit */;
	__le16 sb_id /* Status block ID */;
	u8 sb_index /* index of the protocol index */;
	u8 complete_cqe_flg /* post completion to the CQE ring if set */;
	u8 complete_event_flg /* post completion to the event ring if set */;
	u8 drop_ttl0_flg /* drop packet with ttl0 if set */;
	__le16 num_of_pbl_pages /* Num of pages in CQE PBL */;
/* if set, 802.1q tags will be removed and copied to CQE */
	u8 inner_vlan_removal_en;
	u8 queue_id /* Light L2 RX Queue ID */;
	u8 main_func_queue /* Is this the main queue for the PF */;
/* Duplicate broadcast packets to LL2 main queue in mf_si mode. Valid if
 * main_func_queue is set.
 */
	u8 mf_si_bcast_accept_all;
/* Duplicate multicast packets to LL2 main queue in mf_si mode. Valid if
 * main_func_queue is set.
 */
	u8 mf_si_mcast_accept_all;
/* Specifies how ll2 should deal with packets errors: packet_too_big and
 * no_buff
 */
	struct core_rx_action_on_error action_on_error;
/* set when in GSI offload mode on ROCE connection */
	u8 gsi_offload_flag;
	u8 reserved[7];
};


/*
 * Ramrod data for rx queue stop ramrod
 */
struct core_rx_stop_ramrod_data {
	u8 complete_cqe_flg /* post completion to the CQE ring if set */;
	u8 complete_event_flg /* post completion to the event ring if set */;
	u8 queue_id /* Light L2 RX Queue ID */;
	u8 reserved1;
	__le16 reserved2[2];
};


/*
 * Flags for Core TX BD
 */
struct core_tx_bd_data {
	__le16 as_bitfield;
/* Do not allow additional VLAN manipulations on this packet (DCB) */
#define CORE_TX_BD_DATA_FORCE_VLAN_MODE_MASK      0x1
#define CORE_TX_BD_DATA_FORCE_VLAN_MODE_SHIFT     0
/* Insert VLAN into packet */
#define CORE_TX_BD_DATA_VLAN_INSERTION_MASK       0x1
#define CORE_TX_BD_DATA_VLAN_INSERTION_SHIFT      1
/* This is the first BD of the packet (for debug) */
#define CORE_TX_BD_DATA_START_BD_MASK             0x1
#define CORE_TX_BD_DATA_START_BD_SHIFT            2
/* Calculate the IP checksum for the packet */
#define CORE_TX_BD_DATA_IP_CSUM_MASK              0x1
#define CORE_TX_BD_DATA_IP_CSUM_SHIFT             3
/* Calculate the L4 checksum for the packet */
#define CORE_TX_BD_DATA_L4_CSUM_MASK              0x1
#define CORE_TX_BD_DATA_L4_CSUM_SHIFT             4
/* Packet is IPv6 with extensions */
#define CORE_TX_BD_DATA_IPV6_EXT_MASK             0x1
#define CORE_TX_BD_DATA_IPV6_EXT_SHIFT            5
/* If IPv6+ext, and if l4_csum is 1, than this field indicates L4 protocol:
 * 0-TCP, 1-UDP
 */
#define CORE_TX_BD_DATA_L4_PROTOCOL_MASK          0x1
#define CORE_TX_BD_DATA_L4_PROTOCOL_SHIFT         6
/* The pseudo checksum mode to place in the L4 checksum field. Required only
 * when IPv6+ext and l4_csum is set. (use enum core_l4_pseudo_checksum_mode)
 */
#define CORE_TX_BD_DATA_L4_PSEUDO_CSUM_MODE_MASK  0x1
#define CORE_TX_BD_DATA_L4_PSEUDO_CSUM_MODE_SHIFT 7
/* Number of BDs that make up one packet - width wide enough to present
 * CORE_LL2_TX_MAX_BDS_PER_PACKET
 */
#define CORE_TX_BD_DATA_NBDS_MASK                 0xF
#define CORE_TX_BD_DATA_NBDS_SHIFT                8
/* Use roce_flavor enum - Differentiate between Roce flavors is valid when
 * connType is ROCE (use enum core_roce_flavor_type)
 */
#define CORE_TX_BD_DATA_ROCE_FLAV_MASK            0x1
#define CORE_TX_BD_DATA_ROCE_FLAV_SHIFT           12
/* Calculate ip length */
#define CORE_TX_BD_DATA_IP_LEN_MASK               0x1
#define CORE_TX_BD_DATA_IP_LEN_SHIFT              13
#define CORE_TX_BD_DATA_RESERVED0_MASK            0x3
#define CORE_TX_BD_DATA_RESERVED0_SHIFT           14
};

/*
 * Core TX BD for Light L2
 */
struct core_tx_bd {
	struct regpair addr /* Buffer Address */;
	__le16 nbytes /* Number of Bytes in Buffer */;
/* Network packets: VLAN to insert to packet (if insertion flag set) LoopBack
 * packets: echo data to pass to Rx
 */
	__le16 nw_vlan_or_lb_echo;
	struct core_tx_bd_data bd_data /* BD Flags */;
	__le16 bitfield1;
/* L4 Header Offset from start of packet (in Words). This is needed if both
 * l4_csum and ipv6_ext are set
 */
#define CORE_TX_BD_L4_HDR_OFFSET_W_MASK  0x3FFF
#define CORE_TX_BD_L4_HDR_OFFSET_W_SHIFT 0
/* Packet destination - Network, Loopback or Drop (use enum core_tx_dest) */
#define CORE_TX_BD_TX_DST_MASK           0x3
#define CORE_TX_BD_TX_DST_SHIFT          14
};



/*
 * Light L2 TX Destination
 */
enum core_tx_dest {
	CORE_TX_DEST_NW /* TX Destination to the Network */,
	CORE_TX_DEST_LB /* TX Destination to the Loopback */,
	CORE_TX_DEST_RESERVED,
	CORE_TX_DEST_DROP /* TX Drop */,
	MAX_CORE_TX_DEST
};


/*
 * Ramrod data for tx queue start ramrod
 */
struct core_tx_start_ramrod_data {
	struct regpair pbl_base_addr /* Address of the pbl page */;
	__le16 mtu /* Maximum transmission unit */;
	__le16 sb_id /* Status block ID */;
	u8 sb_index /* Status block protocol index */;
	u8 stats_en /* Statistics Enable */;
	u8 stats_id /* Statistics Counter ID */;
	u8 conn_type /* connection type that loaded ll2 */;
	__le16 pbl_size /* Number of BD pages pointed by PBL */;
	__le16 qm_pq_id /* QM PQ ID */;
/* set when in GSI offload mode on ROCE connection */
	u8 gsi_offload_flag;
	u8 resrved[3];
};


/*
 * Ramrod data for tx queue stop ramrod
 */
struct core_tx_stop_ramrod_data {
	__le32 reserved0[2];
};


/*
 * Enum flag for what type of dcb data to update
 */
enum dcb_dscp_update_mode {
/* use when no change should be done to dcb data */
	DONT_UPDATE_DCB_DSCP,
	UPDATE_DCB /* use to update only l2 (vlan) priority */,
	UPDATE_DSCP /* use to update only l3 dscp */,
	UPDATE_DCB_DSCP /* update vlan pri and dscp */,
	MAX_DCB_DSCP_UPDATE_FLAG
};


struct eth_mstorm_per_pf_stat {
	struct regpair gre_discard_pkts /* Dropped GRE RX packets */;
	struct regpair vxlan_discard_pkts /* Dropped VXLAN RX packets */;
	struct regpair geneve_discard_pkts /* Dropped GENEVE RX packets */;
	struct regpair lb_discard_pkts /* Dropped Tx switched packets */;
};


struct eth_mstorm_per_queue_stat {
/* Number of packets discarded because TTL=0 (in IPv4) or hopLimit=0 (IPv6) */
	struct regpair ttl0_discard;
/* Number of packets discarded because they are bigger than MTU */
	struct regpair packet_too_big_discard;
/* Number of packets discarded due to lack of host buffers (BDs/SGEs/CQEs) */
	struct regpair no_buff_discard;
/* Number of packets discarded because of no active Rx connection */
	struct regpair not_active_discard;
/* number of coalesced packets in all TPA aggregations */
	struct regpair tpa_coalesced_pkts;
/* total number of TPA aggregations */
	struct regpair tpa_coalesced_events;
/* number of aggregations, which abnormally ended */
	struct regpair tpa_aborts_num;
/* total TCP payload length in all TPA aggregations */
	struct regpair tpa_coalesced_bytes;
};


/*
 * Ethernet TX Per PF
 */
struct eth_pstorm_per_pf_stat {
/* number of total ucast bytes sent on loopback port without errors */
	struct regpair sent_lb_ucast_bytes;
/* number of total mcast bytes sent on loopback port without errors */
	struct regpair sent_lb_mcast_bytes;
/* number of total bcast bytes sent on loopback port without errors */
	struct regpair sent_lb_bcast_bytes;
/* number of total ucast packets sent on loopback port without errors */
	struct regpair sent_lb_ucast_pkts;
/* number of total mcast packets sent on loopback port without errors */
	struct regpair sent_lb_mcast_pkts;
/* number of total bcast packets sent on loopback port without errors */
	struct regpair sent_lb_bcast_pkts;
	struct regpair sent_gre_bytes /* Sent GRE bytes */;
	struct regpair sent_vxlan_bytes /* Sent VXLAN bytes */;
	struct regpair sent_geneve_bytes /* Sent GENEVE bytes */;
	struct regpair sent_gre_pkts /* Sent GRE packets */;
	struct regpair sent_vxlan_pkts /* Sent VXLAN packets */;
	struct regpair sent_geneve_pkts /* Sent GENEVE packets */;
	struct regpair gre_drop_pkts /* Dropped GRE TX packets */;
	struct regpair vxlan_drop_pkts /* Dropped VXLAN TX packets */;
	struct regpair geneve_drop_pkts /* Dropped GENEVE TX packets */;
};


/*
 * Ethernet TX Per Queue Stats
 */
struct eth_pstorm_per_queue_stat {
/* number of total bytes sent without errors */
	struct regpair sent_ucast_bytes;
/* number of total bytes sent without errors */
	struct regpair sent_mcast_bytes;
/* number of total bytes sent without errors */
	struct regpair sent_bcast_bytes;
/* number of total packets sent without errors */
	struct regpair sent_ucast_pkts;
/* number of total packets sent without errors */
	struct regpair sent_mcast_pkts;
/* number of total packets sent without errors */
	struct regpair sent_bcast_pkts;
/* number of total packets dropped due to errors */
	struct regpair error_drop_pkts;
};


/*
 * ETH Rx producers data
 */
struct eth_rx_rate_limit {
/* Rate Limit Multiplier - (Storm Clock (MHz) * 8 / Desired Bandwidth (MB/s)) */
	__le16 mult;
/* Constant term to add (or subtract from number of cycles) */
	__le16 cnst;
	u8 add_sub_cnst /* Add (1) or subtract (0) constant term */;
	u8 reserved0;
	__le16 reserved1;
};


struct eth_ustorm_per_pf_stat {
/* number of total ucast bytes received on loopback port without errors */
	struct regpair rcv_lb_ucast_bytes;
/* number of total mcast bytes received on loopback port without errors */
	struct regpair rcv_lb_mcast_bytes;
/* number of total bcast bytes received on loopback port without errors */
	struct regpair rcv_lb_bcast_bytes;
/* number of total ucast packets received on loopback port without errors */
	struct regpair rcv_lb_ucast_pkts;
/* number of total mcast packets received on loopback port without errors */
	struct regpair rcv_lb_mcast_pkts;
/* number of total bcast packets received on loopback port without errors */
	struct regpair rcv_lb_bcast_pkts;
	struct regpair rcv_gre_bytes /* Received GRE bytes */;
	struct regpair rcv_vxlan_bytes /* Received VXLAN bytes */;
	struct regpair rcv_geneve_bytes /* Received GENEVE bytes */;
	struct regpair rcv_gre_pkts /* Received GRE packets */;
	struct regpair rcv_vxlan_pkts /* Received VXLAN packets */;
	struct regpair rcv_geneve_pkts /* Received GENEVE packets */;
};


struct eth_ustorm_per_queue_stat {
	struct regpair rcv_ucast_bytes;
	struct regpair rcv_mcast_bytes;
	struct regpair rcv_bcast_bytes;
	struct regpair rcv_ucast_pkts;
	struct regpair rcv_mcast_pkts;
	struct regpair rcv_bcast_pkts;
};


/*
 * Event Ring Next Page Address
 */
struct event_ring_next_addr {
	struct regpair addr /* Next Page Address */;
	__le32 reserved[2] /* Reserved */;
};

/*
 * Event Ring Element
 */
union event_ring_element {
	struct event_ring_entry entry /* Event Ring Entry */;
/* Event Ring Next Page Address */
	struct event_ring_next_addr next_addr;
};



/*
 * Ports mode
 */
enum fw_flow_ctrl_mode {
	flow_ctrl_pause,
	flow_ctrl_pfc,
	MAX_FW_FLOW_CTRL_MODE
};


/*
 * Major and Minor hsi Versions
 */
struct hsi_fp_ver_struct {
	u8 minor_ver_arr[2] /* Minor Version of hsi loading pf */;
	u8 major_ver_arr[2] /* Major Version of driver loading pf */;
};


/*
 * Integration Phase
 */
enum integ_phase {
	INTEG_PHASE_BB_A0_LATEST = 3 /* BB A0 latest integration phase */,
	INTEG_PHASE_BB_B0_NO_MCP = 10 /* BB B0 without MCP */,
	INTEG_PHASE_BB_B0_WITH_MCP = 11 /* BB B0 with MCP */,
	MAX_INTEG_PHASE
};


/*
 * Ports mode
 */
enum iwarp_ll2_tx_queues {
/* LL2 queue for OOO packets sent in-order by the driver */
	IWARP_LL2_IN_ORDER_TX_QUEUE = 1,
/* LL2 queue for unaligned packets sent aligned by the driver */
	IWARP_LL2_ALIGNED_TX_QUEUE,
/* LL2 queue for unaligned packets sent aligned and was right-trimmed by the
 * driver
 */
	IWARP_LL2_ALIGNED_RIGHT_TRIMMED_TX_QUEUE,
	IWARP_LL2_ERROR /* Error indication */,
	MAX_IWARP_LL2_TX_QUEUES
};


/*
 * Malicious VF error ID
 */
enum malicious_vf_error_id {
	MALICIOUS_VF_NO_ERROR /* Zero placeholder value */,
/* Writing to VF/PF channel when it is not ready */
	VF_PF_CHANNEL_NOT_READY,
	VF_ZONE_MSG_NOT_VALID /* VF channel message is not valid */,
	VF_ZONE_FUNC_NOT_ENABLED /* Parent PF of VF channel is not active */,
/* TX packet is shorter then reported on BDs or from minimal size */
	ETH_PACKET_TOO_SMALL,
/* Tx packet with marked as insert VLAN when its illegal */
	ETH_ILLEGAL_VLAN_MODE,
	ETH_MTU_VIOLATION /* TX packet is greater then MTU */,
/* TX packet has illegal inband tags marked */
	ETH_ILLEGAL_INBAND_TAGS,
/* Vlan cant be added to inband tag */
	ETH_VLAN_INSERT_AND_INBAND_VLAN,
/* indicated number of BDs for the packet is illegal */
	ETH_ILLEGAL_NBDS,
	ETH_FIRST_BD_WO_SOP /* 1st BD must have start_bd flag set */,
/* There are not enough BDs for transmission of even one packet */
	ETH_INSUFFICIENT_BDS,
	ETH_ILLEGAL_LSO_HDR_NBDS /* Header NBDs value is illegal */,
	ETH_ILLEGAL_LSO_MSS /* LSO MSS value is more than allowed */,
/* empty BD (which not contains control flags) is illegal  */
	ETH_ZERO_SIZE_BD,
	ETH_ILLEGAL_LSO_HDR_LEN /* LSO header size is above the limit  */,
/* In LSO its expected that on the local BD ring there will be at least MSS
 * bytes of data
 */
	ETH_INSUFFICIENT_PAYLOAD,
	ETH_EDPM_OUT_OF_SYNC /* Valid BDs on local ring after EDPM L2 sync */,
/* Tunneled packet with IPv6+Ext without a proper number of BDs */
	ETH_TUNN_IPV6_EXT_NBD_ERR,
	ETH_CONTROL_PACKET_VIOLATION /* VF sent control frame such as PFC */,
	ETH_ANTI_SPOOFING_ERR /* Anti-Spoofing verification failure */,
	MAX_MALICIOUS_VF_ERROR_ID
};



/*
 * Mstorm non-triggering VF zone
 */
struct mstorm_non_trigger_vf_zone {
/* VF statistic bucket */
	struct eth_mstorm_per_queue_stat eth_queue_stat;
/* VF RX queues producers */
	struct eth_rx_prod_data
		eth_rx_queue_producers[ETH_MAX_NUM_RX_QUEUES_PER_VF_QUAD];
};


/*
 * Mstorm VF zone
 */
struct mstorm_vf_zone {
/* non-interrupt-triggering zone */
	struct mstorm_non_trigger_vf_zone non_trigger;
};


/*
 * personality per PF
 */
enum personality_type {
	BAD_PERSONALITY_TYP,
	PERSONALITY_ISCSI /* iSCSI and LL2 */,
	PERSONALITY_FCOE /* Fcoe and LL2 */,
	PERSONALITY_RDMA_AND_ETH /* Roce or Iwarp, Eth and LL2 */,
	PERSONALITY_RDMA /* Roce and LL2 */,
	PERSONALITY_CORE /* CORE(LL2) */,
	PERSONALITY_ETH /* Ethernet */,
	PERSONALITY_TOE /* Toe and LL2 */,
	MAX_PERSONALITY_TYPE
};


/*
 * tunnel configuration
 */
struct pf_start_tunnel_config {
/* Set VXLAN tunnel UDP destination port to vxlan_udp_port. If not set -
 * FW will use a default port
 */
	u8 set_vxlan_udp_port_flg;
/* Set GENEVE tunnel UDP destination port to geneve_udp_port. If not set -
 * FW will use a default port
 */
	u8 set_geneve_udp_port_flg;
	u8 tunnel_clss_vxlan /* Rx classification scheme for VXLAN tunnel. */;
/* Rx classification scheme for l2 GENEVE tunnel. */
	u8 tunnel_clss_l2geneve;
/* Rx classification scheme for ip GENEVE tunnel. */
	u8 tunnel_clss_ipgeneve;
	u8 tunnel_clss_l2gre /* Rx classification scheme for l2 GRE tunnel. */;
	u8 tunnel_clss_ipgre /* Rx classification scheme for ip GRE tunnel. */;
	u8 reserved;
/* VXLAN tunnel UDP destination port. Valid if set_vxlan_udp_port_flg=1 */
	__le16 vxlan_udp_port;
/* GENEVE tunnel UDP destination port. Valid if set_geneve_udp_port_flg=1 */
	__le16 geneve_udp_port;
};

/*
 * Ramrod data for PF start ramrod
 */
struct pf_start_ramrod_data {
	struct regpair event_ring_pbl_addr /* Address of event ring PBL */;
/* PBL address of consolidation queue */
	struct regpair consolid_q_pbl_addr;
/* tunnel configuration. */
	struct pf_start_tunnel_config tunnel_config;
	__le32 reserved;
	__le16 event_ring_sb_id /* Status block ID */;
/* All VfIds owned by Pf will be from baseVfId till baseVfId+numVfs */
	u8 base_vf_id;
	u8 num_vfs /* Amount of vfs owned by PF */;
	u8 event_ring_num_pages /* Number of PBL pages in event ring */;
	u8 event_ring_sb_index /* Status block index */;
	u8 path_id /* HW path ID (engine ID) */;
	u8 warning_as_error /* In FW asserts, treat warning as error */;
/* If not set - throw a warning for each ramrod (for debug) */
	u8 dont_log_ramrods;
	u8 personality /* define what type of personality is new PF */;
/* Log type mask. Each bit set enables a corresponding event type logging.
 * Event types are defined as ASSERT_LOG_TYPE_xxx
 */
	__le16 log_type_mask;
	u8 mf_mode /* Multi function mode */;
	u8 integ_phase /* Integration phase */;
/* If set, inter-pf tx switching is allowed in Switch Independent func mode */
	u8 allow_npar_tx_switching;
/* Map from inner to outer priority. Set pri_map_valid when init map */
	u8 inner_to_outer_pri_map[8];
/* If inner_to_outer_pri_map is initialize then set pri_map_valid */
	u8 pri_map_valid;
/* In case mf_mode is MF_OVLAN, this field specifies the outer vlan
 * (lower 16 bits) and ethType to use (higher 16 bits)
 */
	__le32 outer_tag;
/* FP HSI version to be used by FW */
	struct hsi_fp_ver_struct hsi_fp_ver;
};



/*
 * Data for port update ramrod
 */
struct protocol_dcb_data {
	u8 dcb_enable_flag /* dcbEnable flag value */;
	u8 dscp_enable_flag /* If set use dscp value */;
	u8 dcb_priority /* dcbPri flag value */;
	u8 dcb_tc /* dcb TC value */;
	u8 dscp_val /* dscp value to write if dscp_enable_flag is set */;
	u8 reserved0;
};

/*
 * Update tunnel configuration
 */
struct pf_update_tunnel_config {
/* Update RX per PF tunnel classification scheme. */
	u8 update_rx_pf_clss;
/* Update per PORT default tunnel RX classification scheme for traffic with
 * unknown unicast outer MAC in NPAR mode.
 */
	u8 update_rx_def_ucast_clss;
/* Update per PORT default tunnel RX classification scheme for traffic with non
 * unicast outer MAC in NPAR mode.
 */
	u8 update_rx_def_non_ucast_clss;
/* Update VXLAN tunnel UDP destination port. */
	u8 set_vxlan_udp_port_flg;
/* Update GENEVE tunnel UDP destination port. */
	u8 set_geneve_udp_port_flg;
	u8 tunnel_clss_vxlan /* Classification scheme for VXLAN tunnel. */;
/* Classification scheme for l2 GENEVE tunnel. */
	u8 tunnel_clss_l2geneve;
/* Classification scheme for ip GENEVE tunnel. */
	u8 tunnel_clss_ipgeneve;
	u8 tunnel_clss_l2gre /* Classification scheme for l2 GRE tunnel. */;
	u8 tunnel_clss_ipgre /* Classification scheme for ip GRE tunnel. */;
	__le16 vxlan_udp_port /* VXLAN tunnel UDP destination port. */;
	__le16 geneve_udp_port /* GENEVE tunnel UDP destination port. */;
	__le16 reserved;
};

/*
 * Data for port update ramrod
 */
struct pf_update_ramrod_data {
	u8 pf_id;
	u8 update_eth_dcb_data_mode /* Update Eth DCB  data indication */;
	u8 update_fcoe_dcb_data_mode /* Update FCOE DCB  data indication */;
	u8 update_iscsi_dcb_data_mode /* Update iSCSI DCB  data indication */;
	u8 update_roce_dcb_data_mode /* Update ROCE DCB  data indication */;
/* Update RROCE (RoceV2) DCB  data indication */
	u8 update_rroce_dcb_data_mode;
	u8 update_iwarp_dcb_data_mode /* Update IWARP DCB  data indication */;
	u8 update_mf_vlan_flag /* Update MF outer vlan Id */;
	struct protocol_dcb_data eth_dcb_data /* core eth related fields */;
	struct protocol_dcb_data fcoe_dcb_data /* core fcoe related fields */;
/* core iscsi related fields */
	struct protocol_dcb_data iscsi_dcb_data;
	struct protocol_dcb_data roce_dcb_data /* core roce related fields */;
/* core roce related fields */
	struct protocol_dcb_data rroce_dcb_data;
/* core iwarp related fields */
	struct protocol_dcb_data iwarp_dcb_data;
	__le16 mf_vlan /* new outer vlan id value */;
	__le16 reserved;
/* tunnel configuration. */
	struct pf_update_tunnel_config tunnel_config;
};



/*
 * Ports mode
 */
enum ports_mode {
	ENGX2_PORTX1 /* 2 engines x 1 port */,
	ENGX2_PORTX2 /* 2 engines x 2 ports */,
	ENGX1_PORTX1 /* 1 engine  x 1 port */,
	ENGX1_PORTX2 /* 1 engine  x 2 ports */,
	ENGX1_PORTX4 /* 1 engine  x 4 ports */,
	MAX_PORTS_MODE
};



/*
 * use to index in hsi_fp_[major|minor]_ver_arr per protocol
 */
enum protocol_version_array_key {
	ETH_VER_KEY = 0,
	ROCE_VER_KEY,
	MAX_PROTOCOL_VERSION_ARRAY_KEY
};



/*
 * RDMA TX Stats
 */
struct rdma_sent_stats {
	struct regpair sent_bytes /* number of total RDMA bytes sent */;
	struct regpair sent_pkts /* number of total RDMA packets sent */;
};

/*
 * Pstorm non-triggering VF zone
 */
struct pstorm_non_trigger_vf_zone {
/* VF statistic bucket */
	struct eth_pstorm_per_queue_stat eth_queue_stat;
	struct rdma_sent_stats rdma_stats /* RoCE sent statistics */;
};


/*
 * Pstorm VF zone
 */
struct pstorm_vf_zone {
/* non-interrupt-triggering zone */
	struct pstorm_non_trigger_vf_zone non_trigger;
	struct regpair reserved[7] /* vf_zone size mus be power of 2 */;
};


/*
 * Ramrod Header of SPQE
 */
struct ramrod_header {
	__le32 cid /* Slowpath Connection CID */;
	u8 cmd_id /* Ramrod Cmd (Per Protocol Type) */;
	u8 protocol_id /* Ramrod Protocol ID */;
	__le16 echo /* Ramrod echo */;
};


/*
 * RDMA RX Stats
 */
struct rdma_rcv_stats {
	struct regpair rcv_bytes /* number of total RDMA bytes received */;
	struct regpair rcv_pkts /* number of total RDMA packets received */;
};



/*
 * Data for update QCN/DCQCN RL ramrod
 */
struct rl_update_ramrod_data {
	u8 qcn_update_param_flg /* Update QCN global params: timeout. */;
/* Update DCQCN global params: timeout, g, k. */
	u8 dcqcn_update_param_flg;
	u8 rl_init_flg /* Init RL parameters, when RL disabled. */;
	u8 rl_start_flg /* Start RL in IDLE state. Set rate to maximum. */;
	u8 rl_stop_flg /* Stop RL. */;
	u8 rl_id_first /* ID of first or single RL, that will be updated. */;
/* ID of last RL, that will be updated. If clear, single RL will updated. */
	u8 rl_id_last;
	u8 rl_dc_qcn_flg /* If set, RL will used for DCQCN. */;
	__le32 rl_bc_rate /* Byte Counter Limit. */;
	__le16 rl_max_rate /* Maximum rate in 1.6 Mbps resolution. */;
	__le16 rl_r_ai /* Active increase rate. */;
	__le16 rl_r_hai /* Hyper active increase rate. */;
	__le16 dcqcn_g /* DCQCN Alpha update gain in 1/64K resolution . */;
	__le32 dcqcn_k_us /* DCQCN Alpha update interval. */;
	__le32 dcqcn_timeuot_us /* DCQCN timeout. */;
	__le32 qcn_timeuot_us /* QCN timeout. */;
	__le32 reserved[2];
};


/*
 * Slowpath Element (SPQE)
 */
struct slow_path_element {
	struct ramrod_header hdr /* Ramrod Header */;
	struct regpair data_ptr /* Pointer to the Ramrod Data on the Host */;
};


/*
 * Tstorm non-triggering VF zone
 */
struct tstorm_non_trigger_vf_zone {
	struct rdma_rcv_stats rdma_stats /* RoCE received statistics */;
};


struct tstorm_per_port_stat {
/* packet is dropped because it was truncated in NIG */
	struct regpair trunc_error_discard;
/* packet is dropped because of Ethernet FCS error */
	struct regpair mac_error_discard;
/* packet is dropped because classification was unsuccessful */
	struct regpair mftag_filter_discard;
/* packet was passed to Ethernet and dropped because of no mac filter match */
	struct regpair eth_mac_filter_discard;
/* packet passed to Light L2 and dropped because Light L2 is not configured for
 * this PF
 */
	struct regpair ll2_mac_filter_discard;
/* packet passed to Light L2 and dropped because Light L2 is not configured for
 * this PF
 */
	struct regpair ll2_conn_disabled_discard;
/* packet is an ISCSI irregular packet */
	struct regpair iscsi_irregular_pkt;
/* packet is an FCOE irregular packet */
	struct regpair fcoe_irregular_pkt;
/* packet is an ROCE irregular packet */
	struct regpair roce_irregular_pkt;
/* packet is an IWARP irregular packet */
	struct regpair iwarp_irregular_pkt;
/* packet is an ETH irregular packet */
	struct regpair eth_irregular_pkt;
/* packet is an TOE irregular packet */
	struct regpair toe_irregular_pkt;
/* packet is an PREROCE irregular packet */
	struct regpair preroce_irregular_pkt;
	struct regpair eth_gre_tunn_filter_discard /* GRE dropped packets */;
/* VXLAN dropped packets */
	struct regpair eth_vxlan_tunn_filter_discard;
/* GENEVE dropped packets */
	struct regpair eth_geneve_tunn_filter_discard;
};


/*
 * Tstorm VF zone
 */
struct tstorm_vf_zone {
/* non-interrupt-triggering zone */
	struct tstorm_non_trigger_vf_zone non_trigger;
};


/*
 * Tunnel classification scheme
 */
enum tunnel_clss {
/* Use MAC and VLAN from first L2 header for vport classification. */
	TUNNEL_CLSS_MAC_VLAN = 0,
/* Use MAC from first L2 header and VNI from tunnel header for vport
 * classification
 */
	TUNNEL_CLSS_MAC_VNI,
/* Use MAC and VLAN from last L2 header for vport classification */
	TUNNEL_CLSS_INNER_MAC_VLAN,
/* Use MAC from last L2 header and VNI from tunnel header for vport
 * classification
 */
	TUNNEL_CLSS_INNER_MAC_VNI,
/* Use MAC and VLAN from last L2 header for vport classification. If no exact
 * match, use MAC and VLAN from first L2 header for classification.
 */
	TUNNEL_CLSS_MAC_VLAN_DUAL_STAGE,
	MAX_TUNNEL_CLSS
};



/*
 * Ustorm non-triggering VF zone
 */
struct ustorm_non_trigger_vf_zone {
/* VF statistic bucket */
	struct eth_ustorm_per_queue_stat eth_queue_stat;
	struct regpair vf_pf_msg_addr /* VF-PF message address */;
};


/*
 * Ustorm triggering VF zone
 */
struct ustorm_trigger_vf_zone {
	u8 vf_pf_msg_valid /* VF-PF message valid flag */;
	u8 reserved[7];
};


/*
 * Ustorm VF zone
 */
struct ustorm_vf_zone {
/* non-interrupt-triggering zone */
	struct ustorm_non_trigger_vf_zone non_trigger;
	struct ustorm_trigger_vf_zone trigger /* interrupt triggering zone */;
};


/*
 * VF-PF channel data
 */
struct vf_pf_channel_data {
/* 0: VF-PF Channel NOT ready. Waiting for ack from PF driver. 1: VF-PF Channel
 * is ready for a new transaction.
 */
	__le32 ready;
/* 0: VF-PF Channel is invalid because of malicious VF. 1: VF-PF Channel is
 * valid.
 */
	u8 valid;
	u8 reserved0;
	__le16 reserved1;
};


/*
 * Ramrod data for VF start ramrod
 */
struct vf_start_ramrod_data {
	u8 vf_id /* VF ID */;
/* If set, initial cleanup ack will be sent to parent PF SP event queue */
	u8 enable_flr_ack;
	__le16 opaque_fid /* VF opaque FID */;
	u8 personality /* define what type of personality is new VF */;
	u8 reserved[7];
/* FP HSI version to be used by FW */
	struct hsi_fp_ver_struct hsi_fp_ver;
};


/*
 * Ramrod data for VF start ramrod
 */
struct vf_stop_ramrod_data {
	u8 vf_id /* VF ID */;
	u8 reserved0;
	__le16 reserved1;
	__le32 reserved2;
};


/*
 * VF zone size mode.
 */
enum vf_zone_size_mode {
/* Default VF zone size. Up to 192 VF supported. */
	VF_ZONE_SIZE_MODE_DEFAULT,
/* Doubled VF zone size. Up to 96 VF supported. */
	VF_ZONE_SIZE_MODE_DOUBLE,
/* Quad VF zone size. Up to 48 VF supported. */
	VF_ZONE_SIZE_MODE_QUAD,
	MAX_VF_ZONE_SIZE_MODE
};




/*
 * Attentions status block
 */
struct atten_status_block {
	__le32 atten_bits;
	__le32 atten_ack;
	__le16 reserved0;
	__le16 sb_index /* status block running index */;
	__le32 reserved1;
};


/*
 * Igu cleanup bit values to distinguish between clean or producer consumer
 * update.
 */
enum command_type_bit {
	IGU_COMMAND_TYPE_NOP = 0,
	IGU_COMMAND_TYPE_SET = 1,
	MAX_COMMAND_TYPE_BIT
};


/*
 * DMAE command
 */
struct dmae_cmd {
	__le32 opcode;
/* DMA Source. 0 - PCIe, 1 - GRC (use enum dmae_cmd_src_enum) */
#define DMAE_CMD_SRC_MASK              0x1
#define DMAE_CMD_SRC_SHIFT             0
/* DMA destination. 0 - None, 1 - PCIe, 2 - GRC, 3 - None
 * (use enum dmae_cmd_dst_enum)
 */
#define DMAE_CMD_DST_MASK              0x3
#define DMAE_CMD_DST_SHIFT             1
/* Completion destination. 0 - PCie, 1 - GRC (use enum dmae_cmd_c_dst_enum) */
#define DMAE_CMD_C_DST_MASK            0x1
#define DMAE_CMD_C_DST_SHIFT           3
/* Reset the CRC result (do not use the previous result as the seed) */
#define DMAE_CMD_CRC_RESET_MASK        0x1
#define DMAE_CMD_CRC_RESET_SHIFT       4
/* Reset the source address in the next go to the same source address of the
 * previous go
 */
#define DMAE_CMD_SRC_ADDR_RESET_MASK   0x1
#define DMAE_CMD_SRC_ADDR_RESET_SHIFT  5
/* Reset the destination address in the next go to the same destination address
 * of the previous go
 */
#define DMAE_CMD_DST_ADDR_RESET_MASK   0x1
#define DMAE_CMD_DST_ADDR_RESET_SHIFT  6
/* 0   completion function is the same as src function, 1 - 0 completion
 * function is the same as dst function (use enum dmae_cmd_comp_func_enum)
 */
#define DMAE_CMD_COMP_FUNC_MASK        0x1
#define DMAE_CMD_COMP_FUNC_SHIFT       7
/* 0 - Do not write a completion word, 1 - Write a completion word
 * (use enum dmae_cmd_comp_word_en_enum)
 */
#define DMAE_CMD_COMP_WORD_EN_MASK     0x1
#define DMAE_CMD_COMP_WORD_EN_SHIFT    8
/* 0 - Do not write a CRC word, 1 - Write a CRC word
 * (use enum dmae_cmd_comp_crc_en_enum)
 */
#define DMAE_CMD_COMP_CRC_EN_MASK      0x1
#define DMAE_CMD_COMP_CRC_EN_SHIFT     9
/* The CRC word should be taken from the DMAE address space from address 9+X,
 * where X is the value in these bits.
 */
#define DMAE_CMD_COMP_CRC_OFFSET_MASK  0x7
#define DMAE_CMD_COMP_CRC_OFFSET_SHIFT 10
#define DMAE_CMD_RESERVED1_MASK        0x1
#define DMAE_CMD_RESERVED1_SHIFT       13
#define DMAE_CMD_ENDIANITY_MODE_MASK   0x3
#define DMAE_CMD_ENDIANITY_MODE_SHIFT  14
/* The field specifies how the completion word is affected by PCIe read error. 0
 * Send a regular completion, 1 - Send a completion with an error indication,
 * 2 do not send a completion (use enum dmae_cmd_error_handling_enum)
 */
#define DMAE_CMD_ERR_HANDLING_MASK     0x3
#define DMAE_CMD_ERR_HANDLING_SHIFT    16
/* The port ID to be placed on the  RF FID  field of the GRC bus. this field is
 * used both when GRC is the destination and when it is the source of the DMAE
 * transaction.
 */
#define DMAE_CMD_PORT_ID_MASK          0x3
#define DMAE_CMD_PORT_ID_SHIFT         18
/* Source PCI function number [3:0] */
#define DMAE_CMD_SRC_PF_ID_MASK        0xF
#define DMAE_CMD_SRC_PF_ID_SHIFT       20
/* Destination PCI function number [3:0] */
#define DMAE_CMD_DST_PF_ID_MASK        0xF
#define DMAE_CMD_DST_PF_ID_SHIFT       24
#define DMAE_CMD_SRC_VF_ID_VALID_MASK  0x1 /* Source VFID valid */
#define DMAE_CMD_SRC_VF_ID_VALID_SHIFT 28
#define DMAE_CMD_DST_VF_ID_VALID_MASK  0x1 /* Destination VFID valid */
#define DMAE_CMD_DST_VF_ID_VALID_SHIFT 29
#define DMAE_CMD_RESERVED2_MASK        0x3
#define DMAE_CMD_RESERVED2_SHIFT       30
/* PCIe source address low in bytes or GRC source address in DW */
	__le32 src_addr_lo;
/* PCIe source address high in bytes or reserved (if source is GRC) */
	__le32 src_addr_hi;
/* PCIe destination address low in bytes or GRC destination address in DW */
	__le32 dst_addr_lo;
/* PCIe destination address high in bytes or reserved (if destination is GRC) */
	__le32 dst_addr_hi;
	__le16 length_dw /* Length in DW */;
	__le16 opcode_b;
#define DMAE_CMD_SRC_VF_ID_MASK        0xFF /* Source VF id */
#define DMAE_CMD_SRC_VF_ID_SHIFT       0
#define DMAE_CMD_DST_VF_ID_MASK        0xFF /* Destination VF id */
#define DMAE_CMD_DST_VF_ID_SHIFT       8
/* PCIe completion address low in bytes or GRC completion address in DW */
	__le32 comp_addr_lo;
/* PCIe completion address high in bytes or reserved (if completion address is
 * GRC)
 */
	__le32 comp_addr_hi;
	__le32 comp_val /* Value to write to completion address */;
	__le32 crc32 /* crc16 result */;
	__le32 crc_32_c /* crc32_c result */;
	__le16 crc16 /* crc16 result */;
	__le16 crc16_c /* crc16_c result */;
	__le16 crc10 /* crc_t10 result */;
	__le16 reserved;
	__le16 xsum16 /* checksum16 result  */;
	__le16 xsum8 /* checksum8 result  */;
};


enum dmae_cmd_comp_crc_en_enum {
	dmae_cmd_comp_crc_disabled /* Do not write a CRC word */,
	dmae_cmd_comp_crc_enabled /* Write a CRC word */,
	MAX_DMAE_CMD_COMP_CRC_EN_ENUM
};


enum dmae_cmd_comp_func_enum {
/* completion word and/or CRC will be sent to SRC-PCI function/SRC VFID */
	dmae_cmd_comp_func_to_src,
/* completion word and/or CRC will be sent to DST-PCI function/DST VFID */
	dmae_cmd_comp_func_to_dst,
	MAX_DMAE_CMD_COMP_FUNC_ENUM
};


enum dmae_cmd_comp_word_en_enum {
	dmae_cmd_comp_word_disabled /* Do not write a completion word */,
	dmae_cmd_comp_word_enabled /* Write the completion word */,
	MAX_DMAE_CMD_COMP_WORD_EN_ENUM
};


enum dmae_cmd_c_dst_enum {
	dmae_cmd_c_dst_pcie,
	dmae_cmd_c_dst_grc,
	MAX_DMAE_CMD_C_DST_ENUM
};


enum dmae_cmd_dst_enum {
	dmae_cmd_dst_none_0,
	dmae_cmd_dst_pcie,
	dmae_cmd_dst_grc,
	dmae_cmd_dst_none_3,
	MAX_DMAE_CMD_DST_ENUM
};


enum dmae_cmd_error_handling_enum {
/* Send a regular completion (with no error indication) */
	dmae_cmd_error_handling_send_regular_comp,
/* Send a completion with an error indication (i.e. set bit 31 of the completion
 * word)
 */
	dmae_cmd_error_handling_send_comp_with_err,
	dmae_cmd_error_handling_dont_send_comp /* Do not send a completion */,
	MAX_DMAE_CMD_ERROR_HANDLING_ENUM
};


enum dmae_cmd_src_enum {
	dmae_cmd_src_pcie /* The source is the PCIe */,
	dmae_cmd_src_grc /* The source is the GRC */,
	MAX_DMAE_CMD_SRC_ENUM
};


struct e4_mstorm_core_conn_ag_ctx {
	u8 byte0 /* cdu_validation */;
	u8 byte1 /* state */;
	u8 flags0;
#define E4_MSTORM_CORE_CONN_AG_CTX_BIT0_MASK     0x1 /* exist_in_qm0 */
#define E4_MSTORM_CORE_CONN_AG_CTX_BIT0_SHIFT    0
#define E4_MSTORM_CORE_CONN_AG_CTX_BIT1_MASK     0x1 /* exist_in_qm1 */
#define E4_MSTORM_CORE_CONN_AG_CTX_BIT1_SHIFT    1
#define E4_MSTORM_CORE_CONN_AG_CTX_CF0_MASK      0x3 /* cf0 */
#define E4_MSTORM_CORE_CONN_AG_CTX_CF0_SHIFT     2
#define E4_MSTORM_CORE_CONN_AG_CTX_CF1_MASK      0x3 /* cf1 */
#define E4_MSTORM_CORE_CONN_AG_CTX_CF1_SHIFT     4
#define E4_MSTORM_CORE_CONN_AG_CTX_CF2_MASK      0x3 /* cf2 */
#define E4_MSTORM_CORE_CONN_AG_CTX_CF2_SHIFT     6
	u8 flags1;
#define E4_MSTORM_CORE_CONN_AG_CTX_CF0EN_MASK    0x1 /* cf0en */
#define E4_MSTORM_CORE_CONN_AG_CTX_CF0EN_SHIFT   0
#define E4_MSTORM_CORE_CONN_AG_CTX_CF1EN_MASK    0x1 /* cf1en */
#define E4_MSTORM_CORE_CONN_AG_CTX_CF1EN_SHIFT   1
#define E4_MSTORM_CORE_CONN_AG_CTX_CF2EN_MASK    0x1 /* cf2en */
#define E4_MSTORM_CORE_CONN_AG_CTX_CF2EN_SHIFT   2
#define E4_MSTORM_CORE_CONN_AG_CTX_RULE0EN_MASK  0x1 /* rule0en */
#define E4_MSTORM_CORE_CONN_AG_CTX_RULE0EN_SHIFT 3
#define E4_MSTORM_CORE_CONN_AG_CTX_RULE1EN_MASK  0x1 /* rule1en */
#define E4_MSTORM_CORE_CONN_AG_CTX_RULE1EN_SHIFT 4
#define E4_MSTORM_CORE_CONN_AG_CTX_RULE2EN_MASK  0x1 /* rule2en */
#define E4_MSTORM_CORE_CONN_AG_CTX_RULE2EN_SHIFT 5
#define E4_MSTORM_CORE_CONN_AG_CTX_RULE3EN_MASK  0x1 /* rule3en */
#define E4_MSTORM_CORE_CONN_AG_CTX_RULE3EN_SHIFT 6
#define E4_MSTORM_CORE_CONN_AG_CTX_RULE4EN_MASK  0x1 /* rule4en */
#define E4_MSTORM_CORE_CONN_AG_CTX_RULE4EN_SHIFT 7
	__le16 word0 /* word0 */;
	__le16 word1 /* word1 */;
	__le32 reg0 /* reg0 */;
	__le32 reg1 /* reg1 */;
};





struct e4_ystorm_core_conn_ag_ctx {
	u8 byte0 /* cdu_validation */;
	u8 byte1 /* state */;
	u8 flags0;
#define E4_YSTORM_CORE_CONN_AG_CTX_BIT0_MASK     0x1 /* exist_in_qm0 */
#define E4_YSTORM_CORE_CONN_AG_CTX_BIT0_SHIFT    0
#define E4_YSTORM_CORE_CONN_AG_CTX_BIT1_MASK     0x1 /* exist_in_qm1 */
#define E4_YSTORM_CORE_CONN_AG_CTX_BIT1_SHIFT    1
#define E4_YSTORM_CORE_CONN_AG_CTX_CF0_MASK      0x3 /* cf0 */
#define E4_YSTORM_CORE_CONN_AG_CTX_CF0_SHIFT     2
#define E4_YSTORM_CORE_CONN_AG_CTX_CF1_MASK      0x3 /* cf1 */
#define E4_YSTORM_CORE_CONN_AG_CTX_CF1_SHIFT     4
#define E4_YSTORM_CORE_CONN_AG_CTX_CF2_MASK      0x3 /* cf2 */
#define E4_YSTORM_CORE_CONN_AG_CTX_CF2_SHIFT     6
	u8 flags1;
#define E4_YSTORM_CORE_CONN_AG_CTX_CF0EN_MASK    0x1 /* cf0en */
#define E4_YSTORM_CORE_CONN_AG_CTX_CF0EN_SHIFT   0
#define E4_YSTORM_CORE_CONN_AG_CTX_CF1EN_MASK    0x1 /* cf1en */
#define E4_YSTORM_CORE_CONN_AG_CTX_CF1EN_SHIFT   1
#define E4_YSTORM_CORE_CONN_AG_CTX_CF2EN_MASK    0x1 /* cf2en */
#define E4_YSTORM_CORE_CONN_AG_CTX_CF2EN_SHIFT   2
#define E4_YSTORM_CORE_CONN_AG_CTX_RULE0EN_MASK  0x1 /* rule0en */
#define E4_YSTORM_CORE_CONN_AG_CTX_RULE0EN_SHIFT 3
#define E4_YSTORM_CORE_CONN_AG_CTX_RULE1EN_MASK  0x1 /* rule1en */
#define E4_YSTORM_CORE_CONN_AG_CTX_RULE1EN_SHIFT 4
#define E4_YSTORM_CORE_CONN_AG_CTX_RULE2EN_MASK  0x1 /* rule2en */
#define E4_YSTORM_CORE_CONN_AG_CTX_RULE2EN_SHIFT 5
#define E4_YSTORM_CORE_CONN_AG_CTX_RULE3EN_MASK  0x1 /* rule3en */
#define E4_YSTORM_CORE_CONN_AG_CTX_RULE3EN_SHIFT 6
#define E4_YSTORM_CORE_CONN_AG_CTX_RULE4EN_MASK  0x1 /* rule4en */
#define E4_YSTORM_CORE_CONN_AG_CTX_RULE4EN_SHIFT 7
	u8 byte2 /* byte2 */;
	u8 byte3 /* byte3 */;
	__le16 word0 /* word0 */;
	__le32 reg0 /* reg0 */;
	__le32 reg1 /* reg1 */;
	__le16 word1 /* word1 */;
	__le16 word2 /* word2 */;
	__le16 word3 /* word3 */;
	__le16 word4 /* word4 */;
	__le32 reg2 /* reg2 */;
	__le32 reg3 /* reg3 */;
};


/*
 * IGU cleanup command
 */
struct igu_cleanup {
	__le32 sb_id_and_flags;
#define IGU_CLEANUP_RESERVED0_MASK     0x7FFFFFF
#define IGU_CLEANUP_RESERVED0_SHIFT    0
/* cleanup clear - 0, set - 1 */
#define IGU_CLEANUP_CLEANUP_SET_MASK   0x1
#define IGU_CLEANUP_CLEANUP_SET_SHIFT  27
#define IGU_CLEANUP_CLEANUP_TYPE_MASK  0x7
#define IGU_CLEANUP_CLEANUP_TYPE_SHIFT 28
/* must always be set (use enum command_type_bit) */
#define IGU_CLEANUP_COMMAND_TYPE_MASK  0x1
#define IGU_CLEANUP_COMMAND_TYPE_SHIFT 31
	__le32 reserved1;
};


/*
 * IGU firmware driver command
 */
union igu_command {
	struct igu_prod_cons_update prod_cons_update;
	struct igu_cleanup cleanup;
};


/*
 * IGU firmware driver command
 */
struct igu_command_reg_ctrl {
	__le16 opaque_fid;
	__le16 igu_command_reg_ctrl_fields;
#define IGU_COMMAND_REG_CTRL_PXP_BAR_ADDR_MASK  0xFFF
#define IGU_COMMAND_REG_CTRL_PXP_BAR_ADDR_SHIFT 0
#define IGU_COMMAND_REG_CTRL_RESERVED_MASK      0x7
#define IGU_COMMAND_REG_CTRL_RESERVED_SHIFT     12
/* command typ: 0 - read, 1 - write */
#define IGU_COMMAND_REG_CTRL_COMMAND_TYPE_MASK  0x1
#define IGU_COMMAND_REG_CTRL_COMMAND_TYPE_SHIFT 15
};


/*
 * IGU mapping line structure
 */
struct igu_mapping_line {
	__le32 igu_mapping_line_fields;
#define IGU_MAPPING_LINE_VALID_MASK            0x1
#define IGU_MAPPING_LINE_VALID_SHIFT           0
#define IGU_MAPPING_LINE_VECTOR_NUMBER_MASK    0xFF
#define IGU_MAPPING_LINE_VECTOR_NUMBER_SHIFT   1
/* In BB: VF-0-120, PF-0-7; In K2: VF-0-191, PF-0-15 */
#define IGU_MAPPING_LINE_FUNCTION_NUMBER_MASK  0xFF
#define IGU_MAPPING_LINE_FUNCTION_NUMBER_SHIFT 9
#define IGU_MAPPING_LINE_PF_VALID_MASK         0x1 /* PF-1, VF-0 */
#define IGU_MAPPING_LINE_PF_VALID_SHIFT        17
#define IGU_MAPPING_LINE_IPS_GROUP_MASK        0x3F
#define IGU_MAPPING_LINE_IPS_GROUP_SHIFT       18
#define IGU_MAPPING_LINE_RESERVED_MASK         0xFF
#define IGU_MAPPING_LINE_RESERVED_SHIFT        24
};


/*
 * IGU MSIX line structure
 */
struct igu_msix_vector {
	struct regpair address;
	__le32 data;
	__le32 msix_vector_fields;
#define IGU_MSIX_VECTOR_MASK_BIT_MASK      0x1
#define IGU_MSIX_VECTOR_MASK_BIT_SHIFT     0
#define IGU_MSIX_VECTOR_RESERVED0_MASK     0x7FFF
#define IGU_MSIX_VECTOR_RESERVED0_SHIFT    1
#define IGU_MSIX_VECTOR_STEERING_TAG_MASK  0xFF
#define IGU_MSIX_VECTOR_STEERING_TAG_SHIFT 16
#define IGU_MSIX_VECTOR_RESERVED1_MASK     0xFF
#define IGU_MSIX_VECTOR_RESERVED1_SHIFT    24
};


/*
 * per encapsulation type enabling flags
 */
struct prs_reg_encapsulation_type_en {
	u8 flags;
/* Enable bit for Ethernet-over-GRE (L2 GRE) encapsulation. */
#define PRS_REG_ENCAPSULATION_TYPE_EN_ETH_OVER_GRE_ENABLE_MASK     0x1
#define PRS_REG_ENCAPSULATION_TYPE_EN_ETH_OVER_GRE_ENABLE_SHIFT    0
/* Enable bit for IP-over-GRE (IP GRE) encapsulation. */
#define PRS_REG_ENCAPSULATION_TYPE_EN_IP_OVER_GRE_ENABLE_MASK      0x1
#define PRS_REG_ENCAPSULATION_TYPE_EN_IP_OVER_GRE_ENABLE_SHIFT     1
/* Enable bit for VXLAN encapsulation. */
#define PRS_REG_ENCAPSULATION_TYPE_EN_VXLAN_ENABLE_MASK            0x1
#define PRS_REG_ENCAPSULATION_TYPE_EN_VXLAN_ENABLE_SHIFT           2
/* Enable bit for T-Tag encapsulation. */
#define PRS_REG_ENCAPSULATION_TYPE_EN_T_TAG_ENABLE_MASK            0x1
#define PRS_REG_ENCAPSULATION_TYPE_EN_T_TAG_ENABLE_SHIFT           3
/* Enable bit for Ethernet-over-GENEVE (L2 GENEVE) encapsulation. */
#define PRS_REG_ENCAPSULATION_TYPE_EN_ETH_OVER_GENEVE_ENABLE_MASK  0x1
#define PRS_REG_ENCAPSULATION_TYPE_EN_ETH_OVER_GENEVE_ENABLE_SHIFT 4
/* Enable bit for IP-over-GENEVE (IP GENEVE) encapsulation. */
#define PRS_REG_ENCAPSULATION_TYPE_EN_IP_OVER_GENEVE_ENABLE_MASK   0x1
#define PRS_REG_ENCAPSULATION_TYPE_EN_IP_OVER_GENEVE_ENABLE_SHIFT  5
#define PRS_REG_ENCAPSULATION_TYPE_EN_RESERVED_MASK                0x3
#define PRS_REG_ENCAPSULATION_TYPE_EN_RESERVED_SHIFT               6
};


enum pxp_tph_st_hint {
	TPH_ST_HINT_BIDIR /* Read/Write access by Host and Device */,
	TPH_ST_HINT_REQUESTER /* Read/Write access by Device */,
/* Device Write and Host Read, or Host Write and Device Read */
	TPH_ST_HINT_TARGET,
/* Device Write and Host Read, or Host Write and Device Read - with temporal
 * reuse
 */
	TPH_ST_HINT_TARGET_PRIO,
	MAX_PXP_TPH_ST_HINT
};


/*
 * QM hardware structure of enable bypass credit mask
 */
struct qm_rf_bypass_mask {
	u8 flags;
#define QM_RF_BYPASS_MASK_LINEVOQ_MASK    0x1
#define QM_RF_BYPASS_MASK_LINEVOQ_SHIFT   0
#define QM_RF_BYPASS_MASK_RESERVED0_MASK  0x1
#define QM_RF_BYPASS_MASK_RESERVED0_SHIFT 1
#define QM_RF_BYPASS_MASK_PFWFQ_MASK      0x1
#define QM_RF_BYPASS_MASK_PFWFQ_SHIFT     2
#define QM_RF_BYPASS_MASK_VPWFQ_MASK      0x1
#define QM_RF_BYPASS_MASK_VPWFQ_SHIFT     3
#define QM_RF_BYPASS_MASK_PFRL_MASK       0x1
#define QM_RF_BYPASS_MASK_PFRL_SHIFT      4
#define QM_RF_BYPASS_MASK_VPQCNRL_MASK    0x1
#define QM_RF_BYPASS_MASK_VPQCNRL_SHIFT   5
#define QM_RF_BYPASS_MASK_FWPAUSE_MASK    0x1
#define QM_RF_BYPASS_MASK_FWPAUSE_SHIFT   6
#define QM_RF_BYPASS_MASK_RESERVED1_MASK  0x1
#define QM_RF_BYPASS_MASK_RESERVED1_SHIFT 7
};


/*
 * QM hardware structure of opportunistic credit mask
 */
struct qm_rf_opportunistic_mask {
	__le16 flags;
#define QM_RF_OPPORTUNISTIC_MASK_LINEVOQ_MASK     0x1
#define QM_RF_OPPORTUNISTIC_MASK_LINEVOQ_SHIFT    0
#define QM_RF_OPPORTUNISTIC_MASK_BYTEVOQ_MASK     0x1
#define QM_RF_OPPORTUNISTIC_MASK_BYTEVOQ_SHIFT    1
#define QM_RF_OPPORTUNISTIC_MASK_PFWFQ_MASK       0x1
#define QM_RF_OPPORTUNISTIC_MASK_PFWFQ_SHIFT      2
#define QM_RF_OPPORTUNISTIC_MASK_VPWFQ_MASK       0x1
#define QM_RF_OPPORTUNISTIC_MASK_VPWFQ_SHIFT      3
#define QM_RF_OPPORTUNISTIC_MASK_PFRL_MASK        0x1
#define QM_RF_OPPORTUNISTIC_MASK_PFRL_SHIFT       4
#define QM_RF_OPPORTUNISTIC_MASK_VPQCNRL_MASK     0x1
#define QM_RF_OPPORTUNISTIC_MASK_VPQCNRL_SHIFT    5
#define QM_RF_OPPORTUNISTIC_MASK_FWPAUSE_MASK     0x1
#define QM_RF_OPPORTUNISTIC_MASK_FWPAUSE_SHIFT    6
#define QM_RF_OPPORTUNISTIC_MASK_RESERVED0_MASK   0x1
#define QM_RF_OPPORTUNISTIC_MASK_RESERVED0_SHIFT  7
#define QM_RF_OPPORTUNISTIC_MASK_QUEUEEMPTY_MASK  0x1
#define QM_RF_OPPORTUNISTIC_MASK_QUEUEEMPTY_SHIFT 8
#define QM_RF_OPPORTUNISTIC_MASK_RESERVED1_MASK   0x7F
#define QM_RF_OPPORTUNISTIC_MASK_RESERVED1_SHIFT  9
};


/*
 * QM hardware structure of QM map memory
 */
struct qm_rf_pq_map {
	__le32 reg;
#define QM_RF_PQ_MAP_PQ_VALID_MASK          0x1 /* PQ active */
#define QM_RF_PQ_MAP_PQ_VALID_SHIFT         0
#define QM_RF_PQ_MAP_RL_ID_MASK             0xFF /* RL ID */
#define QM_RF_PQ_MAP_RL_ID_SHIFT            1
/* the first PQ associated with the VPORT and VOQ of this PQ */
#define QM_RF_PQ_MAP_VP_PQ_ID_MASK          0x1FF
#define QM_RF_PQ_MAP_VP_PQ_ID_SHIFT         9
#define QM_RF_PQ_MAP_VOQ_MASK               0x1F /* VOQ */
#define QM_RF_PQ_MAP_VOQ_SHIFT              18
#define QM_RF_PQ_MAP_WRR_WEIGHT_GROUP_MASK  0x3 /* WRR weight */
#define QM_RF_PQ_MAP_WRR_WEIGHT_GROUP_SHIFT 23
#define QM_RF_PQ_MAP_RL_VALID_MASK          0x1 /* RL active */
#define QM_RF_PQ_MAP_RL_VALID_SHIFT         25
#define QM_RF_PQ_MAP_RESERVED_MASK          0x3F
#define QM_RF_PQ_MAP_RESERVED_SHIFT         26
};


/*
 * Completion params for aggregated interrupt completion
 */
struct sdm_agg_int_comp_params {
	__le16 params;
/* the number of aggregated interrupt, 0-31 */
#define SDM_AGG_INT_COMP_PARAMS_AGG_INT_INDEX_MASK      0x3F
#define SDM_AGG_INT_COMP_PARAMS_AGG_INT_INDEX_SHIFT     0
/* 1 - set a bit in aggregated vector, 0 - dont set */
#define SDM_AGG_INT_COMP_PARAMS_AGG_VECTOR_ENABLE_MASK  0x1
#define SDM_AGG_INT_COMP_PARAMS_AGG_VECTOR_ENABLE_SHIFT 6
/* Number of bit in the aggregated vector, 0-279 (TBD) */
#define SDM_AGG_INT_COMP_PARAMS_AGG_VECTOR_BIT_MASK     0x1FF
#define SDM_AGG_INT_COMP_PARAMS_AGG_VECTOR_BIT_SHIFT    7
};


/*
 * SDM operation gen command (generate aggregative interrupt)
 */
struct sdm_op_gen {
	__le32 command;
/* completion parameters 0-15 */
#define SDM_OP_GEN_COMP_PARAM_MASK  0xFFFF
#define SDM_OP_GEN_COMP_PARAM_SHIFT 0
#define SDM_OP_GEN_COMP_TYPE_MASK   0xF /* completion type 16-19 */
#define SDM_OP_GEN_COMP_TYPE_SHIFT  16
#define SDM_OP_GEN_RESERVED_MASK    0xFFF /* reserved 20-31 */
#define SDM_OP_GEN_RESERVED_SHIFT   20
};

struct ystorm_core_conn_ag_ctx {
	u8 byte0 /* cdu_validation */;
	u8 byte1 /* state */;
	u8 flags0;
#define YSTORM_CORE_CONN_AG_CTX_BIT0_MASK     0x1 /* exist_in_qm0 */
#define YSTORM_CORE_CONN_AG_CTX_BIT0_SHIFT    0
#define YSTORM_CORE_CONN_AG_CTX_BIT1_MASK     0x1 /* exist_in_qm1 */
#define YSTORM_CORE_CONN_AG_CTX_BIT1_SHIFT    1
#define YSTORM_CORE_CONN_AG_CTX_CF0_MASK      0x3 /* cf0 */
#define YSTORM_CORE_CONN_AG_CTX_CF0_SHIFT     2
#define YSTORM_CORE_CONN_AG_CTX_CF1_MASK      0x3 /* cf1 */
#define YSTORM_CORE_CONN_AG_CTX_CF1_SHIFT     4
#define YSTORM_CORE_CONN_AG_CTX_CF2_MASK      0x3 /* cf2 */
#define YSTORM_CORE_CONN_AG_CTX_CF2_SHIFT     6
	u8 flags1;
#define YSTORM_CORE_CONN_AG_CTX_CF0EN_MASK    0x1 /* cf0en */
#define YSTORM_CORE_CONN_AG_CTX_CF0EN_SHIFT   0
#define YSTORM_CORE_CONN_AG_CTX_CF1EN_MASK    0x1 /* cf1en */
#define YSTORM_CORE_CONN_AG_CTX_CF1EN_SHIFT   1
#define YSTORM_CORE_CONN_AG_CTX_CF2EN_MASK    0x1 /* cf2en */
#define YSTORM_CORE_CONN_AG_CTX_CF2EN_SHIFT   2
#define YSTORM_CORE_CONN_AG_CTX_RULE0EN_MASK  0x1 /* rule0en */
#define YSTORM_CORE_CONN_AG_CTX_RULE0EN_SHIFT 3
#define YSTORM_CORE_CONN_AG_CTX_RULE1EN_MASK  0x1 /* rule1en */
#define YSTORM_CORE_CONN_AG_CTX_RULE1EN_SHIFT 4
#define YSTORM_CORE_CONN_AG_CTX_RULE2EN_MASK  0x1 /* rule2en */
#define YSTORM_CORE_CONN_AG_CTX_RULE2EN_SHIFT 5
#define YSTORM_CORE_CONN_AG_CTX_RULE3EN_MASK  0x1 /* rule3en */
#define YSTORM_CORE_CONN_AG_CTX_RULE3EN_SHIFT 6
#define YSTORM_CORE_CONN_AG_CTX_RULE4EN_MASK  0x1 /* rule4en */
#define YSTORM_CORE_CONN_AG_CTX_RULE4EN_SHIFT 7
	u8 byte2 /* byte2 */;
	u8 byte3 /* byte3 */;
	__le16 word0 /* word0 */;
	__le32 reg0 /* reg0 */;
	__le32 reg1 /* reg1 */;
	__le16 word1 /* word1 */;
	__le16 word2 /* word2 */;
	__le16 word3 /* word3 */;
	__le16 word4 /* word4 */;
	__le32 reg2 /* reg2 */;
	__le32 reg3 /* reg3 */;
};

#endif /* __ECORE_HSI_COMMON__ */
