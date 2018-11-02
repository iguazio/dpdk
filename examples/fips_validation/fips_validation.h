/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */

#ifndef _FIPS_VALIDATION_H_
#define _FIPS_VALIDATION_H_

#define FIPS_PARSE_ERR(fmt, args)					\
	RTE_LOG(ERR, USER1, "FIPS parse error" ## fmt ## "\n", ## args)

#define ERR_MSG_SIZE		128
#define MAX_CASE_LINE		15
#define MAX_LINE_CHAR		204800 /*< max number of characters per line */
#define MAX_NB_TESTS		10240
#define MAX_BUF_SIZE		2048
#define MAX_STRING_SIZE		64

#define POSITIVE_TEST		0
#define NEGATIVE_TEST		-1

#define REQ_FILE_PERFIX		"req"
#define RSP_FILE_PERFIX		"rsp"
#define FAX_FILE_PERFIX		"fax"

enum fips_test_algorithms {
		FIPS_TEST_ALGO_MAX
};

enum file_types {
	FIPS_TYPE_REQ = 1,
	FIPS_TYPE_FAX,
	FIPS_TYPE_RSP
};

enum fips_test_op {
	FIPS_TEST_ENC_AUTH_GEN = 1,
	FIPS_TEST_DEC_AUTH_VERIF,
};

#define MAX_LINE_PER_VECTOR            16

struct fips_val {
	uint8_t *val;
	uint32_t len;
};

struct fips_test_vector {
	union {
		struct {
			struct fips_val key;
			struct fips_val digest;
			struct fips_val auth_aad;
			struct fips_val aad;
		} cipher_auth;
		struct {
			struct fips_val key;
			struct fips_val digest;
			struct fips_val aad;
		} aead;
	};

	struct fips_val pt;
	struct fips_val ct;
	struct fips_val iv;

	enum rte_crypto_op_status status;
};

typedef int (*post_prcess_t)(struct fips_val *val);

typedef int (*parse_callback_t)(const char *key, char *text,
		struct fips_val *val);

struct fips_test_callback {
	const char *key;
	parse_callback_t cb;
	struct fips_val *val;
};

struct fips_test_interim_info {
	FILE *fp_rd;
	FILE *fp_wr;
	enum file_types file_type;
	enum fips_test_algorithms algo;
	char *one_line_text;
	char *vec[MAX_LINE_PER_VECTOR];
	uint32_t nb_vec_lines;
	char device_name[MAX_STRING_SIZE];

	enum fips_test_op op;

	const struct fips_test_callback *callbacks;
	const struct fips_test_callback *interim_callbacks;
	const struct fips_test_callback *writeback_callbacks;

	post_prcess_t parse_writeback;
	post_prcess_t kat_check;
};

extern struct fips_test_vector vec;
extern struct fips_test_interim_info info;

int
fips_test_init(const char *req_file_path, const char *rsp_file_path,
		const char *device_name);

void
fips_test_clear(void);

int
fips_test_fetch_one_block(void);

int
fips_test_parse_one_case(void);

void
fips_test_write_one_case(void);

int
parser_read_uint8_hex(uint8_t *value, const char *p);

int
parse_uint8_hex_str(const char *key, char *src, struct fips_val *val);

int
parse_uint8_known_len_hex_str(const char *key, char *src, struct fips_val *val);

int
parser_read_uint32_val(const char *key, char *src, struct fips_val *val);

int
parser_read_uint32_bit_val(const char *key, char *src, struct fips_val *val);

int
parser_read_uint32(uint32_t *value, char *p);

int
parser_read_uint32_val(const char *key, char *src, struct fips_val *val);

int
writeback_hex_str(const char *key, char *dst, struct fips_val *val);

void
parse_write_hex_str(struct fips_val *src);

int
update_info_vec(uint32_t count);

#endif
