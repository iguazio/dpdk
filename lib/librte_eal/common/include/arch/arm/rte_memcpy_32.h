/*
 *   BSD LICENSE
 *
 *   Copyright(c) 2015 RehiveTech. All rights reserved.
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
 *     * Neither the name of RehiveTech nor the names of its
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

#ifndef _RTE_MEMCPY_ARM32_H_
#define _RTE_MEMCPY_ARM32_H_

#include <stdint.h>
#include <string.h>
/* ARM NEON Intrinsics are used to copy data */
#include <arm_neon.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "generic/rte_memcpy.h"

static inline void
rte_mov16(uint8_t *dst, const uint8_t *src)
{
	vst1q_u8(dst, vld1q_u8(src));
}

static inline void
rte_mov32(uint8_t *dst, const uint8_t *src)
{
	asm volatile (
		"vld1.8 {d0-d3}, [%0]\n\t"
		"vst1.8 {d0-d3}, [%1]\n\t"
		: "+r" (src), "+r" (dst)
		: : "memory", "d0", "d1", "d2", "d3");
}

static inline void
rte_mov48(uint8_t *dst, const uint8_t *src)
{
	asm volatile (
		"vld1.8 {d0-d3}, [%0]!\n\t"
		"vld1.8 {d4-d5}, [%0]\n\t"
		"vst1.8 {d0-d3}, [%1]!\n\t"
		"vst1.8 {d4-d5}, [%1]\n\t"
		: "+r" (src), "+r" (dst)
		:
		: "memory", "d0", "d1", "d2", "d3", "d4", "d5");
}

static inline void
rte_mov64(uint8_t *dst, const uint8_t *src)
{
	asm volatile (
		"vld1.8 {d0-d3}, [%0]!\n\t"
		"vld1.8 {d4-d7}, [%0]\n\t"
		"vst1.8 {d0-d3}, [%1]!\n\t"
		"vst1.8 {d4-d7}, [%1]\n\t"
		: "+r" (src), "+r" (dst)
		:
		: "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7");
}

static inline void
rte_mov128(uint8_t *dst, const uint8_t *src)
{
	asm volatile ("pld [%0, #64]" : : "r" (src));
	asm volatile (
		"vld1.8 {d0-d3},   [%0]!\n\t"
		"vld1.8 {d4-d7},   [%0]!\n\t"
		"vld1.8 {d8-d11},  [%0]!\n\t"
		"vld1.8 {d12-d15}, [%0]\n\t"
		"vst1.8 {d0-d3},   [%1]!\n\t"
		"vst1.8 {d4-d7},   [%1]!\n\t"
		"vst1.8 {d8-d11},  [%1]!\n\t"
		"vst1.8 {d12-d15}, [%1]\n\t"
		: "+r" (src), "+r" (dst)
		:
		: "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
		"d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15");
}

static inline void
rte_mov256(uint8_t *dst, const uint8_t *src)
{
	asm volatile ("pld [%0,  #64]" : : "r" (src));
	asm volatile ("pld [%0, #128]" : : "r" (src));
	asm volatile ("pld [%0, #192]" : : "r" (src));
	asm volatile ("pld [%0, #256]" : : "r" (src));
	asm volatile ("pld [%0, #320]" : : "r" (src));
	asm volatile ("pld [%0, #384]" : : "r" (src));
	asm volatile ("pld [%0, #448]" : : "r" (src));
	asm volatile (
		"vld1.8 {d0-d3},   [%0]!\n\t"
		"vld1.8 {d4-d7},   [%0]!\n\t"
		"vld1.8 {d8-d11},  [%0]!\n\t"
		"vld1.8 {d12-d15}, [%0]!\n\t"
		"vld1.8 {d16-d19}, [%0]!\n\t"
		"vld1.8 {d20-d23}, [%0]!\n\t"
		"vld1.8 {d24-d27}, [%0]!\n\t"
		"vld1.8 {d28-d31}, [%0]\n\t"
		"vst1.8 {d0-d3},   [%1]!\n\t"
		"vst1.8 {d4-d7},   [%1]!\n\t"
		"vst1.8 {d8-d11},  [%1]!\n\t"
		"vst1.8 {d12-d15}, [%1]!\n\t"
		"vst1.8 {d16-d19}, [%1]!\n\t"
		"vst1.8 {d20-d23}, [%1]!\n\t"
		"vst1.8 {d24-d27}, [%1]!\n\t"
		"vst1.8 {d28-d31}, [%1]!\n\t"
		: "+r" (src), "+r" (dst)
		:
		: "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
		"d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
		"d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
		"d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31");
}

#define rte_memcpy(dst, src, n)              \
	({ (__builtin_constant_p(n)) ?       \
	memcpy((dst), (src), (n)) :          \
	rte_memcpy_func((dst), (src), (n)); })

static inline void *
rte_memcpy_func(void *dst, const void *src, size_t n)
{
	void *ret = dst;

	/* We can't copy < 16 bytes using XMM registers so do it manually. */
	if (n < 16) {
		if (n & 0x01) {
			*(uint8_t *)dst = *(const uint8_t *)src;
			dst = (uint8_t *)dst + 1;
			src = (const uint8_t *)src + 1;
		}
		if (n & 0x02) {
			*(uint16_t *)dst = *(const uint16_t *)src;
			dst = (uint16_t *)dst + 1;
			src = (const uint16_t *)src + 1;
		}
		if (n & 0x04) {
			*(uint32_t *)dst = *(const uint32_t *)src;
			dst = (uint32_t *)dst + 1;
			src = (const uint32_t *)src + 1;
		}
		if (n & 0x08) {
			/* ARMv7 can not handle unaligned access to long long
			 * (uint64_t). Therefore two uint32_t operations are
			 * used.
			 */
			*(uint32_t *)dst = *(const uint32_t *)src;
			dst = (uint32_t *)dst + 1;
			src = (const uint32_t *)src + 1;
			*(uint32_t *)dst = *(const uint32_t *)src;
		}
		return ret;
	}

	/* Special fast cases for <= 128 bytes */
	if (n <= 32) {
		rte_mov16((uint8_t *)dst, (const uint8_t *)src);
		rte_mov16((uint8_t *)dst - 16 + n,
			(const uint8_t *)src - 16 + n);
		return ret;
	}

	if (n <= 64) {
		rte_mov32((uint8_t *)dst, (const uint8_t *)src);
		rte_mov32((uint8_t *)dst - 32 + n,
			(const uint8_t *)src - 32 + n);
		return ret;
	}

	if (n <= 128) {
		rte_mov64((uint8_t *)dst, (const uint8_t *)src);
		rte_mov64((uint8_t *)dst - 64 + n,
			(const uint8_t *)src - 64 + n);
		return ret;
	}

	/*
	 * For large copies > 128 bytes. This combination of 256, 64 and 16 byte
	 * copies was found to be faster than doing 128 and 32 byte copies as
	 * well.
	 */
	for ( ; n >= 256; n -= 256) {
		rte_mov256((uint8_t *)dst, (const uint8_t *)src);
		dst = (uint8_t *)dst + 256;
		src = (const uint8_t *)src + 256;
	}

	/*
	 * We split the remaining bytes (which will be less than 256) into
	 * 64byte (2^6) chunks.
	 * Using incrementing integers in the case labels of a switch statement
	 * enourages the compiler to use a jump table. To get incrementing
	 * integers, we shift the 2 relevant bits to the LSB position to first
	 * get decrementing integers, and then subtract.
	 */
	switch (3 - (n >> 6)) {
	case 0x00:
		rte_mov64((uint8_t *)dst, (const uint8_t *)src);
		n -= 64;
		dst = (uint8_t *)dst + 64;
		src = (const uint8_t *)src + 64;      /* fallthrough */
	case 0x01:
		rte_mov64((uint8_t *)dst, (const uint8_t *)src);
		n -= 64;
		dst = (uint8_t *)dst + 64;
		src = (const uint8_t *)src + 64;      /* fallthrough */
	case 0x02:
		rte_mov64((uint8_t *)dst, (const uint8_t *)src);
		n -= 64;
		dst = (uint8_t *)dst + 64;
		src = (const uint8_t *)src + 64;      /* fallthrough */
	default:
		break;
	}

	/*
	 * We split the remaining bytes (which will be less than 64) into
	 * 16byte (2^4) chunks, using the same switch structure as above.
	 */
	switch (3 - (n >> 4)) {
	case 0x00:
		rte_mov16((uint8_t *)dst, (const uint8_t *)src);
		n -= 16;
		dst = (uint8_t *)dst + 16;
		src = (const uint8_t *)src + 16;      /* fallthrough */
	case 0x01:
		rte_mov16((uint8_t *)dst, (const uint8_t *)src);
		n -= 16;
		dst = (uint8_t *)dst + 16;
		src = (const uint8_t *)src + 16;      /* fallthrough */
	case 0x02:
		rte_mov16((uint8_t *)dst, (const uint8_t *)src);
		n -= 16;
		dst = (uint8_t *)dst + 16;
		src = (const uint8_t *)src + 16;      /* fallthrough */
	default:
		break;
	}

	/* Copy any remaining bytes, without going beyond end of buffers */
	if (n != 0)
		rte_mov16((uint8_t *)dst - 16 + n,
			(const uint8_t *)src - 16 + n);
	return ret;
}

#ifdef __cplusplus
}
#endif

#endif /* _RTE_MEMCPY_ARM32_H_ */
