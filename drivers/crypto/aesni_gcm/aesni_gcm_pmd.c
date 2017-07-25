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

#include <rte_common.h>
#include <rte_config.h>
#include <rte_hexdump.h>
#include <rte_cryptodev.h>
#include <rte_cryptodev_pmd.h>
#include <rte_cryptodev_vdev.h>
#include <rte_vdev.h>
#include <rte_malloc.h>
#include <rte_cpuflags.h>
#include <rte_byteorder.h>

#include "aesni_gcm_pmd_private.h"

static uint8_t cryptodev_driver_id;

/** Parse crypto xform chain and set private session parameters */
int
aesni_gcm_set_session_parameters(const struct aesni_gcm_ops *gcm_ops,
		struct aesni_gcm_session *sess,
		const struct rte_crypto_sym_xform *xform)
{
	const struct rte_crypto_sym_xform *auth_xform;
	const struct rte_crypto_sym_xform *aead_xform;
	uint16_t digest_length;
	uint8_t key_length;
	uint8_t *key;

	/* AES-GMAC */
	if (xform->type == RTE_CRYPTO_SYM_XFORM_AUTH) {
		auth_xform = xform;
		if (auth_xform->auth.algo != RTE_CRYPTO_AUTH_AES_GMAC) {
			GCM_LOG_ERR("Only AES GMAC is supported as an "
					"authentication only algorithm");
			return -ENOTSUP;
		}
		/* Set IV parameters */
		sess->iv.offset = auth_xform->auth.iv.offset;
		sess->iv.length = auth_xform->auth.iv.length;

		/* Select Crypto operation */
		if (auth_xform->auth.op == RTE_CRYPTO_AUTH_OP_GENERATE)
			sess->op = AESNI_GMAC_OP_GENERATE;
		else
			sess->op = AESNI_GMAC_OP_VERIFY;

		key_length = auth_xform->auth.key.length;
		key = auth_xform->auth.key.data;
		digest_length = auth_xform->auth.digest_length;

	/* AES-GCM */
	} else if (xform->type == RTE_CRYPTO_SYM_XFORM_AEAD) {
		aead_xform = xform;

		if (aead_xform->aead.algo != RTE_CRYPTO_AEAD_AES_GCM) {
			GCM_LOG_ERR("The only combined operation "
						"supported is AES GCM");
			return -ENOTSUP;
		}

		/* Set IV parameters */
		sess->iv.offset = aead_xform->aead.iv.offset;
		sess->iv.length = aead_xform->aead.iv.length;

		/* Select Crypto operation */
		if (aead_xform->aead.op == RTE_CRYPTO_AEAD_OP_ENCRYPT)
			sess->op = AESNI_GCM_OP_AUTHENTICATED_ENCRYPTION;
		else
			sess->op = AESNI_GCM_OP_AUTHENTICATED_DECRYPTION;

		key_length = aead_xform->aead.key.length;
		key = aead_xform->aead.key.data;

		sess->aad_length = aead_xform->aead.aad_length;
		digest_length = aead_xform->aead.digest_length;
	} else {
		GCM_LOG_ERR("Wrong xform type, has to be AEAD or authentication");
		return -ENOTSUP;
	}


	/* IV check */
	if (sess->iv.length != 16 && sess->iv.length != 12 &&
			sess->iv.length != 0) {
		GCM_LOG_ERR("Wrong IV length");
		return -EINVAL;
	}

	/* Check key length and calculate GCM pre-compute. */
	switch (key_length) {
	case 16:
		sess->key = AESNI_GCM_KEY_128;
		break;
	case 24:
		sess->key = AESNI_GCM_KEY_192;
		break;
	case 32:
		sess->key = AESNI_GCM_KEY_256;
		break;
	default:
		GCM_LOG_ERR("Invalid key length");
		return -EINVAL;
	}

	gcm_ops[sess->key].precomp(key, &sess->gdata_key);

	/* Digest check */
	if (digest_length != 16 &&
			digest_length != 12 &&
			digest_length != 8) {
		GCM_LOG_ERR("digest");
		return -EINVAL;
	}
	sess->digest_length = digest_length;

	return 0;
}

/** Get gcm session */
static struct aesni_gcm_session *
aesni_gcm_get_session(struct aesni_gcm_qp *qp, struct rte_crypto_op *op)
{
	struct aesni_gcm_session *sess = NULL;
	struct rte_crypto_sym_op *sym_op = op->sym;

	if (op->sess_type == RTE_CRYPTO_OP_WITH_SESSION) {
		if (likely(sym_op->session != NULL))
			sess = (struct aesni_gcm_session *)
					get_session_private_data(
					sym_op->session,
					cryptodev_driver_id);
	} else  {
		void *_sess;
		void *_sess_private_data = NULL;

		if (rte_mempool_get(qp->sess_mp, (void **)&_sess))
			return NULL;

		if (rte_mempool_get(qp->sess_mp, (void **)&_sess_private_data))
			return NULL;

		sess = (struct aesni_gcm_session *)_sess_private_data;

		if (unlikely(aesni_gcm_set_session_parameters(qp->ops,
				sess, sym_op->xform) != 0)) {
			rte_mempool_put(qp->sess_mp, _sess);
			rte_mempool_put(qp->sess_mp, _sess_private_data);
			sess = NULL;
		}
		sym_op->session = (struct rte_cryptodev_sym_session *)_sess;
		set_session_private_data(sym_op->session, cryptodev_driver_id,
			_sess_private_data);
	}

	if (unlikely(sess == NULL))
		op->status = RTE_CRYPTO_OP_STATUS_INVALID_SESSION;

	return sess;
}

/**
 * Process a crypto operation, calling
 * the GCM API from the multi buffer library.
 *
 * @param	qp		queue pair
 * @param	op		symmetric crypto operation
 * @param	session		GCM session
 *
 * @return
 *
 */
static int
process_gcm_crypto_op(struct aesni_gcm_qp *qp, struct rte_crypto_op *op,
		struct aesni_gcm_session *session)
{
	uint8_t *src, *dst;
	uint8_t *iv_ptr;
	struct rte_crypto_sym_op *sym_op = op->sym;
	struct rte_mbuf *m_src = sym_op->m_src;
	uint32_t offset, data_offset, data_length;
	uint32_t part_len, total_len, data_len;

	if (session->op == AESNI_GCM_OP_AUTHENTICATED_ENCRYPTION ||
			session->op == AESNI_GCM_OP_AUTHENTICATED_DECRYPTION) {
		offset = sym_op->aead.data.offset;
		data_offset = offset;
		data_length = sym_op->aead.data.length;
	} else {
		offset = sym_op->auth.data.offset;
		data_offset = offset;
		data_length = sym_op->auth.data.length;
	}

	RTE_ASSERT(m_src != NULL);

	while (offset >= m_src->data_len) {
		offset -= m_src->data_len;
		m_src = m_src->next;

		RTE_ASSERT(m_src != NULL);
	}

	data_len = m_src->data_len - offset;
	part_len = (data_len < data_length) ? data_len :
			data_length;

	/* Destination buffer is required when segmented source buffer */
	RTE_ASSERT((part_len == data_length) ||
			((part_len != data_length) &&
					(sym_op->m_dst != NULL)));
	/* Segmented destination buffer is not supported */
	RTE_ASSERT((sym_op->m_dst == NULL) ||
			((sym_op->m_dst != NULL) &&
					rte_pktmbuf_is_contiguous(sym_op->m_dst)));


	dst = sym_op->m_dst ?
			rte_pktmbuf_mtod_offset(sym_op->m_dst, uint8_t *,
					data_offset) :
			rte_pktmbuf_mtod_offset(sym_op->m_src, uint8_t *,
					data_offset);

	src = rte_pktmbuf_mtod_offset(m_src, uint8_t *, offset);

	iv_ptr = rte_crypto_op_ctod_offset(op, uint8_t *,
				session->iv.offset);
	/*
	 * GCM working in 12B IV mode => 16B pre-counter block we need
	 * to set BE LSB to 1, driver expects that 16B is allocated
	 */
	if (session->iv.length == 12) {
		uint32_t *iv_padd = (uint32_t *)&(iv_ptr[12]);
		*iv_padd = rte_bswap32(1);
	}

	if (session->op == AESNI_GCM_OP_AUTHENTICATED_ENCRYPTION) {

		qp->ops[session->key].init(&session->gdata_key,
				&qp->gdata_ctx,
				iv_ptr,
				sym_op->aead.aad.data,
				(uint64_t)session->aad_length);

		qp->ops[session->key].update_enc(&session->gdata_key,
				&qp->gdata_ctx, dst, src,
				(uint64_t)part_len);
		total_len = data_length - part_len;

		while (total_len) {
			dst += part_len;
			m_src = m_src->next;

			RTE_ASSERT(m_src != NULL);

			src = rte_pktmbuf_mtod(m_src, uint8_t *);
			part_len = (m_src->data_len < total_len) ?
					m_src->data_len : total_len;

			qp->ops[session->key].update_enc(&session->gdata_key,
					&qp->gdata_ctx, dst, src,
					(uint64_t)part_len);
			total_len -= part_len;
		}

		qp->ops[session->key].finalize(&session->gdata_key,
				&qp->gdata_ctx,
				sym_op->aead.digest.data,
				(uint64_t)session->digest_length);
	} else if (session->op == AESNI_GCM_OP_AUTHENTICATED_DECRYPTION) {
		uint8_t *auth_tag = (uint8_t *)rte_pktmbuf_append(sym_op->m_dst ?
				sym_op->m_dst : sym_op->m_src,
				session->digest_length);

		if (!auth_tag) {
			GCM_LOG_ERR("auth_tag");
			return -1;
		}

		qp->ops[session->key].init(&session->gdata_key,
				&qp->gdata_ctx,
				iv_ptr,
				sym_op->aead.aad.data,
				(uint64_t)session->aad_length);

		qp->ops[session->key].update_dec(&session->gdata_key,
				&qp->gdata_ctx, dst, src,
				(uint64_t)part_len);
		total_len = data_length - part_len;

		while (total_len) {
			dst += part_len;
			m_src = m_src->next;

			RTE_ASSERT(m_src != NULL);

			src = rte_pktmbuf_mtod(m_src, uint8_t *);
			part_len = (m_src->data_len < total_len) ?
					m_src->data_len : total_len;

			qp->ops[session->key].update_dec(&session->gdata_key,
					&qp->gdata_ctx,
					dst, src,
					(uint64_t)part_len);
			total_len -= part_len;
		}

		qp->ops[session->key].finalize(&session->gdata_key,
				&qp->gdata_ctx,
				auth_tag,
				(uint64_t)session->digest_length);
	} else if (session->op == AESNI_GMAC_OP_GENERATE) {
		qp->ops[session->key].init(&session->gdata_key,
				&qp->gdata_ctx,
				iv_ptr,
				src,
				(uint64_t)data_length);
		qp->ops[session->key].finalize(&session->gdata_key,
				&qp->gdata_ctx,
				sym_op->auth.digest.data,
				(uint64_t)session->digest_length);
	} else { /* AESNI_GMAC_OP_VERIFY */
		uint8_t *auth_tag = (uint8_t *)rte_pktmbuf_append(sym_op->m_dst ?
				sym_op->m_dst : sym_op->m_src,
				session->digest_length);

		if (!auth_tag) {
			GCM_LOG_ERR("auth_tag");
			return -1;
		}

		qp->ops[session->key].init(&session->gdata_key,
				&qp->gdata_ctx,
				iv_ptr,
				src,
				(uint64_t)data_length);

		qp->ops[session->key].finalize(&session->gdata_key,
				&qp->gdata_ctx,
				auth_tag,
				(uint64_t)session->digest_length);
	}

	return 0;
}

/**
 * Process a completed job and return rte_mbuf which job processed
 *
 * @param job	JOB_AES_HMAC job to process
 *
 * @return
 * - Returns processed mbuf which is trimmed of output digest used in
 * verification of supplied digest in the case of a HASH_CIPHER operation
 * - Returns NULL on invalid job
 */
static void
post_process_gcm_crypto_op(struct rte_crypto_op *op,
		struct aesni_gcm_session *session)
{
	struct rte_mbuf *m = op->sym->m_dst ? op->sym->m_dst : op->sym->m_src;

	op->status = RTE_CRYPTO_OP_STATUS_SUCCESS;

	/* Verify digest if required */
	if (session->op == AESNI_GCM_OP_AUTHENTICATED_DECRYPTION ||
			session->op == AESNI_GMAC_OP_VERIFY) {
		uint8_t *digest;

		uint8_t *tag = rte_pktmbuf_mtod_offset(m, uint8_t *,
				m->data_len - session->digest_length);

		if (session->op == AESNI_GMAC_OP_VERIFY)
			digest = op->sym->auth.digest.data;
		else
			digest = op->sym->aead.digest.data;

#ifdef RTE_LIBRTE_PMD_AESNI_GCM_DEBUG
		rte_hexdump(stdout, "auth tag (orig):",
				digest, session->digest_length);
		rte_hexdump(stdout, "auth tag (calc):",
				tag, session->digest_length);
#endif

		if (memcmp(tag, digest,	session->digest_length) != 0)
			op->status = RTE_CRYPTO_OP_STATUS_AUTH_FAILED;

		/* trim area used for digest from mbuf */
		rte_pktmbuf_trim(m, session->digest_length);
	}
}

/**
 * Process a completed GCM request
 *
 * @param qp		Queue Pair to process
 * @param op		Crypto operation
 * @param job		JOB_AES_HMAC job
 *
 * @return
 * - Number of processed jobs
 */
static void
handle_completed_gcm_crypto_op(struct aesni_gcm_qp *qp,
		struct rte_crypto_op *op,
		struct aesni_gcm_session *sess)
{
	post_process_gcm_crypto_op(op, sess);

	/* Free session if a session-less crypto op */
	if (op->sess_type == RTE_CRYPTO_OP_SESSIONLESS) {
		memset(sess, 0, sizeof(struct aesni_gcm_session));
		memset(op->sym->session, 0,
				rte_cryptodev_get_header_session_size());
		rte_mempool_put(qp->sess_mp, sess);
		rte_mempool_put(qp->sess_mp, op->sym->session);
		op->sym->session = NULL;
	}
}

static uint16_t
aesni_gcm_pmd_dequeue_burst(void *queue_pair,
		struct rte_crypto_op **ops, uint16_t nb_ops)
{
	struct aesni_gcm_session *sess;
	struct aesni_gcm_qp *qp = queue_pair;

	int retval = 0;
	unsigned int i, nb_dequeued;

	nb_dequeued = rte_ring_dequeue_burst(qp->processed_pkts,
			(void **)ops, nb_ops, NULL);

	for (i = 0; i < nb_dequeued; i++) {

		sess = aesni_gcm_get_session(qp, ops[i]);
		if (unlikely(sess == NULL)) {
			ops[i]->status = RTE_CRYPTO_OP_STATUS_INVALID_ARGS;
			qp->qp_stats.dequeue_err_count++;
			break;
		}

		retval = process_gcm_crypto_op(qp, ops[i], sess);
		if (retval < 0) {
			ops[i]->status = RTE_CRYPTO_OP_STATUS_INVALID_ARGS;
			qp->qp_stats.dequeue_err_count++;
			break;
		}

		handle_completed_gcm_crypto_op(qp, ops[i], sess);
	}

	qp->qp_stats.dequeued_count += i;

	return i;
}

static uint16_t
aesni_gcm_pmd_enqueue_burst(void *queue_pair,
		struct rte_crypto_op **ops, uint16_t nb_ops)
{
	struct aesni_gcm_qp *qp = queue_pair;

	unsigned int nb_enqueued;

	nb_enqueued = rte_ring_enqueue_burst(qp->processed_pkts,
			(void **)ops, nb_ops, NULL);
	qp->qp_stats.enqueued_count += nb_enqueued;

	return nb_enqueued;
}

static int aesni_gcm_remove(struct rte_vdev_device *vdev);

static int
aesni_gcm_create(const char *name,
		struct rte_vdev_device *vdev,
		struct rte_crypto_vdev_init_params *init_params)
{
	struct rte_cryptodev *dev;
	struct aesni_gcm_private *internals;
	enum aesni_gcm_vector_mode vector_mode;

	if (init_params->name[0] == '\0')
		snprintf(init_params->name, sizeof(init_params->name),
				"%s", name);

	/* Check CPU for support for AES instruction set */
	if (!rte_cpu_get_flag_enabled(RTE_CPUFLAG_AES)) {
		GCM_LOG_ERR("AES instructions not supported by CPU");
		return -EFAULT;
	}

	/* Check CPU for supported vector instruction set */
	if (rte_cpu_get_flag_enabled(RTE_CPUFLAG_AVX2))
		vector_mode = RTE_AESNI_GCM_AVX2;
	else if (rte_cpu_get_flag_enabled(RTE_CPUFLAG_AVX))
		vector_mode = RTE_AESNI_GCM_AVX;
	else
		vector_mode = RTE_AESNI_GCM_SSE;

	dev = rte_cryptodev_vdev_pmd_init(init_params->name,
			sizeof(struct aesni_gcm_private), init_params->socket_id,
			vdev);
	if (dev == NULL) {
		GCM_LOG_ERR("failed to create cryptodev vdev");
		goto init_error;
	}

	dev->driver_id = cryptodev_driver_id;
	dev->dev_ops = rte_aesni_gcm_pmd_ops;

	/* register rx/tx burst functions for data path */
	dev->dequeue_burst = aesni_gcm_pmd_dequeue_burst;
	dev->enqueue_burst = aesni_gcm_pmd_enqueue_burst;

	dev->feature_flags = RTE_CRYPTODEV_FF_SYMMETRIC_CRYPTO |
			RTE_CRYPTODEV_FF_SYM_OPERATION_CHAINING |
			RTE_CRYPTODEV_FF_CPU_AESNI |
			RTE_CRYPTODEV_FF_MBUF_SCATTER_GATHER;

	switch (vector_mode) {
	case RTE_AESNI_GCM_SSE:
		dev->feature_flags |= RTE_CRYPTODEV_FF_CPU_SSE;
		break;
	case RTE_AESNI_GCM_AVX:
		dev->feature_flags |= RTE_CRYPTODEV_FF_CPU_AVX;
		break;
	case RTE_AESNI_GCM_AVX2:
		dev->feature_flags |= RTE_CRYPTODEV_FF_CPU_AVX2;
		break;
	default:
		break;
	}

	internals = dev->data->dev_private;

	internals->vector_mode = vector_mode;

	internals->max_nb_queue_pairs = init_params->max_nb_queue_pairs;
	internals->max_nb_sessions = init_params->max_nb_sessions;

	return 0;

init_error:
	GCM_LOG_ERR("driver %s: create failed", init_params->name);

	aesni_gcm_remove(vdev);
	return -EFAULT;
}

static int
aesni_gcm_probe(struct rte_vdev_device *vdev)
{
	struct rte_crypto_vdev_init_params init_params = {
		RTE_CRYPTODEV_VDEV_DEFAULT_MAX_NB_QUEUE_PAIRS,
		RTE_CRYPTODEV_VDEV_DEFAULT_MAX_NB_SESSIONS,
		rte_socket_id(),
		{0}
	};
	const char *name;
	const char *input_args;

	name = rte_vdev_device_name(vdev);
	if (name == NULL)
		return -EINVAL;
	input_args = rte_vdev_device_args(vdev);
	rte_cryptodev_vdev_parse_init_params(&init_params, input_args);

	RTE_LOG(INFO, PMD, "Initialising %s on NUMA node %d\n", name,
			init_params.socket_id);
	if (init_params.name[0] != '\0')
		RTE_LOG(INFO, PMD, "  User defined name = %s\n",
			init_params.name);
	RTE_LOG(INFO, PMD, "  Max number of queue pairs = %d\n",
			init_params.max_nb_queue_pairs);
	RTE_LOG(INFO, PMD, "  Max number of sessions = %d\n",
			init_params.max_nb_sessions);

	return aesni_gcm_create(name, vdev, &init_params);
}

static int
aesni_gcm_remove(struct rte_vdev_device *vdev)
{
	const char *name;

	name = rte_vdev_device_name(vdev);
	if (name == NULL)
		return -EINVAL;

	GCM_LOG_INFO("Closing AESNI crypto device %s on numa socket %u\n",
			name, rte_socket_id());

	return 0;
}

static struct rte_vdev_driver aesni_gcm_pmd_drv = {
	.probe = aesni_gcm_probe,
	.remove = aesni_gcm_remove
};

RTE_PMD_REGISTER_VDEV(CRYPTODEV_NAME_AESNI_GCM_PMD, aesni_gcm_pmd_drv);
RTE_PMD_REGISTER_ALIAS(CRYPTODEV_NAME_AESNI_GCM_PMD, cryptodev_aesni_gcm_pmd);
RTE_PMD_REGISTER_PARAM_STRING(CRYPTODEV_NAME_AESNI_GCM_PMD,
	"max_nb_queue_pairs=<int> "
	"max_nb_sessions=<int> "
	"socket_id=<int>");
RTE_PMD_REGISTER_CRYPTO_DRIVER(aesni_gcm_pmd_drv, cryptodev_driver_id);
