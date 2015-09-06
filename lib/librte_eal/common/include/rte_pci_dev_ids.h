/*-
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 *   redistributing this file, you may do so under either license.
 *
 *   GPL LICENSE SUMMARY
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution
 *   in the file called LICENSE.GPL.
 *
 *   Contact Information:
 *   Intel Corporation
 *
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
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
 *
 */

/**
 * @file
 *
 * This file contains a list of the PCI device IDs recognised by DPDK, which
 * can be used to fill out an array of structures describing the devices.
 *
 * Currently four families of devices are recognised: those supported by the
 * IGB driver, by EM driver, those supported by the IXGBE driver, and by virtio
 * driver which is a para virtualization driver running in guest virtual machine.
 * The inclusion of these in an array built using this file depends on the
 * definition of
 * RTE_PCI_DEV_ID_DECL_EM
 * RTE_PCI_DEV_ID_DECL_IGB
 * RTE_PCI_DEV_ID_DECL_IGBVF
 * RTE_PCI_DEV_ID_DECL_IXGBE
 * RTE_PCI_DEV_ID_DECL_IXGBEVF
 * RTE_PCI_DEV_ID_DECL_I40E
 * RTE_PCI_DEV_ID_DECL_I40EVF
 * RTE_PCI_DEV_ID_DECL_VIRTIO
 * at the time when this file is included.
 *
 * In order to populate an array, the user of this file must define this macro:
 * RTE_PCI_DEV_ID_DECL_IXGBE(vendorID, deviceID). For example:
 *
 * @code
 * struct device {
 *     int vend;
 *     int dev;
 * };
 *
 * struct device devices[] = {
 * #define RTE_PCI_DEV_ID_DECL_IXGBE(vendorID, deviceID) {vend, dev},
 * #include <rte_pci_dev_ids.h>
 * };
 * @endcode
 *
 * Note that this file can be included multiple times within the same file.
 */

#ifndef RTE_PCI_DEV_ID_DECL_EM
#define RTE_PCI_DEV_ID_DECL_EM(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_IGB
#define RTE_PCI_DEV_ID_DECL_IGB(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_IGBVF
#define RTE_PCI_DEV_ID_DECL_IGBVF(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_IXGBE
#define RTE_PCI_DEV_ID_DECL_IXGBE(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_IXGBEVF
#define RTE_PCI_DEV_ID_DECL_IXGBEVF(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_I40E
#define RTE_PCI_DEV_ID_DECL_I40E(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_I40EVF
#define RTE_PCI_DEV_ID_DECL_I40EVF(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_VIRTIO
#define RTE_PCI_DEV_ID_DECL_VIRTIO(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_VMXNET3
#define RTE_PCI_DEV_ID_DECL_VMXNET3(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_FM10K
#define RTE_PCI_DEV_ID_DECL_FM10K(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_FM10KVF
#define RTE_PCI_DEV_ID_DECL_FM10KVF(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_ENIC
#define RTE_PCI_DEV_ID_DECL_ENIC(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_BNX2X
#define RTE_PCI_DEV_ID_DECL_BNX2X(vend, dev)
#endif

#ifndef RTE_PCI_DEV_ID_DECL_BNX2XVF
#define RTE_PCI_DEV_ID_DECL_BNX2XVF(vend, dev)
#endif

#ifndef PCI_VENDOR_ID_INTEL
/** Vendor ID used by Intel devices */
#define PCI_VENDOR_ID_INTEL 0x8086
#endif

#ifndef PCI_VENDOR_ID_QUMRANET
/** Vendor ID used by virtio devices */
#define PCI_VENDOR_ID_QUMRANET 0x1AF4
#endif

#ifndef PCI_VENDOR_ID_VMWARE
/** Vendor ID used by VMware devices */
#define PCI_VENDOR_ID_VMWARE 0x15AD
#endif

#ifndef PCI_VENDOR_ID_CISCO
/** Vendor ID used by Cisco VIC devices */
#define PCI_VENDOR_ID_CISCO 0x1137
#endif

#ifndef PCI_VENDOR_ID_BROADCOM
/** Vendor ID used by Broadcom devices */
#define PCI_VENDOR_ID_BROADCOM 0x14E4
#endif

/******************** Physical EM devices from e1000_hw.h ********************/

#define E1000_DEV_ID_82542                    0x1000
#define E1000_DEV_ID_82543GC_FIBER            0x1001
#define E1000_DEV_ID_82543GC_COPPER           0x1004
#define E1000_DEV_ID_82544EI_COPPER           0x1008
#define E1000_DEV_ID_82544EI_FIBER            0x1009
#define E1000_DEV_ID_82544GC_COPPER           0x100C
#define E1000_DEV_ID_82544GC_LOM              0x100D
#define E1000_DEV_ID_82540EM                  0x100E
#define E1000_DEV_ID_82540EM_LOM              0x1015
#define E1000_DEV_ID_82540EP_LOM              0x1016
#define E1000_DEV_ID_82540EP                  0x1017
#define E1000_DEV_ID_82540EP_LP               0x101E
#define E1000_DEV_ID_82545EM_COPPER           0x100F
#define E1000_DEV_ID_82545EM_FIBER            0x1011
#define E1000_DEV_ID_82545GM_COPPER           0x1026
#define E1000_DEV_ID_82545GM_FIBER            0x1027
#define E1000_DEV_ID_82545GM_SERDES           0x1028
#define E1000_DEV_ID_82546EB_COPPER           0x1010
#define E1000_DEV_ID_82546EB_FIBER            0x1012
#define E1000_DEV_ID_82546EB_QUAD_COPPER      0x101D
#define E1000_DEV_ID_82546GB_COPPER           0x1079
#define E1000_DEV_ID_82546GB_FIBER            0x107A
#define E1000_DEV_ID_82546GB_SERDES           0x107B
#define E1000_DEV_ID_82546GB_PCIE             0x108A
#define E1000_DEV_ID_82546GB_QUAD_COPPER      0x1099
#define E1000_DEV_ID_82546GB_QUAD_COPPER_KSP3 0x10B5
#define E1000_DEV_ID_82541EI                  0x1013
#define E1000_DEV_ID_82541EI_MOBILE           0x1018
#define E1000_DEV_ID_82541ER_LOM              0x1014
#define E1000_DEV_ID_82541ER                  0x1078
#define E1000_DEV_ID_82541GI                  0x1076
#define E1000_DEV_ID_82541GI_LF               0x107C
#define E1000_DEV_ID_82541GI_MOBILE           0x1077
#define E1000_DEV_ID_82547EI                  0x1019
#define E1000_DEV_ID_82547EI_MOBILE           0x101A
#define E1000_DEV_ID_82547GI                  0x1075
#define E1000_DEV_ID_82571EB_COPPER           0x105E
#define E1000_DEV_ID_82571EB_FIBER            0x105F
#define E1000_DEV_ID_82571EB_SERDES           0x1060
#define E1000_DEV_ID_82571EB_SERDES_DUAL      0x10D9
#define E1000_DEV_ID_82571EB_SERDES_QUAD      0x10DA
#define E1000_DEV_ID_82571EB_QUAD_COPPER      0x10A4
#define E1000_DEV_ID_82571PT_QUAD_COPPER      0x10D5
#define E1000_DEV_ID_82571EB_QUAD_FIBER       0x10A5
#define E1000_DEV_ID_82571EB_QUAD_COPPER_LP   0x10BC
#define E1000_DEV_ID_82572EI_COPPER           0x107D
#define E1000_DEV_ID_82572EI_FIBER            0x107E
#define E1000_DEV_ID_82572EI_SERDES           0x107F
#define E1000_DEV_ID_82572EI                  0x10B9
#define E1000_DEV_ID_82573E                   0x108B
#define E1000_DEV_ID_82573E_IAMT              0x108C
#define E1000_DEV_ID_82573L                   0x109A
#define E1000_DEV_ID_82574L                   0x10D3
#define E1000_DEV_ID_82574LA                  0x10F6
#define E1000_DEV_ID_82583V                   0x150C
#define E1000_DEV_ID_80003ES2LAN_COPPER_DPT   0x1096
#define E1000_DEV_ID_80003ES2LAN_SERDES_DPT   0x1098
#define E1000_DEV_ID_80003ES2LAN_COPPER_SPT   0x10BA
#define E1000_DEV_ID_80003ES2LAN_SERDES_SPT   0x10BB
#define E1000_DEV_ID_ICH8_82567V_3            0x1501
#define E1000_DEV_ID_ICH8_IGP_M_AMT           0x1049
#define E1000_DEV_ID_ICH8_IGP_AMT             0x104A
#define E1000_DEV_ID_ICH8_IGP_C               0x104B
#define E1000_DEV_ID_ICH8_IFE                 0x104C
#define E1000_DEV_ID_ICH8_IFE_GT              0x10C4
#define E1000_DEV_ID_ICH8_IFE_G               0x10C5
#define E1000_DEV_ID_ICH8_IGP_M               0x104D
#define E1000_DEV_ID_ICH9_IGP_M               0x10BF
#define E1000_DEV_ID_ICH9_IGP_M_AMT           0x10F5
#define E1000_DEV_ID_ICH9_IGP_M_V             0x10CB
#define E1000_DEV_ID_ICH9_IGP_AMT             0x10BD
#define E1000_DEV_ID_ICH9_BM                  0x10E5
#define E1000_DEV_ID_ICH9_IGP_C               0x294C
#define E1000_DEV_ID_ICH9_IFE                 0x10C0
#define E1000_DEV_ID_ICH9_IFE_GT              0x10C3
#define E1000_DEV_ID_ICH9_IFE_G               0x10C2
#define E1000_DEV_ID_ICH10_R_BM_LM            0x10CC
#define E1000_DEV_ID_ICH10_R_BM_LF            0x10CD
#define E1000_DEV_ID_ICH10_R_BM_V             0x10CE
#define E1000_DEV_ID_ICH10_D_BM_LM            0x10DE
#define E1000_DEV_ID_ICH10_D_BM_LF            0x10DF
#define E1000_DEV_ID_ICH10_D_BM_V             0x1525

#define E1000_DEV_ID_PCH_M_HV_LM              0x10EA
#define E1000_DEV_ID_PCH_M_HV_LC              0x10EB
#define E1000_DEV_ID_PCH_D_HV_DM              0x10EF
#define E1000_DEV_ID_PCH_D_HV_DC              0x10F0
#define E1000_DEV_ID_PCH2_LV_LM               0x1502
#define E1000_DEV_ID_PCH2_LV_V                0x1503
#define E1000_DEV_ID_PCH_LPT_I217_LM          0x153A
#define E1000_DEV_ID_PCH_LPT_I217_V           0x153B
#define E1000_DEV_ID_PCH_LPTLP_I218_LM	      0x155A
#define E1000_DEV_ID_PCH_LPTLP_I218_V	      0x1559

/*
 * Tested (supported) on VM emulated HW.
 */

RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82540EM)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82545EM_COPPER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82545EM_FIBER)

/*
 * Tested (supported) on real HW.
 */

RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82546EB_COPPER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82546EB_FIBER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82546EB_QUAD_COPPER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82571EB_COPPER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82571EB_FIBER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82571EB_SERDES)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82571EB_SERDES_DUAL)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82571EB_SERDES_QUAD)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82571EB_QUAD_COPPER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82571PT_QUAD_COPPER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82571EB_QUAD_FIBER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82571EB_QUAD_COPPER_LP)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82572EI_COPPER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82572EI_FIBER)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82572EI_SERDES)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82572EI)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82573L)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82574L)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82574LA)
RTE_PCI_DEV_ID_DECL_EM(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82583V)

/******************** Physical IGB devices from e1000_hw.h ********************/

#define E1000_DEV_ID_82576                      0x10C9
#define E1000_DEV_ID_82576_FIBER                0x10E6
#define E1000_DEV_ID_82576_SERDES               0x10E7
#define E1000_DEV_ID_82576_QUAD_COPPER          0x10E8
#define E1000_DEV_ID_82576_QUAD_COPPER_ET2      0x1526
#define E1000_DEV_ID_82576_NS                   0x150A
#define E1000_DEV_ID_82576_NS_SERDES            0x1518
#define E1000_DEV_ID_82576_SERDES_QUAD          0x150D
#define E1000_DEV_ID_82575EB_COPPER             0x10A7
#define E1000_DEV_ID_82575EB_FIBER_SERDES       0x10A9
#define E1000_DEV_ID_82575GB_QUAD_COPPER        0x10D6
#define E1000_DEV_ID_82580_COPPER               0x150E
#define E1000_DEV_ID_82580_FIBER                0x150F
#define E1000_DEV_ID_82580_SERDES               0x1510
#define E1000_DEV_ID_82580_SGMII                0x1511
#define E1000_DEV_ID_82580_COPPER_DUAL          0x1516
#define E1000_DEV_ID_82580_QUAD_FIBER           0x1527
#define E1000_DEV_ID_I350_COPPER                0x1521
#define E1000_DEV_ID_I350_FIBER                 0x1522
#define E1000_DEV_ID_I350_SERDES                0x1523
#define E1000_DEV_ID_I350_SGMII                 0x1524
#define E1000_DEV_ID_I350_DA4                   0x1546
#define E1000_DEV_ID_I210_COPPER                0x1533
#define E1000_DEV_ID_I210_COPPER_OEM1           0x1534
#define E1000_DEV_ID_I210_COPPER_IT             0x1535
#define E1000_DEV_ID_I210_FIBER                 0x1536
#define E1000_DEV_ID_I210_SERDES                0x1537
#define E1000_DEV_ID_I210_SGMII                 0x1538
#define E1000_DEV_ID_I210_COPPER_FLASHLESS      0x157B
#define E1000_DEV_ID_I210_SERDES_FLASHLESS      0x157C
#define E1000_DEV_ID_I211_COPPER                0x1539
#define E1000_DEV_ID_I354_BACKPLANE_1GBPS       0x1F40
#define E1000_DEV_ID_I354_SGMII                 0x1F41
#define E1000_DEV_ID_I354_BACKPLANE_2_5GBPS     0x1F45
#define E1000_DEV_ID_DH89XXCC_SGMII             0x0438
#define E1000_DEV_ID_DH89XXCC_SERDES            0x043A
#define E1000_DEV_ID_DH89XXCC_BACKPLANE         0x043C
#define E1000_DEV_ID_DH89XXCC_SFP               0x0440

RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82576)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82576_FIBER)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82576_SERDES)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82576_QUAD_COPPER)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82576_QUAD_COPPER_ET2)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82576_NS)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82576_NS_SERDES)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82576_SERDES_QUAD)

RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82575EB_COPPER)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82575EB_FIBER_SERDES)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82575GB_QUAD_COPPER)

RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82580_COPPER)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82580_FIBER)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82580_SERDES)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82580_SGMII)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82580_COPPER_DUAL)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82580_QUAD_FIBER)

RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I350_COPPER)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I350_FIBER)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I350_SERDES)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I350_SGMII)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I350_DA4)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I210_COPPER)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I210_COPPER_OEM1)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I210_COPPER_IT)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I210_FIBER)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I210_SERDES)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I210_SGMII)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I211_COPPER)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I354_BACKPLANE_1GBPS)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I354_SGMII)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I354_BACKPLANE_2_5GBPS)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_DH89XXCC_SGMII)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_DH89XXCC_SERDES)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_DH89XXCC_BACKPLANE)
RTE_PCI_DEV_ID_DECL_IGB(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_DH89XXCC_SFP)

/****************** Physical IXGBE devices from ixgbe_type.h ******************/

#define IXGBE_DEV_ID_82598                      0x10B6
#define IXGBE_DEV_ID_82598_BX                   0x1508
#define IXGBE_DEV_ID_82598AF_DUAL_PORT          0x10C6
#define IXGBE_DEV_ID_82598AF_SINGLE_PORT        0x10C7
#define IXGBE_DEV_ID_82598AT                    0x10C8
#define IXGBE_DEV_ID_82598AT2                   0x150B
#define IXGBE_DEV_ID_82598EB_SFP_LOM            0x10DB
#define IXGBE_DEV_ID_82598EB_CX4                0x10DD
#define IXGBE_DEV_ID_82598_CX4_DUAL_PORT        0x10EC
#define IXGBE_DEV_ID_82598_DA_DUAL_PORT         0x10F1
#define IXGBE_DEV_ID_82598_SR_DUAL_PORT_EM      0x10E1
#define IXGBE_DEV_ID_82598EB_XF_LR              0x10F4
#define IXGBE_DEV_ID_82599_KX4                  0x10F7
#define IXGBE_DEV_ID_82599_KX4_MEZZ             0x1514
#define IXGBE_DEV_ID_82599_KR                   0x1517
#define IXGBE_DEV_ID_82599_COMBO_BACKPLANE      0x10F8
#define IXGBE_SUBDEV_ID_82599_KX4_KR_MEZZ       0x000C
#define IXGBE_DEV_ID_82599_CX4                  0x10F9
#define IXGBE_DEV_ID_82599_SFP                  0x10FB
#define IXGBE_SUBDEV_ID_82599_SFP               0x11A9
#define IXGBE_SUBDEV_ID_82599_RNDC              0x1F72
#define IXGBE_SUBDEV_ID_82599_560FLR            0x17D0
#define IXGBE_SUBDEV_ID_82599_ECNA_DP           0x0470
#define IXGBE_DEV_ID_82599_BACKPLANE_FCOE       0x152A
#define IXGBE_DEV_ID_82599_SFP_FCOE             0x1529
#define IXGBE_DEV_ID_82599_SFP_EM               0x1507
#define IXGBE_DEV_ID_82599_SFP_SF2              0x154D
#define IXGBE_DEV_ID_82599_SFP_SF_QP            0x154A
#define IXGBE_DEV_ID_82599_QSFP_SF_QP           0x1558
#define IXGBE_DEV_ID_82599EN_SFP                0x1557
#define IXGBE_DEV_ID_82599_XAUI_LOM             0x10FC
#define IXGBE_DEV_ID_82599_T3_LOM               0x151C
#define IXGBE_DEV_ID_82599_LS                   0x154F
#define IXGBE_DEV_ID_X540T                      0x1528
#define IXGBE_DEV_ID_X540T1                     0x1560
#define IXGBE_DEV_ID_X550EM_X_SFP               0x15AC
#define IXGBE_DEV_ID_X550EM_X_10G_T             0x15AD
#define IXGBE_DEV_ID_X550EM_X_1G_T              0x15AE
#define IXGBE_DEV_ID_X550T                      0x1563
#define IXGBE_DEV_ID_X550EM_X_KX4               0x15AA
#define IXGBE_DEV_ID_X550EM_X_KR                0x15AB

#ifdef RTE_NIC_BYPASS
#define IXGBE_DEV_ID_82599_BYPASS               0x155D
#endif

RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82598)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82598_BX)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82598AF_DUAL_PORT)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, \
	IXGBE_DEV_ID_82598AF_SINGLE_PORT)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82598AT)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82598AT2)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82598EB_SFP_LOM)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82598EB_CX4)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82598_CX4_DUAL_PORT)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82598_DA_DUAL_PORT)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, \
	IXGBE_DEV_ID_82598_SR_DUAL_PORT_EM)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82598EB_XF_LR)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_KX4)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_KX4_MEZZ)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_KR)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, \
	IXGBE_DEV_ID_82599_COMBO_BACKPLANE)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, \
	IXGBE_SUBDEV_ID_82599_KX4_KR_MEZZ)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_CX4)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_SFP)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_SUBDEV_ID_82599_SFP)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_SUBDEV_ID_82599_RNDC)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_SUBDEV_ID_82599_560FLR)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_SUBDEV_ID_82599_ECNA_DP)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_BACKPLANE_FCOE)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_SFP_FCOE)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_SFP_EM)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_SFP_SF2)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_SFP_SF_QP)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_QSFP_SF_QP)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599EN_SFP)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_XAUI_LOM)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_T3_LOM)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_LS)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X540T)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X540T1)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X550EM_X_SFP)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X550EM_X_10G_T)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X550EM_X_1G_T)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X550T)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X550EM_X_KX4)
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X550EM_X_KR)

#ifdef RTE_NIC_BYPASS
RTE_PCI_DEV_ID_DECL_IXGBE(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_BYPASS)
#endif

/*************** Physical I40E devices from i40e_type.h *****************/

#define I40E_DEV_ID_SFP_XL710           0x1572
#define I40E_DEV_ID_QEMU                0x1574
#define I40E_DEV_ID_KX_A                0x157F
#define I40E_DEV_ID_KX_B                0x1580
#define I40E_DEV_ID_KX_C                0x1581
#define I40E_DEV_ID_QSFP_A              0x1583
#define I40E_DEV_ID_QSFP_B              0x1584
#define I40E_DEV_ID_QSFP_C              0x1585
#define I40E_DEV_ID_10G_BASE_T          0x1586
#define I40E_DEV_ID_20G_KR2             0x1587
#define I40E_DEV_ID_20G_KR2_A           0x1588
#define I40E_DEV_ID_10G_BASE_T4         0x1589

RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_SFP_XL710)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_QEMU)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_KX_A)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_KX_B)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_KX_C)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_QSFP_A)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_QSFP_B)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_QSFP_C)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_10G_BASE_T)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_20G_KR2)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_20G_KR2_A)
RTE_PCI_DEV_ID_DECL_I40E(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_10G_BASE_T4)

/*************** Physical FM10K devices from fm10k_type.h ***************/

#define FM10K_DEV_ID_PF                   0x15A4

RTE_PCI_DEV_ID_DECL_FM10K(PCI_VENDOR_ID_INTEL, FM10K_DEV_ID_PF)

/****************** Virtual IGB devices from e1000_hw.h ******************/

#define E1000_DEV_ID_82576_VF                   0x10CA
#define E1000_DEV_ID_82576_VF_HV                0x152D
#define E1000_DEV_ID_I350_VF                    0x1520
#define E1000_DEV_ID_I350_VF_HV                 0x152F

RTE_PCI_DEV_ID_DECL_IGBVF(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82576_VF)
RTE_PCI_DEV_ID_DECL_IGBVF(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82576_VF_HV)
RTE_PCI_DEV_ID_DECL_IGBVF(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I350_VF)
RTE_PCI_DEV_ID_DECL_IGBVF(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_I350_VF_HV)

/****************** Virtual IXGBE devices from ixgbe_type.h ******************/

#define IXGBE_DEV_ID_82599_VF                   0x10ED
#define IXGBE_DEV_ID_82599_VF_HV                0x152E
#define IXGBE_DEV_ID_X540_VF                    0x1515
#define IXGBE_DEV_ID_X540_VF_HV                 0x1530
#define IXGBE_DEV_ID_X550_VF_HV                 0x1564
#define IXGBE_DEV_ID_X550_VF                    0x1565
#define IXGBE_DEV_ID_X550EM_X_VF                0x15A8
#define IXGBE_DEV_ID_X550EM_X_VF_HV             0x15A9

RTE_PCI_DEV_ID_DECL_IXGBEVF(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_VF)
RTE_PCI_DEV_ID_DECL_IXGBEVF(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_82599_VF_HV)
RTE_PCI_DEV_ID_DECL_IXGBEVF(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X540_VF)
RTE_PCI_DEV_ID_DECL_IXGBEVF(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X540_VF_HV)
RTE_PCI_DEV_ID_DECL_IXGBEVF(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X550_VF_HV)
RTE_PCI_DEV_ID_DECL_IXGBEVF(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X550_VF)
RTE_PCI_DEV_ID_DECL_IXGBEVF(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X550EM_X_VF)
RTE_PCI_DEV_ID_DECL_IXGBEVF(PCI_VENDOR_ID_INTEL, IXGBE_DEV_ID_X550EM_X_VF_HV)

/****************** Virtual I40E devices from i40e_type.h ********************/

#define I40E_DEV_ID_VF                  0x154C
#define I40E_DEV_ID_VF_HV               0x1571

RTE_PCI_DEV_ID_DECL_I40EVF(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_VF)
RTE_PCI_DEV_ID_DECL_I40EVF(PCI_VENDOR_ID_INTEL, I40E_DEV_ID_VF_HV)

/****************** Virtio devices from virtio.h ******************/

#define QUMRANET_DEV_ID_VIRTIO                  0x1000

RTE_PCI_DEV_ID_DECL_VIRTIO(PCI_VENDOR_ID_QUMRANET, QUMRANET_DEV_ID_VIRTIO)

/****************** VMware VMXNET3 devices ******************/

#define VMWARE_DEV_ID_VMXNET3                   0x07B0

RTE_PCI_DEV_ID_DECL_VMXNET3(PCI_VENDOR_ID_VMWARE, VMWARE_DEV_ID_VMXNET3)

/*************** Virtual FM10K devices from fm10k_type.h ***************/

#define FM10K_DEV_ID_VF                   0x15A5

RTE_PCI_DEV_ID_DECL_FM10KVF(PCI_VENDOR_ID_INTEL, FM10K_DEV_ID_VF)

/****************** Cisco VIC devices ******************/

#define PCI_DEVICE_ID_CISCO_VIC_ENET         0x0043  /* ethernet vnic */
#define PCI_DEVICE_ID_CISCO_VIC_ENET_VF      0x0071  /* enet SRIOV VF */

RTE_PCI_DEV_ID_DECL_ENIC(PCI_VENDOR_ID_CISCO, PCI_DEVICE_ID_CISCO_VIC_ENET)
RTE_PCI_DEV_ID_DECL_ENIC(PCI_VENDOR_ID_CISCO, PCI_DEVICE_ID_CISCO_VIC_ENET_VF)

/****************** QLogic devices ******************/

/* Broadcom/QLogic BNX2X */
#define BNX2X_DEV_ID_57710	0x164e
#define BNX2X_DEV_ID_57711	0x164f
#define BNX2X_DEV_ID_57711E	0x1650
#define BNX2X_DEV_ID_57712	0x1662
#define BNX2X_DEV_ID_57712_MF	0x1663
#define BNX2X_DEV_ID_57712_VF	0x166f
#define BNX2X_DEV_ID_57713	0x1651
#define BNX2X_DEV_ID_57713E	0x1652
#define BNX2X_DEV_ID_57800	0x168a
#define BNX2X_DEV_ID_57800_MF	0x16a5
#define BNX2X_DEV_ID_57800_VF	0x16a9
#define BNX2X_DEV_ID_57810	0x168e
#define BNX2X_DEV_ID_57810_MF	0x16ae
#define BNX2X_DEV_ID_57810_VF	0x16af
#define BNX2X_DEV_ID_57811	0x163d
#define BNX2X_DEV_ID_57811_MF	0x163e
#define BNX2X_DEV_ID_57811_VF	0x163f

#define BNX2X_DEV_ID_57840_OBS		0x168d
#define BNX2X_DEV_ID_57840_OBS_MF	0x16ab
#define BNX2X_DEV_ID_57840_4_10		0x16a1
#define BNX2X_DEV_ID_57840_2_20		0x16a2
#define BNX2X_DEV_ID_57840_MF		0x16a4
#define BNX2X_DEV_ID_57840_VF		0x16ad

RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57800)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57800_VF)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57711)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57810)
RTE_PCI_DEV_ID_DECL_BNX2XVF(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57810_VF)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57811)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57811_VF)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57840_OBS)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57840_4_10)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57840_2_20)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57840_VF)
#ifdef RTE_LIBRTE_BNX2X_MF_SUPPORT
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57810_MF)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57811_MF)
RTE_PCI_DEV_ID_DECL_BNX2X(PCI_VENDOR_ID_BROADCOM, BNX2X_DEV_ID_57840_MF)
#endif

/*
 * Undef all RTE_PCI_DEV_ID_DECL_* here.
 */
#undef RTE_PCI_DEV_ID_DECL_BNX2X
#undef RTE_PCI_DEV_ID_DECL_BNX2XVF
#undef RTE_PCI_DEV_ID_DECL_EM
#undef RTE_PCI_DEV_ID_DECL_IGB
#undef RTE_PCI_DEV_ID_DECL_IGBVF
#undef RTE_PCI_DEV_ID_DECL_IXGBE
#undef RTE_PCI_DEV_ID_DECL_IXGBEVF
#undef RTE_PCI_DEV_ID_DECL_I40E
#undef RTE_PCI_DEV_ID_DECL_I40EVF
#undef RTE_PCI_DEV_ID_DECL_VIRTIO
#undef RTE_PCI_DEV_ID_DECL_VMXNET3
#undef RTE_PCI_DEV_ID_DECL_FM10K
#undef RTE_PCI_DEV_ID_DECL_FM10KVF
