/*
 *   BSD LICENSE
 *
 *   Copyright (C) Cavium, Inc. 2016.
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
 *     * Neither the name of Cavium, Inc nor the names of its
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

#ifndef __THUNDERX_NICVF_BSVF_H__
#define __THUNDERX_NICVF_BSVF_H__

#include <sys/queue.h>

struct nicvf;

/**
 * The base queue structure to hold secondary qsets.
 */
struct svf_entry {
	STAILQ_ENTRY(svf_entry) next; /**< Next element's pointer */
	struct nicvf *vf;              /**< Holder of a secondary qset */
};

/**
 * Enqueue new entry to secondary qsets.
 *
 * @param entry
 *   Entry to be enqueued.
 */
void
nicvf_bsvf_push(struct svf_entry *entry);

/**
 * Dequeue an entry from secondary qsets.
 *
 * @return
 *   Dequeued entry.
 */
struct svf_entry *
nicvf_bsvf_pop(void);

/**
 * Check if the queue of secondary qsets is empty.
 *
 * @return
 *   0 on non-empty
 *   otherwise empty
 */
int
nicvf_bsvf_empty(void);

#endif /* __THUNDERX_NICVF_BSVF_H__  */
