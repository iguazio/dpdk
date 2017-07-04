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

#ifndef _AESNI_GCM_OPS_H_
#define _AESNI_GCM_OPS_H_

#ifndef LINUX
#define LINUX
#endif

#include <gcm_defines.h>
#include <aux_funcs.h>

/** Supported vector modes */
enum aesni_gcm_vector_mode {
	RTE_AESNI_GCM_NOT_SUPPORTED = 0,
	RTE_AESNI_GCM_SSE,
	RTE_AESNI_GCM_AVX,
	RTE_AESNI_GCM_AVX2,
	RTE_AESNI_GCM_VECTOR_NUM
};

enum aesni_gcm_key {
	AESNI_GCM_KEY_128,
	AESNI_GCM_KEY_192,
	AESNI_GCM_KEY_256,
	AESNI_GCM_KEY_NUM
};


typedef void (*aesni_gcm_t)(const struct gcm_key_data *gcm_key_data,
		struct gcm_context_data *gcm_ctx_data, uint8_t *out,
		const uint8_t *in, uint64_t plaintext_len, const uint8_t *iv,
		const uint8_t *aad, uint64_t aad_len,
		uint8_t *auth_tag, uint64_t auth_tag_len);

typedef void (*aesni_gcm_precomp_t)(const void *key, struct gcm_key_data *gcm_data);

typedef void (*aesni_gcm_init_t)(const struct gcm_key_data *gcm_key_data,
		struct gcm_context_data *gcm_ctx_data,
		const uint8_t *iv,
		uint8_t const *aad,
		uint64_t aad_len);

typedef void (*aesni_gcm_update_t)(const struct gcm_key_data *gcm_key_data,
		struct gcm_context_data *gcm_ctx_data,
		uint8_t *out,
		const uint8_t *in,
		uint64_t plaintext_len);

typedef void (*aesni_gcm_finalize_t)(const struct gcm_key_data *gcm_key_data,
		struct gcm_context_data *gcm_ctx_data,
		uint8_t *auth_tag,
		uint64_t auth_tag_len);

/** GCM library function pointer table */
struct aesni_gcm_ops {
	aesni_gcm_t enc;        /**< GCM encode function pointer */
	aesni_gcm_t dec;        /**< GCM decode function pointer */
	aesni_gcm_precomp_t precomp;    /**< GCM pre-compute */
	aesni_gcm_init_t init;
	aesni_gcm_update_t update_enc;
	aesni_gcm_update_t update_dec;
	aesni_gcm_finalize_t finalize;
};

#define AES_GCM_FN(keylen, arch) \
aes_gcm_enc_##keylen##_##arch,\
aes_gcm_dec_##keylen##_##arch,\
aes_gcm_pre_##keylen##_##arch,\
aes_gcm_init_##keylen##_##arch,\
aes_gcm_enc_##keylen##_update_##arch,\
aes_gcm_dec_##keylen##_update_##arch,\
aes_gcm_enc_##keylen##_finalize_##arch,

static const struct aesni_gcm_ops gcm_ops[RTE_AESNI_GCM_VECTOR_NUM][AESNI_GCM_KEY_NUM] = {
	[RTE_AESNI_GCM_NOT_SUPPORTED] = {
		[AESNI_GCM_KEY_128] = {NULL},
		[AESNI_GCM_KEY_192] = {NULL},
		[AESNI_GCM_KEY_256] = {NULL}
	},
	[RTE_AESNI_GCM_SSE] = {
		[AESNI_GCM_KEY_128] = {
			AES_GCM_FN(128, sse)
		},
		[AESNI_GCM_KEY_192] = {
			AES_GCM_FN(192, sse)
		},
		[AESNI_GCM_KEY_256] = {
			AES_GCM_FN(256, sse)
		}
	},
	[RTE_AESNI_GCM_AVX] = {
		[AESNI_GCM_KEY_128] = {
			AES_GCM_FN(128, avx_gen2)
		},
		[AESNI_GCM_KEY_192] = {
			AES_GCM_FN(192, avx_gen2)
		},
		[AESNI_GCM_KEY_256] = {
			AES_GCM_FN(256, avx_gen2)
		}
	},
	[RTE_AESNI_GCM_AVX2] = {
		[AESNI_GCM_KEY_128] = {
			AES_GCM_FN(128, avx_gen4)
		},
		[AESNI_GCM_KEY_192] = {
			AES_GCM_FN(192, avx_gen4)
		},
		[AESNI_GCM_KEY_256] = {
			AES_GCM_FN(256, avx_gen4)
		}
	}
};
#endif /* _AESNI_GCM_OPS_H_ */
