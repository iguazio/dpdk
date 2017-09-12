/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2017 Intel Corporation. All rights reserved.
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

#include <stdbool.h>

#include <rte_crypto.h>
#include <rte_cryptodev.h>
#include <rte_cycles.h>
#include <rte_malloc.h>

#include "cperf_ops.h"
#include "cperf_test_pmd_cyclecount.h"

#define PRETTY_HDR_FMT "%12s%12s%12s%12s%12s%12s%12s%12s%12s%12s\n\n"
#define PRETTY_LINE_FMT "%12u%12u%12u%12u%12u%12u%12u%12.0f%12.0f%12.0f\n"
#define CSV_HDR_FMT "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n"
#define CSV_LINE_FMT "%10u;%10u;%u;%u;%u;%u;%u;%.f3;%.f3;%.f3\n"

struct cperf_pmd_cyclecount_ctx {
	uint8_t dev_id;
	uint16_t qp_id;
	uint8_t lcore_id;

	struct rte_mempool *pkt_mbuf_pool_in;
	struct rte_mempool *pkt_mbuf_pool_out;
	struct rte_mbuf **mbufs_in;
	struct rte_mbuf **mbufs_out;

	struct rte_mempool *crypto_op_pool;
	struct rte_crypto_op **ops;
	struct rte_crypto_op **ops_processed;

	struct rte_cryptodev_sym_session *sess;

	cperf_populate_ops_t populate_ops;

	const struct cperf_options *options;
	const struct cperf_test_vector *test_vector;
};

struct pmd_cyclecount_state {
	struct cperf_pmd_cyclecount_ctx *ctx;
	const struct cperf_options *opts;
	uint32_t lcore;
	uint64_t delay;
	int linearize;
	uint32_t ops_enqd;
	uint32_t ops_deqd;
	uint32_t ops_enq_retries;
	uint32_t ops_deq_retries;
	double cycles_per_build;
	double cycles_per_enq;
	double cycles_per_deq;
};

static const uint16_t iv_offset =
		sizeof(struct rte_crypto_op) + sizeof(struct rte_crypto_sym_op);

static void
cperf_pmd_cyclecount_test_free(struct cperf_pmd_cyclecount_ctx *ctx,
		uint32_t mbuf_nb)
{
	uint32_t i;

	if (ctx) {
		if (ctx->sess) {
			rte_cryptodev_sym_session_clear(ctx->dev_id, ctx->sess);
			rte_cryptodev_sym_session_free(ctx->sess);
		}

		if (ctx->mbufs_in) {
			for (i = 0; i < mbuf_nb; i++)
				rte_pktmbuf_free(ctx->mbufs_in[i]);

			rte_free(ctx->mbufs_in);
		}

		if (ctx->mbufs_out) {
			for (i = 0; i < mbuf_nb; i++) {
				if (ctx->mbufs_out[i] != NULL)
					rte_pktmbuf_free(ctx->mbufs_out[i]);
			}

			rte_free(ctx->mbufs_out);
		}

		if (ctx->pkt_mbuf_pool_in)
			rte_mempool_free(ctx->pkt_mbuf_pool_in);

		if (ctx->pkt_mbuf_pool_out)
			rte_mempool_free(ctx->pkt_mbuf_pool_out);

		if (ctx->ops)
			rte_free(ctx->ops);

		if (ctx->ops_processed)
			rte_free(ctx->ops_processed);

		if (ctx->crypto_op_pool)
			rte_mempool_free(ctx->crypto_op_pool);

		rte_free(ctx);
	}
}

static struct rte_mbuf *
cperf_mbuf_create(struct rte_mempool *mempool, uint32_t segments_nb,
		const struct cperf_options *options,
		const struct cperf_test_vector *test_vector)
{
	struct rte_mbuf *mbuf;
	uint32_t segment_sz = options->max_buffer_size / segments_nb;
	uint32_t last_sz = options->max_buffer_size % segments_nb;
	uint8_t *mbuf_data;
	uint8_t *test_data =
			(options->cipher_op == RTE_CRYPTO_CIPHER_OP_ENCRYPT) ?
			test_vector->plaintext.data :
			test_vector->ciphertext.data;

	mbuf = rte_pktmbuf_alloc(mempool);
	if (mbuf == NULL)
		goto error;

	mbuf_data = (uint8_t *)rte_pktmbuf_append(mbuf, segment_sz);
	if (mbuf_data == NULL)
		goto error;

	memcpy(mbuf_data, test_data, segment_sz);
	test_data += segment_sz;
	segments_nb--;

	while (segments_nb) {
		struct rte_mbuf *m;

		m = rte_pktmbuf_alloc(mempool);
		if (m == NULL)
			goto error;

		rte_pktmbuf_chain(mbuf, m);

		mbuf_data = (uint8_t *)rte_pktmbuf_append(mbuf, segment_sz);
		if (mbuf_data == NULL)
			goto error;

		memcpy(mbuf_data, test_data, segment_sz);
		test_data += segment_sz;
		segments_nb--;
	}

	if (last_sz) {
		mbuf_data = (uint8_t *)rte_pktmbuf_append(mbuf, last_sz);
		if (mbuf_data == NULL)
			goto error;

		memcpy(mbuf_data, test_data, last_sz);
	}

	if (options->op_type != CPERF_CIPHER_ONLY) {
		mbuf_data = (uint8_t *)rte_pktmbuf_append(
				mbuf, options->digest_sz);
		if (mbuf_data == NULL)
			goto error;
	}

	if (options->op_type == CPERF_AEAD) {
		uint8_t *aead = (uint8_t *)rte_pktmbuf_prepend(
				mbuf, RTE_ALIGN_CEIL(options->aead_aad_sz, 16));

		if (aead == NULL)
			goto error;

		memcpy(aead, test_vector->aad.data, test_vector->aad.length);
	}

	return mbuf;
error:
	if (mbuf != NULL)
		rte_pktmbuf_free(mbuf);

	return NULL;
}

void *
cperf_pmd_cyclecount_test_constructor(struct rte_mempool *sess_mp,
		uint8_t dev_id, uint16_t qp_id,
		const struct cperf_options *options,
		const struct cperf_test_vector *test_vector,
		const struct cperf_op_fns *op_fns)
{
	struct cperf_pmd_cyclecount_ctx *ctx = NULL;
	unsigned int mbuf_idx = 0;
	char pool_name[32] = "";
	uint16_t dataroom_sz = RTE_PKTMBUF_HEADROOM +
			RTE_CACHE_LINE_ROUNDUP(
					(options->max_buffer_size /
							options->segments_nb) +
					(options->max_buffer_size %
							options->segments_nb) +
					options->digest_sz);

	/* preallocate buffers for crypto ops as they can get quite big */
	size_t alloc_sz = sizeof(struct rte_crypto_op *) *
			options->nb_descriptors;

	ctx = rte_malloc(NULL, sizeof(struct cperf_pmd_cyclecount_ctx), 0);
	if (ctx == NULL)
		goto err;

	ctx->dev_id = dev_id;
	ctx->qp_id = qp_id;

	ctx->populate_ops = op_fns->populate_ops;
	ctx->options = options;
	ctx->test_vector = test_vector;

	/* IV goes at the end of the crypto operation */
	uint16_t iv_offset = sizeof(struct rte_crypto_op) +
			sizeof(struct rte_crypto_sym_op);

	ctx->sess = op_fns->sess_create(
			sess_mp, dev_id, options, test_vector, iv_offset);
	if (ctx->sess == NULL)
		goto err;

	snprintf(pool_name, sizeof(pool_name), "cperf_pool_in_cdev_%d", dev_id);

	ctx->pkt_mbuf_pool_in = rte_pktmbuf_pool_create(pool_name,
			options->pool_sz * options->segments_nb, 0, 0,
			dataroom_sz, rte_socket_id());

	if (ctx->pkt_mbuf_pool_in == NULL)
		goto err;

	/* Generate mbufs_in with plaintext populated for test */
	ctx->mbufs_in = rte_malloc(NULL,
			(sizeof(struct rte_mbuf *) * options->pool_sz), 0);

	for (mbuf_idx = 0; mbuf_idx < options->pool_sz; mbuf_idx++) {
		ctx->mbufs_in[mbuf_idx] = cperf_mbuf_create(
				ctx->pkt_mbuf_pool_in, options->segments_nb,
				options, test_vector);
		if (ctx->mbufs_in[mbuf_idx] == NULL)
			goto err;
	}

	if (options->out_of_place == 1) {
		snprintf(pool_name, sizeof(pool_name), "cperf_pool_out_cdev_%d",
				dev_id);

		ctx->pkt_mbuf_pool_out = rte_pktmbuf_pool_create(pool_name,
				options->pool_sz, 0, 0, dataroom_sz,
				rte_socket_id());

		if (ctx->pkt_mbuf_pool_out == NULL)
			goto err;
	}

	ctx->mbufs_out = rte_malloc(NULL,
			(sizeof(struct rte_mbuf *) * options->pool_sz), 0);

	for (mbuf_idx = 0; mbuf_idx < options->pool_sz; mbuf_idx++) {
		if (options->out_of_place == 1) {
			ctx->mbufs_out[mbuf_idx] = cperf_mbuf_create(
					ctx->pkt_mbuf_pool_out, 1, options,
					test_vector);
			if (ctx->mbufs_out[mbuf_idx] == NULL)
				goto err;
		} else {
			ctx->mbufs_out[mbuf_idx] = NULL;
		}
	}

	snprintf(pool_name, sizeof(pool_name), "cperf_op_pool_cdev_%d", dev_id);

	uint16_t priv_size = test_vector->cipher_iv.length +
			test_vector->auth_iv.length +
			test_vector->aead_iv.length;

	ctx->crypto_op_pool = rte_crypto_op_pool_create(pool_name,
			RTE_CRYPTO_OP_TYPE_SYMMETRIC, options->pool_sz, 512,
			priv_size, rte_socket_id());
	if (ctx->crypto_op_pool == NULL)
		goto err;

	ctx->ops = rte_malloc("ops", alloc_sz, 0);
	if (!ctx->ops)
		goto err;

	ctx->ops_processed = rte_malloc("ops_processed", alloc_sz, 0);
	if (!ctx->ops_processed)
		goto err;

	return ctx;

err:
	cperf_pmd_cyclecount_test_free(ctx, mbuf_idx);

	return NULL;
}

/* benchmark alloc-build-free of ops */
static inline int
pmd_cyclecount_bench_ops(struct pmd_cyclecount_state *state, uint32_t cur_op,
		uint16_t test_burst_size)
{
	uint32_t iter_ops_left = state->opts->total_ops - cur_op;
	uint32_t iter_ops_needed =
			RTE_MIN(state->opts->nb_descriptors, iter_ops_left);
	uint32_t cur_iter_op;

	for (cur_iter_op = 0; cur_iter_op < iter_ops_needed;
			cur_iter_op += test_burst_size) {
		uint32_t burst_size = RTE_MIN(state->opts->total_ops - cur_op,
				test_burst_size);
		struct rte_crypto_op **ops = &state->ctx->ops[cur_iter_op];

		if (burst_size != rte_crypto_op_bulk_alloc(
				state->ctx->crypto_op_pool,
				RTE_CRYPTO_OP_TYPE_SYMMETRIC,
				ops, burst_size))
			return -1;

		/* Setup crypto op, attach mbuf etc */
		(state->ctx->populate_ops)(ops,
				&state->ctx->mbufs_in[cur_iter_op],
				&state->ctx->mbufs_out[cur_iter_op], burst_size,
				state->ctx->sess, state->opts,
				state->ctx->test_vector, iv_offset);

#ifdef CPERF_LINEARIZATION_ENABLE
		/* Check if source mbufs require coalescing */
		if (state->linearize) {
			uint8_t i;
			for (i = 0; i < burst_size; i++) {
				struct rte_mbuf *src = ops[i]->sym->m_src;
				rte_pktmbuf_linearize(src);
			}
		}
#endif /* CPERF_LINEARIZATION_ENABLE */
		rte_mempool_put_bulk(state->ctx->crypto_op_pool, (void **)ops,
				burst_size);
	}

	return 0;
}

/* allocate and build ops (no free) */
static int
pmd_cyclecount_build_ops(struct pmd_cyclecount_state *state,
		uint32_t iter_ops_needed, uint16_t test_burst_size)
{
	uint32_t cur_iter_op;

	for (cur_iter_op = 0; cur_iter_op < iter_ops_needed;
			cur_iter_op += test_burst_size) {
		uint32_t burst_size = RTE_MIN(
				iter_ops_needed - cur_iter_op, test_burst_size);
		struct rte_crypto_op **ops = &state->ctx->ops[cur_iter_op];

		if (burst_size != rte_crypto_op_bulk_alloc(
				state->ctx->crypto_op_pool,
				RTE_CRYPTO_OP_TYPE_SYMMETRIC,
				ops, burst_size))
			return -1;

		/* Setup crypto op, attach mbuf etc */
		(state->ctx->populate_ops)(ops,
				&state->ctx->mbufs_in[cur_iter_op],
				&state->ctx->mbufs_out[cur_iter_op], burst_size,
				state->ctx->sess, state->opts,
				state->ctx->test_vector, iv_offset);
	}
	return 0;
}

/* benchmark enqueue, returns number of ops enqueued */
static uint32_t
pmd_cyclecount_bench_enq(struct pmd_cyclecount_state *state,
		uint32_t iter_ops_needed, uint16_t test_burst_size)
{
	/* Enqueue full descriptor ring of ops on crypto device */
	uint32_t cur_iter_op = 0;
	while (cur_iter_op < iter_ops_needed) {
		uint32_t burst_size = RTE_MIN(iter_ops_needed - cur_iter_op,
				test_burst_size);
		struct rte_crypto_op **ops = &state->ctx->ops[cur_iter_op];
		uint32_t burst_enqd;

		burst_enqd = rte_cryptodev_enqueue_burst(state->ctx->dev_id,
				state->ctx->qp_id, ops, burst_size);

		/* if we couldn't enqueue anything, the queue is full */
		if (!burst_enqd) {
			/* don't try to dequeue anything we didn't enqueue */
			return cur_iter_op;
		}

		if (burst_enqd < burst_size)
			state->ops_enq_retries++;
		state->ops_enqd += burst_enqd;
		cur_iter_op += burst_enqd;
	}
	return iter_ops_needed;
}

/* benchmark dequeue */
static void
pmd_cyclecount_bench_deq(struct pmd_cyclecount_state *state,
		uint32_t iter_ops_needed, uint16_t test_burst_size)
{
	/* Dequeue full descriptor ring of ops on crypto device */
	uint32_t cur_iter_op = 0;
	while (cur_iter_op < iter_ops_needed) {
		uint32_t burst_size = RTE_MIN(iter_ops_needed - cur_iter_op,
				test_burst_size);
		struct rte_crypto_op **ops_processed =
				&state->ctx->ops[cur_iter_op];
		uint32_t burst_deqd;

		burst_deqd = rte_cryptodev_dequeue_burst(state->ctx->dev_id,
				state->ctx->qp_id, ops_processed, burst_size);

		if (burst_deqd < burst_size)
			state->ops_deq_retries++;
		state->ops_deqd += burst_deqd;
		cur_iter_op += burst_deqd;
	}
}

/* run benchmark per burst size */
static inline int
pmd_cyclecount_bench_burst_sz(
		struct pmd_cyclecount_state *state, uint16_t test_burst_size)
{
	uint64_t tsc_start;
	uint64_t tsc_end;
	uint64_t tsc_op;
	uint64_t tsc_enq;
	uint64_t tsc_deq;
	uint32_t cur_op;

	/* reset all counters */
	tsc_enq = 0;
	tsc_deq = 0;
	state->ops_enqd = 0;
	state->ops_enq_retries = 0;
	state->ops_deqd = 0;
	state->ops_deq_retries = 0;

	/*
	 * Benchmark crypto op alloc-build-free separately.
	 */
	tsc_start = rte_rdtsc_precise();

	for (cur_op = 0; cur_op < state->opts->total_ops;
			cur_op += state->opts->nb_descriptors) {
		if (unlikely(pmd_cyclecount_bench_ops(
				state, cur_op, test_burst_size)))
			return -1;
	}

	tsc_end = rte_rdtsc_precise();
	tsc_op = tsc_end - tsc_start;


	/*
	 * Hardware acceleration cyclecount benchmarking loop.
	 *
	 * We're benchmarking raw enq/deq performance by filling up the device
	 * queue, so we never get any failed enqs unless the driver won't accept
	 * the exact number of descriptors we requested, or the driver won't
	 * wrap around the end of the TX ring. However, since we're only
	 * dequeueing once we've filled up the queue, we have to benchmark it
	 * piecemeal and then average out the results.
	 */
	cur_op = 0;
	while (cur_op < state->opts->total_ops) {
		uint32_t iter_ops_left = state->opts->total_ops - cur_op;
		uint32_t iter_ops_needed = RTE_MIN(
				state->opts->nb_descriptors, iter_ops_left);
		uint32_t iter_ops_allocd = iter_ops_needed;

		/* allocate and build ops */
		if (unlikely(pmd_cyclecount_build_ops(state, iter_ops_needed,
				test_burst_size)))
			return -1;

		tsc_start = rte_rdtsc_precise();

		/* fill up TX ring */
		iter_ops_needed = pmd_cyclecount_bench_enq(state,
				iter_ops_needed, test_burst_size);

		tsc_end = rte_rdtsc_precise();

		tsc_enq += tsc_end - tsc_start;

		/* allow for HW to catch up */
		if (state->delay)
			rte_delay_us_block(state->delay);

		tsc_start = rte_rdtsc_precise();

		/* drain RX ring */
		pmd_cyclecount_bench_deq(state, iter_ops_needed,
				test_burst_size);

		tsc_end = rte_rdtsc_precise();

		tsc_deq += tsc_end - tsc_start;

		cur_op += iter_ops_needed;

		/*
		 * we may not have processed all ops that we allocated, so
		 * free everything we've allocated.
		 */
		rte_mempool_put_bulk(state->ctx->crypto_op_pool,
				(void **)state->ctx->ops, iter_ops_allocd);
	}

	state->cycles_per_build = (double)tsc_op / state->opts->total_ops;
	state->cycles_per_enq = (double)tsc_enq / state->ops_enqd;
	state->cycles_per_deq = (double)tsc_deq / state->ops_deqd;

	return 0;
}

int
cperf_pmd_cyclecount_test_runner(void *test_ctx)
{
	struct pmd_cyclecount_state state = {0};
	const struct cperf_options *opts;
	uint16_t test_burst_size;
	uint8_t burst_size_idx = 0;

	state.ctx = test_ctx;
	opts = state.ctx->options;
	state.opts = opts;
	state.lcore = rte_lcore_id();
	state.linearize = 0;

	static int only_once;
	static bool warmup = true;

	/*
	 * We need a small delay to allow for hardware to process all the crypto
	 * operations. We can't automatically figure out what the delay should
	 * be, so we leave it up to the user (by default it's 0).
	 */
	state.delay = 1000 * opts->pmdcc_delay;

#ifdef CPERF_LINEARIZATION_ENABLE
	struct rte_cryptodev_info dev_info;

	/* Check if source mbufs require coalescing */
	if (opts->segments_nb > 1) {
		rte_cryptodev_info_get(state.ctx->dev_id, &dev_info);
		if ((dev_info.feature_flags &
				    RTE_CRYPTODEV_FF_MBUF_SCATTER_GATHER) ==
				0) {
			state.linearize = 1;
		}
	}
#endif /* CPERF_LINEARIZATION_ENABLE */

	state.ctx->lcore_id = state.lcore;

	/* Get first size from range or list */
	if (opts->inc_burst_size != 0)
		test_burst_size = opts->min_burst_size;
	else
		test_burst_size = opts->burst_size_list[0];

	while (test_burst_size <= opts->max_burst_size) {
		/* do a benchmark run */
		if (pmd_cyclecount_bench_burst_sz(&state, test_burst_size))
			return -1;

		/*
		 * First run is always a warm up run.
		 */
		if (warmup) {
			warmup = false;
			continue;
		}

		if (!opts->csv) {
			if (!only_once)
				printf(PRETTY_HDR_FMT, "lcore id", "Buf Size",
						"Burst Size", "Enqueued",
						"Dequeued", "Enq Retries",
						"Deq Retries", "Cycles/Op",
						"Cycles/Enq", "Cycles/Deq");
			only_once = 1;

			printf(PRETTY_LINE_FMT, state.ctx->lcore_id,
					opts->test_buffer_size, test_burst_size,
					state.ops_enqd, state.ops_deqd,
					state.ops_enq_retries,
					state.ops_deq_retries,
					state.cycles_per_build,
					state.cycles_per_enq,
					state.cycles_per_deq);
		} else {
			if (!only_once)
				printf(CSV_HDR_FMT, "# lcore id", "Buf Size",
						"Burst Size", "Enqueued",
						"Dequeued", "Enq Retries",
						"Deq Retries", "Cycles/Op",
						"Cycles/Enq", "Cycles/Deq");
			only_once = 1;

			printf(CSV_LINE_FMT, state.ctx->lcore_id,
					opts->test_buffer_size, test_burst_size,
					state.ops_enqd, state.ops_deqd,
					state.ops_enq_retries,
					state.ops_deq_retries,
					state.cycles_per_build,
					state.cycles_per_enq,
					state.cycles_per_deq);
		}

		/* Get next size from range or list */
		if (opts->inc_burst_size != 0)
			test_burst_size += opts->inc_burst_size;
		else {
			if (++burst_size_idx == opts->burst_size_count)
				break;
			test_burst_size = opts->burst_size_list[burst_size_idx];
		}
	}

	return 0;
}

void
cperf_pmd_cyclecount_test_destructor(void *arg)
{
	struct cperf_pmd_cyclecount_ctx *ctx = arg;

	if (ctx == NULL)
		return;

	cperf_pmd_cyclecount_test_free(ctx, ctx->options->pool_sz);
}
