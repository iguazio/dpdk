/*-
 *   BSD LICENSE
 *
 *   Copyright 2017 NXP.
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
 *     * Neither the name of NXP nor the names of its
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

#ifndef __DPAA_BITS_H
#define __DPAA_BITS_H

/* Bitfield stuff. */
#define BITS_PER_ULONG	(sizeof(unsigned long) << 3)
#define SHIFT_PER_ULONG	(((1 << 5) == BITS_PER_ULONG) ? 5 : 6)
#define BITS_MASK(idx)	(1UL << ((idx) & (BITS_PER_ULONG - 1)))
#define BITS_IDX(idx)	((idx) >> SHIFT_PER_ULONG)

static inline void dpaa_set_bits(unsigned long mask,
				 volatile unsigned long *p)
{
	*p |= mask;
}

static inline void dpaa_set_bit(int idx, volatile unsigned long *bits)
{
	dpaa_set_bits(BITS_MASK(idx), bits + BITS_IDX(idx));
}

static inline void dpaa_clear_bits(unsigned long mask,
				   volatile unsigned long *p)
{
	*p &= ~mask;
}

static inline void dpaa_clear_bit(int idx,
				  volatile unsigned long *bits)
{
	dpaa_clear_bits(BITS_MASK(idx), bits + BITS_IDX(idx));
}

#endif /* __DPAA_BITS_H */
