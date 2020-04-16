/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2016 Intel Corporation
 */

#ifndef TEST_CRYPTODEV_DES_TEST_VECTORS_H_
#define TEST_CRYPTODEV_DES_TEST_VECTORS_H_

static const uint8_t plaintext_des[] = {
	"What a lousy earth! He wondered how many people "
	"were destitute that same night even in his own "
	"prosperous country, how many homes were "
	"shanties, how many husbands were drunk and "
	"wives socked, and how many children were "
	"bullied, abused, or abandoned. How many "
	"families hungered for food they could not "
	"afford to buy? How many hearts were broken? How "
	"many suicides would take place that same night, "
	"how many people would go insane? How many "
	"cockroaches and landlords would triumph? How "
	"many winners were losers, successes failures, "
	"and rich men poor men? How many wise guys were "
	"stupid? How many happy endings were unhappy "
	"endings? How many honest men were liars, brave "
	"men cowards, loyal men traitors, how many "
	"sainted men were corrupt, how many people in "
	"positions of trust had sold their souls to "
	"bodyguards, how many had never had souls? How "
	"many straight-and-narrow paths were crooked "
	"paths? How many best families were worst "
	"families and how many good people were bad "
	"people? When you added them all up and then "
	"subtracted, you might be left with only the "
	"children, and perhaps with Albert Einstein and "
	"an old violinist or sculptor somewhere."
};

static const uint8_t ciphertext512_des128ctr[] = {
	0x13, 0x39, 0x3B, 0xBC, 0x1D, 0xE3, 0x23, 0x09,
	0x9B, 0x08, 0xD1, 0x09, 0x52, 0x93, 0x78, 0x29,
	0x11, 0x21, 0xBA, 0x01, 0x15, 0xCD, 0xEC, 0xAA,
	0x79, 0x77, 0x58, 0xAE, 0xAE, 0xBC, 0x97, 0x33,
	0x94, 0xA9, 0x2D, 0xC0, 0x0A, 0xA9, 0xA4, 0x4B,
	0x19, 0x07, 0x88, 0x06, 0x7E, 0x81, 0x0F, 0xB5,
	0x60, 0xCF, 0xA7, 0xC3, 0x2A, 0x43, 0xFF, 0x16,
	0x3A, 0x5F, 0x11, 0x2D, 0x11, 0x38, 0x37, 0x94,
	0x2A, 0xC8, 0x3D, 0x20, 0xBB, 0x93, 0x95, 0x54,
	0x12, 0xFF, 0x0C, 0x47, 0x89, 0x7D, 0x73, 0xD1,
	0x2E, 0x3A, 0x80, 0x52, 0xA8, 0x92, 0x93, 0x99,
	0x16, 0xB8, 0x12, 0x1B, 0x8B, 0xA8, 0xC1, 0x81,
	0x95, 0x18, 0x82, 0xD6, 0x5A, 0xA7, 0xFE, 0xCF,
	0xC4, 0xAC, 0x85, 0x91, 0x0C, 0x2F, 0x1D, 0x10,
	0x9A, 0x65, 0x07, 0xB0, 0x2E, 0x5A, 0x2D, 0x48,
	0x26, 0xF8, 0x17, 0x7A, 0x53, 0xD6, 0xB8, 0xDF,
	0xB1, 0x10, 0x48, 0x7E, 0x8F, 0xBE, 0x2E, 0xA1,
	0x0D, 0x9E, 0xA9, 0xF1, 0x3B, 0x3B, 0x33, 0xCD,
	0xDC, 0x52, 0x7E, 0xC0, 0x0E, 0xA0, 0xD8, 0xA7,
	0xC6, 0x34, 0x5A, 0xAA, 0x29, 0x8B, 0xA9, 0xAC,
	0x1F, 0x78, 0xAD, 0xEE, 0x34, 0x59, 0x30, 0xFB,
	0x2A, 0x20, 0x3D, 0x4D, 0x30, 0xA7, 0x7D, 0xD8,
	0xA0, 0xC6, 0xA2, 0xD3, 0x9A, 0xFB, 0x50, 0x97,
	0x4D, 0x25, 0xA2, 0x37, 0x51, 0x54, 0xB7, 0xEB,
	0xED, 0x77, 0xDB, 0x94, 0x35, 0x8B, 0x70, 0x95,
	0x4A, 0x00, 0xA7, 0xF1, 0x8A, 0x66, 0x0E, 0xC6,
	0x05, 0x7B, 0x69, 0x05, 0x42, 0x03, 0x96, 0x2C,
	0x55, 0x00, 0x1B, 0xC0, 0x19, 0x4D, 0x0D, 0x2E,
	0xF5, 0x81, 0x11, 0x64, 0xCA, 0xBB, 0xF2, 0x0F,
	0x9C, 0x60, 0xE2, 0xCC, 0x02, 0x6E, 0x83, 0xD5,
	0x24, 0xF4, 0x12, 0x0E, 0x6A, 0xEA, 0x4F, 0x6C,
	0x79, 0x69, 0x65, 0x67, 0xDB, 0xF7, 0xEA, 0x98,
	0x5D, 0x56, 0x98, 0xB7, 0x88, 0xE7, 0x23, 0xC9,
	0x17, 0x32, 0x92, 0x33, 0x5A, 0x0C, 0x15, 0x20,
	0x3B, 0x1C, 0xF9, 0x0F, 0x4D, 0xD1, 0xE8, 0xE6,
	0x9E, 0x5E, 0x24, 0x1B, 0xA4, 0xB8, 0xB9, 0xE9,
	0x2F, 0xFC, 0x89, 0xB4, 0xB9, 0xF4, 0xA6, 0xAD,
	0x55, 0xF4, 0xDF, 0x58, 0x63, 0x25, 0xE3, 0x41,
	0x70, 0xDF, 0x10, 0xE7, 0x13, 0x87, 0x8D, 0xB3,
	0x62, 0x4F, 0xF5, 0x86, 0x85, 0x8F, 0x59, 0xF0,
	0x21, 0x0E, 0x8F, 0x11, 0xAD, 0xBF, 0xDD, 0x61,
	0x68, 0x3F, 0x54, 0x57, 0x49, 0x38, 0xC8, 0x24,
	0x8E, 0x0A, 0xAC, 0xCA, 0x2C, 0x36, 0x3E, 0x5F,
	0x0A, 0xCE, 0xFD, 0x1A, 0x60, 0x63, 0x5A, 0xE6,
	0x06, 0x64, 0xB5, 0x94, 0x3C, 0xC9, 0xAF, 0x7C,
	0xCD, 0x49, 0x10, 0xCF, 0xAF, 0x0E, 0x2E, 0x79,
	0x27, 0xB2, 0x67, 0x02, 0xED, 0xEE, 0x80, 0x77,
	0x7C, 0x6D, 0x4B, 0xDB, 0xCF, 0x8D, 0x68, 0x00,
	0x2E, 0xD9, 0xF0, 0x8E, 0x08, 0xBF, 0xA6, 0x9B,
	0xFE, 0xA4, 0xFB, 0x19, 0x46, 0xAF, 0x1B, 0xA9,
	0xF8, 0x22, 0x81, 0x21, 0x97, 0xFC, 0xC0, 0x8A,
	0x26, 0x58, 0x13, 0x29, 0xB6, 0x69, 0x94, 0x4B,
	0xAB, 0xB3, 0x88, 0x0D, 0xA9, 0x48, 0x0E, 0xE8,
	0x70, 0xFC, 0xA1, 0x21, 0xC4, 0x2C, 0xE5, 0x99,
	0xB4, 0xF1, 0x6F, 0xB2, 0x4B, 0x4B, 0xCD, 0x48,
	0x15, 0x47, 0x2D, 0x72, 0x39, 0x99, 0x9D, 0x24,
	0x0C, 0x8B, 0xDC, 0xA1, 0xEE, 0xF6, 0xF4, 0x73,
	0xC3, 0xB8, 0x0C, 0x23, 0x0D, 0xA7, 0xC4, 0x7D,
	0x27, 0xE2, 0x14, 0x11, 0x53, 0x19, 0xE7, 0xCA,
	0x94, 0x4E, 0x0D, 0x2C, 0xF7, 0x36, 0x47, 0xDB,
	0x77, 0x3C, 0x22, 0xAC, 0xBE, 0xE1, 0x06, 0x55,
	0xE5, 0xDD, 0x8B, 0x65, 0xE8, 0xE9, 0x91, 0x52,
	0x59, 0x97, 0xFC, 0x8C, 0xEE, 0x96, 0x22, 0x60,
	0xEE, 0xBF, 0x82, 0xF0, 0xCA, 0x14, 0xF9, 0xD3
};

static const struct blockcipher_test_data
triple_des128ctr_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CTR,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A
		},
		.len = 16
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des128ctr,
		.len = 512
	}
};

static const struct blockcipher_test_data
triple_des128ctr_sha1_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CTR,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A
		},
		.len = 16
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des128ctr,
		.len = 512
	},
	.auth_algo = RTE_CRYPTO_AUTH_SHA1,
	.digest = {
		.data = {
			0xC3, 0x40, 0xD5, 0xD9, 0x8F, 0x8A, 0xC0, 0xF0,
			0x46, 0x28, 0x02, 0x01, 0xB5, 0xC1, 0x87, 0x4D,
			0xAC, 0xFE, 0x48, 0x76
		},
		.len = 20
	}
};

static const struct blockcipher_test_data
triple_des128ctr_hmac_sha1_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CTR,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A
		},
		.len = 16
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des128ctr,
		.len = 512
	},
	.auth_algo = RTE_CRYPTO_AUTH_SHA1_HMAC,
	.auth_key = {
		.data = {
			0xF8, 0x2A, 0xC7, 0x54, 0xDB, 0x96, 0x18, 0xAA,
			0xC3, 0xA1, 0x53, 0xF6, 0x1F, 0x17, 0x60, 0xBD,
			0xDE, 0xF4, 0xDE, 0xAD
		},
		.len = 20
	},
	.digest = {
		.data = {
			0xF1, 0xC1, 0xDB, 0x4D, 0xFA, 0x7F, 0x2F, 0xE5,
			0xF8, 0x49, 0xEA, 0x1D, 0x7F, 0xCB, 0x42, 0x59,
			0xC4, 0x1E, 0xB1, 0x18
		},
		.len = 20
	}
};

static const uint8_t ciphertext512_des192ctr[] = {
	0xFF, 0x32, 0x52, 0x97, 0x10, 0xBF, 0x0B, 0x10,
	0x68, 0x0F, 0x4F, 0x56, 0x8B, 0x2C, 0x7B, 0x8E,
	0x39, 0x1E, 0x1A, 0x2F, 0x83, 0xDE, 0x5E, 0x35,
	0xC8, 0x4B, 0xDF, 0xD5, 0xBC, 0x84, 0x50, 0x1A,
	0x02, 0xDF, 0xB3, 0x11, 0xE4, 0xDA, 0xB8, 0x0E,
	0x47, 0xC6, 0x0C, 0x51, 0x09, 0x62, 0x9C, 0x5D,
	0x71, 0x40, 0x49, 0xD8, 0x55, 0xBD, 0x7D, 0x90,
	0x71, 0xC5, 0xF7, 0x07, 0x6F, 0x08, 0x71, 0x2A,
	0xB1, 0x77, 0x9B, 0x0F, 0xA1, 0xB0, 0xD6, 0x10,
	0xB2, 0xE5, 0x31, 0xEC, 0x21, 0x13, 0x89, 0x2A,
	0x09, 0x7E, 0x30, 0xDB, 0xA0, 0xF0, 0xDC, 0xE4,
	0x74, 0x64, 0x39, 0xA3, 0xB0, 0xB1, 0x80, 0x66,
	0x52, 0xD4, 0x4E, 0xC9, 0x5A, 0x52, 0x6A, 0xC7,
	0xB5, 0x2B, 0x61, 0xD5, 0x17, 0xD5, 0xF3, 0xCC,
	0x41, 0x61, 0xD2, 0xA6, 0xF4, 0x51, 0x24, 0x3A,
	0x63, 0x5D, 0x23, 0xB1, 0xF0, 0x22, 0xE7, 0x45,
	0xFA, 0x5F, 0x7E, 0x99, 0x00, 0x11, 0x28, 0x35,
	0xA3, 0xF4, 0x61, 0x94, 0x0E, 0x98, 0xCE, 0x35,
	0xDD, 0x91, 0x1B, 0x0B, 0x4D, 0xEE, 0xFF, 0xFF,
	0x0B, 0xD4, 0xDC, 0x56, 0xFC, 0x71, 0xE9, 0xEC,
	0xE8, 0x36, 0x51, 0xF8, 0x8B, 0x6A, 0xE1, 0x8C,
	0x2B, 0x25, 0x91, 0x91, 0x9B, 0x92, 0x76, 0xB5,
	0x3D, 0x26, 0xA8, 0x53, 0xEA, 0x30, 0x5B, 0x4D,
	0xDA, 0x16, 0xDA, 0x7D, 0x04, 0x88, 0xF5, 0x22,
	0xA8, 0x0C, 0xB9, 0x41, 0xC7, 0x91, 0x64, 0x86,
	0x99, 0x7D, 0x18, 0xB9, 0x67, 0xA2, 0x6E, 0x05,
	0x1A, 0x82, 0x8F, 0xA2, 0xEB, 0x4D, 0x0B, 0x8C,
	0x88, 0x2D, 0xBA, 0x77, 0x87, 0x32, 0x50, 0x3C,
	0x4C, 0xD8, 0xD3, 0x50, 0x39, 0xFA, 0xDF, 0x48,
	0x3E, 0x30, 0xF5, 0x76, 0x06, 0xB0, 0x1A, 0x05,
	0x60, 0x2C, 0xD3, 0xA0, 0x63, 0x1A, 0x19, 0x2D,
	0x6B, 0x76, 0xF2, 0x31, 0x4C, 0xA7, 0xE6, 0x5C,
	0x1B, 0x23, 0x20, 0x41, 0x32, 0xE5, 0x83, 0x47,
	0x04, 0xB6, 0x3E, 0xE0, 0xFD, 0x49, 0x1E, 0x1B,
	0x75, 0x10, 0x11, 0x46, 0xE9, 0xF9, 0x96, 0x9A,
	0xD7, 0x59, 0xFE, 0x38, 0x31, 0xFE, 0x79, 0xC4,
	0xC8, 0x46, 0x88, 0xDE, 0x2E, 0xAE, 0x20, 0xED,
	0x77, 0x50, 0x40, 0x38, 0x26, 0xD3, 0x35, 0xF6,
	0x29, 0x55, 0x6A, 0x6B, 0x38, 0x69, 0xFE, 0x90,
	0x5B, 0xA7, 0xFA, 0x6B, 0x73, 0x4F, 0xB9, 0x5D,
	0xDC, 0x6F, 0x98, 0xC3, 0x6A, 0xC4, 0xB5, 0x09,
	0xC5, 0x84, 0xA5, 0x6A, 0x84, 0xA4, 0xB3, 0x8A,
	0x5F, 0xCA, 0x92, 0x64, 0x9E, 0xC3, 0x0F, 0x84,
	0x8B, 0x2D, 0x48, 0xC6, 0x67, 0xAE, 0x07, 0xE0,
	0x28, 0x38, 0x6D, 0xC4, 0x4D, 0x13, 0x87, 0xE0,
	0xB2, 0x2F, 0xAA, 0xC0, 0xCF, 0x68, 0xD7, 0x9C,
	0xB8, 0x07, 0xE4, 0x51, 0xD7, 0x75, 0x86, 0xFA,
	0x0C, 0x50, 0x74, 0x68, 0x00, 0x64, 0x2A, 0x27,
	0x59, 0xE9, 0x80, 0xEB, 0xC2, 0xA3, 0xFA, 0x58,
	0xCC, 0x03, 0xE7, 0x7B, 0x66, 0x53, 0xFF, 0x90,
	0xA0, 0x85, 0xE2, 0xF8, 0x82, 0xFE, 0xC6, 0x2B,
	0xFF, 0x5E, 0x70, 0x85, 0x34, 0xB7, 0x22, 0x38,
	0xDB, 0xBC, 0x15, 0x30, 0x59, 0xC1, 0x48, 0x42,
	0xE5, 0x38, 0x8D, 0x37, 0x59, 0xDB, 0xA3, 0x20,
	0x17, 0x36, 0x1D, 0x4B, 0xBF, 0x4E, 0xA4, 0x35,
	0xCC, 0xFE, 0xF5, 0x7A, 0x73, 0xB4, 0x6D, 0x20,
	0x1D, 0xC0, 0xE5, 0x21, 0x5C, 0xD2, 0x8A, 0x65,
	0x08, 0xB6, 0x63, 0xAC, 0x9A, 0x1E, 0x3F, 0x3C,
	0xAB, 0xB6, 0x6D, 0x34, 0xB2, 0x3A, 0x08, 0xDA,
	0x29, 0x63, 0xD1, 0xA4, 0x83, 0x52, 0xB0, 0x63,
	0x1B, 0x89, 0x35, 0x57, 0x59, 0x2C, 0x0F, 0x72,
	0x72, 0xFD, 0xA0, 0xAC, 0xDB, 0xB4, 0xA3, 0xA1,
	0x18, 0x10, 0x12, 0x97, 0x99, 0x63, 0x38, 0x98,
	0x96, 0xB5, 0x16, 0x07, 0x4E, 0xE9, 0x2C, 0x97
};

static const struct blockcipher_test_data
triple_des192ctr_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CTR,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A,
			0xD4, 0xC3, 0xA3, 0xAA, 0x33, 0x62, 0x61, 0xE0
		},
		.len = 24
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des192ctr,
		.len = 512
	}
};

static const struct blockcipher_test_data
triple_des192ctr_sha1_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CTR,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A,
			0xD4, 0xC3, 0xA3, 0xAA, 0x33, 0x62, 0x61, 0xE0
		},
		.len = 24
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des192ctr,
		.len = 512
	},
	.auth_algo = RTE_CRYPTO_AUTH_SHA1,
	.digest = {
		.data = {
			0xEA, 0x62, 0xB9, 0xB2, 0x78, 0x6C, 0x8E, 0xDB,
			0xA3, 0xB6, 0xFF, 0x23, 0x3A, 0x47, 0xD8, 0xC8,
			0xED, 0x5E, 0x20, 0x1D
		},
		.len = 20
	}
};

static const struct blockcipher_test_data
triple_des192ctr_hmac_sha1_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CTR,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A,
			0xD4, 0xC3, 0xA3, 0xAA, 0x33, 0x62, 0x61, 0xE0
		},
		.len = 24
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des192ctr,
		.len = 512
	},
	.auth_algo = RTE_CRYPTO_AUTH_SHA1_HMAC,
	.auth_key = {
		.data = {
			0xF8, 0x2A, 0xC7, 0x54, 0xDB, 0x96, 0x18, 0xAA,
			0xC3, 0xA1, 0x53, 0xF6, 0x1F, 0x17, 0x60, 0xBD,
			0xDE, 0xF4, 0xDE, 0xAD
		},
		.len = 20
	},
	.digest = {
		.data = {
			0x32, 0xD5, 0x19, 0x8F, 0x79, 0x3A, 0xAA, 0x7B,
			0x70, 0x67, 0x4E, 0x63, 0x88, 0xA3, 0x9A, 0x82,
			0x07, 0x33, 0x12, 0x94
		},
		.len = 20
	}
};

static const uint8_t ciphertext512_des128cbc[] = {
	0x28, 0x2a, 0xff, 0x15, 0x5c, 0xdf, 0xd9, 0x6b,
	0x54, 0xbc, 0x7b, 0xfb, 0xc5, 0x64, 0x4d, 0xdd,
	0x3e, 0xf2, 0x9e, 0xb7, 0x53, 0x65, 0x37, 0x05,
	0xe0, 0xdf, 0xae, 0xf7, 0xc9, 0x27, 0xe4, 0xec,
	0x11, 0x27, 0xc2, 0x9e, 0x02, 0x4e, 0x03, 0x3b,
	0x33, 0xf2, 0x66, 0x08, 0x24, 0x5f, 0xab, 0xc2,
	0x7e, 0x21, 0x19, 0x5d, 0x51, 0xc3, 0xe2, 0x97,
	0x6f, 0x2e, 0xb4, 0xaa, 0x34, 0x70, 0x88, 0x78,
	0x4e, 0xe7, 0x3d, 0xe1, 0x9f, 0x87, 0x1c, 0x8b,
	0xac, 0x8d, 0xa1, 0x1a, 0xcd, 0xb0, 0xf8, 0xb6,
	0x24, 0x36, 0xe3, 0x8c, 0x07, 0xe7, 0xe4, 0x92,
	0x13, 0x86, 0x6f, 0x13, 0xec, 0x04, 0x5c, 0xe9,
	0xb9, 0xca, 0x45, 0x8a, 0x2c, 0x46, 0xda, 0x54,
	0x1d, 0xb5, 0x81, 0xb1, 0xcd, 0xf3, 0x7d, 0x11,
	0x6b, 0xb3, 0x0a, 0x45, 0xe5, 0x6e, 0x51, 0x3e,
	0x2c, 0xac, 0x7c, 0xbc, 0xa7, 0x7e, 0x22, 0x4d,
	0xe6, 0x02, 0xe3, 0x3f, 0x77, 0xd7, 0x73, 0x72,
	0x0e, 0xfb, 0x42, 0x85, 0x80, 0xdf, 0xa8, 0x91,
	0x60, 0x40, 0x48, 0xcd, 0x1b, 0xd9, 0xbf, 0x2f,
	0xf2, 0xdf, 0xd0, 0xbd, 0x3f, 0x82, 0xce, 0x15,
	0x9d, 0x6e, 0xc6, 0x59, 0x6f, 0x27, 0x0d, 0xf9,
	0x26, 0xe2, 0x11, 0x29, 0x50, 0xc3, 0x0a, 0xb7,
	0xde, 0x9d, 0xe9, 0x55, 0xa1, 0xe9, 0x01, 0x33,
	0x56, 0x51, 0xa7, 0x3a, 0x9e, 0x63, 0xc5, 0x08,
	0x01, 0x3b, 0x03, 0x4b, 0xc6, 0xc4, 0xa1, 0xc0,
	0xc0, 0xd0, 0x0e, 0x48, 0xe5, 0x4c, 0x55, 0x6b,
	0x4a, 0xc1, 0x0a, 0x24, 0x4b, 0xd0, 0x02, 0xf4,
	0x31, 0x63, 0x11, 0xbd, 0xa6, 0x1f, 0xf4, 0xae,
	0x23, 0x5a, 0x40, 0x7e, 0x0e, 0x4e, 0x63, 0x8b,
	0x66, 0x3d, 0x55, 0x46, 0x6e, 0x5c, 0x76, 0xa7,
	0x68, 0x31, 0xce, 0x5d, 0xca, 0xe2, 0xb4, 0xb0,
	0xc1, 0x1f, 0x66, 0x18, 0x75, 0x64, 0x73, 0xa9,
	0x9e, 0xd5, 0x0e, 0x0e, 0xf7, 0x77, 0x61, 0xf8,
	0x89, 0xc6, 0xcf, 0x0c, 0x41, 0xd3, 0x8f, 0xfd,
	0x22, 0x52, 0x4f, 0x94, 0x5c, 0x19, 0x11, 0x3a,
	0xb5, 0x63, 0xe8, 0x81, 0x33, 0x13, 0x54, 0x3c,
	0x93, 0x36, 0xb5, 0x5b, 0x51, 0xaf, 0x51, 0xa2,
	0x08, 0xae, 0x83, 0x15, 0x77, 0x07, 0x28, 0x0d,
	0x98, 0xe1, 0x2f, 0x69, 0x0e, 0xfb, 0x9a, 0x2e,
	0x27, 0x27, 0xb0, 0xd5, 0xce, 0xf8, 0x16, 0x55,
	0xfd, 0xaa, 0xd7, 0x1a, 0x1b, 0x2e, 0x4c, 0x86,
	0x7a, 0x6a, 0x90, 0xf7, 0x0a, 0x07, 0xd3, 0x81,
	0x4b, 0x75, 0x6a, 0x79, 0xdb, 0x63, 0x45, 0x0f,
	0x31, 0x7e, 0xd0, 0x2a, 0x14, 0xff, 0xee, 0xcc,
	0x97, 0x8a, 0x7d, 0x74, 0xbd, 0x9d, 0xaf, 0x00,
	0xdb, 0x7e, 0xf3, 0xe6, 0x22, 0x76, 0x77, 0x58,
	0xba, 0x1c, 0x06, 0x96, 0xfb, 0x6f, 0x41, 0x71,
	0x66, 0x98, 0xae, 0x31, 0x7d, 0x29, 0x18, 0x71,
	0x0e, 0xe4, 0x98, 0x7e, 0x59, 0x5a, 0xc9, 0x78,
	0x9c, 0xfb, 0x6c, 0x81, 0x44, 0xb4, 0x0f, 0x5e,
	0x18, 0x53, 0xb8, 0x6f, 0xbc, 0x3b, 0x15, 0xf0,
	0x10, 0xdd, 0x0d, 0x4b, 0x0a, 0x36, 0x0e, 0xb4,
	0x76, 0x0f, 0x16, 0xa7, 0x5c, 0x9d, 0xcf, 0xb0,
	0x6d, 0x38, 0x02, 0x07, 0x05, 0xe9, 0xe9, 0x46,
	0x08, 0xb8, 0x52, 0xd6, 0xd9, 0x4c, 0x81, 0x63,
	0x1d, 0xe2, 0x5b, 0xd0, 0xf6, 0x5e, 0x1e, 0x81,
	0x48, 0x08, 0x66, 0x3a, 0x85, 0xed, 0x65, 0xfe,
	0xe8, 0x05, 0x7a, 0xe1, 0xe6, 0x12, 0xf2, 0x52,
	0x83, 0xdd, 0x82, 0xbe, 0xf6, 0x34, 0x8a, 0x6f,
	0xc5, 0x83, 0xcd, 0x3f, 0xbe, 0x58, 0x8b, 0x11,
	0x78, 0xdc, 0x0c, 0x83, 0x72, 0x5d, 0x05, 0x2a,
	0x01, 0x29, 0xee, 0x48, 0x9a, 0x67, 0x00, 0x6e,
	0x14, 0x60, 0x2d, 0x00, 0x52, 0x87, 0x98, 0x5e,
	0x43, 0xfe, 0xf1, 0x10, 0x14, 0xf1, 0x91, 0xcc
};


static const uint8_t ciphertext512_des[] = {
		0x1A, 0x46, 0xDB, 0x69, 0x43, 0x45, 0x0F, 0x2F,
		0xDC, 0x27, 0xF9, 0x41, 0x0E, 0x01, 0x58, 0xB4,
		0x5E, 0xCC, 0x13, 0xF5, 0x92, 0x99, 0xE4, 0xF2,
		0xD5, 0xF9, 0x16, 0xFE, 0x0F, 0x7E, 0xDE, 0xA0,
		0xF5, 0x32, 0xFE, 0x20, 0x67, 0x93, 0xCA, 0xE1,
		0x8E, 0x4D, 0x72, 0xA3, 0x50, 0x72, 0x14, 0x15,
		0x70, 0xE7, 0xAB, 0x49, 0x25, 0x88, 0x0E, 0x01,
		0x5C, 0x52, 0x87, 0xE2, 0x27, 0xDC, 0xD4, 0xD1,
		0x14, 0x1B, 0x08, 0x9F, 0x42, 0x48, 0x93, 0xA9,
		0xD1, 0x2F, 0x2C, 0x69, 0x48, 0x16, 0x59, 0xCF,
		0x8B, 0xF6, 0x8B, 0xD9, 0x34, 0xD4, 0xD7, 0xE4,
		0xAE, 0x35, 0xFD, 0xDA, 0x73, 0xBE, 0xDC, 0x6B,
		0x10, 0x90, 0x75, 0x2D, 0x4C, 0x14, 0x37, 0x8B,
		0xC8, 0xC7, 0xDF, 0x6E, 0x6F, 0xED, 0xF3, 0xE3,
		0xD3, 0x21, 0x29, 0xCD, 0x06, 0xB6, 0x5B, 0xF4,
		0xB9, 0xBD, 0x77, 0xA2, 0xF7, 0x91, 0xF4, 0x95,
		0xF0, 0xE0, 0x62, 0x03, 0x46, 0xAE, 0x1B, 0xEB,
		0xE2, 0xA9, 0xCF, 0xB9, 0x0E, 0x3B, 0xB9, 0xDA,
		0x5C, 0x1B, 0x45, 0x3F, 0xDD, 0xCC, 0xCC, 0xB3,
		0xF0, 0xDD, 0x36, 0x26, 0x11, 0x57, 0x97, 0xA7,
		0xF6, 0xF4, 0xE1, 0x4F, 0xBB, 0x31, 0xBB, 0x07,
		0x4B, 0xA3, 0xB4, 0x83, 0xF9, 0x23, 0xA1, 0xCD,
		0x8C, 0x1C, 0x76, 0x92, 0x45, 0xA5, 0xEB, 0x7D,
		0xEB, 0x22, 0x88, 0xB1, 0x9F, 0xFB, 0xE9, 0x06,
		0x8F, 0x67, 0xA6, 0x8A, 0xB7, 0x0B, 0xCD, 0x8F,
		0x34, 0x40, 0x4F, 0x4F, 0xAD, 0xA0, 0xF2, 0xDC,
		0x2C, 0x53, 0xE1, 0xCA, 0xA5, 0x7A, 0x03, 0xEF,
		0x08, 0x00, 0xCC, 0x52, 0xA6, 0xAB, 0x56, 0xD2,
		0xF1, 0xCD, 0xC7, 0xED, 0xBE, 0xCB, 0x78, 0x37,
		0x4B, 0x61, 0xA9, 0xD2, 0x3C, 0x8D, 0xCC, 0xFD,
		0x21, 0xFD, 0x0F, 0xE4, 0x4E, 0x3D, 0x6F, 0x8F,
		0x2A, 0xEC, 0x69, 0xFA, 0x20, 0x50, 0x99, 0x35,
		0xA1, 0xCC, 0x3B, 0xFD, 0xD6, 0xAC, 0xE9, 0xBE,
		0x14, 0xF1, 0xBC, 0x71, 0x70, 0xFE, 0x13, 0xD1,
		0x48, 0xCC, 0xBE, 0x7B, 0xCB, 0xC0, 0x20, 0xD9,
		0x28, 0xD7, 0xD4, 0x0F, 0x66, 0x7A, 0x60, 0xAB,
		0x20, 0xA9, 0x23, 0x41, 0x03, 0x34, 0xC3, 0x63,
		0x91, 0x69, 0x02, 0xD5, 0xBC, 0x41, 0xDA, 0xA8,
		0xD1, 0x48, 0xC9, 0x8E, 0x4F, 0xCD, 0x0F, 0x21,
		0x5B, 0x4D, 0x5F, 0xF5, 0x1B, 0x2A, 0x44, 0x10,
		0x16, 0xA7, 0xFD, 0xC0, 0x55, 0xE1, 0x98, 0xBB,
		0x76, 0xB5, 0xAB, 0x39, 0x6B, 0x9B, 0xAB, 0x85,
		0x45, 0x4B, 0x9C, 0x64, 0x7D, 0x78, 0x3F, 0x61,
		0x22, 0xB1, 0xDE, 0x0E, 0x39, 0x2B, 0x21, 0x26,
		0xE2, 0x1D, 0x5A, 0xD7, 0xAC, 0xDF, 0xD4, 0x12,
		0x69, 0xD1, 0xE8, 0x9B, 0x1A, 0xCE, 0x6C, 0xA0,
		0x3B, 0x23, 0xDC, 0x03, 0x2B, 0x97, 0x16, 0xD0,
		0xD0, 0x46, 0x98, 0x36, 0x53, 0xCE, 0x88, 0x6E,
		0xCA, 0x2C, 0x15, 0x0E, 0x49, 0xED, 0xBE, 0xE5,
		0xBF, 0xBD, 0x7B, 0xC2, 0x21, 0xE1, 0x09, 0xFF,
		0x71, 0xA8, 0xBE, 0x8F, 0xB4, 0x1D, 0x25, 0x5C,
		0x37, 0xCA, 0x26, 0xD2, 0x1E, 0x63, 0xE1, 0x7F,
		0x0D, 0x89, 0x10, 0xEF, 0x78, 0xB0, 0xDB, 0xD0,
		0x72, 0x44, 0x60, 0x1D, 0xCF, 0x7C, 0x25, 0x1A,
		0xBB, 0xC3, 0x92, 0x53, 0x8E, 0x9F, 0x27, 0xC7,
		0xE8, 0x08, 0xFC, 0x5D, 0x50, 0x3E, 0xFC, 0xB0,
		0x00, 0xE2, 0x48, 0xB2, 0x4B, 0xF8, 0xF2, 0xE3,
		0xD3, 0x8B, 0x71, 0x64, 0xB8, 0xF0, 0x6E, 0x4A,
		0x23, 0xA0, 0xA4, 0x88, 0xA4, 0x36, 0x45, 0x6B,
		0x5A, 0xE7, 0x57, 0x65, 0xEA, 0xC9, 0xF8, 0xE8,
		0x7A, 0x80, 0x22, 0x67, 0x1A, 0x05, 0xF2, 0x78,
		0x81, 0x17, 0xCD, 0x87, 0xFB, 0x0D, 0x25, 0x84,
		0x49, 0x06, 0x25, 0xCE, 0xFC, 0x38, 0x06, 0x18,
		0x2E, 0x1D, 0xE1, 0x33, 0x97, 0xB6, 0x7E, 0xAB,
};


static const struct blockcipher_test_data
triple_des128cbc_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CBC,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A
		},
		.len = 16
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des128cbc,
		.len = 512
	}
};

static const struct blockcipher_test_data
triple_des128cbc_sha1_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CBC,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A
		},
		.len = 16
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des128cbc,
		.len = 512
	},
	.auth_algo = RTE_CRYPTO_AUTH_SHA1,
	.digest = {
		.data = {
			0x94, 0x45, 0x7B, 0xDF, 0xFE, 0x80, 0xB9, 0xA6,
			0xA0, 0x7A, 0xE8, 0x93, 0x40, 0x7B, 0x85, 0x02,
			0x1C, 0xD7, 0xE8, 0x87
		},
		.len = 20
	}
};

static const struct blockcipher_test_data
triple_des128cbc_hmac_sha1_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CBC,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A
		},
		.len = 16
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des128cbc,
		.len = 512
	},
	.auth_algo = RTE_CRYPTO_AUTH_SHA1_HMAC,
	.auth_key = {
		.data = {
			0xF8, 0x2A, 0xC7, 0x54, 0xDB, 0x96, 0x18, 0xAA,
			0xC3, 0xA1, 0x53, 0xF6, 0x1F, 0x17, 0x60, 0xBD,
			0xDE, 0xF4, 0xDE, 0xAD
		},
		.len = 20
	},
	.digest = {
		.data = {
			0x7E, 0xBA, 0xFF, 0x86, 0x8D, 0x65, 0xCD, 0x08,
			0x76, 0x34, 0x94, 0xE9, 0x9A, 0xCD, 0xB2, 0xBB,
			0xBF, 0x65, 0xF5, 0x42
		},
		.len = 20
	}
};

static const uint8_t ciphertext512_des192cbc[] = {
	0xd0, 0xc9, 0xdc, 0x51, 0x29, 0x97, 0x03, 0x64,
	0xcd, 0x22, 0xba, 0x3d, 0x2b, 0xbc, 0x21, 0x37,
	0x7b, 0x1e, 0x29, 0x23, 0xeb, 0x51, 0x6e, 0xac,
	0xbe, 0x5b, 0xd3, 0x67, 0xe0, 0x3f, 0xc3, 0xb5,
	0xe3, 0x04, 0x17, 0x42, 0x2b, 0xaa, 0xdd, 0xd6,
	0x0e, 0x69, 0xd0, 0x8f, 0x8a, 0xfc, 0xb4, 0x55,
	0x67, 0x06, 0x51, 0xbb, 0x00, 0x57, 0xee, 0x95,
	0x28, 0x79, 0x3f, 0xd9, 0x97, 0x2b, 0xb0, 0x02,
	0x35, 0x08, 0xce, 0x7a, 0xc3, 0x43, 0x2c, 0x87,
	0xaa, 0x97, 0x6a, 0xad, 0xf0, 0x26, 0xea, 0x1d,
	0xbb, 0x08, 0xe9, 0x52, 0x11, 0xd3, 0xaf, 0x36,
	0x17, 0x14, 0x21, 0xb2, 0xbc, 0x42, 0x51, 0x33,
	0x27, 0x8c, 0xd8, 0x45, 0xb9, 0x76, 0xa0, 0x11,
	0x24, 0x34, 0xde, 0x4d, 0x13, 0x67, 0x1b, 0xc3,
	0x31, 0x12, 0x66, 0x56, 0x59, 0xd2, 0xb1, 0x8f,
	0xec, 0x1e, 0xc0, 0x10, 0x7a, 0x86, 0xb1, 0x60,
	0xc3, 0x01, 0xd6, 0xa8, 0x55, 0xad, 0x58, 0x63,
	0xca, 0x68, 0xa9, 0x33, 0xe3, 0x93, 0x90, 0x7d,
	0x8f, 0xca, 0xf8, 0x1c, 0xc2, 0x9e, 0xfb, 0xde,
	0x9c, 0xc7, 0xf2, 0x6c, 0xff, 0xcc, 0x39, 0x17,
	0x49, 0x33, 0x0d, 0x7c, 0xed, 0x07, 0x99, 0x91,
	0x91, 0x6c, 0x5f, 0x3f, 0x02, 0x09, 0xdc, 0x70,
	0xf9, 0x3b, 0x8d, 0xaa, 0xf4, 0xbc, 0x0e, 0xec,
	0xf2, 0x26, 0xfb, 0xb2, 0x1c, 0x31, 0xae, 0xc6,
	0x72, 0xe8, 0x0b, 0x75, 0x05, 0x57, 0x58, 0x98,
	0x92, 0x37, 0x27, 0x8e, 0x3b, 0x0c, 0x25, 0xfb,
	0xcf, 0x82, 0x02, 0xd5, 0x0b, 0x1f, 0x89, 0x49,
	0xcd, 0x0f, 0xa1, 0xa7, 0x08, 0x63, 0x56, 0xa7,
	0x1f, 0x80, 0x3a, 0xef, 0x24, 0x89, 0x57, 0x1a,
	0x02, 0xdc, 0x2e, 0x51, 0xbd, 0x4a, 0x10, 0x23,
	0xfc, 0x02, 0x1a, 0x3f, 0x34, 0xbf, 0x1c, 0x98,
	0x1a, 0x40, 0x0a, 0x96, 0x8e, 0x41, 0xd5, 0x09,
	0x55, 0x37, 0xe9, 0x25, 0x11, 0x83, 0xf8, 0xf3,
	0xd4, 0xb0, 0xdb, 0x16, 0xd7, 0x51, 0x7e, 0x94,
	0xf7, 0xb4, 0x26, 0xe0, 0xf4, 0x80, 0x01, 0x65,
	0x51, 0xeb, 0xbc, 0xb0, 0x65, 0x8f, 0xdd, 0xb5,
	0xf7, 0x00, 0xec, 0x40, 0xab, 0x7d, 0x96, 0xcc,
	0x8d, 0xec, 0x89, 0x80, 0x31, 0x39, 0xa2, 0x5c,
	0xb0, 0x55, 0x4c, 0xee, 0xdd, 0x15, 0x2b, 0xa9,
	0x86, 0x4e, 0x23, 0x14, 0x36, 0xc5, 0x57, 0xf5,
	0xe3, 0xe8, 0x89, 0xc9, 0xb7, 0xf8, 0xeb, 0x08,
	0xe5, 0x93, 0x12, 0x5c, 0x0f, 0x79, 0xa1, 0x86,
	0xe4, 0xc2, 0xeb, 0xa6, 0xa0, 0x50, 0x6a, 0xec,
	0xd3, 0xce, 0x50, 0x78, 0x4e, 0x4f, 0x93, 0xd8,
	0xdc, 0xb4, 0xec, 0x02, 0xe9, 0xbd, 0x17, 0x99,
	0x1e, 0x16, 0x4e, 0xd7, 0xb0, 0x07, 0x02, 0x55,
	0x63, 0x24, 0x4f, 0x7b, 0x8f, 0xc5, 0x7a, 0x12,
	0x29, 0xff, 0x5d, 0xc1, 0xe7, 0xae, 0x48, 0xc8,
	0x57, 0x53, 0xe7, 0xcd, 0x10, 0x6c, 0x19, 0xfc,
	0xcc, 0xb9, 0xb1, 0xbe, 0x48, 0x9f, 0x2d, 0x3f,
	0x39, 0x2e, 0xdd, 0x71, 0xde, 0x1b, 0x54, 0xee,
	0x7d, 0x94, 0x8f, 0x27, 0x23, 0xe9, 0x74, 0x92,
	0x14, 0x93, 0x84, 0x65, 0xc9, 0x22, 0x7c, 0xa8,
	0x1b, 0x72, 0x73, 0xb1, 0x23, 0xa0, 0x6b, 0xcc,
	0xb5, 0x22, 0x06, 0x15, 0xe5, 0x96, 0x03, 0x4a,
	0x52, 0x8d, 0x1d, 0xbf, 0x3e, 0x82, 0x45, 0x9c,
	0x75, 0x9e, 0xa9, 0x3a, 0x97, 0xb6, 0x5d, 0xc4,
	0x75, 0x67, 0xa1, 0xf3, 0x0f, 0x7a, 0xfd, 0x71,
	0x58, 0x04, 0xf9, 0xa7, 0xc2, 0x56, 0x74, 0x04,
	0x74, 0x68, 0x6d, 0x8a, 0xf6, 0x6c, 0x5d, 0xd8,
	0xb5, 0xed, 0x70, 0x23, 0x32, 0x4d, 0x75, 0x92,
	0x88, 0x7b, 0x39, 0x37, 0x02, 0x4b, 0xb2, 0x1c,
	0x1f, 0x7e, 0x5b, 0x1b, 0x10, 0xfc, 0x17, 0x21,
	0x66, 0x62, 0x63, 0xc2, 0xcd, 0x16, 0x96, 0x3e
};

static const struct blockcipher_test_data
triple_des192cbc_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CBC,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A,
			0xD4, 0xC3, 0xA3, 0xAA, 0x33, 0x62, 0x61, 0xE0
		},
		.len = 24
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des192cbc,
		.len = 512
	}
};

static const struct blockcipher_test_data
triple_des192cbc_sha1_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CBC,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A,
			0xD4, 0xC3, 0xA3, 0xAA, 0x33, 0x62, 0x61, 0xE0
		},
		.len = 24
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des192cbc,
		.len = 512
	},
	.auth_algo = RTE_CRYPTO_AUTH_SHA1,
	.digest = {
		.data = {
			0x53, 0x27, 0xC0, 0xE6, 0xD6, 0x1B, 0xD6, 0x45,
			0x94, 0x2D, 0xCE, 0x8B, 0x29, 0xA3, 0x52, 0x14,
			0xC1, 0x6B, 0x87, 0x99
		},
		.len = 20
	}
};

static const struct blockcipher_test_data
triple_des192cbc_hmac_sha1_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CBC,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
			0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A,
			0xD4, 0xC3, 0xA3, 0xAA, 0x33, 0x62, 0x61, 0xE0
		},
		.len = 24
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des192cbc,
		.len = 512
	},
	.auth_algo = RTE_CRYPTO_AUTH_SHA1_HMAC,
	.auth_key = {
		.data = {
			0xF8, 0x2A, 0xC7, 0x54, 0xDB, 0x96, 0x18, 0xAA,
			0xC3, 0xA1, 0x53, 0xF6, 0x1F, 0x17, 0x60, 0xBD,
			0xDE, 0xF4, 0xDE, 0xAD
		},
		.len = 20
	},
	.digest = {
		.data = {
			0xBA, 0xAC, 0x74, 0x19, 0x43, 0xB0, 0x72, 0xB8,
			0x08, 0xF5, 0x24, 0xC4, 0x09, 0xBD, 0x48, 0xC1,
			0x3C, 0x50, 0x1C, 0xDD
		},
		.len = 20
	}
};
static const struct blockcipher_test_data
triple_des64cbc_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_3DES_CBC,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2
		},
		.len = 8
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des,
		.len = 512
	},
};

static const struct blockcipher_test_data
des_cbc_test_vector = {
	.crypto_algo = RTE_CRYPTO_CIPHER_DES_CBC,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2
		},
		.len = 8
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des,
		.len = 512
	},
};

static const struct blockcipher_test_case des_cipheronly_test_cases[] = {
	{
		.test_descr = "DES-CBC Encryption",
		.test_data = &des_cbc_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
	},
	{
		.test_descr = "DES-CBC Decryption",
		.test_data = &des_cbc_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
	},

};

/* DES-DOCSIS-BPI test vectors */

static const uint8_t plaintext_des_docsis_bpi_cfb[] = {
	0x00, 0x01, 0x02, 0x88, 0xEE, 0x59, 0x7E
};

static const uint8_t ciphertext_des_docsis_bpi_cfb[] = {
	0x17, 0x86, 0xA8, 0x03, 0xA0, 0x85, 0x75
};

static const uint8_t plaintext_des_docsis_bpi_cbc_cfb[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x91,
	0xD2, 0xD1, 0x9F
};

static const uint8_t ciphertext_des_docsis_bpi_cbc_cfb[] = {
	0x0D, 0xDA, 0x5A, 0xCB, 0xD0, 0x5E, 0x55, 0x67,
	0x51, 0x47, 0x46, 0x86, 0x8A, 0x71, 0xE5, 0x77,
	0xEF, 0xAC, 0x88
};

/* Multiple of DES block size */
static const struct blockcipher_test_data des_test_data_1 = {
	.crypto_algo = RTE_CRYPTO_CIPHER_DES_DOCSISBPI,
	.cipher_key = {
		.data = {
			0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2
		},
		.len = 8
	},
	.iv = {
		.data = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des,
		.len = 512
	},
	.ciphertext = {
		.data = ciphertext512_des,
		.len = 512
	},
};

/* Less than DES block size */
static const struct blockcipher_test_data des_test_data_2 = {
	.crypto_algo = RTE_CRYPTO_CIPHER_DES_DOCSISBPI,
	.cipher_key = {
		.data = {

			0xE6, 0x60, 0x0F, 0xD8, 0x85, 0x2E, 0xF5, 0xAB
		},
		.len = 8
	},
	.iv = {
		.data = {
			0x81, 0x0E, 0x52, 0x8E, 0x1C, 0x5F, 0xDA, 0x1A
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des_docsis_bpi_cfb,
		.len = 7
	},
	.ciphertext = {
		.data = ciphertext_des_docsis_bpi_cfb,
		.len = 7
	}
};

/* Not multiple of DES block size */
static const struct blockcipher_test_data des_test_data_3 = {
	.crypto_algo = RTE_CRYPTO_CIPHER_DES_DOCSISBPI,
	.cipher_key = {
		.data = {
			0xE6, 0x60, 0x0F, 0xD8, 0x85, 0x2E, 0xF5, 0xAB
		},
		.len = 8
	},
	.iv = {
		.data = {
			0x81, 0x0E, 0x52, 0x8E, 0x1C, 0x5F, 0xDA, 0x1A
		},
		.len = 8
	},
	.plaintext = {
		.data = plaintext_des_docsis_bpi_cbc_cfb,
		.len = 19
	},
	.ciphertext = {
		.data = ciphertext_des_docsis_bpi_cbc_cfb,
		.len = 19
	}
};
static const struct blockcipher_test_case des_docsis_test_cases[] = {
	{
		.test_descr = "DES-DOCSIS-BPI Full Block Encryption",
		.test_data = &des_test_data_1,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
	},
	{
		.test_descr = "DES-DOCSIS-BPI Runt Block Encryption",
		.test_data = &des_test_data_2,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
	},
	{
		.test_descr = "DES-DOCSIS-BPI Uneven Encryption",
		.test_data = &des_test_data_3,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
	},
	{
		.test_descr = "DES-DOCSIS-BPI Full Block Decryption",
		.test_data = &des_test_data_1,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
	},
	{
		.test_descr = "DES-DOCSIS-BPI Runt Block Decryption",
		.test_data = &des_test_data_2,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
	},
	{
		.test_descr = "DES-DOCSIS-BPI Uneven Decryption",
		.test_data = &des_test_data_3,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
	},
	{
		.test_descr = "DES-DOCSIS-BPI OOP Full Block Encryption",
		.test_data = &des_test_data_1,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
		.feature_mask = BLOCKCIPHER_TEST_FEATURE_OOP,
	},
	{
		.test_descr = "DES-DOCSIS-BPI OOP Runt Block Encryption",
		.test_data = &des_test_data_2,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
		.feature_mask = BLOCKCIPHER_TEST_FEATURE_OOP,
	},
	{
		.test_descr = "DES-DOCSIS-BPI OOP Uneven Encryption",
		.test_data = &des_test_data_3,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
		.feature_mask = BLOCKCIPHER_TEST_FEATURE_OOP,
	},
	{
		.test_descr = "DES-DOCSIS-BPI OOP Full Block Decryption",
		.test_data = &des_test_data_1,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
		.feature_mask = BLOCKCIPHER_TEST_FEATURE_OOP,
	},
	{
		.test_descr = "DES-DOCSIS-BPI OOP Runt Block Decryption",
		.test_data = &des_test_data_2,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
		.feature_mask = BLOCKCIPHER_TEST_FEATURE_OOP,
	},
	{
		.test_descr = "DES-DOCSIS-BPI OOP Uneven Decryption",
		.test_data = &des_test_data_3,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
		.feature_mask = BLOCKCIPHER_TEST_FEATURE_OOP,
	}
};

static const struct blockcipher_test_case triple_des_chain_test_cases[] = {
	{
		.test_descr = "3DES-128-CBC HMAC-SHA1 Encryption Digest",
		.test_data = &triple_des128cbc_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENC_AUTH_GEN,
	},
	{
		.test_descr = "3DES-128-CBC HMAC-SHA1 Decryption Digest Verify",
		.test_data = &triple_des128cbc_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_AUTH_VERIFY_DEC,
	},
	{
		.test_descr = "3DES-128-CBC SHA1 Encryption Digest",
		.test_data = &triple_des128cbc_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENC_AUTH_GEN,
	},
	{
		.test_descr = "3DES-128-CBC SHA1 Decryption Digest Verify",
		.test_data = &triple_des128cbc_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_AUTH_VERIFY_DEC,
	},
	{
		.test_descr = "3DES-192-CBC HMAC-SHA1 Encryption Digest",
		.test_data = &triple_des192cbc_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENC_AUTH_GEN,
	},
	{
		.test_descr = "3DES-192-CBC HMAC-SHA1 Decryption Digest Verify",
		.test_data = &triple_des192cbc_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_AUTH_VERIFY_DEC,
	},
	{
		.test_descr = "3DES-192-CBC SHA1 Encryption Digest",
		.test_data = &triple_des192cbc_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENC_AUTH_GEN,
	},
	{
		.test_descr = "3DES-192-CBC SHA1 Decryption Digest Verify",
		.test_data = &triple_des192cbc_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_AUTH_VERIFY_DEC,
	},
	{
		.test_descr = "3DES-128-CTR HMAC-SHA1 Encryption Digest",
		.test_data = &triple_des128ctr_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENC_AUTH_GEN,
	},
	{
		.test_descr = "3DES-128-CTR HMAC-SHA1 Decryption Digest Verify",
		.test_data = &triple_des128ctr_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_AUTH_VERIFY_DEC,
	},
	{
		.test_descr = "3DES-128-CTR SHA1 Encryption Digest",
		.test_data = &triple_des128ctr_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENC_AUTH_GEN,
	},
	{
		.test_descr = "3DES-128-CTR SHA1 Decryption Digest Verify",
		.test_data = &triple_des128ctr_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_AUTH_VERIFY_DEC,
	},
	{
		.test_descr = "3DES-192-CTR HMAC-SHA1 Encryption Digest",
		.test_data = &triple_des192ctr_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENC_AUTH_GEN,
	},
	{
		.test_descr = "3DES-192-CTR HMAC-SHA1 Decryption Digest Verify",
		.test_data = &triple_des192ctr_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_AUTH_VERIFY_DEC,
	},
	{
		.test_descr = "3DES-192-CTR SHA1 Encryption Digest",
		.test_data = &triple_des192ctr_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENC_AUTH_GEN,
	},
	{
		.test_descr = "3DES-192-CTR SHA1 Decryption Digest Verify",
		.test_data = &triple_des192ctr_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_AUTH_VERIFY_DEC,
	},
	{
		.test_descr = "3DES-128-CBC HMAC-SHA1 Encryption Digest OOP",
		.test_data = &triple_des128cbc_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENC_AUTH_GEN,
		.feature_mask = BLOCKCIPHER_TEST_FEATURE_OOP,
	},
	{
		.test_descr = "3DES-128-CBC HMAC-SHA1 Decryption Digest"
				" Verify OOP",
		.test_data = &triple_des128cbc_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_AUTH_VERIFY_DEC,
		.feature_mask = BLOCKCIPHER_TEST_FEATURE_OOP,
	},
	{
		.test_descr = "3DES-128-CBC HMAC-SHA1 Encryption Digest"
				" Sessionless",
		.test_data = &triple_des128cbc_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENC_AUTH_GEN,
		.feature_mask = BLOCKCIPHER_TEST_FEATURE_SESSIONLESS,
	},
	{
		.test_descr =
				"3DES-128-CBC HMAC-SHA1 Decryption Digest"
				" Verify Sessionless",
		.test_data = &triple_des128cbc_hmac_sha1_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_AUTH_VERIFY_DEC,
		.feature_mask = BLOCKCIPHER_TEST_FEATURE_SESSIONLESS,
	},
};

static const struct blockcipher_test_case triple_des_cipheronly_test_cases[] = {
	{
		.test_descr = "3DES-64-CBC Encryption",
		.test_data = &triple_des64cbc_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
	},
	{
		.test_descr = "3DES-64-CBC Decryption",
		.test_data = &triple_des64cbc_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
	},
	{
		.test_descr = "3DES-128-CBC Encryption",
		.test_data = &triple_des128cbc_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
	},
	{
		.test_descr = "3DES-128-CBC Decryption",
		.test_data = &triple_des128cbc_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
	},
	{
		.test_descr = "3DES-192-CBC Encryption",
		.test_data = &triple_des192cbc_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
	},
	{
		.test_descr = "3DES-192-CBC Decryption",
		.test_data = &triple_des192cbc_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
	},
	{
		.test_descr = "3DES-128-CTR Encryption",
		.test_data = &triple_des128ctr_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
	},
	{
		.test_descr = "3DES-128-CTR Decryption",
		.test_data = &triple_des128ctr_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
	},
	{
		.test_descr = "3DES-192-CTR Encryption",
		.test_data = &triple_des192ctr_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_ENCRYPT,
	},
	{
		.test_descr = "3DES-192-CTR Decryption",
		.test_data = &triple_des192ctr_test_vector,
		.op_mask = BLOCKCIPHER_TEST_OP_DECRYPT,
	}
};

#endif /* TEST_CRYPTODEV_DES_TEST_VECTORS_H_ */
