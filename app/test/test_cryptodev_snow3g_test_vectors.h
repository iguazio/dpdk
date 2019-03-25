/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2015-2017 Intel Corporation
 */

#ifndef TEST_CRYPTODEV_SNOW3G_TEST_VECTORS_H_
#define TEST_CRYPTODEV_SNOW3G_TEST_VECTORS_H_

struct snow3g_test_data {
	struct {
		uint8_t data[64];
		unsigned len;
	} key;

	struct {
		uint8_t data[64] __rte_aligned(16);
		unsigned len;
	} cipher_iv;

	struct {
		uint8_t data[1024];
		unsigned len; /* length must be in Bits */
	} plaintext;

	struct {
		uint8_t data[1024];
		unsigned len; /* length must be in Bits */
	} ciphertext;

	struct {
		unsigned len;
	} validDataLenInBits;

	struct {
		unsigned len;
	} validCipherLenInBits;

	struct {
		unsigned len;
	} validAuthLenInBits;

	struct {
		uint8_t data[64];
		unsigned len;
	} auth_iv;

	struct {
		uint8_t data[64];
		unsigned int len; /* length must be in Bytes */
		unsigned int offset_bytes; /* offset must be in Bytes */
	} digest;

	struct {
		unsigned int len_bits; /* length must be in Bits */
		unsigned int offset_bits;
	} cipher;

	struct {
		unsigned int len_bits; /* length must be in Bits */
		unsigned int offset_bits;
	} auth;
};
struct snow3g_test_data snow3g_test_case_1 = {
	.key = {
		.data = {
			0x2B, 0xD6, 0x45, 0x9F, 0x82, 0xC5, 0xB3, 0x00,
			0x95, 0x2C, 0x49, 0x10, 0x48, 0x81, 0xFF, 0x48
		},
		.len = 16
	},
	.cipher_iv = {
		.data = {
			0x72, 0xA4, 0xF2, 0x0F, 0x64, 0x00, 0x00, 0x00,
			0x72, 0xA4, 0xF2, 0x0F, 0x64, 0x00, 0x00, 0x00
		},
		.len = 16
	},
	.plaintext = {
		.data = {
			0x7E, 0xC6, 0x12, 0x72, 0x74, 0x3B, 0xF1, 0x61,
			0x47, 0x26, 0x44, 0x6A, 0x6C, 0x38, 0xCE, 0xD1,
			0x66, 0xF6, 0xCA, 0x76, 0xEB, 0x54, 0x30, 0x04,
			0x42, 0x86, 0x34, 0x6C, 0xEF, 0x13, 0x0F, 0x92,
			0x92, 0x2B, 0x03, 0x45, 0x0D, 0x3A, 0x99, 0x75,
			0xE5, 0xBD, 0x2E, 0xA0, 0xEB, 0x55, 0xAD, 0x8E,
			0x1B, 0x19, 0x9E, 0x3E, 0xC4, 0x31, 0x60, 0x20,
			0xE9, 0xA1, 0xB2, 0x85, 0xE7, 0x62, 0x79, 0x53,
			0x59, 0xB7, 0xBD, 0xFD, 0x39, 0xBE, 0xF4, 0xB2,
			0x48, 0x45, 0x83, 0xD5, 0xAF, 0xE0, 0x82, 0xAE,
			0xE6, 0x38, 0xBF, 0x5F, 0xD5, 0xA6, 0x06, 0x19,
			0x39, 0x01, 0xA0, 0x8F, 0x4A, 0xB4, 0x1A, 0xAB,
			0x9B, 0x13, 0x48, 0x80
		},
		.len = 800
	},
	.ciphertext = {
		.data = {
			0x8C, 0xEB, 0xA6, 0x29, 0x43, 0xDC, 0xED, 0x3A,
			0x09, 0x90, 0xB0, 0x6E, 0xA1, 0xB0, 0xA2, 0xC4,
			0xFB, 0x3C, 0xED, 0xC7, 0x1B, 0x36, 0x9F, 0x42,
			0xBA, 0x64, 0xC1, 0xEB, 0x66, 0x65, 0xE7, 0x2A,
			0xA1, 0xC9, 0xBB, 0x0D, 0xEA, 0xA2, 0x0F, 0xE8,
			0x60, 0x58, 0xB8, 0xBA, 0xEE, 0x2C, 0x2E, 0x7F,
			0x0B, 0xEC, 0xCE, 0x48, 0xB5, 0x29, 0x32, 0xA5,
			0x3C, 0x9D, 0x5F, 0x93, 0x1A, 0x3A, 0x7C, 0x53,
			0x22, 0x59, 0xAF, 0x43, 0x25, 0xE2, 0xA6, 0x5E,
			0x30, 0x84, 0xAD, 0x5F, 0x6A, 0x51, 0x3B, 0x7B,
			0xDD, 0xC1, 0xB6, 0x5F, 0x0A, 0xA0, 0xD9, 0x7A,
			0x05, 0x3D, 0xB5, 0x5A, 0x88, 0xC4, 0xC4, 0xF9,
			0x60, 0x5E, 0x41, 0x40
		},
		.len = 800
	},
	.cipher = {
		.offset_bits = 0
	},
	.validDataLenInBits = {
		.len = 798
	},
	.validCipherLenInBits = {
		.len = 800
	},
	.auth_iv = {
		.data = {
			 0x72, 0xA4, 0xF2, 0x0F, 0x64, 0x00, 0x00, 0x00,
			 0x72, 0xA4, 0xF2, 0x0F, 0x64, 0x00, 0x00, 0x00
		},
		.len = 16
	}
};

struct snow3g_test_data snow3g_test_case_2 = {
	.key = {
		.data = {
			0xEF, 0xA8, 0xB2, 0x22, 0x9E, 0x72, 0x0C, 0x2A,
			0x7C, 0x36, 0xEA, 0x55, 0xE9, 0x60, 0x56, 0x95
		},
		.len = 16
	},
	.cipher_iv = {
	       .data = {
			0xE2, 0x8B, 0xCF, 0x7B, 0xC0, 0x00, 0x00, 0x00,
			0xE2, 0x8B, 0xCF, 0x7B, 0xC0, 0x00, 0x00, 0x00
		},
	       .len = 16
	},
	.plaintext = {
		.data = {
			0x10, 0x11, 0x12, 0x31, 0xE0, 0x60, 0x25, 0x3A,
			0x43, 0xFD, 0x3F, 0x57, 0xE3, 0x76, 0x07, 0xAB,
			0x28, 0x27, 0xB5, 0x99, 0xB6, 0xB1, 0xBB, 0xDA,
			0x37, 0xA8, 0xAB, 0xCC, 0x5A, 0x8C, 0x55, 0x0D,
			0x1B, 0xFB, 0x2F, 0x49, 0x46, 0x24, 0xFB, 0x50,
			0x36, 0x7F, 0xA3, 0x6C, 0xE3, 0xBC, 0x68, 0xF1,
			0x1C, 0xF9, 0x3B, 0x15, 0x10, 0x37, 0x6B, 0x02,
			0x13, 0x0F, 0x81, 0x2A, 0x9F, 0xA1, 0x69, 0xD8
		},
		.len = 512
	},
	.ciphertext = {
		.data = {
				0xE0, 0xDA, 0x15, 0xCA, 0x8E, 0x25, 0x54, 0xF5,
				0xE5, 0x6C, 0x94, 0x68, 0xDC, 0x6C, 0x7C, 0x12,
				0x9C, 0x56, 0x8A, 0xA5, 0x03, 0x23, 0x17, 0xE0,
				0x4E, 0x07, 0x29, 0x64, 0x6C, 0xAB, 0xEF, 0xA6,
				0x89, 0x86, 0x4C, 0x41, 0x0F, 0x24, 0xF9, 0x19,
				0xE6, 0x1E, 0x3D, 0xFD, 0xFA, 0xD7, 0x7E, 0x56,
				0x0D, 0xB0, 0xA9, 0xCD, 0x36, 0xC3, 0x4A, 0xE4,
				0x18, 0x14, 0x90, 0xB2, 0x9F, 0x5F, 0xA2, 0xFC
		},
		.len = 512
	},
	.cipher = {
		.offset_bits = 0
	},
	.validDataLenInBits = {
		.len = 510
	},
	.validCipherLenInBits = {
		.len = 512
	},
	.auth_iv = {
		.data = {
			 0xE2, 0x8B, 0xCF, 0x7B, 0xC0, 0x00, 0x00, 0x00,
			 0xE2, 0x8B, 0xCF, 0x7B, 0xC0, 0x00, 0x00, 0x00
		},
		.len = 16
	}
};

struct snow3g_test_data snow3g_test_case_3 = {
	.key = {
		.data = {
			 0x5A, 0xCB, 0x1D, 0x64, 0x4C, 0x0D, 0x51, 0x20,
			 0x4E, 0xA5, 0xF1, 0x45, 0x10, 0x10, 0xD8, 0x52
		},
		.len = 16
	},
	.cipher_iv = {
		.data = {
			0xFA, 0x55, 0x6B, 0x26, 0x1C, 0x00, 0x00, 0x00,
			0xFA, 0x55, 0x6B, 0x26, 0x1C, 0x00, 0x00, 0x00
		},
		.len = 16
	},
	.plaintext = {
		.data = {
			0xAD, 0x9C, 0x44, 0x1F, 0x89, 0x0B, 0x38, 0xC4,
			0x57, 0xA4, 0x9D, 0x42, 0x14, 0x07, 0xE8
		},
		.len = 120
	},
	.ciphertext = {
		.data = {
			0xBA, 0x0F, 0x31, 0x30, 0x03, 0x34, 0xC5, 0x6B,
			0x52, 0xA7, 0x49, 0x7C, 0xBA, 0xC0, 0x46
		},
		.len = 120
	},
	.cipher = {
		.offset_bits = 0
	},
	.validDataLenInBits = {
		.len = 120
	},
	.validCipherLenInBits = {
		.len = 120
	},
	.auth_iv = {
		.data = {
			0xFA, 0x55, 0x6B, 0x26, 0x1C, 0x00, 0x00, 0x00,
			0xFA, 0x55, 0x6B, 0x26, 0x1C, 0x00, 0x00, 0x00
		},
		.len = 16
	},
	.digest = {
		.data = {0xE8, 0x60, 0x5A, 0x3E},
		.len  = 4
	},
	.validAuthLenInBits = {
		.len = 120
	}
};

struct snow3g_test_data snow3g_test_case_4 = {
	.key = {
		.data = {
			0xD3, 0xC5, 0xD5, 0x92, 0x32, 0x7F, 0xB1, 0x1C,
			0x40, 0x35, 0xC6, 0x68, 0x0A, 0xF8, 0xC6, 0xD1
		},
		.len = 16
	},
	.cipher_iv = {
		.data = {
			0x39, 0x8A, 0x59, 0xB4, 0x2C, 0x00, 0x00, 0x00,
			0x39, 0x8A, 0x59, 0xB4, 0x2C, 0x00, 0x00, 0x00
		},
		.len = 16
	},
	.plaintext = {
		.data = {
			0x98, 0x1B, 0xA6, 0x82, 0x4C, 0x1B, 0xFB, 0x1A,
			0xB4, 0x85, 0x47, 0x20, 0x29, 0xB7, 0x1D, 0x80,
			0x8C, 0xE3, 0x3E, 0x2C, 0xC3, 0xC0, 0xB5, 0xFC,
			0x1F, 0x3D, 0xE8, 0xA6, 0xDC, 0x66, 0xB1, 0xF0
		},
		.len = 256
	},
	.ciphertext = {
		.data = {
			0x98, 0x9B, 0x71, 0x9C, 0xDC, 0x33, 0xCE, 0xB7,
			0xCF, 0x27, 0x6A, 0x52, 0x82, 0x7C, 0xEF, 0x94,
			0xA5, 0x6C, 0x40, 0xC0, 0xAB, 0x9D, 0x81, 0xF7,
			0xA2, 0xA9, 0xBA, 0xC6, 0x0E, 0x11, 0xC4, 0xB0
		},
		.len = 256
	},
	.cipher = {
		.offset_bits = 0
	},
	.validDataLenInBits = {
		.len = 253
	},
	.validCipherLenInBits = {
		.len = 256
	}
};

struct snow3g_test_data snow3g_test_case_5 = {
	.key = {
		.data = {
			0x60, 0x90, 0xEA, 0xE0, 0x4C, 0x83, 0x70, 0x6E,
			0xEC, 0xBF, 0x65, 0x2B, 0xE8, 0xE3, 0x65, 0x66
		},
		.len = 16
	},
	.cipher_iv = {
		.data = {
			0x72, 0xA4, 0xF2, 0x0F, 0x48, 0x00, 0x00, 0x00,
			0x72, 0xA4, 0xF2, 0x0F, 0x48, 0x00, 0x00, 0x00
		},
		.len = 16},
	.plaintext = {
		.data = {
			0x40, 0x98, 0x1B, 0xA6, 0x82, 0x4C, 0x1B, 0xFB,
			0x42, 0x86, 0xB2, 0x99, 0x78, 0x3D, 0xAF, 0x44,
			0x2C, 0x09, 0x9F, 0x7A, 0xB0, 0xF5, 0x8D, 0x5C,
			0x8E, 0x46, 0xB1, 0x04, 0xF0, 0x8F, 0x01, 0xB4,
			0x1A, 0xB4, 0x85, 0x47, 0x20, 0x29, 0xB7, 0x1D,
			0x36, 0xBD, 0x1A, 0x3D, 0x90, 0xDC, 0x3A, 0x41,
			0xB4, 0x6D, 0x51, 0x67, 0x2A, 0xC4, 0xC9, 0x66,
			0x3A, 0x2B, 0xE0, 0x63, 0xDA, 0x4B, 0xC8, 0xD2,
			0x80, 0x8C, 0xE3, 0x3E, 0x2C, 0xCC, 0xBF, 0xC6,
			0x34, 0xE1, 0xB2, 0x59, 0x06, 0x08, 0x76, 0xA0,
			0xFB, 0xB5, 0xA4, 0x37, 0xEB, 0xCC, 0x8D, 0x31,
			0xC1, 0x9E, 0x44, 0x54, 0x31, 0x87, 0x45, 0xE3,
			0x98, 0x76, 0x45, 0x98, 0x7A, 0x98, 0x6F, 0x2C,
			0xB0
		},
		.len = 840
	},
	.ciphertext = {
		.data = {
			0x58, 0x92, 0xBB, 0xA8, 0x8B, 0xBB, 0xCA, 0xAE,
			0xAE, 0x76, 0x9A, 0xA0, 0x6B, 0x68, 0x3D, 0x3A,
			0x17, 0xCC, 0x04, 0xA3, 0x69, 0x88, 0x16, 0x97,
			0x43, 0x5E, 0x44, 0xFE, 0xD5, 0xFF, 0x9A, 0xF5,
			0x7B, 0x9E, 0x89, 0x0D, 0x4D, 0x5C, 0x64, 0x70,
			0x98, 0x85, 0xD4, 0x8A, 0xE4, 0x06, 0x90, 0xEC,
			0x04, 0x3B, 0xAA, 0xE9, 0x70, 0x57, 0x96, 0xE4,
			0xA9, 0xFF, 0x5A, 0x4B, 0x8D, 0x8B, 0x36, 0xD7,
			0xF3, 0xFE, 0x57, 0xCC, 0x6C, 0xFD, 0x6C, 0xD0,
			0x05, 0xCD, 0x38, 0x52, 0xA8, 0x5E, 0x94, 0xCE,
			0x6B, 0xCD, 0x90, 0xD0, 0xD0, 0x78, 0x39, 0xCE,
			0x09, 0x73, 0x35, 0x44, 0xCA, 0x8E, 0x35, 0x08,
			0x43, 0x24, 0x85, 0x50, 0x92, 0x2A, 0xC1, 0x28,
			0x18
		},
		.len = 840
	},
	.cipher = {
		.offset_bits = 0
	},
	.validDataLenInBits = {
		.len = 837
	},
	.validCipherLenInBits = {
		.len = 840
	},
};
struct snow3g_test_data snow3g_test_case_6 = {
	.key = {
		.data = {
			0xC7, 0x36, 0xC6, 0xAA, 0xB2, 0x2B, 0xFF, 0xF9,
			0x1E, 0x26, 0x98, 0xD2, 0xE2, 0x2A, 0xD5, 0x7E
		},
		.len = 16
	},
	.cipher_iv = {
		.data = {
			0x14, 0x79, 0x3E, 0x41, 0x03, 0x97, 0xE8, 0xFD,
			0x94, 0x79, 0x3E, 0x41, 0x03, 0x97, 0x68, 0xFD
		},
		.len = 16
	},
	.auth_iv = {
		.data = {
			0x14, 0x79, 0x3E, 0x41, 0x03, 0x97, 0xE8, 0xFD,
			0x94, 0x79, 0x3E, 0x41, 0x03, 0x97, 0x68, 0xFD
		},
		.len = 16
	},
	.plaintext = {
		.data = {
			0xD0, 0xA7, 0xD4, 0x63, 0xDF, 0x9F, 0xB2, 0xB2,
			0x78, 0x83, 0x3F, 0xA0, 0x2E, 0x23, 0x5A, 0xA1,
			0x72, 0xBD, 0x97, 0x0C, 0x14, 0x73, 0xE1, 0x29,
			0x07, 0xFB, 0x64, 0x8B, 0x65, 0x99, 0xAA, 0xA0,
			0xB2, 0x4A, 0x03, 0x86, 0x65, 0x42, 0x2B, 0x20,
			0xA4, 0x99, 0x27, 0x6A, 0x50, 0x42, 0x70, 0x09
		},
		.len = 384
	},
	.ciphertext = {
	   .data = {
			0x95, 0x2E, 0x5A, 0xE1, 0x50, 0xB8, 0x59, 0x2A,
			0x9B, 0xA0, 0x38, 0xA9, 0x8E, 0x2F, 0xED, 0xAB,
			0xFD, 0xC8, 0x3B, 0x47, 0x46, 0x0B, 0x50, 0x16,
			0xEC, 0x88, 0x45, 0xB6, 0x05, 0xC7, 0x54, 0xF8,
			0xBD, 0x91, 0xAA, 0xB6, 0xA4, 0xDC, 0x64, 0xB4,
			0xCB, 0xEB, 0x97, 0x06, 0x4C, 0xF7, 0x02, 0x3D
		},
		.len = 384
	},
	.cipher = {
		.len_bits = 384,
		.offset_bits = 0
	},
	.auth = {
		.len_bits = 384,
		.offset_bits = 0
	},
	.digest = {
		.data = {0x38, 0xB5, 0x54, 0xC0 },
		.len  = 4,
		.offset_bytes = 0
	},
	.validDataLenInBits = {
		.len = 384
	},
	.validCipherLenInBits = {
		.len = 384
	},
	.validAuthLenInBits = {
		.len = 384
	},
};


struct snow3g_test_data snow3g_test_case_7 = {
	.key = {
		.data = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10

			},
		.len = 16
	},
	.cipher_iv = {
		.data = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
		},
		.len = 16
	},
	.auth_iv = {
		.data = {
			 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		},
		.len = 16
	},
	.plaintext = {
		.data = {
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,  0x5A,
			0x5A,  0x5A,  0x5A,  0x5A,  0xF1,  0x9E,  0x2B,  0x6F,
		},
		.len = 128 << 3
	},
	.ciphertext = {
		.data = {
			0x5A,  0x5A,  0xE4,  0xAD,  0x29,  0xA2,  0x6A,  0xA6,
			0x20,  0x1D,  0xCD,  0x08,  0x50,  0xD6,  0xE6,  0x47,
			0xBC,  0x88,  0x08,  0x01,  0x17,  0xFA,  0x47,  0x5B,
			0x90,  0x40,  0xBA,  0x0C,  0xB5,  0x58,  0xF3,  0x0C,
			0xA0,  0xD4,  0x98,  0x83,  0x1B,  0xCE,  0x54,  0xE3,
			0x29,  0x00,  0x3C,  0xA4,  0xAD,  0x74,  0xEE,  0x05,
			0xA3,  0x6C,  0xD4,  0xAC,  0xC6,  0x30,  0x33,  0xC9,
			0x37,  0x57,  0x41,  0x9B,  0xD4,  0x73,  0xB9,  0x77,
			0x70,  0x8B,  0x63,  0xDD,  0x22,  0xB8,  0xE1,  0x85,
			0xB2,  0x92,  0x7C,  0x37,  0xD3,  0x2E,  0xD9,  0xF4,
			0x4A,  0x69,  0x25,  0x30,  0xE3,  0x5B,  0x8B,  0xF6,
			0x0F,  0xDE,  0x0B,  0x92,  0xD5,  0x25,  0x52,  0x6D,
			0x26,  0xEB,  0x2F,  0x8A,  0x3B,  0x8B,  0x38,  0xE2,
			0x48,  0xD3,  0x4A,  0x98,  0xF7,  0x3A,  0xC2,  0x46,
			0x69,  0x8D,  0x73,  0x3E,  0x57,  0x88,  0x2C,  0x80,
			0xF0,  0xF2,  0x75,  0xB8,  0x7D,  0x27,  0xC6,  0xDA,

		},
		.len = 128 << 3
	},
	.cipher = {
		.len_bits = 126 << 3,
		.offset_bits = 2 << 3
	},
	.auth = {
		.len_bits = 124 << 3,
		.offset_bits = 0
	},
	.digest = {
		.data = {
			0x7D, 0x27, 0xC6, 0xDA
		},
		.len = 4,
		.offset_bytes = 124
	},
	.validDataLenInBits = {
		.len = 128 << 3
	},
	.validCipherLenInBits = {
		.len = 126 << 3
	},
	.validAuthLenInBits = {
		.len = 124 << 3
	},
};

#endif /* TEST_CRYPTODEV_SNOW3G_TEST_VECTORS_H_ */
