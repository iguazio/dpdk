/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2016-2017 Intel Corporation. All rights reserved.
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

#include <rte_malloc.h>
#include <rte_cycles.h>
#include <rte_crypto.h>
#include <rte_cryptodev.h>

#include "cperf_test_throughput.h"
#include "cperf_ops.h"
#include "cperf_test_common.h"

struct cperf_throughput_ctx {
	uint8_t dev_id;
	uint16_t qp_id;
	uint8_t lcore_id;

	struct rte_mempool *pkt_mbuf_pool_in;
	struct rte_mempool *pkt_mbuf_pool_out;
	struct rte_mbuf **mbufs_in;
	struct rte_mbuf **mbufs_out;

	struct rte_mempool *crypto_op_pool;

	struct rte_cryptodev_sym_session *sess;

	cperf_populate_ops_t populate_ops;

	const struct cperf_options *options;
	const struct cperf_test_vector *test_vector;
};

static void
cperf_throughput_test_free(struct cperf_throughput_ctx *ctx)
{
	if (ctx) {
		if (ctx->sess) {
			rte_cryptodev_sym_session_clear(ctx->dev_id, ctx->sess);
			rte_cryptodev_sym_session_free(ctx->sess);
		}

		cperf_free_common_memory(ctx->options,
				ctx->pkt_mbuf_pool_in,
				ctx->pkt_mbuf_pool_out,
				ctx->mbufs_in, ctx->mbufs_out,
				ctx->crypto_op_pool);

		rte_free(ctx);
	}
}

void *
cperf_throughput_test_constructor(struct rte_mempool *sess_mp,
		uint8_t dev_id, uint16_t qp_id,
		const struct cperf_options *options,
		const struct cperf_test_vector *test_vector,
		const struct cperf_op_fns *op_fns)
{
	struct cperf_throughput_ctx *ctx = NULL;

	ctx = rte_malloc(NULL, sizeof(struct cperf_throughput_ctx), 0);
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

	ctx->sess = op_fns->sess_create(sess_mp, dev_id, options, test_vector,
					iv_offset);
	if (ctx->sess == NULL)
		goto err;

	if (cperf_alloc_common_memory(options, test_vector, dev_id, 0,
			&ctx->pkt_mbuf_pool_in, &ctx->pkt_mbuf_pool_out,
			&ctx->mbufs_in, &ctx->mbufs_out,
			&ctx->crypto_op_pool) < 0)
		goto err;

	return ctx;
err:
	cperf_throughput_test_free(ctx);

	return NULL;
}

int
cperf_throughput_test_runner(void *test_ctx)
{
	struct cperf_throughput_ctx *ctx = test_ctx;
	uint16_t test_burst_size;
	uint8_t burst_size_idx = 0;

	static int only_once;

	struct rte_crypto_op *ops[ctx->options->max_burst_size];
	struct rte_crypto_op *ops_processed[ctx->options->max_burst_size];
	uint64_t i;

	uint32_t lcore = rte_lcore_id();

#ifdef CPERF_LINEARIZATION_ENABLE
	struct rte_cryptodev_info dev_info;
	int linearize = 0;

	/* Check if source mbufs require coalescing */
	if (ctx->options->segment_sz < ctx->options->max_buffer_size) {
		rte_cryptodev_info_get(ctx->dev_id, &dev_info);
		if ((dev_info.feature_flags &
				RTE_CRYPTODEV_FF_MBUF_SCATTER_GATHER) == 0)
			linearize = 1;
	}
#endif /* CPERF_LINEARIZATION_ENABLE */

	ctx->lcore_id = lcore;

	/* Warm up the host CPU before starting the test */
	for (i = 0; i < ctx->options->total_ops; i++)
		rte_cryptodev_enqueue_burst(ctx->dev_id, ctx->qp_id, NULL, 0);

	/* Get first size from range or list */
	if (ctx->options->inc_burst_size != 0)
		test_burst_size = ctx->options->min_burst_size;
	else
		test_burst_size = ctx->options->burst_size_list[0];

	uint16_t iv_offset = sizeof(struct rte_crypto_op) +
		sizeof(struct rte_crypto_sym_op);

	while (test_burst_size <= ctx->options->max_burst_size) {
		uint64_t ops_enqd = 0, ops_enqd_total = 0, ops_enqd_failed = 0;
		uint64_t ops_deqd = 0, ops_deqd_total = 0, ops_deqd_failed = 0;

		uint64_t m_idx = 0, tsc_start, tsc_end, tsc_duration;

		uint16_t ops_unused = 0;

		tsc_start = rte_rdtsc_precise();

		while (ops_enqd_total < ctx->options->total_ops) {

			uint16_t burst_size = ((ops_enqd_total + test_burst_size)
					<= ctx->options->total_ops) ?
							test_burst_size :
							ctx->options->total_ops -
							ops_enqd_total;

			uint16_t ops_needed = burst_size - ops_unused;

			/* Allocate crypto ops from pool */
			if (ops_needed != rte_crypto_op_bulk_alloc(
					ctx->crypto_op_pool,
					RTE_CRYPTO_OP_TYPE_SYMMETRIC,
					ops, ops_needed)) {
				RTE_LOG(ERR, USER1,
					"Failed to allocate more crypto operations "
					"from the the crypto operation pool.\n"
					"Consider increasing the pool size "
					"with --pool-sz\n");
				return -1;
			}

			/* Setup crypto op, attach mbuf etc */
			(ctx->populate_ops)(ops, &ctx->mbufs_in[m_idx],
					&ctx->mbufs_out[m_idx],
					ops_needed, ctx->sess, ctx->options,
					ctx->test_vector, iv_offset);

			/**
			 * When ops_needed is smaller than ops_enqd, the
			 * unused ops need to be moved to the front for
			 * next round use.
			 */
			if (unlikely(ops_enqd > ops_needed)) {
				size_t nb_b_to_mov = ops_unused * sizeof(
						struct rte_crypto_op *);

				memmove(&ops[ops_needed], &ops[ops_enqd],
					nb_b_to_mov);
			}

#ifdef CPERF_LINEARIZATION_ENABLE
			if (linearize) {
				/* PMD doesn't support scatter-gather and source buffer
				 * is segmented.
				 * We need to linearize it before enqueuing.
				 */
				for (i = 0; i < burst_size; i++)
					rte_pktmbuf_linearize(ops[i]->sym->m_src);
			}
#endif /* CPERF_LINEARIZATION_ENABLE */

			/* Enqueue burst of ops on crypto device */
			ops_enqd = rte_cryptodev_enqueue_burst(ctx->dev_id, ctx->qp_id,
					ops, burst_size);
			if (ops_enqd < burst_size)
				ops_enqd_failed++;

			/**
			 * Calculate number of ops not enqueued (mainly for hw
			 * accelerators whose ingress queue can fill up).
			 */
			ops_unused = burst_size - ops_enqd;
			ops_enqd_total += ops_enqd;


			/* Dequeue processed burst of ops from crypto device */
			ops_deqd = rte_cryptodev_dequeue_burst(ctx->dev_id, ctx->qp_id,
					ops_processed, test_burst_size);

			if (likely(ops_deqd))  {
				/* free crypto ops so they can be reused. We don't free
				 * the mbufs here as we don't want to reuse them as
				 * the crypto operation will change the data and cause
				 * failures.
				 */
				rte_mempool_put_bulk(ctx->crypto_op_pool,
						(void **)ops_processed, ops_deqd);

				ops_deqd_total += ops_deqd;
			} else {
				/**
				 * Count dequeue polls which didn't return any
				 * processed operations. This statistic is mainly
				 * relevant to hw accelerators.
				 */
				ops_deqd_failed++;
			}

			m_idx += ops_needed;
			m_idx = m_idx + test_burst_size > ctx->options->pool_sz ?
					0 : m_idx;
		}

		/* Dequeue any operations still in the crypto device */

		while (ops_deqd_total < ctx->options->total_ops) {
			/* Sending 0 length burst to flush sw crypto device */
			rte_cryptodev_enqueue_burst(ctx->dev_id, ctx->qp_id, NULL, 0);

			/* dequeue burst */
			ops_deqd = rte_cryptodev_dequeue_burst(ctx->dev_id, ctx->qp_id,
					ops_processed, test_burst_size);
			if (ops_deqd == 0)
				ops_deqd_failed++;
			else {
				rte_mempool_put_bulk(ctx->crypto_op_pool,
						(void **)ops_processed, ops_deqd);

				ops_deqd_total += ops_deqd;
			}
		}

		tsc_end = rte_rdtsc_precise();
		tsc_duration = (tsc_end - tsc_start);

		/* Calculate average operations processed per second */
		double ops_per_second = ((double)ctx->options->total_ops /
				tsc_duration) * rte_get_tsc_hz();

		/* Calculate average throughput (Gbps) in bits per second */
		double throughput_gbps = ((ops_per_second *
				ctx->options->test_buffer_size * 8) / 1000000000);

		/* Calculate average cycles per packet */
		double cycles_per_packet = ((double)tsc_duration /
				ctx->options->total_ops);

		if (!ctx->options->csv) {
			if (!only_once)
				printf("%12s%12s%12s%12s%12s%12s%12s%12s%12s%12s\n\n",
					"lcore id", "Buf Size", "Burst Size",
					"Enqueued", "Dequeued", "Failed Enq",
					"Failed Deq", "MOps", "Gbps",
					"Cycles/Buf");
			only_once = 1;

			printf("%12u%12u%12u%12"PRIu64"%12"PRIu64"%12"PRIu64
					"%12"PRIu64"%12.4f%12.4f%12.2f\n",
					ctx->lcore_id,
					ctx->options->test_buffer_size,
					test_burst_size,
					ops_enqd_total,
					ops_deqd_total,
					ops_enqd_failed,
					ops_deqd_failed,
					ops_per_second/1000000,
					throughput_gbps,
					cycles_per_packet);
		} else {
			if (!only_once)
				printf("#lcore id,Buffer Size(B),"
					"Burst Size,Enqueued,Dequeued,Failed Enq,"
					"Failed Deq,Ops(Millions),Throughput(Gbps),"
					"Cycles/Buf\n\n");
			only_once = 1;

			printf("%u;%u;%u;%"PRIu64";%"PRIu64";%"PRIu64";%"PRIu64";"
					"%.3f;%.3f;%.3f\n",
					ctx->lcore_id,
					ctx->options->test_buffer_size,
					test_burst_size,
					ops_enqd_total,
					ops_deqd_total,
					ops_enqd_failed,
					ops_deqd_failed,
					ops_per_second/1000000,
					throughput_gbps,
					cycles_per_packet);
		}

		/* Get next size from range or list */
		if (ctx->options->inc_burst_size != 0)
			test_burst_size += ctx->options->inc_burst_size;
		else {
			if (++burst_size_idx == ctx->options->burst_size_count)
				break;
			test_burst_size = ctx->options->burst_size_list[burst_size_idx];
		}

	}

	return 0;
}


void
cperf_throughput_test_destructor(void *arg)
{
	struct cperf_throughput_ctx *ctx = arg;

	if (ctx == NULL)
		return;

	rte_cryptodev_stop(ctx->dev_id);

	cperf_throughput_test_free(ctx);
}
