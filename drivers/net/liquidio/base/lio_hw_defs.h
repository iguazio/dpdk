/*
 *   BSD LICENSE
 *
 *   Copyright(c) 2017 Cavium, Inc.. All rights reserved.
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
 *     * Neither the name of Cavium, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER(S) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LIO_HW_DEFS_H_
#define _LIO_HW_DEFS_H_

#include <rte_io.h>

#ifndef PCI_VENDOR_ID_CAVIUM
#define PCI_VENDOR_ID_CAVIUM	0x177D
#endif

#define LIO_CN23XX_VF_VID	0x9712

#define LIO_DEVICE_NAME_LEN		32

/* Routines for reading and writing CSRs */
#ifdef RTE_LIBRTE_LIO_DEBUG_REGS
#define lio_write_csr(lio_dev, reg_off, value)				\
	do {								\
		typeof(lio_dev) _dev = lio_dev;				\
		typeof(reg_off) _reg_off = reg_off;			\
		typeof(value) _value = value;				\
		PMD_REGS_LOG(_dev,					\
			     "Write32: Reg: 0x%08lx Val: 0x%08lx\n",	\
			     (unsigned long)_reg_off,			\
			     (unsigned long)_value);			\
		rte_write32(_value, _dev->hw_addr + _reg_off);		\
	} while (0)

#define lio_write_csr64(lio_dev, reg_off, val64)			\
	do {								\
		typeof(lio_dev) _dev = lio_dev;				\
		typeof(reg_off) _reg_off = reg_off;			\
		typeof(val64) _val64 = val64;				\
		PMD_REGS_LOG(						\
		    _dev,						\
		    "Write64: Reg: 0x%08lx Val: 0x%016llx\n",		\
		    (unsigned long)_reg_off,				\
		    (unsigned long long)_val64);			\
		rte_write64(_val64, _dev->hw_addr + _reg_off);		\
	} while (0)

#define lio_read_csr(lio_dev, reg_off)					\
	({								\
		typeof(lio_dev) _dev = lio_dev;				\
		typeof(reg_off) _reg_off = reg_off;			\
		uint32_t val = rte_read32(_dev->hw_addr + _reg_off);	\
		PMD_REGS_LOG(_dev,					\
			     "Read32: Reg: 0x%08lx Val: 0x%08lx\n",	\
			     (unsigned long)_reg_off,			\
			     (unsigned long)val);			\
		val;							\
	})

#define lio_read_csr64(lio_dev, reg_off)				\
	({								\
		typeof(lio_dev) _dev = lio_dev;				\
		typeof(reg_off) _reg_off = reg_off;			\
		uint64_t val64 = rte_read64(_dev->hw_addr + _reg_off);	\
		PMD_REGS_LOG(						\
		    _dev,						\
		    "Read64: Reg: 0x%08lx Val: 0x%016llx\n",		\
		    (unsigned long)_reg_off,				\
		    (unsigned long long)val64);				\
		val64;							\
	})
#else
#define lio_write_csr(lio_dev, reg_off, value)				\
	rte_write32(value, (lio_dev)->hw_addr + (reg_off))

#define lio_write_csr64(lio_dev, reg_off, val64)			\
	rte_write64(val64, (lio_dev)->hw_addr + (reg_off))

#define lio_read_csr(lio_dev, reg_off)					\
	rte_read32((lio_dev)->hw_addr + (reg_off))

#define lio_read_csr64(lio_dev, reg_off)				\
	rte_read64((lio_dev)->hw_addr + (reg_off))
#endif
#endif /* _LIO_HW_DEFS_H_ */
