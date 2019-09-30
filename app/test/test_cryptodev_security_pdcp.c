/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2015-2016 Freescale Semiconductor,Inc.
 * Copyright 2018-2019 NXP
 */

#include <time.h>

#include <rte_common.h>
#include <rte_hexdump.h>
#include <rte_mbuf.h>
#include <rte_malloc.h>
#include <rte_memcpy.h>
#include <rte_pause.h>
#include <rte_bus_vdev.h>
#include <rte_byteorder.h>

#include <rte_crypto.h>
#include <rte_cryptodev.h>
#include <rte_cryptodev_pmd.h>
#include <rte_security.h>

#include <rte_lcore.h>
#include "test.h"
#include "test_cryptodev_security_pdcp_test_func.h"

static int cplane_encap(uint32_t sn_size, uint8_t dir,
			enum enc_alg_off enc_alg_off,
			enum auth_alg_off auth_alg_off)
{
	int i = 0;

	switch (sn_size) {
	case 5:
		i = PDCP_CPLANE_OFFSET + enc_alg_off +
			auth_alg_off + ((dir == 0) ?
				UPLINK : DOWNLINK);
		break;
	case 12:
		i = PDCP_CPLANE_LONG_SN_OFFSET + enc_alg_off +
			auth_alg_off + ((dir == 0) ?
				UPLINK : DOWNLINK);
		break;
	default:
		printf("\nInvalid SN: %u for %s\n", sn_size, __func__);
	}

	return test_pdcp_proto_cplane_encap(i);
}

#define TEST_PDCP_COUNT(func) do {			\
	if (func == TEST_SUCCESS)  {			\
		printf("\t%d)", n++);			\
		printf(#func"-PASS\n");			\
		i++;					\
	} else {					\
		printf("\t%d)", n++);			\
		printf("+++++ FAILED:" #func"\n");	\
	}						\
} while (0)

int
test_PDCP_PROTO_cplane_encap_all(void)
{
	int i = 0, n = 0;

	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, NULL_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, NULL_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, NULL_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, NULL_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, NULL_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, NULL_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, NULL_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, NULL_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, SNOW_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, SNOW_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, SNOW_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, SNOW_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, SNOW_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, SNOW_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, SNOW_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, SNOW_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, AES_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, AES_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, AES_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, AES_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, AES_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, AES_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, AES_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, AES_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, ZUC_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, ZUC_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, ZUC_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, ZUC_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, ZUC_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, ZUC_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, UPLINK, ZUC_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(5, DOWNLINK, ZUC_ENC, ZUC_AUTH));

	/* For 12-bit SN */
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, NULL_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, NULL_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, NULL_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, NULL_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, NULL_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, NULL_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, NULL_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, NULL_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, SNOW_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, SNOW_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, SNOW_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, SNOW_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, SNOW_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, SNOW_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, SNOW_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, SNOW_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, AES_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, AES_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, AES_ENC, SNOW_AUTH));

	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, AES_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, AES_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, AES_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, AES_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, AES_ENC, ZUC_AUTH));

	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, ZUC_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, ZUC_ENC, NULL_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, ZUC_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, ZUC_ENC, SNOW_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, ZUC_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, ZUC_ENC, AES_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, UPLINK, ZUC_ENC, ZUC_AUTH));
	TEST_PDCP_COUNT(cplane_encap(12, DOWNLINK, ZUC_ENC, ZUC_AUTH));

	if (n - i)
		printf("## %s: %d passed out of %d\n", __func__, i, n);

	return n - i;
};
