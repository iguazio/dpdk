/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020 Mellanox Technologies, Ltd
 */

#ifndef _MLX5_RXP_CSRS_H_
#define _MLX5_RXP_CSRS_H_

/*
 * Common to all RXP implementations
 */
#define MLX5_RXP_CSR_BASE_ADDRESS 0x0000ul
#define MLX5_RXP_RTRU_CSR_BASE_ADDRESS 0x0100ul
#define MLX5_RXP_STATS_CSR_BASE_ADDRESS	0x0200ul
#define MLX5_RXP_ROYALTY_CSR_BASE_ADDRESS 0x0600ul

#define MLX5_RXP_CSR_WIDTH 4

/* This is the identifier we expect to see in the first RXP CSR */
#define MLX5_RXP_IDENTIFIER 0x5254

/* Hyperion specific BAR0 offsets */
#define MLX5_RXP_FPGA_BASE_ADDRESS 0x0000ul
#define MLX5_RXP_PCIE_BASE_ADDRESS 0x1000ul
#define MLX5_RXP_IDMA_BASE_ADDRESS 0x2000ul
#define MLX5_RXP_EDMA_BASE_ADDRESS 0x3000ul
#define MLX5_RXP_SYSMON_BASE_ADDRESS 0xf300ul
#define MLX5_RXP_ISP_CSR_BASE_ADDRESS 0xf400ul

/* Offset to the RXP common 4K CSR space */
#define MLX5_RXP_PCIE_CSR_BASE_ADDRESS 0xf000ul

/* FPGA CSRs */

#define MLX5_RXP_FPGA_VERSION (MLX5_RXP_FPGA_BASE_ADDRESS + \
			       MLX5_RXP_CSR_WIDTH * 0)

/* PCIe CSRs */
#define MLX5_RXP_PCIE_INIT_ISR (MLX5_RXP_PCIE_BASE_ADDRESS + \
				MLX5_RXP_CSR_WIDTH * 0)
#define MLX5_RXP_PCIE_INIT_IMR (MLX5_RXP_PCIE_BASE_ADDRESS + \
				MLX5_RXP_CSR_WIDTH * 1)
#define MLX5_RXP_PCIE_INIT_CFG_STAT (MLX5_RXP_PCIE_BASE_ADDRESS + \
				     MLX5_RXP_CSR_WIDTH * 2)
#define MLX5_RXP_PCIE_INIT_FLR (MLX5_RXP_PCIE_BASE_ADDRESS + \
				MLX5_RXP_CSR_WIDTH * 3)
#define MLX5_RXP_PCIE_INIT_CTRL	(MLX5_RXP_PCIE_BASE_ADDRESS + \
				 MLX5_RXP_CSR_WIDTH * 4)

/* IDMA CSRs */
#define MLX5_RXP_IDMA_ISR (MLX5_RXP_IDMA_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 0)
#define MLX5_RXP_IDMA_IMR (MLX5_RXP_IDMA_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 1)
#define MLX5_RXP_IDMA_CSR (MLX5_RXP_IDMA_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 4)
#define MLX5_RXP_IDMA_CSR_RST_MSK 0x0001
#define MLX5_RXP_IDMA_CSR_PDONE_MSK 0x0002
#define MLX5_RXP_IDMA_CSR_INIT_MSK 0x0004
#define MLX5_RXP_IDMA_CSR_EN_MSK 0x0008
#define MLX5_RXP_IDMA_QCR (MLX5_RXP_IDMA_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 5)
#define MLX5_RXP_IDMA_QCR_QAVAIL_MSK 0x00FF
#define MLX5_RXP_IDMA_QCR_QEN_MSK 0xFF00
#define MLX5_RXP_IDMA_DCR (MLX5_RXP_IDMA_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 6)
#define MLX5_RXP_IDMA_DWCTR (MLX5_RXP_IDMA_BASE_ADDRESS + \
			     MLX5_RXP_CSR_WIDTH * 7)
#define MLX5_RXP_IDMA_DWTOR (MLX5_RXP_IDMA_BASE_ADDRESS + \
			     MLX5_RXP_CSR_WIDTH * 8)
#define MLX5_RXP_IDMA_PADCR (MLX5_RXP_IDMA_BASE_ADDRESS + \
			     MLX5_RXP_CSR_WIDTH * 9)
#define MLX5_RXP_IDMA_DFCR (MLX5_RXP_IDMA_BASE_ADDRESS + \
			    MLX5_RXP_CSR_WIDTH * 10)
#define MLX5_RXP_IDMA_FOFLR0 (MLX5_RXP_IDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 16)
#define MLX5_RXP_IDMA_FOFLR1 (MLX5_RXP_IDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 17)
#define MLX5_RXP_IDMA_FOFLR2 (MLX5_RXP_IDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 18)
#define MLX5_RXP_IDMA_FUFLR0 (MLX5_RXP_IDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 24)
#define MLX5_RXP_IDMA_FUFLR1 (MLX5_RXP_IDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 25)
#define MLX5_RXP_IDMA_FUFLR2 (MLX5_RXP_IDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 26)

#define MLX5_RXP_IDMA_QCSR_BASE	(MLX5_RXP_IDMA_BASE_ADDRESS + \
				 MLX5_RXP_CSR_WIDTH * 128)
#define MLX5_RXP_IDMA_QCSR_RST_MSK 0x0001
#define MLX5_RXP_IDMA_QCSR_PDONE_MSK 0x0002
#define MLX5_RXP_IDMA_QCSR_INIT_MSK 0x0004
#define MLX5_RXP_IDMA_QCSR_EN_MSK 0x0008
#define MLX5_RXP_IDMA_QDPTR_BASE (MLX5_RXP_IDMA_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 192)
#define MLX5_RXP_IDMA_QTPTR_BASE (MLX5_RXP_IDMA_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 256)
#define MLX5_RXP_IDMA_QDRPTR_BASE (MLX5_RXP_IDMA_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 320)
#define MLX5_RXP_IDMA_QDRALR_BASE (MLX5_RXP_IDMA_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 384)
#define MLX5_RXP_IDMA_QDRAHR_BASE (MLX5_RXP_IDMA_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 385)

/* EDMA CSRs */
#define MLX5_RXP_EDMA_ISR (MLX5_RXP_EDMA_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 0)
#define MLX5_RXP_EDMA_IMR (MLX5_RXP_EDMA_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 1)
#define MLX5_RXP_EDMA_CSR (MLX5_RXP_EDMA_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 4)
#define MLX5_RXP_EDMA_CSR_RST_MSK 0x0001
#define MLX5_RXP_EDMA_CSR_PDONE_MSK 0x0002
#define MLX5_RXP_EDMA_CSR_INIT_MSK 0x0004
#define MLX5_RXP_EDMA_CSR_EN_MSK 0x0008
#define MLX5_RXP_EDMA_QCR (MLX5_RXP_EDMA_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 5)
#define MLX5_RXP_EDMA_QCR_QAVAIL_MSK 0x00FF
#define MLX5_RXP_EDMA_QCR_QEN_MSK 0xFF00
#define MLX5_RXP_EDMA_DCR (MLX5_RXP_EDMA_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 6)
#define MLX5_RXP_EDMA_DWCTR (MLX5_RXP_EDMA_BASE_ADDRESS + \
			     MLX5_RXP_CSR_WIDTH * 7)
#define MLX5_RXP_EDMA_DWTOR (MLX5_RXP_EDMA_BASE_ADDRESS + \
			     MLX5_RXP_CSR_WIDTH * 8)
#define MLX5_RXP_EDMA_DFCR (MLX5_RXP_EDMA_BASE_ADDRESS + \
			    MLX5_RXP_CSR_WIDTH * 10)
#define MLX5_RXP_EDMA_FOFLR0 (MLX5_RXP_EDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 16)
#define MLX5_RXP_EDMA_FOFLR1 (MLX5_RXP_EDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 17)
#define MLX5_RXP_EDMA_FOFLR2 (MLX5_RXP_EDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 18)
#define MLX5_RXP_EDMA_FUFLR0 (MLX5_RXP_EDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 24)
#define MLX5_RXP_EDMA_FUFLR1 (MLX5_RXP_EDMA_BASE_ADDRESS +\
			      MLX5_RXP_CSR_WIDTH * 25)
#define MLX5_RXP_EDMA_FUFLR2 (MLX5_RXP_EDMA_BASE_ADDRESS + \
			      MLX5_RXP_CSR_WIDTH * 26)

#define MLX5_RXP_EDMA_QCSR_BASE	(MLX5_RXP_EDMA_BASE_ADDRESS + \
				 MLX5_RXP_CSR_WIDTH * 128)
#define MLX5_RXP_EDMA_QCSR_RST_MSK 0x0001
#define MLX5_RXP_EDMA_QCSR_PDONE_MSK 0x0002
#define MLX5_RXP_EDMA_QCSR_INIT_MSK 0x0004
#define MLX5_RXP_EDMA_QCSR_EN_MSK 0x0008
#define MLX5_RXP_EDMA_QTPTR_BASE (MLX5_RXP_EDMA_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 256)
#define MLX5_RXP_EDMA_QDRPTR_BASE (MLX5_RXP_EDMA_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 320)
#define MLX5_RXP_EDMA_QDRALR_BASE (MLX5_RXP_EDMA_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 384)
#define MLX5_RXP_EDMA_QDRAHR_BASE (MLX5_RXP_EDMA_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 385)

/* Main CSRs */
#define MLX5_RXP_CSR_IDENTIFIER	(MLX5_RXP_CSR_BASE_ADDRESS + \
				 MLX5_RXP_CSR_WIDTH * 0)
#define MLX5_RXP_CSR_REVISION (MLX5_RXP_CSR_BASE_ADDRESS + \
			       MLX5_RXP_CSR_WIDTH * 1)
#define MLX5_RXP_CSR_CAPABILITY_0 (MLX5_RXP_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 2)
#define MLX5_RXP_CSR_CAPABILITY_1 (MLX5_RXP_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 3)
#define MLX5_RXP_CSR_CAPABILITY_2 (MLX5_RXP_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 4)
#define MLX5_RXP_CSR_CAPABILITY_3 (MLX5_RXP_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 5)
#define MLX5_RXP_CSR_CAPABILITY_4 (MLX5_RXP_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 6)
#define MLX5_RXP_CSR_CAPABILITY_5 (MLX5_RXP_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 7)
#define MLX5_RXP_CSR_CAPABILITY_6 (MLX5_RXP_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 8)
#define MLX5_RXP_CSR_CAPABILITY_7 (MLX5_RXP_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 9)
#define MLX5_RXP_CSR_STATUS (MLX5_RXP_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 10)
#define MLX5_RXP_CSR_STATUS_INIT_DONE 0x0001
#define MLX5_RXP_CSR_STATUS_GOING 0x0008
#define MLX5_RXP_CSR_STATUS_IDLE 0x0040
#define MLX5_RXP_CSR_STATUS_TRACKER_OK 0x0080
#define MLX5_RXP_CSR_STATUS_TRIAL_TIMEOUT 0x0100
#define MLX5_RXP_CSR_FIFO_STATUS_0 (MLX5_RXP_CSR_BASE_ADDRESS + \
				    MLX5_RXP_CSR_WIDTH * 11)
#define MLX5_RXP_CSR_FIFO_STATUS_1 (MLX5_RXP_CSR_BASE_ADDRESS + \
				    MLX5_RXP_CSR_WIDTH * 12)
#define MLX5_RXP_CSR_JOB_DDOS_COUNT (MLX5_RXP_CSR_BASE_ADDRESS + \
				     MLX5_RXP_CSR_WIDTH * 13)
/* 14 + 15 reserved */
#define MLX5_RXP_CSR_CORE_CLK_COUNT (MLX5_RXP_CSR_BASE_ADDRESS + \
				     MLX5_RXP_CSR_WIDTH * 16)
#define MLX5_RXP_CSR_WRITE_COUNT (MLX5_RXP_CSR_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 17)
#define MLX5_RXP_CSR_JOB_COUNT (MLX5_RXP_CSR_BASE_ADDRESS + \
				MLX5_RXP_CSR_WIDTH * 18)
#define MLX5_RXP_CSR_JOB_ERROR_COUNT (MLX5_RXP_CSR_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 19)
#define MLX5_RXP_CSR_JOB_BYTE_COUNT0 (MLX5_RXP_CSR_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 20)
#define MLX5_RXP_CSR_JOB_BYTE_COUNT1 (MLX5_RXP_CSR_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 21)
#define MLX5_RXP_CSR_RESPONSE_COUNT (MLX5_RXP_CSR_BASE_ADDRESS + \
				     MLX5_RXP_CSR_WIDTH * 22)
#define MLX5_RXP_CSR_MATCH_COUNT (MLX5_RXP_CSR_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 23)
#define MLX5_RXP_CSR_CTRL (MLX5_RXP_CSR_BASE_ADDRESS + MLX5_RXP_CSR_WIDTH * 24)
#define MLX5_RXP_CSR_CTRL_INIT 0x0001
#define MLX5_RXP_CSR_CTRL_GO 0x0008
#define MLX5_RXP_CSR_MAX_MATCH (MLX5_RXP_CSR_BASE_ADDRESS + \
				MLX5_RXP_CSR_WIDTH * 25)
#define MLX5_RXP_CSR_MAX_PREFIX	(MLX5_RXP_CSR_BASE_ADDRESS + \
				 MLX5_RXP_CSR_WIDTH * 26)
#define MLX5_RXP_CSR_MAX_PRI_THREAD (MLX5_RXP_CSR_BASE_ADDRESS + \
				     MLX5_RXP_CSR_WIDTH * 27)
#define MLX5_RXP_CSR_MAX_LATENCY (MLX5_RXP_CSR_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 28)
#define MLX5_RXP_CSR_SCRATCH_1 (MLX5_RXP_CSR_BASE_ADDRESS + \
				MLX5_RXP_CSR_WIDTH * 29)
#define MLX5_RXP_CSR_CLUSTER_MASK (MLX5_RXP_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 30)
#define MLX5_RXP_CSR_INTRA_CLUSTER_MASK (MLX5_RXP_CSR_BASE_ADDRESS + \
					 MLX5_RXP_CSR_WIDTH * 31)

/* Runtime Rule Update CSRs */
/* 0 + 1 reserved */
#define MLX5_RXP_RTRU_CSR_CAPABILITY (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 2)
/* 3-9 reserved */
#define MLX5_RXP_RTRU_CSR_STATUS (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 10)
#define MLX5_RXP_RTRU_CSR_STATUS_UPDATE_DONE 0x0002
#define MLX5_RXP_RTRU_CSR_STATUS_IM_INIT_DONE 0x0010
#define MLX5_RXP_RTRU_CSR_STATUS_L1C_INIT_DONE 0x0020
#define MLX5_RXP_RTRU_CSR_STATUS_L2C_INIT_DONE 0x0040
#define MLX5_RXP_RTRU_CSR_STATUS_EM_INIT_DONE 0x0080
#define MLX5_RXP_RTRU_CSR_FIFO_STAT (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				     MLX5_RXP_CSR_WIDTH * 11)
/* 12-15 reserved */
#define MLX5_RXP_RTRU_CSR_CHECKSUM_0 (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 16)
#define MLX5_RXP_RTRU_CSR_CHECKSUM_1 (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 17)
#define MLX5_RXP_RTRU_CSR_CHECKSUM_2 (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 18)
/* 19 + 20 reserved */
#define MLX5_RXP_RTRU_CSR_RTRU_COUNT (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 21)
#define MLX5_RXP_RTRU_CSR_ROF_REV (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				   MLX5_RXP_CSR_WIDTH * 22)
/* 23 reserved */
#define MLX5_RXP_RTRU_CSR_CTRL (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				MLX5_RXP_CSR_WIDTH * 24)
#define MLX5_RXP_RTRU_CSR_CTRL_INIT 0x0001
#define MLX5_RXP_RTRU_CSR_CTRL_GO 0x0002
#define MLX5_RXP_RTRU_CSR_CTRL_SIP 0x0004
#define MLX5_RXP_RTRU_CSR_CTRL_INIT_MODE_MASK (3 << 4)
#define MLX5_RXP_RTRU_CSR_CTRL_INIT_MODE_IM_L1_L2_EM (0 << 4)
#define MLX5_RXP_RTRU_CSR_CTRL_INIT_MODE_IM_L1_L2 (1 << 4)
#define MLX5_RXP_RTRU_CSR_CTRL_INIT_MODE_L1_L2 (2 << 4)
#define MLX5_RXP_RTRU_CSR_CTRL_INIT_MODE_EM (3 << 4)
#define MLX5_RXP_RTRU_CSR_ADDR (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				MLX5_RXP_CSR_WIDTH * 25)
#define MLX5_RXP_RTRU_CSR_DATA_0 (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 26)
#define MLX5_RXP_RTRU_CSR_DATA_1 (MLX5_RXP_RTRU_CSR_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 27)
/* 28-31 reserved */

/* Statistics CSRs */
#define MLX5_RXP_STATS_CSR_CLUSTER (MLX5_RXP_STATS_CSR_BASE_ADDRESS + \
				    MLX5_RXP_CSR_WIDTH * 0)
#define MLX5_RXP_STATS_CSR_L2_CACHE (MLX5_RXP_STATS_CSR_BASE_ADDRESS + \
				     MLX5_RXP_CSR_WIDTH * 24)
#define MLX5_RXP_STATS_CSR_MPFE_FIFO (MLX5_RXP_STATS_CSR_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 25)
#define MLX5_RXP_STATS_CSR_PE (MLX5_RXP_STATS_CSR_BASE_ADDRESS + \
			       MLX5_RXP_CSR_WIDTH * 28)
#define MLX5_RXP_STATS_CSR_CP (MLX5_RXP_STATS_CSR_BASE_ADDRESS + \
			       MLX5_RXP_CSR_WIDTH * 30)
#define MLX5_RXP_STATS_CSR_DP (MLX5_RXP_STATS_CSR_BASE_ADDRESS + \
			       MLX5_RXP_CSR_WIDTH * 31)

/* Sysmon Stats CSRs */
#define MLX5_RXP_SYSMON_CSR_T_FPGA (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				    MLX5_RXP_CSR_WIDTH * 0)
#define MLX5_RXP_SYSMON_CSR_V_VCCINT (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 1)
#define MLX5_RXP_SYSMON_CSR_V_VCCAUX (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 2)
#define MLX5_RXP_SYSMON_CSR_T_U1 (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 20)
#define MLX5_RXP_SYSMON_CSR_I_EDG12V (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 21)
#define MLX5_RXP_SYSMON_CSR_I_VCC3V3 (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 22)
#define MLX5_RXP_SYSMON_CSR_I_VCC2V5 (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 23)
#define MLX5_RXP_SYSMON_CSR_T_U2 (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				  MLX5_RXP_CSR_WIDTH * 28)
#define MLX5_RXP_SYSMON_CSR_I_AUX12V (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 29)
#define MLX5_RXP_SYSMON_CSR_I_VCC1V8 (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				      MLX5_RXP_CSR_WIDTH * 30)
#define MLX5_RXP_SYSMON_CSR_I_VDDR3 (MLX5_RXP_SYSMON_BASE_ADDRESS + \
				     MLX5_RXP_CSR_WIDTH * 31)

/* In Service Programming CSRs */

/* RXP-F1 and RXP-ZYNQ specific CSRs */
#define MLX5_RXP_MQ_CP_BASE (0x0500ul)
#define MLX5_RXP_MQ_CP_CAPABILITY_BASE (MLX5_RXP_MQ_CP_BASE + \
					2 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXP_MQ_CP_CAPABILITY_0 (MLX5_RXP_MQ_CP_CAPABILITY_BASE + \
				     0 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXP_MQ_CP_CAPABILITY_1 (MLX5_RXP_MQ_CP_CAPABILITY_BASE + \
				     1 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXP_MQ_CP_CAPABILITY_2 (MLX5_RXP_MQ_CP_CAPABILITY_BASE + \
				     2 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXP_MQ_CP_CAPABILITY_3 (MLX5_RXP_MQ_CP_CAPABILITY_BASE + \
				     3 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXP_MQ_CP_FIFO_STATUS_BASE (MLX5_RXP_MQ_CP_BASE + \
					 11 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXP_MQ_CP_FIFO_STATUS_C0 (MLX5_RXP_MQ_CP_FIFO_STATUS_BASE + \
				       0 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXP_MQ_CP_FIFO_STATUS_C1 (MLX5_RXP_MQ_CP_FIFO_STATUS_BASE + \
				       1 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXP_MQ_CP_FIFO_STATUS_C2 (MLX5_RXP_MQ_CP_FIFO_STATUS_BASE + \
				       2 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXP_MQ_CP_FIFO_STATUS_C3 (MLX5_RXP_MQ_CP_FIFO_STATUS_BASE + \
				       3 * MLX5_RXP_CSR_WIDTH)

/* Royalty tracker / licensing related CSRs */
#define MLX5_RXPL__CSR_IDENT (MLX5_RXP_ROYALTY_CSR_BASE_ADDRESS + \
			      0 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXPL__IDENTIFIER 0x4c505852 /* MLX5_RXPL_ */
#define MLX5_RXPL__CSR_CAPABILITY (MLX5_RXP_ROYALTY_CSR_BASE_ADDRESS + \
				   2 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXPL__TYPE_MASK 0xFF
#define MLX5_RXPL__TYPE_NONE 0
#define MLX5_RXPL__TYPE_MAXIM 1
#define MLX5_RXPL__TYPE_XILINX_DNA 2
#define MLX5_RXPL__CSR_STATUS (MLX5_RXP_ROYALTY_CSR_BASE_ADDRESS + \
			       10 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXPL__CSR_IDENT_0 (MLX5_RXP_ROYALTY_CSR_BASE_ADDRESS + \
				16 * MLX5_RXP_CSR_WIDTH)
#define MLX5_RXPL__CSR_KEY_0 (MLX5_RXP_ROYALTY_CSR_BASE_ADDRESS + \
			      24 * MLX5_RXP_CSR_WIDTH)

#endif /* _MLX5_RXP_CSRS_H_ */
