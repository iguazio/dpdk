/*
 * Copyright (c) 2016 QLogic Corporation.
 * All rights reserved.
 * www.qlogic.com
 *
 * See LICENSE.qede_pmd for copyright and licensing details.
 */

#ifndef __RT_DEFS_H__
#define __RT_DEFS_H__

/* Runtime array offsets */
#define DORQ_REG_PF_MAX_ICID_0_RT_OFFSET                            0
#define DORQ_REG_PF_MAX_ICID_1_RT_OFFSET                            1
#define DORQ_REG_PF_MAX_ICID_2_RT_OFFSET                            2
#define DORQ_REG_PF_MAX_ICID_3_RT_OFFSET                            3
#define DORQ_REG_PF_MAX_ICID_4_RT_OFFSET                            4
#define DORQ_REG_PF_MAX_ICID_5_RT_OFFSET                            5
#define DORQ_REG_PF_MAX_ICID_6_RT_OFFSET                            6
#define DORQ_REG_PF_MAX_ICID_7_RT_OFFSET                            7
#define DORQ_REG_VF_MAX_ICID_0_RT_OFFSET                            8
#define DORQ_REG_VF_MAX_ICID_1_RT_OFFSET                            9
#define DORQ_REG_VF_MAX_ICID_2_RT_OFFSET                            10
#define DORQ_REG_VF_MAX_ICID_3_RT_OFFSET                            11
#define DORQ_REG_VF_MAX_ICID_4_RT_OFFSET                            12
#define DORQ_REG_VF_MAX_ICID_5_RT_OFFSET                            13
#define DORQ_REG_VF_MAX_ICID_6_RT_OFFSET                            14
#define DORQ_REG_VF_MAX_ICID_7_RT_OFFSET                            15
#define DORQ_REG_PF_WAKE_ALL_RT_OFFSET                              16
#define DORQ_REG_TAG1_ETHERTYPE_RT_OFFSET                           17
#define IGU_REG_PF_CONFIGURATION_RT_OFFSET                          18
#define IGU_REG_VF_CONFIGURATION_RT_OFFSET                          19
#define IGU_REG_ATTN_MSG_ADDR_L_RT_OFFSET                           20
#define IGU_REG_ATTN_MSG_ADDR_H_RT_OFFSET                           21
#define IGU_REG_LEADING_EDGE_LATCH_RT_OFFSET                        22
#define IGU_REG_TRAILING_EDGE_LATCH_RT_OFFSET                       23
#define CAU_REG_CQE_AGG_UNIT_SIZE_RT_OFFSET                         24
#define CAU_REG_SB_VAR_MEMORY_RT_OFFSET                             761
#define CAU_REG_SB_VAR_MEMORY_RT_SIZE                               736
#define CAU_REG_SB_VAR_MEMORY_RT_OFFSET                             761
#define CAU_REG_SB_VAR_MEMORY_RT_SIZE                               736
#define CAU_REG_SB_ADDR_MEMORY_RT_OFFSET                            1497
#define CAU_REG_SB_ADDR_MEMORY_RT_SIZE                              736
#define CAU_REG_PI_MEMORY_RT_OFFSET                                 2233
#define CAU_REG_PI_MEMORY_RT_SIZE                                   4416
#define PRS_REG_SEARCH_RESP_INITIATOR_TYPE_RT_OFFSET                6649
#define PRS_REG_TASK_ID_MAX_INITIATOR_PF_RT_OFFSET                  6650
#define PRS_REG_TASK_ID_MAX_INITIATOR_VF_RT_OFFSET                  6651
#define PRS_REG_TASK_ID_MAX_TARGET_PF_RT_OFFSET                     6652
#define PRS_REG_TASK_ID_MAX_TARGET_VF_RT_OFFSET                     6653
#define PRS_REG_SEARCH_TCP_RT_OFFSET                                6654
#define PRS_REG_SEARCH_FCOE_RT_OFFSET                               6655
#define PRS_REG_SEARCH_ROCE_RT_OFFSET                               6656
#define PRS_REG_ROCE_DEST_QP_MAX_VF_RT_OFFSET                       6657
#define PRS_REG_ROCE_DEST_QP_MAX_PF_RT_OFFSET                       6658
#define PRS_REG_SEARCH_OPENFLOW_RT_OFFSET                           6659
#define PRS_REG_SEARCH_NON_IP_AS_OPENFLOW_RT_OFFSET                 6660
#define PRS_REG_OPENFLOW_SUPPORT_ONLY_KNOWN_OVER_IP_RT_OFFSET       6661
#define PRS_REG_OPENFLOW_SEARCH_KEY_MASK_RT_OFFSET                  6662
#define PRS_REG_TAG_ETHERTYPE_0_RT_OFFSET                           6663
#define PRS_REG_LIGHT_L2_ETHERTYPE_EN_RT_OFFSET                     6664
#define SRC_REG_FIRSTFREE_RT_OFFSET                                 6665
#define SRC_REG_FIRSTFREE_RT_SIZE                                   2
#define SRC_REG_LASTFREE_RT_OFFSET                                  6667
#define SRC_REG_LASTFREE_RT_SIZE                                    2
#define SRC_REG_COUNTFREE_RT_OFFSET                                 6669
#define SRC_REG_NUMBER_HASH_BITS_RT_OFFSET                          6670
#define PSWRQ2_REG_CDUT_P_SIZE_RT_OFFSET                            6671
#define PSWRQ2_REG_CDUC_P_SIZE_RT_OFFSET                            6672
#define PSWRQ2_REG_TM_P_SIZE_RT_OFFSET                              6673
#define PSWRQ2_REG_QM_P_SIZE_RT_OFFSET                              6674
#define PSWRQ2_REG_SRC_P_SIZE_RT_OFFSET                             6675
#define PSWRQ2_REG_TSDM_P_SIZE_RT_OFFSET                            6676
#define PSWRQ2_REG_TM_FIRST_ILT_RT_OFFSET                           6677
#define PSWRQ2_REG_TM_LAST_ILT_RT_OFFSET                            6678
#define PSWRQ2_REG_QM_FIRST_ILT_RT_OFFSET                           6679
#define PSWRQ2_REG_QM_LAST_ILT_RT_OFFSET                            6680
#define PSWRQ2_REG_SRC_FIRST_ILT_RT_OFFSET                          6681
#define PSWRQ2_REG_SRC_LAST_ILT_RT_OFFSET                           6682
#define PSWRQ2_REG_CDUC_FIRST_ILT_RT_OFFSET                         6683
#define PSWRQ2_REG_CDUC_LAST_ILT_RT_OFFSET                          6684
#define PSWRQ2_REG_CDUT_FIRST_ILT_RT_OFFSET                         6685
#define PSWRQ2_REG_CDUT_LAST_ILT_RT_OFFSET                          6686
#define PSWRQ2_REG_TSDM_FIRST_ILT_RT_OFFSET                         6687
#define PSWRQ2_REG_TSDM_LAST_ILT_RT_OFFSET                          6688
#define PSWRQ2_REG_TM_NUMBER_OF_PF_BLOCKS_RT_OFFSET                 6689
#define PSWRQ2_REG_CDUT_NUMBER_OF_PF_BLOCKS_RT_OFFSET               6690
#define PSWRQ2_REG_CDUC_NUMBER_OF_PF_BLOCKS_RT_OFFSET               6691
#define PSWRQ2_REG_TM_VF_BLOCKS_RT_OFFSET                           6692
#define PSWRQ2_REG_CDUT_VF_BLOCKS_RT_OFFSET                         6693
#define PSWRQ2_REG_CDUC_VF_BLOCKS_RT_OFFSET                         6694
#define PSWRQ2_REG_TM_BLOCKS_FACTOR_RT_OFFSET                       6695
#define PSWRQ2_REG_CDUT_BLOCKS_FACTOR_RT_OFFSET                     6696
#define PSWRQ2_REG_CDUC_BLOCKS_FACTOR_RT_OFFSET                     6697
#define PSWRQ2_REG_VF_BASE_RT_OFFSET                                6698
#define PSWRQ2_REG_VF_LAST_ILT_RT_OFFSET                            6699
#define PSWRQ2_REG_WR_MBS0_RT_OFFSET                                6700
#define PSWRQ2_REG_RD_MBS0_RT_OFFSET                                6701
#define PSWRQ2_REG_DRAM_ALIGN_WR_RT_OFFSET                          6702
#define PSWRQ2_REG_DRAM_ALIGN_RD_RT_OFFSET                          6703
#define PSWRQ2_REG_ILT_MEMORY_RT_OFFSET                             6704
#define PSWRQ2_REG_ILT_MEMORY_RT_SIZE                               22000
#define PGLUE_REG_B_VF_BASE_RT_OFFSET                               28704
#define PGLUE_REG_B_MSDM_OFFSET_MASK_B_RT_OFFSET                    28705
#define PGLUE_REG_B_MSDM_VF_SHIFT_B_RT_OFFSET                       28706
#define PGLUE_REG_B_CACHE_LINE_SIZE_RT_OFFSET                       28707
#define PGLUE_REG_B_PF_BAR0_SIZE_RT_OFFSET                          28708
#define PGLUE_REG_B_PF_BAR1_SIZE_RT_OFFSET                          28709
#define PGLUE_REG_B_VF_BAR1_SIZE_RT_OFFSET                          28710
#define TM_REG_VF_ENABLE_CONN_RT_OFFSET                             28711
#define TM_REG_PF_ENABLE_CONN_RT_OFFSET                             28712
#define TM_REG_PF_ENABLE_TASK_RT_OFFSET                             28713
#define TM_REG_GROUP_SIZE_RESOLUTION_CONN_RT_OFFSET                 28714
#define TM_REG_GROUP_SIZE_RESOLUTION_TASK_RT_OFFSET                 28715
#define TM_REG_CONFIG_CONN_MEM_RT_OFFSET                            28716
#define TM_REG_CONFIG_CONN_MEM_RT_SIZE                              416
#define TM_REG_CONFIG_TASK_MEM_RT_OFFSET                            29132
#define TM_REG_CONFIG_TASK_MEM_RT_SIZE                              512
#define QM_REG_MAXPQSIZE_0_RT_OFFSET                                29644
#define QM_REG_MAXPQSIZE_1_RT_OFFSET                                29645
#define QM_REG_MAXPQSIZE_2_RT_OFFSET                                29646
#define QM_REG_MAXPQSIZETXSEL_0_RT_OFFSET                           29647
#define QM_REG_MAXPQSIZETXSEL_1_RT_OFFSET                           29648
#define QM_REG_MAXPQSIZETXSEL_2_RT_OFFSET                           29649
#define QM_REG_MAXPQSIZETXSEL_3_RT_OFFSET                           29650
#define QM_REG_MAXPQSIZETXSEL_4_RT_OFFSET                           29651
#define QM_REG_MAXPQSIZETXSEL_5_RT_OFFSET                           29652
#define QM_REG_MAXPQSIZETXSEL_6_RT_OFFSET                           29653
#define QM_REG_MAXPQSIZETXSEL_7_RT_OFFSET                           29654
#define QM_REG_MAXPQSIZETXSEL_8_RT_OFFSET                           29655
#define QM_REG_MAXPQSIZETXSEL_9_RT_OFFSET                           29656
#define QM_REG_MAXPQSIZETXSEL_10_RT_OFFSET                          29657
#define QM_REG_MAXPQSIZETXSEL_11_RT_OFFSET                          29658
#define QM_REG_MAXPQSIZETXSEL_12_RT_OFFSET                          29659
#define QM_REG_MAXPQSIZETXSEL_13_RT_OFFSET                          29660
#define QM_REG_MAXPQSIZETXSEL_14_RT_OFFSET                          29661
#define QM_REG_MAXPQSIZETXSEL_15_RT_OFFSET                          29662
#define QM_REG_MAXPQSIZETXSEL_16_RT_OFFSET                          29663
#define QM_REG_MAXPQSIZETXSEL_17_RT_OFFSET                          29664
#define QM_REG_MAXPQSIZETXSEL_18_RT_OFFSET                          29665
#define QM_REG_MAXPQSIZETXSEL_19_RT_OFFSET                          29666
#define QM_REG_MAXPQSIZETXSEL_20_RT_OFFSET                          29667
#define QM_REG_MAXPQSIZETXSEL_21_RT_OFFSET                          29668
#define QM_REG_MAXPQSIZETXSEL_22_RT_OFFSET                          29669
#define QM_REG_MAXPQSIZETXSEL_23_RT_OFFSET                          29670
#define QM_REG_MAXPQSIZETXSEL_24_RT_OFFSET                          29671
#define QM_REG_MAXPQSIZETXSEL_25_RT_OFFSET                          29672
#define QM_REG_MAXPQSIZETXSEL_26_RT_OFFSET                          29673
#define QM_REG_MAXPQSIZETXSEL_27_RT_OFFSET                          29674
#define QM_REG_MAXPQSIZETXSEL_28_RT_OFFSET                          29675
#define QM_REG_MAXPQSIZETXSEL_29_RT_OFFSET                          29676
#define QM_REG_MAXPQSIZETXSEL_30_RT_OFFSET                          29677
#define QM_REG_MAXPQSIZETXSEL_31_RT_OFFSET                          29678
#define QM_REG_MAXPQSIZETXSEL_32_RT_OFFSET                          29679
#define QM_REG_MAXPQSIZETXSEL_33_RT_OFFSET                          29680
#define QM_REG_MAXPQSIZETXSEL_34_RT_OFFSET                          29681
#define QM_REG_MAXPQSIZETXSEL_35_RT_OFFSET                          29682
#define QM_REG_MAXPQSIZETXSEL_36_RT_OFFSET                          29683
#define QM_REG_MAXPQSIZETXSEL_37_RT_OFFSET                          29684
#define QM_REG_MAXPQSIZETXSEL_38_RT_OFFSET                          29685
#define QM_REG_MAXPQSIZETXSEL_39_RT_OFFSET                          29686
#define QM_REG_MAXPQSIZETXSEL_40_RT_OFFSET                          29687
#define QM_REG_MAXPQSIZETXSEL_41_RT_OFFSET                          29688
#define QM_REG_MAXPQSIZETXSEL_42_RT_OFFSET                          29689
#define QM_REG_MAXPQSIZETXSEL_43_RT_OFFSET                          29690
#define QM_REG_MAXPQSIZETXSEL_44_RT_OFFSET                          29691
#define QM_REG_MAXPQSIZETXSEL_45_RT_OFFSET                          29692
#define QM_REG_MAXPQSIZETXSEL_46_RT_OFFSET                          29693
#define QM_REG_MAXPQSIZETXSEL_47_RT_OFFSET                          29694
#define QM_REG_MAXPQSIZETXSEL_48_RT_OFFSET                          29695
#define QM_REG_MAXPQSIZETXSEL_49_RT_OFFSET                          29696
#define QM_REG_MAXPQSIZETXSEL_50_RT_OFFSET                          29697
#define QM_REG_MAXPQSIZETXSEL_51_RT_OFFSET                          29698
#define QM_REG_MAXPQSIZETXSEL_52_RT_OFFSET                          29699
#define QM_REG_MAXPQSIZETXSEL_53_RT_OFFSET                          29700
#define QM_REG_MAXPQSIZETXSEL_54_RT_OFFSET                          29701
#define QM_REG_MAXPQSIZETXSEL_55_RT_OFFSET                          29702
#define QM_REG_MAXPQSIZETXSEL_56_RT_OFFSET                          29703
#define QM_REG_MAXPQSIZETXSEL_57_RT_OFFSET                          29704
#define QM_REG_MAXPQSIZETXSEL_58_RT_OFFSET                          29705
#define QM_REG_MAXPQSIZETXSEL_59_RT_OFFSET                          29706
#define QM_REG_MAXPQSIZETXSEL_60_RT_OFFSET                          29707
#define QM_REG_MAXPQSIZETXSEL_61_RT_OFFSET                          29708
#define QM_REG_MAXPQSIZETXSEL_62_RT_OFFSET                          29709
#define QM_REG_MAXPQSIZETXSEL_63_RT_OFFSET                          29710
#define QM_REG_BASEADDROTHERPQ_RT_OFFSET                            29711
#define QM_REG_BASEADDROTHERPQ_RT_SIZE                              128
#define QM_REG_VOQCRDLINE_RT_OFFSET                                 29839
#define QM_REG_VOQCRDLINE_RT_SIZE                                   20
#define QM_REG_VOQINITCRDLINE_RT_OFFSET                             29859
#define QM_REG_VOQINITCRDLINE_RT_SIZE                               20
#define QM_REG_AFULLQMBYPTHRPFWFQ_RT_OFFSET                         29879
#define QM_REG_AFULLQMBYPTHRVPWFQ_RT_OFFSET                         29880
#define QM_REG_AFULLQMBYPTHRPFRL_RT_OFFSET                          29881
#define QM_REG_AFULLQMBYPTHRGLBLRL_RT_OFFSET                        29882
#define QM_REG_AFULLOPRTNSTCCRDMASK_RT_OFFSET                       29883
#define QM_REG_WRROTHERPQGRP_0_RT_OFFSET                            29884
#define QM_REG_WRROTHERPQGRP_1_RT_OFFSET                            29885
#define QM_REG_WRROTHERPQGRP_2_RT_OFFSET                            29886
#define QM_REG_WRROTHERPQGRP_3_RT_OFFSET                            29887
#define QM_REG_WRROTHERPQGRP_4_RT_OFFSET                            29888
#define QM_REG_WRROTHERPQGRP_5_RT_OFFSET                            29889
#define QM_REG_WRROTHERPQGRP_6_RT_OFFSET                            29890
#define QM_REG_WRROTHERPQGRP_7_RT_OFFSET                            29891
#define QM_REG_WRROTHERPQGRP_8_RT_OFFSET                            29892
#define QM_REG_WRROTHERPQGRP_9_RT_OFFSET                            29893
#define QM_REG_WRROTHERPQGRP_10_RT_OFFSET                           29894
#define QM_REG_WRROTHERPQGRP_11_RT_OFFSET                           29895
#define QM_REG_WRROTHERPQGRP_12_RT_OFFSET                           29896
#define QM_REG_WRROTHERPQGRP_13_RT_OFFSET                           29897
#define QM_REG_WRROTHERPQGRP_14_RT_OFFSET                           29898
#define QM_REG_WRROTHERPQGRP_15_RT_OFFSET                           29899
#define QM_REG_WRROTHERGRPWEIGHT_0_RT_OFFSET                        29900
#define QM_REG_WRROTHERGRPWEIGHT_1_RT_OFFSET                        29901
#define QM_REG_WRROTHERGRPWEIGHT_2_RT_OFFSET                        29902
#define QM_REG_WRROTHERGRPWEIGHT_3_RT_OFFSET                        29903
#define QM_REG_WRRTXGRPWEIGHT_0_RT_OFFSET                           29904
#define QM_REG_WRRTXGRPWEIGHT_1_RT_OFFSET                           29905
#define QM_REG_PQTX2PF_0_RT_OFFSET                                  29906
#define QM_REG_PQTX2PF_1_RT_OFFSET                                  29907
#define QM_REG_PQTX2PF_2_RT_OFFSET                                  29908
#define QM_REG_PQTX2PF_3_RT_OFFSET                                  29909
#define QM_REG_PQTX2PF_4_RT_OFFSET                                  29910
#define QM_REG_PQTX2PF_5_RT_OFFSET                                  29911
#define QM_REG_PQTX2PF_6_RT_OFFSET                                  29912
#define QM_REG_PQTX2PF_7_RT_OFFSET                                  29913
#define QM_REG_PQTX2PF_8_RT_OFFSET                                  29914
#define QM_REG_PQTX2PF_9_RT_OFFSET                                  29915
#define QM_REG_PQTX2PF_10_RT_OFFSET                                 29916
#define QM_REG_PQTX2PF_11_RT_OFFSET                                 29917
#define QM_REG_PQTX2PF_12_RT_OFFSET                                 29918
#define QM_REG_PQTX2PF_13_RT_OFFSET                                 29919
#define QM_REG_PQTX2PF_14_RT_OFFSET                                 29920
#define QM_REG_PQTX2PF_15_RT_OFFSET                                 29921
#define QM_REG_PQTX2PF_16_RT_OFFSET                                 29922
#define QM_REG_PQTX2PF_17_RT_OFFSET                                 29923
#define QM_REG_PQTX2PF_18_RT_OFFSET                                 29924
#define QM_REG_PQTX2PF_19_RT_OFFSET                                 29925
#define QM_REG_PQTX2PF_20_RT_OFFSET                                 29926
#define QM_REG_PQTX2PF_21_RT_OFFSET                                 29927
#define QM_REG_PQTX2PF_22_RT_OFFSET                                 29928
#define QM_REG_PQTX2PF_23_RT_OFFSET                                 29929
#define QM_REG_PQTX2PF_24_RT_OFFSET                                 29930
#define QM_REG_PQTX2PF_25_RT_OFFSET                                 29931
#define QM_REG_PQTX2PF_26_RT_OFFSET                                 29932
#define QM_REG_PQTX2PF_27_RT_OFFSET                                 29933
#define QM_REG_PQTX2PF_28_RT_OFFSET                                 29934
#define QM_REG_PQTX2PF_29_RT_OFFSET                                 29935
#define QM_REG_PQTX2PF_30_RT_OFFSET                                 29936
#define QM_REG_PQTX2PF_31_RT_OFFSET                                 29937
#define QM_REG_PQTX2PF_32_RT_OFFSET                                 29938
#define QM_REG_PQTX2PF_33_RT_OFFSET                                 29939
#define QM_REG_PQTX2PF_34_RT_OFFSET                                 29940
#define QM_REG_PQTX2PF_35_RT_OFFSET                                 29941
#define QM_REG_PQTX2PF_36_RT_OFFSET                                 29942
#define QM_REG_PQTX2PF_37_RT_OFFSET                                 29943
#define QM_REG_PQTX2PF_38_RT_OFFSET                                 29944
#define QM_REG_PQTX2PF_39_RT_OFFSET                                 29945
#define QM_REG_PQTX2PF_40_RT_OFFSET                                 29946
#define QM_REG_PQTX2PF_41_RT_OFFSET                                 29947
#define QM_REG_PQTX2PF_42_RT_OFFSET                                 29948
#define QM_REG_PQTX2PF_43_RT_OFFSET                                 29949
#define QM_REG_PQTX2PF_44_RT_OFFSET                                 29950
#define QM_REG_PQTX2PF_45_RT_OFFSET                                 29951
#define QM_REG_PQTX2PF_46_RT_OFFSET                                 29952
#define QM_REG_PQTX2PF_47_RT_OFFSET                                 29953
#define QM_REG_PQTX2PF_48_RT_OFFSET                                 29954
#define QM_REG_PQTX2PF_49_RT_OFFSET                                 29955
#define QM_REG_PQTX2PF_50_RT_OFFSET                                 29956
#define QM_REG_PQTX2PF_51_RT_OFFSET                                 29957
#define QM_REG_PQTX2PF_52_RT_OFFSET                                 29958
#define QM_REG_PQTX2PF_53_RT_OFFSET                                 29959
#define QM_REG_PQTX2PF_54_RT_OFFSET                                 29960
#define QM_REG_PQTX2PF_55_RT_OFFSET                                 29961
#define QM_REG_PQTX2PF_56_RT_OFFSET                                 29962
#define QM_REG_PQTX2PF_57_RT_OFFSET                                 29963
#define QM_REG_PQTX2PF_58_RT_OFFSET                                 29964
#define QM_REG_PQTX2PF_59_RT_OFFSET                                 29965
#define QM_REG_PQTX2PF_60_RT_OFFSET                                 29966
#define QM_REG_PQTX2PF_61_RT_OFFSET                                 29967
#define QM_REG_PQTX2PF_62_RT_OFFSET                                 29968
#define QM_REG_PQTX2PF_63_RT_OFFSET                                 29969
#define QM_REG_PQOTHER2PF_0_RT_OFFSET                               29970
#define QM_REG_PQOTHER2PF_1_RT_OFFSET                               29971
#define QM_REG_PQOTHER2PF_2_RT_OFFSET                               29972
#define QM_REG_PQOTHER2PF_3_RT_OFFSET                               29973
#define QM_REG_PQOTHER2PF_4_RT_OFFSET                               29974
#define QM_REG_PQOTHER2PF_5_RT_OFFSET                               29975
#define QM_REG_PQOTHER2PF_6_RT_OFFSET                               29976
#define QM_REG_PQOTHER2PF_7_RT_OFFSET                               29977
#define QM_REG_PQOTHER2PF_8_RT_OFFSET                               29978
#define QM_REG_PQOTHER2PF_9_RT_OFFSET                               29979
#define QM_REG_PQOTHER2PF_10_RT_OFFSET                              29980
#define QM_REG_PQOTHER2PF_11_RT_OFFSET                              29981
#define QM_REG_PQOTHER2PF_12_RT_OFFSET                              29982
#define QM_REG_PQOTHER2PF_13_RT_OFFSET                              29983
#define QM_REG_PQOTHER2PF_14_RT_OFFSET                              29984
#define QM_REG_PQOTHER2PF_15_RT_OFFSET                              29985
#define QM_REG_RLGLBLPERIOD_0_RT_OFFSET                             29986
#define QM_REG_RLGLBLPERIOD_1_RT_OFFSET                             29987
#define QM_REG_RLGLBLPERIODTIMER_0_RT_OFFSET                        29988
#define QM_REG_RLGLBLPERIODTIMER_1_RT_OFFSET                        29989
#define QM_REG_RLGLBLPERIODSEL_0_RT_OFFSET                          29990
#define QM_REG_RLGLBLPERIODSEL_1_RT_OFFSET                          29991
#define QM_REG_RLGLBLPERIODSEL_2_RT_OFFSET                          29992
#define QM_REG_RLGLBLPERIODSEL_3_RT_OFFSET                          29993
#define QM_REG_RLGLBLPERIODSEL_4_RT_OFFSET                          29994
#define QM_REG_RLGLBLPERIODSEL_5_RT_OFFSET                          29995
#define QM_REG_RLGLBLPERIODSEL_6_RT_OFFSET                          29996
#define QM_REG_RLGLBLPERIODSEL_7_RT_OFFSET                          29997
#define QM_REG_RLGLBLINCVAL_RT_OFFSET                               29998
#define QM_REG_RLGLBLINCVAL_RT_SIZE                                 256
#define QM_REG_RLGLBLUPPERBOUND_RT_OFFSET                           30254
#define QM_REG_RLGLBLUPPERBOUND_RT_SIZE                             256
#define QM_REG_RLGLBLCRD_RT_OFFSET                                  30510
#define QM_REG_RLGLBLCRD_RT_SIZE                                    256
#define QM_REG_RLGLBLENABLE_RT_OFFSET                               30766
#define QM_REG_RLPFPERIOD_RT_OFFSET                                 30767
#define QM_REG_RLPFPERIODTIMER_RT_OFFSET                            30768
#define QM_REG_RLPFINCVAL_RT_OFFSET                                 30769
#define QM_REG_RLPFINCVAL_RT_SIZE                                   16
#define QM_REG_RLPFUPPERBOUND_RT_OFFSET                             30785
#define QM_REG_RLPFUPPERBOUND_RT_SIZE                               16
#define QM_REG_RLPFCRD_RT_OFFSET                                    30801
#define QM_REG_RLPFCRD_RT_SIZE                                      16
#define QM_REG_RLPFENABLE_RT_OFFSET                                 30817
#define QM_REG_RLPFVOQENABLE_RT_OFFSET                              30818
#define QM_REG_WFQPFWEIGHT_RT_OFFSET                                30819
#define QM_REG_WFQPFWEIGHT_RT_SIZE                                  16
#define QM_REG_WFQPFUPPERBOUND_RT_OFFSET                            30835
#define QM_REG_WFQPFUPPERBOUND_RT_SIZE                              16
#define QM_REG_WFQPFCRD_RT_OFFSET                                   30851
#define QM_REG_WFQPFCRD_RT_SIZE                                     160
#define QM_REG_WFQPFENABLE_RT_OFFSET                                31011
#define QM_REG_WFQVPENABLE_RT_OFFSET                                31012
#define QM_REG_BASEADDRTXPQ_RT_OFFSET                               31013
#define QM_REG_BASEADDRTXPQ_RT_SIZE                                 512
#define QM_REG_TXPQMAP_RT_OFFSET                                    31525
#define QM_REG_TXPQMAP_RT_SIZE                                      512
#define QM_REG_WFQVPWEIGHT_RT_OFFSET                                32037
#define QM_REG_WFQVPWEIGHT_RT_SIZE                                  512
#define QM_REG_WFQVPCRD_RT_OFFSET                                   32549
#define QM_REG_WFQVPCRD_RT_SIZE                                     512
#define QM_REG_WFQVPMAP_RT_OFFSET                                   33061
#define QM_REG_WFQVPMAP_RT_SIZE                                     512
#define QM_REG_WFQPFCRD_MSB_RT_OFFSET                               33573
#define QM_REG_WFQPFCRD_MSB_RT_SIZE                                 160
#define NIG_REG_TAG_ETHERTYPE_0_RT_OFFSET                           33733
#define NIG_REG_OUTER_TAG_VALUE_LIST0_RT_OFFSET                     33734
#define NIG_REG_OUTER_TAG_VALUE_LIST1_RT_OFFSET                     33735
#define NIG_REG_OUTER_TAG_VALUE_LIST2_RT_OFFSET                     33736
#define NIG_REG_OUTER_TAG_VALUE_LIST3_RT_OFFSET                     33737
#define NIG_REG_OUTER_TAG_VALUE_MASK_RT_OFFSET                      33738
#define NIG_REG_LLH_FUNC_TAGMAC_CLS_TYPE_RT_OFFSET                  33739
#define NIG_REG_LLH_FUNC_TAG_EN_RT_OFFSET                           33740
#define NIG_REG_LLH_FUNC_TAG_EN_RT_SIZE                             4
#define NIG_REG_LLH_FUNC_TAG_HDR_SEL_RT_OFFSET                      33744
#define NIG_REG_LLH_FUNC_TAG_HDR_SEL_RT_SIZE                        4
#define NIG_REG_LLH_FUNC_TAG_VALUE_RT_OFFSET                        33748
#define NIG_REG_LLH_FUNC_TAG_VALUE_RT_SIZE                          4
#define NIG_REG_LLH_FUNC_NO_TAG_RT_OFFSET                           33752
#define NIG_REG_LLH_FUNC_FILTER_VALUE_RT_OFFSET                     33753
#define NIG_REG_LLH_FUNC_FILTER_VALUE_RT_SIZE                       32
#define NIG_REG_LLH_FUNC_FILTER_EN_RT_OFFSET                        33785
#define NIG_REG_LLH_FUNC_FILTER_EN_RT_SIZE                          16
#define NIG_REG_LLH_FUNC_FILTER_MODE_RT_OFFSET                      33801
#define NIG_REG_LLH_FUNC_FILTER_MODE_RT_SIZE                        16
#define NIG_REG_LLH_FUNC_FILTER_PROTOCOL_TYPE_RT_OFFSET             33817
#define NIG_REG_LLH_FUNC_FILTER_PROTOCOL_TYPE_RT_SIZE               16
#define NIG_REG_LLH_FUNC_FILTER_HDR_SEL_RT_OFFSET                   33833
#define NIG_REG_LLH_FUNC_FILTER_HDR_SEL_RT_SIZE                     16
#define NIG_REG_TX_EDPM_CTRL_RT_OFFSET                              33849
#define NIG_REG_ROCE_DUPLICATE_TO_HOST_RT_OFFSET                    33850
#define CDU_REG_CID_ADDR_PARAMS_RT_OFFSET                           33851
#define CDU_REG_SEGMENT0_PARAMS_RT_OFFSET                           33852
#define CDU_REG_SEGMENT1_PARAMS_RT_OFFSET                           33853
#define CDU_REG_PF_SEG0_TYPE_OFFSET_RT_OFFSET                       33854
#define CDU_REG_PF_SEG1_TYPE_OFFSET_RT_OFFSET                       33855
#define CDU_REG_PF_SEG2_TYPE_OFFSET_RT_OFFSET                       33856
#define CDU_REG_PF_SEG3_TYPE_OFFSET_RT_OFFSET                       33857
#define CDU_REG_PF_FL_SEG0_TYPE_OFFSET_RT_OFFSET                    33858
#define CDU_REG_PF_FL_SEG1_TYPE_OFFSET_RT_OFFSET                    33859
#define CDU_REG_PF_FL_SEG2_TYPE_OFFSET_RT_OFFSET                    33860
#define CDU_REG_PF_FL_SEG3_TYPE_OFFSET_RT_OFFSET                    33861
#define CDU_REG_VF_SEG_TYPE_OFFSET_RT_OFFSET                        33862
#define CDU_REG_VF_FL_SEG_TYPE_OFFSET_RT_OFFSET                     33863
#define PBF_REG_TAG_ETHERTYPE_0_RT_OFFSET                           33864
#define PBF_REG_BTB_SHARED_AREA_SIZE_RT_OFFSET                      33865
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ0_RT_OFFSET                    33866
#define PBF_REG_BTB_GUARANTEED_VOQ0_RT_OFFSET                       33867
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ0_RT_OFFSET                33868
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ1_RT_OFFSET                    33869
#define PBF_REG_BTB_GUARANTEED_VOQ1_RT_OFFSET                       33870
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ1_RT_OFFSET                33871
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ2_RT_OFFSET                    33872
#define PBF_REG_BTB_GUARANTEED_VOQ2_RT_OFFSET                       33873
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ2_RT_OFFSET                33874
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ3_RT_OFFSET                    33875
#define PBF_REG_BTB_GUARANTEED_VOQ3_RT_OFFSET                       33876
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ3_RT_OFFSET                33877
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ4_RT_OFFSET                    33878
#define PBF_REG_BTB_GUARANTEED_VOQ4_RT_OFFSET                       33879
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ4_RT_OFFSET                33880
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ5_RT_OFFSET                    33881
#define PBF_REG_BTB_GUARANTEED_VOQ5_RT_OFFSET                       33882
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ5_RT_OFFSET                33883
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ6_RT_OFFSET                    33884
#define PBF_REG_BTB_GUARANTEED_VOQ6_RT_OFFSET                       33885
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ6_RT_OFFSET                33886
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ7_RT_OFFSET                    33887
#define PBF_REG_BTB_GUARANTEED_VOQ7_RT_OFFSET                       33888
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ7_RT_OFFSET                33889
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ8_RT_OFFSET                    33890
#define PBF_REG_BTB_GUARANTEED_VOQ8_RT_OFFSET                       33891
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ8_RT_OFFSET                33892
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ9_RT_OFFSET                    33893
#define PBF_REG_BTB_GUARANTEED_VOQ9_RT_OFFSET                       33894
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ9_RT_OFFSET                33895
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ10_RT_OFFSET                   33896
#define PBF_REG_BTB_GUARANTEED_VOQ10_RT_OFFSET                      33897
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ10_RT_OFFSET               33898
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ11_RT_OFFSET                   33899
#define PBF_REG_BTB_GUARANTEED_VOQ11_RT_OFFSET                      33900
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ11_RT_OFFSET               33901
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ12_RT_OFFSET                   33902
#define PBF_REG_BTB_GUARANTEED_VOQ12_RT_OFFSET                      33903
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ12_RT_OFFSET               33904
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ13_RT_OFFSET                   33905
#define PBF_REG_BTB_GUARANTEED_VOQ13_RT_OFFSET                      33906
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ13_RT_OFFSET               33907
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ14_RT_OFFSET                   33908
#define PBF_REG_BTB_GUARANTEED_VOQ14_RT_OFFSET                      33909
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ14_RT_OFFSET               33910
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ15_RT_OFFSET                   33911
#define PBF_REG_BTB_GUARANTEED_VOQ15_RT_OFFSET                      33912
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ15_RT_OFFSET               33913
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ16_RT_OFFSET                   33914
#define PBF_REG_BTB_GUARANTEED_VOQ16_RT_OFFSET                      33915
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ16_RT_OFFSET               33916
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ17_RT_OFFSET                   33917
#define PBF_REG_BTB_GUARANTEED_VOQ17_RT_OFFSET                      33918
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ17_RT_OFFSET               33919
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ18_RT_OFFSET                   33920
#define PBF_REG_BTB_GUARANTEED_VOQ18_RT_OFFSET                      33921
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ18_RT_OFFSET               33922
#define PBF_REG_YCMD_QS_NUM_LINES_VOQ19_RT_OFFSET                   33923
#define PBF_REG_BTB_GUARANTEED_VOQ19_RT_OFFSET                      33924
#define PBF_REG_BTB_SHARED_AREA_SETUP_VOQ19_RT_OFFSET               33925
#define XCM_REG_CON_PHY_Q3_RT_OFFSET                                33926

#define RUNTIME_ARRAY_SIZE 33927

#endif /* __RT_DEFS_H__ */
