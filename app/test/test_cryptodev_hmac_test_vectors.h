/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2016 Intel Corporation. All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *	 * Redistributions of source code must retain the above copyright
 *	   notice, this list of conditions and the following disclaimer.
 *	 * Redistributions in binary form must reproduce the above copyright
 *	   notice, this list of conditions and the following disclaimer in
 *	   the documentation and/or other materials provided with the
 *	   distribution.
 *	 * Neither the name of Intel Corporation nor the names of its
 *	   contributors may be used to endorse or promote products derived
 *	   from this software without specific prior written permission.
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

#ifndef APP_TEST_TEST_CRYPTODEV_HMAC_TEST_VECTORS_H_
#define APP_TEST_TEST_CRYPTODEV_HMAC_TEST_VECTORS_H_

/* *** MD5 test vectors *** */

#define MD5_DIGEST_LEN	16

struct HMAC_MD5_vector {
	struct {
		uint8_t data[64];
		uint16_t len;
	} key;

	struct {
			uint8_t data[1024];
			uint16_t len;
	} plaintext;

	struct {
			uint8_t data[16];
			uint16_t len;
	} auth_tag;
};

static const struct
HMAC_MD5_vector HMAC_MD5_test_case_1 = {
	.key = {
		.data = {
			0xF8, 0x2A, 0xC7, 0x54, 0xDB, 0x96, 0x18, 0xAA,
			0xC3, 0xA1, 0x53, 0xF6, 0x1F, 0x17, 0x60, 0xBD
		},
		.len = 16
	},
	.plaintext = {
		.data = {
			0x87, 0x4D, 0x61, 0x91, 0xB6, 0x20, 0xE3, 0x26,
			0x1B, 0xEF, 0x68, 0x64, 0x99, 0x0D, 0xB6, 0xCE,
			0x98, 0x06, 0xF6, 0x6B, 0x79, 0x70, 0xFD, 0xFF,
			0x86, 0x17, 0x18, 0x7B, 0xB9, 0xFF, 0xFD, 0xFF,
			0x5A, 0xE4, 0xDF, 0x3E, 0xDB, 0xD5, 0xD3, 0x5E,
			0x5B, 0x4F, 0x09, 0x02, 0x0D, 0xB0, 0x3E, 0xAB,
			0x1E, 0x03, 0x1D, 0xDA, 0x2F, 0xBE, 0x03, 0xD1,
			0x79, 0x21, 0x70, 0xA0, 0xF3, 0x00, 0x9C, 0xEE
		},
		.len = 64
	},
	.auth_tag = {
		.data = {
			0x67, 0x83, 0xE1, 0x0F, 0xB0, 0xBF, 0x33, 0x49,
			0x22, 0x04, 0x89, 0xDF, 0x86, 0xD0, 0x5F, 0x0C
		},
		.len = MD5_DIGEST_LEN
	}
};

static const struct
HMAC_MD5_vector HMAC_MD5_test_case_2 = {
	.key = {
		.data = {
			0xF8, 0x2A, 0xC7, 0x54, 0xDB, 0x96, 0x18, 0xAA,
			0xC3, 0xA1, 0x53, 0xF6, 0x1F, 0x17, 0x60, 0xBD,
			0xF8, 0x2A, 0xC7, 0x54, 0xDB, 0x96, 0x18, 0xAA,
			0xC3, 0xA1, 0x53, 0xF6, 0x1F, 0x17, 0x60, 0xBD
		},
		.len = 32
	},
	.plaintext = {
		.data = {
			0x87, 0x4D, 0x61, 0x91, 0xB6, 0x20, 0xE3, 0x26,
			0x1B, 0xEF, 0x68, 0x64, 0x99, 0x0D, 0xB6, 0xCE,
			0x98, 0x06, 0xF6, 0x6B, 0x79, 0x70, 0xFD, 0xFF,
			0x86, 0x17, 0x18, 0x7B, 0xB9, 0xFF, 0xFD, 0xFF,
			0x5A, 0xE4, 0xDF, 0x3E, 0xDB, 0xD5, 0xD3, 0x5E,
			0x5B, 0x4F, 0x09, 0x02, 0x0D, 0xB0, 0x3E, 0xAB,
			0x1E, 0x03, 0x1D, 0xDA, 0x2F, 0xBE, 0x03, 0xD1,
			0x79, 0x21, 0x70, 0xA0, 0xF3, 0x00, 0x9C, 0xEE
		},
		.len = 64
	},
	.auth_tag = {
		.data = {
			0x39, 0x24, 0x70, 0x7A, 0x30, 0x38, 0x1E, 0x2B,
			0x9F, 0x6B, 0xD9, 0x3C, 0xAD, 0xC2, 0x73, 0x52
		},
		.len = MD5_DIGEST_LEN
	}
};

#endif /* APP_TEST_TEST_CRYPTODEV_HMAC_TEST_VECTORS_H_ */
