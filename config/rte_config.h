/*
 *   BSD LICENSE
 *
 *   Copyright(c) 2017 Intel Corporation.
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
 */

/**
 * @file Header file containing DPDK compilation parameters
 *
 * Header file containing DPDK compilation parameters. Also include the
 * meson-generated header file containing the detected parameters that
 * are variable across builds or build environments.
 *
 * NOTE: This file is only used for meson+ninja builds. For builds done
 * using make/gmake, the rte_config.h file is autogenerated from the
 * defconfig_* files in the config directory.
 */
#ifndef _RTE_CONFIG_H_
#define _RTE_CONFIG_H_

#include <rte_build_config.h>

/****** library defines ********/

/* EAL defines */
#define RTE_MAX_MEMSEG 512
#define RTE_MAX_MEMZONE 2560
#define RTE_MAX_TAILQ 32
#define RTE_LOG_LEVEL RTE_LOG_INFO
#define RTE_LOG_DP_LEVEL RTE_LOG_INFO
#define RTE_BACKTRACE 1
#define RTE_EAL_VFIO 1

/* mbuf defines */
#define RTE_MBUF_DEFAULT_MEMPOOL_OPS "ring_mp_mc"

#endif /* _RTE_CONFIG_H_ */
