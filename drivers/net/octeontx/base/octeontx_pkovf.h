/*
 *   BSD LICENSE
 *
 *   Copyright (C) Cavium Inc. 2017. All rights reserved.
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
 *     * Neither the name of Cavium networks nor the names of its
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

#ifndef	__OCTEONTX_PKO_H__
#define	__OCTEONTX_PKO_H__

/* PKO maximum constants */
#define	PKO_VF_MAX			(32)
#define	PKO_VF_NUM_DQ			(8)
#define PKO_MAX_NUM_DQ			(8)
#define	PKO_DQ_DRAIN_TO			(1000)

#define PKO_DQ_FC_SKID			(4)
#define PKO_DQ_FC_DEPTH_PAGES		(2048)
#define PKO_DQ_FC_STRIDE_16		(16)
#define PKO_DQ_FC_STRIDE_128		(128)
#define PKO_DQ_FC_STRIDE		PKO_DQ_FC_STRIDE_16

#define PKO_DQ_KIND_BIT			49
#define PKO_DQ_STATUS_BIT		60
#define PKO_DQ_OP_BIT			48

/* PKO VF register offsets from VF_BAR0 */
#define	PKO_VF_DQ_SW_XOFF(gdq)		(0x000100 | (gdq) << 17)
#define	PKO_VF_DQ_WM_CTL(gdq)		(0x000130 | (gdq) << 17)
#define	PKO_VF_DQ_WM_CNT(gdq)		(0x000150 | (gdq) << 17)
#define	PKO_VF_DQ_FC_CONFIG		(0x000160)
#define	PKO_VF_DQ_FC_STATUS(gdq)	(0x000168 | (gdq) << 17)
#define	PKO_VF_DQ_OP_SEND(gdq, op)	(0x001000 | (gdq) << 17 | (op) << 3)
#define	PKO_VF_DQ_OP_OPEN(gdq)		(0x001100 | (gdq) << 17)
#define	PKO_VF_DQ_OP_CLOSE(gdq)		(0x001200 | (gdq) << 17)
#define	PKO_VF_DQ_OP_QUERY(gdq)		(0x001300 | (gdq) << 17)

#endif /* __OCTEONTX_PKO_H__ */
