/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/queue.h>

#include <rte_common.h>
#include <rte_errno.h>
#include <rte_debug.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_random.h>
#include <rte_cycles.h>
#include <rte_malloc.h>

#include "test.h"

#define MBUF_DATA_SIZE          2048
#define NB_MBUF                 128
#define MBUF_TEST_DATA_LEN      1464
#define MBUF_TEST_DATA_LEN2     50
#define MBUF_TEST_HDR1_LEN      20
#define MBUF_TEST_HDR2_LEN      30
#define MBUF_TEST_ALL_HDRS_LEN  (MBUF_TEST_HDR1_LEN+MBUF_TEST_HDR2_LEN)

/* chain length in bulk test */
#define CHAIN_LEN 16

/* size of private data for mbuf in pktmbuf_pool2 */
#define MBUF2_PRIV_SIZE         128

#define REFCNT_MAX_ITER         64
#define REFCNT_MAX_TIMEOUT      10
#define REFCNT_MAX_REF          (RTE_MAX_LCORE)
#define REFCNT_MBUF_NUM         64
#define REFCNT_RING_SIZE        (REFCNT_MBUF_NUM * REFCNT_MAX_REF)

#define MAGIC_DATA              0x42424242

#define MAKE_STRING(x)          # x

#ifdef RTE_MBUF_REFCNT_ATOMIC

static volatile uint32_t refcnt_stop_slaves;
static unsigned refcnt_lcore[RTE_MAX_LCORE];

#endif

/*
 * MBUF
 * ====
 *
 * #. Allocate a mbuf pool.
 *
 *    - The pool contains NB_MBUF elements, where each mbuf is MBUF_SIZE
 *      bytes long.
 *
 * #. Test multiple allocations of mbufs from this pool.
 *
 *    - Allocate NB_MBUF and store pointers in a table.
 *    - If an allocation fails, return an error.
 *    - Free all these mbufs.
 *    - Repeat the same test to check that mbufs were freed correctly.
 *
 * #. Test data manipulation in pktmbuf.
 *
 *    - Alloc an mbuf.
 *    - Append data using rte_pktmbuf_append().
 *    - Test for error in rte_pktmbuf_append() when len is too large.
 *    - Trim data at the end of mbuf using rte_pktmbuf_trim().
 *    - Test for error in rte_pktmbuf_trim() when len is too large.
 *    - Prepend a header using rte_pktmbuf_prepend().
 *    - Test for error in rte_pktmbuf_prepend() when len is too large.
 *    - Remove data at the beginning of mbuf using rte_pktmbuf_adj().
 *    - Test for error in rte_pktmbuf_adj() when len is too large.
 *    - Check that appended data is not corrupt.
 *    - Free the mbuf.
 *    - Between all these tests, check data_len and pkt_len, and
 *      that the mbuf is contiguous.
 *    - Repeat the test to check that allocation operations
 *      reinitialize the mbuf correctly.
 *
 * #. Test packet cloning
 *    - Clone a mbuf and verify the data
 *    - Clone the cloned mbuf and verify the data
 *    - Attach a mbuf to another that does not have the same priv_size.
 */

#define GOTO_FAIL(str, ...) do {					\
		printf("mbuf test FAILED (l.%d): <" str ">\n",		\
		       __LINE__,  ##__VA_ARGS__);			\
		goto fail;						\
} while(0)

/*
 * test data manipulation in mbuf with non-ascii data
 */
static int
test_pktmbuf_with_non_ascii_data(struct rte_mempool *pktmbuf_pool)
{
	struct rte_mbuf *m = NULL;
	char *data;

	m = rte_pktmbuf_alloc(pktmbuf_pool);
	if (m == NULL)
		GOTO_FAIL("Cannot allocate mbuf");
	if (rte_pktmbuf_pkt_len(m) != 0)
		GOTO_FAIL("Bad length");

	data = rte_pktmbuf_append(m, MBUF_TEST_DATA_LEN);
	if (data == NULL)
		GOTO_FAIL("Cannot append data");
	if (rte_pktmbuf_pkt_len(m) != MBUF_TEST_DATA_LEN)
		GOTO_FAIL("Bad pkt length");
	if (rte_pktmbuf_data_len(m) != MBUF_TEST_DATA_LEN)
		GOTO_FAIL("Bad data length");
	memset(data, 0xff, rte_pktmbuf_pkt_len(m));
	if (!rte_pktmbuf_is_contiguous(m))
		GOTO_FAIL("Buffer should be continuous");
	rte_pktmbuf_dump(stdout, m, MBUF_TEST_DATA_LEN);

	rte_pktmbuf_free(m);

	return 0;

fail:
	if(m) {
		rte_pktmbuf_free(m);
	}
	return -1;
}

/*
 * test data manipulation in mbuf
 */
static int
test_one_pktmbuf(struct rte_mempool *pktmbuf_pool)
{
	struct rte_mbuf *m = NULL;
	char *data, *data2, *hdr;
	unsigned i;

	printf("Test pktmbuf API\n");

	/* alloc a mbuf */

	m = rte_pktmbuf_alloc(pktmbuf_pool);
	if (m == NULL)
		GOTO_FAIL("Cannot allocate mbuf");
	if (rte_pktmbuf_pkt_len(m) != 0)
		GOTO_FAIL("Bad length");

	rte_pktmbuf_dump(stdout, m, 0);

	/* append data */

	data = rte_pktmbuf_append(m, MBUF_TEST_DATA_LEN);
	if (data == NULL)
		GOTO_FAIL("Cannot append data");
	if (rte_pktmbuf_pkt_len(m) != MBUF_TEST_DATA_LEN)
		GOTO_FAIL("Bad pkt length");
	if (rte_pktmbuf_data_len(m) != MBUF_TEST_DATA_LEN)
		GOTO_FAIL("Bad data length");
	memset(data, 0x66, rte_pktmbuf_pkt_len(m));
	if (!rte_pktmbuf_is_contiguous(m))
		GOTO_FAIL("Buffer should be continuous");
	rte_pktmbuf_dump(stdout, m, MBUF_TEST_DATA_LEN);
	rte_pktmbuf_dump(stdout, m, 2*MBUF_TEST_DATA_LEN);

	/* this append should fail */

	data2 = rte_pktmbuf_append(m, (uint16_t)(rte_pktmbuf_tailroom(m) + 1));
	if (data2 != NULL)
		GOTO_FAIL("Append should not succeed");

	/* append some more data */

	data2 = rte_pktmbuf_append(m, MBUF_TEST_DATA_LEN2);
	if (data2 == NULL)
		GOTO_FAIL("Cannot append data");
	if (rte_pktmbuf_pkt_len(m) != MBUF_TEST_DATA_LEN + MBUF_TEST_DATA_LEN2)
		GOTO_FAIL("Bad pkt length");
	if (rte_pktmbuf_data_len(m) != MBUF_TEST_DATA_LEN + MBUF_TEST_DATA_LEN2)
		GOTO_FAIL("Bad data length");
	if (!rte_pktmbuf_is_contiguous(m))
		GOTO_FAIL("Buffer should be continuous");

	/* trim data at the end of mbuf */

	if (rte_pktmbuf_trim(m, MBUF_TEST_DATA_LEN2) < 0)
		GOTO_FAIL("Cannot trim data");
	if (rte_pktmbuf_pkt_len(m) != MBUF_TEST_DATA_LEN)
		GOTO_FAIL("Bad pkt length");
	if (rte_pktmbuf_data_len(m) != MBUF_TEST_DATA_LEN)
		GOTO_FAIL("Bad data length");
	if (!rte_pktmbuf_is_contiguous(m))
		GOTO_FAIL("Buffer should be continuous");

	/* this trim should fail */

	if (rte_pktmbuf_trim(m, (uint16_t)(rte_pktmbuf_data_len(m) + 1)) == 0)
		GOTO_FAIL("trim should not succeed");

	/* prepend one header */

	hdr = rte_pktmbuf_prepend(m, MBUF_TEST_HDR1_LEN);
	if (hdr == NULL)
		GOTO_FAIL("Cannot prepend");
	if (data - hdr != MBUF_TEST_HDR1_LEN)
		GOTO_FAIL("Prepend failed");
	if (rte_pktmbuf_pkt_len(m) != MBUF_TEST_DATA_LEN + MBUF_TEST_HDR1_LEN)
		GOTO_FAIL("Bad pkt length");
	if (rte_pktmbuf_data_len(m) != MBUF_TEST_DATA_LEN + MBUF_TEST_HDR1_LEN)
		GOTO_FAIL("Bad data length");
	if (!rte_pktmbuf_is_contiguous(m))
		GOTO_FAIL("Buffer should be continuous");
	memset(hdr, 0x55, MBUF_TEST_HDR1_LEN);

	/* prepend another header */

	hdr = rte_pktmbuf_prepend(m, MBUF_TEST_HDR2_LEN);
	if (hdr == NULL)
		GOTO_FAIL("Cannot prepend");
	if (data - hdr != MBUF_TEST_ALL_HDRS_LEN)
		GOTO_FAIL("Prepend failed");
	if (rte_pktmbuf_pkt_len(m) != MBUF_TEST_DATA_LEN + MBUF_TEST_ALL_HDRS_LEN)
		GOTO_FAIL("Bad pkt length");
	if (rte_pktmbuf_data_len(m) != MBUF_TEST_DATA_LEN + MBUF_TEST_ALL_HDRS_LEN)
		GOTO_FAIL("Bad data length");
	if (!rte_pktmbuf_is_contiguous(m))
		GOTO_FAIL("Buffer should be continuous");
	memset(hdr, 0x55, MBUF_TEST_HDR2_LEN);

	rte_mbuf_sanity_check(m, 1);
	rte_mbuf_sanity_check(m, 0);
	rte_pktmbuf_dump(stdout, m, 0);

	/* this prepend should fail */

	hdr = rte_pktmbuf_prepend(m, (uint16_t)(rte_pktmbuf_headroom(m) + 1));
	if (hdr != NULL)
		GOTO_FAIL("prepend should not succeed");

	/* remove data at beginning of mbuf (adj) */

	if (data != rte_pktmbuf_adj(m, MBUF_TEST_ALL_HDRS_LEN))
		GOTO_FAIL("rte_pktmbuf_adj failed");
	if (rte_pktmbuf_pkt_len(m) != MBUF_TEST_DATA_LEN)
		GOTO_FAIL("Bad pkt length");
	if (rte_pktmbuf_data_len(m) != MBUF_TEST_DATA_LEN)
		GOTO_FAIL("Bad data length");
	if (!rte_pktmbuf_is_contiguous(m))
		GOTO_FAIL("Buffer should be continuous");

	/* this adj should fail */

	if (rte_pktmbuf_adj(m, (uint16_t)(rte_pktmbuf_data_len(m) + 1)) != NULL)
		GOTO_FAIL("rte_pktmbuf_adj should not succeed");

	/* check data */

	if (!rte_pktmbuf_is_contiguous(m))
		GOTO_FAIL("Buffer should be continuous");

	for (i=0; i<MBUF_TEST_DATA_LEN; i++) {
		if (data[i] != 0x66)
			GOTO_FAIL("Data corrupted at offset %u", i);
	}

	/* free mbuf */

	rte_pktmbuf_free(m);
	m = NULL;
	return 0;

fail:
	if (m)
		rte_pktmbuf_free(m);
	return -1;
}

static int
testclone_testupdate_testdetach(struct rte_mempool *pktmbuf_pool)
{
	struct rte_mbuf *m = NULL;
	struct rte_mbuf *clone = NULL;
	struct rte_mbuf *clone2 = NULL;
	unaligned_uint32_t *data;

	/* alloc a mbuf */
	m = rte_pktmbuf_alloc(pktmbuf_pool);
	if (m == NULL)
		GOTO_FAIL("ooops not allocating mbuf");

	if (rte_pktmbuf_pkt_len(m) != 0)
		GOTO_FAIL("Bad length");

	rte_pktmbuf_append(m, sizeof(uint32_t));
	data = rte_pktmbuf_mtod(m, unaligned_uint32_t *);
	*data = MAGIC_DATA;

	/* clone the allocated mbuf */
	clone = rte_pktmbuf_clone(m, pktmbuf_pool);
	if (clone == NULL)
		GOTO_FAIL("cannot clone data\n");

	data = rte_pktmbuf_mtod(clone, unaligned_uint32_t *);
	if (*data != MAGIC_DATA)
		GOTO_FAIL("invalid data in clone\n");

	if (rte_mbuf_refcnt_read(m) != 2)
		GOTO_FAIL("invalid refcnt in m\n");

	/* free the clone */
	rte_pktmbuf_free(clone);
	clone = NULL;

	/* same test with a chained mbuf */
	m->next = rte_pktmbuf_alloc(pktmbuf_pool);
	if (m->next == NULL)
		GOTO_FAIL("Next Pkt Null\n");
	m->nb_segs = 2;

	rte_pktmbuf_append(m->next, sizeof(uint32_t));
	m->pkt_len = 2 * sizeof(uint32_t);

	data = rte_pktmbuf_mtod(m->next, unaligned_uint32_t *);
	*data = MAGIC_DATA;

	clone = rte_pktmbuf_clone(m, pktmbuf_pool);
	if (clone == NULL)
		GOTO_FAIL("cannot clone data\n");

	data = rte_pktmbuf_mtod(clone, unaligned_uint32_t *);
	if (*data != MAGIC_DATA)
		GOTO_FAIL("invalid data in clone\n");

	data = rte_pktmbuf_mtod(clone->next, unaligned_uint32_t *);
	if (*data != MAGIC_DATA)
		GOTO_FAIL("invalid data in clone->next\n");

	if (rte_mbuf_refcnt_read(m) != 2)
		GOTO_FAIL("invalid refcnt in m\n");

	if (rte_mbuf_refcnt_read(m->next) != 2)
		GOTO_FAIL("invalid refcnt in m->next\n");

	/* try to clone the clone */

	clone2 = rte_pktmbuf_clone(clone, pktmbuf_pool);
	if (clone2 == NULL)
		GOTO_FAIL("cannot clone the clone\n");

	data = rte_pktmbuf_mtod(clone2, unaligned_uint32_t *);
	if (*data != MAGIC_DATA)
		GOTO_FAIL("invalid data in clone2\n");

	data = rte_pktmbuf_mtod(clone2->next, unaligned_uint32_t *);
	if (*data != MAGIC_DATA)
		GOTO_FAIL("invalid data in clone2->next\n");

	if (rte_mbuf_refcnt_read(m) != 3)
		GOTO_FAIL("invalid refcnt in m\n");

	if (rte_mbuf_refcnt_read(m->next) != 3)
		GOTO_FAIL("invalid refcnt in m->next\n");

	/* free mbuf */
	rte_pktmbuf_free(m);
	rte_pktmbuf_free(clone);
	rte_pktmbuf_free(clone2);

	m = NULL;
	clone = NULL;
	clone2 = NULL;
	printf("%s ok\n", __func__);
	return 0;

fail:
	if (m)
		rte_pktmbuf_free(m);
	if (clone)
		rte_pktmbuf_free(clone);
	if (clone2)
		rte_pktmbuf_free(clone2);
	return -1;
}

static int
test_pktmbuf_copy(struct rte_mempool *pktmbuf_pool)
{
	struct rte_mbuf *m = NULL;
	struct rte_mbuf *copy = NULL;
	struct rte_mbuf *copy2 = NULL;
	struct rte_mbuf *clone = NULL;
	unaligned_uint32_t *data;

	/* alloc a mbuf */
	m = rte_pktmbuf_alloc(pktmbuf_pool);
	if (m == NULL)
		GOTO_FAIL("ooops not allocating mbuf");

	if (rte_pktmbuf_pkt_len(m) != 0)
		GOTO_FAIL("Bad length");

	rte_pktmbuf_append(m, sizeof(uint32_t));
	data = rte_pktmbuf_mtod(m, unaligned_uint32_t *);
	*data = MAGIC_DATA;

	/* copy the allocated mbuf */
	copy = rte_pktmbuf_copy(m, pktmbuf_pool, 0, UINT32_MAX);
	if (copy == NULL)
		GOTO_FAIL("cannot copy data\n");

	if (rte_pktmbuf_pkt_len(copy) != sizeof(uint32_t))
		GOTO_FAIL("copy length incorrect\n");

	if (rte_pktmbuf_data_len(copy) != sizeof(uint32_t))
		GOTO_FAIL("copy data length incorrect\n");

	data = rte_pktmbuf_mtod(copy, unaligned_uint32_t *);
	if (*data != MAGIC_DATA)
		GOTO_FAIL("invalid data in copy\n");

	/* free the copy */
	rte_pktmbuf_free(copy);
	copy = NULL;

	/* same test with a cloned mbuf */
	clone = rte_pktmbuf_clone(m, pktmbuf_pool);
	if (clone == NULL)
		GOTO_FAIL("cannot clone data\n");

	if (!RTE_MBUF_CLONED(clone))
		GOTO_FAIL("clone did not give a cloned mbuf\n");

	copy = rte_pktmbuf_copy(clone, pktmbuf_pool, 0, UINT32_MAX);
	if (copy == NULL)
		GOTO_FAIL("cannot copy cloned mbuf\n");

	if (RTE_MBUF_CLONED(copy))
		GOTO_FAIL("copy of clone is cloned?\n");

	if (rte_pktmbuf_pkt_len(copy) != sizeof(uint32_t))
		GOTO_FAIL("copy clone length incorrect\n");

	if (rte_pktmbuf_data_len(copy) != sizeof(uint32_t))
		GOTO_FAIL("copy clone data length incorrect\n");

	data = rte_pktmbuf_mtod(copy, unaligned_uint32_t *);
	if (*data != MAGIC_DATA)
		GOTO_FAIL("invalid data in clone copy\n");
	rte_pktmbuf_free(clone);
	rte_pktmbuf_free(copy);
	copy = NULL;
	clone = NULL;


	/* same test with a chained mbuf */
	m->next = rte_pktmbuf_alloc(pktmbuf_pool);
	if (m->next == NULL)
		GOTO_FAIL("Next Pkt Null\n");
	m->nb_segs = 2;

	rte_pktmbuf_append(m->next, sizeof(uint32_t));
	m->pkt_len = 2 * sizeof(uint32_t);
	data = rte_pktmbuf_mtod(m->next, unaligned_uint32_t *);
	*data = MAGIC_DATA + 1;

	copy = rte_pktmbuf_copy(m, pktmbuf_pool, 0, UINT32_MAX);
	if (copy == NULL)
		GOTO_FAIL("cannot copy data\n");

	if (rte_pktmbuf_pkt_len(copy) != 2 * sizeof(uint32_t))
		GOTO_FAIL("chain copy length incorrect\n");

	if (rte_pktmbuf_data_len(copy) != 2 * sizeof(uint32_t))
		GOTO_FAIL("chain copy data length incorrect\n");

	data = rte_pktmbuf_mtod(copy, unaligned_uint32_t *);
	if (data[0] != MAGIC_DATA || data[1] != MAGIC_DATA + 1)
		GOTO_FAIL("invalid data in copy\n");

	rte_pktmbuf_free(copy2);

	/* test offset copy */
	copy2 = rte_pktmbuf_copy(copy, pktmbuf_pool,
				 sizeof(uint32_t), UINT32_MAX);
	if (copy2 == NULL)
		GOTO_FAIL("cannot copy the copy\n");

	if (rte_pktmbuf_pkt_len(copy2) != sizeof(uint32_t))
		GOTO_FAIL("copy with offset, length incorrect\n");

	if (rte_pktmbuf_data_len(copy2) != sizeof(uint32_t))
		GOTO_FAIL("copy with offset, data length incorrect\n");

	data = rte_pktmbuf_mtod(copy2, unaligned_uint32_t *);
	if (data[0] != MAGIC_DATA + 1)
		GOTO_FAIL("copy with offset, invalid data\n");

	rte_pktmbuf_free(copy2);

	/* test truncation copy */
	copy2 = rte_pktmbuf_copy(copy, pktmbuf_pool,
				 0, sizeof(uint32_t));
	if (copy2 == NULL)
		GOTO_FAIL("cannot copy the copy\n");

	if (rte_pktmbuf_pkt_len(copy2) != sizeof(uint32_t))
		GOTO_FAIL("copy with truncate, length incorrect\n");

	if (rte_pktmbuf_data_len(copy2) != sizeof(uint32_t))
		GOTO_FAIL("copy with truncate, data length incorrect\n");

	data = rte_pktmbuf_mtod(copy2, unaligned_uint32_t *);
	if (data[0] != MAGIC_DATA)
		GOTO_FAIL("copy with truncate, invalid data\n");

	/* free mbuf */
	rte_pktmbuf_free(m);
	rte_pktmbuf_free(copy);
	rte_pktmbuf_free(copy2);

	m = NULL;
	copy = NULL;
	copy2 = NULL;
	printf("%s ok\n", __func__);
	return 0;

fail:
	if (m)
		rte_pktmbuf_free(m);
	if (copy)
		rte_pktmbuf_free(copy);
	if (copy2)
		rte_pktmbuf_free(copy2);
	return -1;
}

static int
test_attach_from_different_pool(struct rte_mempool *pktmbuf_pool,
				struct rte_mempool *pktmbuf_pool2)
{
	struct rte_mbuf *m = NULL;
	struct rte_mbuf *clone = NULL;
	struct rte_mbuf *clone2 = NULL;
	char *data, *c_data, *c_data2;

	/* alloc a mbuf */
	m = rte_pktmbuf_alloc(pktmbuf_pool);
	if (m == NULL)
		GOTO_FAIL("cannot allocate mbuf");

	if (rte_pktmbuf_pkt_len(m) != 0)
		GOTO_FAIL("Bad length");

	data = rte_pktmbuf_mtod(m, char *);

	/* allocate a new mbuf from the second pool, and attach it to the first
	 * mbuf */
	clone = rte_pktmbuf_alloc(pktmbuf_pool2);
	if (clone == NULL)
		GOTO_FAIL("cannot allocate mbuf from second pool\n");

	/* check data room size and priv size, and erase priv */
	if (rte_pktmbuf_data_room_size(clone->pool) != 0)
		GOTO_FAIL("data room size should be 0\n");
	if (rte_pktmbuf_priv_size(clone->pool) != MBUF2_PRIV_SIZE)
		GOTO_FAIL("data room size should be %d\n", MBUF2_PRIV_SIZE);
	memset(clone + 1, 0, MBUF2_PRIV_SIZE);

	/* save data pointer to compare it after detach() */
	c_data = rte_pktmbuf_mtod(clone, char *);
	if (c_data != (char *)clone + sizeof(*clone) + MBUF2_PRIV_SIZE)
		GOTO_FAIL("bad data pointer in clone");
	if (rte_pktmbuf_headroom(clone) != 0)
		GOTO_FAIL("bad headroom in clone");

	rte_pktmbuf_attach(clone, m);

	if (rte_pktmbuf_mtod(clone, char *) != data)
		GOTO_FAIL("clone was not attached properly\n");
	if (rte_pktmbuf_headroom(clone) != RTE_PKTMBUF_HEADROOM)
		GOTO_FAIL("bad headroom in clone after attach");
	if (rte_mbuf_refcnt_read(m) != 2)
		GOTO_FAIL("invalid refcnt in m\n");

	/* allocate a new mbuf from the second pool, and attach it to the first
	 * cloned mbuf */
	clone2 = rte_pktmbuf_alloc(pktmbuf_pool2);
	if (clone2 == NULL)
		GOTO_FAIL("cannot allocate clone2 from second pool\n");

	/* check data room size and priv size, and erase priv */
	if (rte_pktmbuf_data_room_size(clone2->pool) != 0)
		GOTO_FAIL("data room size should be 0\n");
	if (rte_pktmbuf_priv_size(clone2->pool) != MBUF2_PRIV_SIZE)
		GOTO_FAIL("data room size should be %d\n", MBUF2_PRIV_SIZE);
	memset(clone2 + 1, 0, MBUF2_PRIV_SIZE);

	/* save data pointer to compare it after detach() */
	c_data2 = rte_pktmbuf_mtod(clone2, char *);
	if (c_data2 != (char *)clone2 + sizeof(*clone2) + MBUF2_PRIV_SIZE)
		GOTO_FAIL("bad data pointer in clone2");
	if (rte_pktmbuf_headroom(clone2) != 0)
		GOTO_FAIL("bad headroom in clone2");

	rte_pktmbuf_attach(clone2, clone);

	if (rte_pktmbuf_mtod(clone2, char *) != data)
		GOTO_FAIL("clone2 was not attached properly\n");
	if (rte_pktmbuf_headroom(clone2) != RTE_PKTMBUF_HEADROOM)
		GOTO_FAIL("bad headroom in clone2 after attach");
	if (rte_mbuf_refcnt_read(m) != 3)
		GOTO_FAIL("invalid refcnt in m\n");

	/* detach the clones */
	rte_pktmbuf_detach(clone);
	if (c_data != rte_pktmbuf_mtod(clone, char *))
		GOTO_FAIL("clone was not detached properly\n");
	if (rte_mbuf_refcnt_read(m) != 2)
		GOTO_FAIL("invalid refcnt in m\n");

	rte_pktmbuf_detach(clone2);
	if (c_data2 != rte_pktmbuf_mtod(clone2, char *))
		GOTO_FAIL("clone2 was not detached properly\n");
	if (rte_mbuf_refcnt_read(m) != 1)
		GOTO_FAIL("invalid refcnt in m\n");

	/* free the clones and the initial mbuf */
	rte_pktmbuf_free(clone2);
	rte_pktmbuf_free(clone);
	rte_pktmbuf_free(m);
	printf("%s ok\n", __func__);
	return 0;

fail:
	if (m)
		rte_pktmbuf_free(m);
	if (clone)
		rte_pktmbuf_free(clone);
	if (clone2)
		rte_pktmbuf_free(clone2);
	return -1;
}
#undef GOTO_FAIL

/*
 * test allocation and free of mbufs
 */
static int
test_pktmbuf_pool(struct rte_mempool *pktmbuf_pool)
{
	unsigned i;
	struct rte_mbuf *m[NB_MBUF];
	int ret = 0;

	for (i=0; i<NB_MBUF; i++)
		m[i] = NULL;

	/* alloc NB_MBUF mbufs */
	for (i=0; i<NB_MBUF; i++) {
		m[i] = rte_pktmbuf_alloc(pktmbuf_pool);
		if (m[i] == NULL) {
			printf("rte_pktmbuf_alloc() failed (%u)\n", i);
			ret = -1;
		}
	}
	struct rte_mbuf *extra = NULL;
	extra = rte_pktmbuf_alloc(pktmbuf_pool);
	if(extra != NULL) {
		printf("Error pool not empty");
		ret = -1;
	}
	extra = rte_pktmbuf_clone(m[0], pktmbuf_pool);
	if(extra != NULL) {
		printf("Error pool not empty");
		ret = -1;
	}
	/* free them */
	for (i=0; i<NB_MBUF; i++) {
		if (m[i] != NULL)
			rte_pktmbuf_free(m[i]);
	}

	return ret;
}

/*
 * test bulk allocation and bulk free of mbufs
 */
static int
test_pktmbuf_pool_bulk(void)
{
	struct rte_mempool *pool = NULL;
	struct rte_mempool *pool2 = NULL;
	unsigned int i;
	struct rte_mbuf *m;
	struct rte_mbuf *mbufs[NB_MBUF];
	int ret = 0;

	/* We cannot use the preallocated mbuf pools because their caches
	 * prevent us from bulk allocating all objects in them.
	 * So we create our own mbuf pools without caches.
	 */
	printf("Create mbuf pools for bulk allocation.\n");
	pool = rte_pktmbuf_pool_create("test_pktmbuf_bulk",
			NB_MBUF, 0, 0, MBUF_DATA_SIZE, SOCKET_ID_ANY);
	if (pool == NULL) {
		printf("rte_pktmbuf_pool_create() failed. rte_errno %d\n",
		       rte_errno);
		goto err;
	}
	pool2 = rte_pktmbuf_pool_create("test_pktmbuf_bulk2",
			NB_MBUF, 0, 0, MBUF_DATA_SIZE, SOCKET_ID_ANY);
	if (pool2 == NULL) {
		printf("rte_pktmbuf_pool_create() failed. rte_errno %d\n",
		       rte_errno);
		goto err;
	}

	/* Preconditions: Mempools must be full. */
	if (!(rte_mempool_full(pool) && rte_mempool_full(pool2))) {
		printf("Test precondition failed: mempools not full\n");
		goto err;
	}
	if (!(rte_mempool_avail_count(pool) == NB_MBUF &&
			rte_mempool_avail_count(pool2) == NB_MBUF)) {
		printf("Test precondition failed: mempools: %u+%u != %u+%u",
		       rte_mempool_avail_count(pool),
		       rte_mempool_avail_count(pool2),
		       NB_MBUF, NB_MBUF);
		goto err;
	}

	printf("Test single bulk alloc, followed by multiple bulk free.\n");

	/* Bulk allocate all mbufs in the pool, in one go. */
	ret = rte_pktmbuf_alloc_bulk(pool, mbufs, NB_MBUF);
	if (ret != 0) {
		printf("rte_pktmbuf_alloc_bulk() failed: %d\n", ret);
		goto err;
	}
	/* Test that they have been removed from the pool. */
	if (!rte_mempool_empty(pool)) {
		printf("mempool not empty\n");
		goto err;
	}
	/* Bulk free all mbufs, in four steps. */
	RTE_BUILD_BUG_ON(NB_MBUF % 4 != 0);
	for (i = 0; i < NB_MBUF; i += NB_MBUF / 4) {
		rte_pktmbuf_free_bulk(&mbufs[i], NB_MBUF / 4);
		/* Test that they have been returned to the pool. */
		if (rte_mempool_avail_count(pool) != i + NB_MBUF / 4) {
			printf("mempool avail count incorrect\n");
			goto err;
		}
	}

	printf("Test multiple bulk alloc, followed by single bulk free.\n");

	/* Bulk allocate all mbufs in the pool, in four steps. */
	for (i = 0; i < NB_MBUF; i += NB_MBUF / 4) {
		ret = rte_pktmbuf_alloc_bulk(pool, &mbufs[i], NB_MBUF / 4);
		if (ret != 0) {
			printf("rte_pktmbuf_alloc_bulk() failed: %d\n", ret);
			goto err;
		}
	}
	/* Test that they have been removed from the pool. */
	if (!rte_mempool_empty(pool)) {
		printf("mempool not empty\n");
		goto err;
	}
	/* Bulk free all mbufs, in one go. */
	rte_pktmbuf_free_bulk(mbufs, NB_MBUF);
	/* Test that they have been returned to the pool. */
	if (!rte_mempool_full(pool)) {
		printf("mempool not full\n");
		goto err;
	}

	printf("Test bulk free of single long chain.\n");

	/* Bulk allocate all mbufs in the pool, in one go. */
	ret = rte_pktmbuf_alloc_bulk(pool, mbufs, NB_MBUF);
	if (ret != 0) {
		printf("rte_pktmbuf_alloc_bulk() failed: %d\n", ret);
		goto err;
	}
	/* Create a long mbuf chain. */
	for (i = 1; i < NB_MBUF; i++) {
		ret = rte_pktmbuf_chain(mbufs[0], mbufs[i]);
		if (ret != 0) {
			printf("rte_pktmbuf_chain() failed: %d\n", ret);
			goto err;
		}
		mbufs[i] = NULL;
	}
	/* Free the mbuf chain containing all the mbufs. */
	rte_pktmbuf_free_bulk(mbufs, 1);
	/* Test that they have been returned to the pool. */
	if (!rte_mempool_full(pool)) {
		printf("mempool not full\n");
		goto err;
	}

	printf("Test bulk free of multiple chains using multiple pools.\n");

	/* Create mbuf chains containing mbufs from different pools. */
	RTE_BUILD_BUG_ON(CHAIN_LEN % 2 != 0);
	RTE_BUILD_BUG_ON(NB_MBUF % (CHAIN_LEN / 2) != 0);
	for (i = 0; i < NB_MBUF * 2; i++) {
		m = rte_pktmbuf_alloc((i & 4) ? pool2 : pool);
		if (m == NULL) {
			printf("rte_pktmbuf_alloc() failed (%u)\n", i);
			goto err;
		}
		if ((i % CHAIN_LEN) == 0)
			mbufs[i / CHAIN_LEN] = m;
		else
			rte_pktmbuf_chain(mbufs[i / CHAIN_LEN], m);
	}
	/* Test that both pools have been emptied. */
	if (!(rte_mempool_empty(pool) && rte_mempool_empty(pool2))) {
		printf("mempools not empty\n");
		goto err;
	}
	/* Free one mbuf chain. */
	rte_pktmbuf_free_bulk(mbufs, 1);
	/* Test that the segments have been returned to the pools. */
	if (!(rte_mempool_avail_count(pool) == CHAIN_LEN / 2 &&
			rte_mempool_avail_count(pool2) == CHAIN_LEN / 2)) {
		printf("all segments of first mbuf have not been returned\n");
		goto err;
	}
	/* Free the remaining mbuf chains. */
	rte_pktmbuf_free_bulk(&mbufs[1], NB_MBUF * 2 / CHAIN_LEN - 1);
	/* Test that they have been returned to the pools. */
	if (!(rte_mempool_full(pool) && rte_mempool_full(pool2))) {
		printf("mempools not full\n");
		goto err;
	}

	ret = 0;
	goto done;

err:
	ret = -1;

done:
	printf("Free mbuf pools for bulk allocation.\n");
	rte_mempool_free(pool);
	rte_mempool_free(pool2);
	return ret;
}

/*
 * test that the pointer to the data on a packet mbuf is set properly
 */
static int
test_pktmbuf_pool_ptr(struct rte_mempool *pktmbuf_pool)
{
	unsigned i;
	struct rte_mbuf *m[NB_MBUF];
	int ret = 0;

	for (i=0; i<NB_MBUF; i++)
		m[i] = NULL;

	/* alloc NB_MBUF mbufs */
	for (i=0; i<NB_MBUF; i++) {
		m[i] = rte_pktmbuf_alloc(pktmbuf_pool);
		if (m[i] == NULL) {
			printf("rte_pktmbuf_alloc() failed (%u)\n", i);
			ret = -1;
			break;
		}
		m[i]->data_off += 64;
	}

	/* free them */
	for (i=0; i<NB_MBUF; i++) {
		if (m[i] != NULL)
			rte_pktmbuf_free(m[i]);
	}

	for (i=0; i<NB_MBUF; i++)
		m[i] = NULL;

	/* alloc NB_MBUF mbufs */
	for (i=0; i<NB_MBUF; i++) {
		m[i] = rte_pktmbuf_alloc(pktmbuf_pool);
		if (m[i] == NULL) {
			printf("rte_pktmbuf_alloc() failed (%u)\n", i);
			ret = -1;
			break;
		}
		if (m[i]->data_off != RTE_PKTMBUF_HEADROOM) {
			printf("invalid data_off\n");
			ret = -1;
		}
	}

	/* free them */
	for (i=0; i<NB_MBUF; i++) {
		if (m[i] != NULL)
			rte_pktmbuf_free(m[i]);
	}

	return ret;
}

static int
test_pktmbuf_free_segment(struct rte_mempool *pktmbuf_pool)
{
	unsigned i;
	struct rte_mbuf *m[NB_MBUF];
	int ret = 0;

	for (i=0; i<NB_MBUF; i++)
		m[i] = NULL;

	/* alloc NB_MBUF mbufs */
	for (i=0; i<NB_MBUF; i++) {
		m[i] = rte_pktmbuf_alloc(pktmbuf_pool);
		if (m[i] == NULL) {
			printf("rte_pktmbuf_alloc() failed (%u)\n", i);
			ret = -1;
		}
	}

	/* free them */
	for (i=0; i<NB_MBUF; i++) {
		if (m[i] != NULL) {
			struct rte_mbuf *mb, *mt;

			mb = m[i];
			while(mb != NULL) {
				mt = mb;
				mb = mb->next;
				rte_pktmbuf_free_seg(mt);
			}
		}
	}

	return ret;
}

/*
 * Stress test for rte_mbuf atomic refcnt.
 * Implies that RTE_MBUF_REFCNT_ATOMIC is defined.
 * For more efficiency, recommended to run with RTE_LIBRTE_MBUF_DEBUG defined.
 */

#ifdef RTE_MBUF_REFCNT_ATOMIC

static int
test_refcnt_slave(void *arg)
{
	unsigned lcore, free;
	void *mp = 0;
	struct rte_ring *refcnt_mbuf_ring = arg;

	lcore = rte_lcore_id();
	printf("%s started at lcore %u\n", __func__, lcore);

	free = 0;
	while (refcnt_stop_slaves == 0) {
		if (rte_ring_dequeue(refcnt_mbuf_ring, &mp) == 0) {
			free++;
			rte_pktmbuf_free(mp);
		}
	}

	refcnt_lcore[lcore] += free;
	printf("%s finished at lcore %u, "
	       "number of freed mbufs: %u\n",
	       __func__, lcore, free);
	return 0;
}

static void
test_refcnt_iter(unsigned int lcore, unsigned int iter,
		 struct rte_mempool *refcnt_pool,
		 struct rte_ring *refcnt_mbuf_ring)
{
	uint16_t ref;
	unsigned i, n, tref, wn;
	struct rte_mbuf *m;

	tref = 0;

	/* For each mbuf in the pool:
	 * - allocate mbuf,
	 * - increment it's reference up to N+1,
	 * - enqueue it N times into the ring for slave cores to free.
	 */
	for (i = 0, n = rte_mempool_avail_count(refcnt_pool);
	    i != n && (m = rte_pktmbuf_alloc(refcnt_pool)) != NULL;
	    i++) {
		ref = RTE_MAX(rte_rand() % REFCNT_MAX_REF, 1UL);
		tref += ref;
		if ((ref & 1) != 0) {
			rte_pktmbuf_refcnt_update(m, ref);
			while (ref-- != 0)
				rte_ring_enqueue(refcnt_mbuf_ring, m);
		} else {
			while (ref-- != 0) {
				rte_pktmbuf_refcnt_update(m, 1);
				rte_ring_enqueue(refcnt_mbuf_ring, m);
			}
		}
		rte_pktmbuf_free(m);
	}

	if (i != n)
		rte_panic("(lcore=%u, iter=%u): was able to allocate only "
		          "%u from %u mbufs\n", lcore, iter, i, n);

	/* wait till slave lcores  will consume all mbufs */
	while (!rte_ring_empty(refcnt_mbuf_ring))
		;

	/* check that all mbufs are back into mempool by now */
	for (wn = 0; wn != REFCNT_MAX_TIMEOUT; wn++) {
		if ((i = rte_mempool_avail_count(refcnt_pool)) == n) {
			refcnt_lcore[lcore] += tref;
			printf("%s(lcore=%u, iter=%u) completed, "
			    "%u references processed\n",
			    __func__, lcore, iter, tref);
			return;
		}
		rte_delay_ms(100);
	}

	rte_panic("(lcore=%u, iter=%u): after %us only "
	          "%u of %u mbufs left free\n", lcore, iter, wn, i, n);
}

static int
test_refcnt_master(struct rte_mempool *refcnt_pool,
		   struct rte_ring *refcnt_mbuf_ring)
{
	unsigned i, lcore;

	lcore = rte_lcore_id();
	printf("%s started at lcore %u\n", __func__, lcore);

	for (i = 0; i != REFCNT_MAX_ITER; i++)
		test_refcnt_iter(lcore, i, refcnt_pool, refcnt_mbuf_ring);

	refcnt_stop_slaves = 1;
	rte_wmb();

	printf("%s finished at lcore %u\n", __func__, lcore);
	return 0;
}

#endif

static int
test_refcnt_mbuf(void)
{
#ifdef RTE_MBUF_REFCNT_ATOMIC
	unsigned int master, slave, tref;
	int ret = -1;
	struct rte_mempool *refcnt_pool = NULL;
	struct rte_ring *refcnt_mbuf_ring = NULL;

	if (rte_lcore_count() < 2) {
		printf("Not enough cores for test_refcnt_mbuf, expecting at least 2\n");
		return TEST_SKIPPED;
	}

	printf("starting %s, at %u lcores\n", __func__, rte_lcore_count());

	/* create refcnt pool & ring if they don't exist */

	refcnt_pool = rte_pktmbuf_pool_create(MAKE_STRING(refcnt_pool),
					      REFCNT_MBUF_NUM, 0, 0, 0,
					      SOCKET_ID_ANY);
	if (refcnt_pool == NULL) {
		printf("%s: cannot allocate " MAKE_STRING(refcnt_pool) "\n",
		    __func__);
		return -1;
	}

	refcnt_mbuf_ring = rte_ring_create("refcnt_mbuf_ring",
			rte_align32pow2(REFCNT_RING_SIZE), SOCKET_ID_ANY,
					RING_F_SP_ENQ);
	if (refcnt_mbuf_ring == NULL) {
		printf("%s: cannot allocate " MAKE_STRING(refcnt_mbuf_ring)
		    "\n", __func__);
		goto err;
	}

	refcnt_stop_slaves = 0;
	memset(refcnt_lcore, 0, sizeof (refcnt_lcore));

	rte_eal_mp_remote_launch(test_refcnt_slave, refcnt_mbuf_ring,
				 SKIP_MASTER);

	test_refcnt_master(refcnt_pool, refcnt_mbuf_ring);

	rte_eal_mp_wait_lcore();

	/* check that we porcessed all references */
	tref = 0;
	master = rte_get_master_lcore();

	RTE_LCORE_FOREACH_SLAVE(slave)
		tref += refcnt_lcore[slave];

	if (tref != refcnt_lcore[master])
		rte_panic("refernced mbufs: %u, freed mbufs: %u\n",
		          tref, refcnt_lcore[master]);

	rte_mempool_dump(stdout, refcnt_pool);
	rte_ring_dump(stdout, refcnt_mbuf_ring);

	ret = 0;

err:
	rte_mempool_free(refcnt_pool);
	rte_ring_free(refcnt_mbuf_ring);
	return ret;
#else
	return 0;
#endif
}

#include <unistd.h>
#include <sys/wait.h>

/* use fork() to test mbuf errors panic */
static int
verify_mbuf_check_panics(struct rte_mbuf *buf)
{
	int pid;
	int status;

	pid = fork();

	if (pid == 0) {
		rte_mbuf_sanity_check(buf, 1); /* should panic */
		exit(0);  /* return normally if it doesn't panic */
	} else if (pid < 0){
		printf("Fork Failed\n");
		return -1;
	}
	wait(&status);
	if(status == 0)
		return -1;

	return 0;
}

static int
test_failing_mbuf_sanity_check(struct rte_mempool *pktmbuf_pool)
{
	struct rte_mbuf *buf;
	struct rte_mbuf badbuf;

	printf("Checking rte_mbuf_sanity_check for failure conditions\n");

	/* get a good mbuf to use to make copies */
	buf = rte_pktmbuf_alloc(pktmbuf_pool);
	if (buf == NULL)
		return -1;
	printf("Checking good mbuf initially\n");
	if (verify_mbuf_check_panics(buf) != -1)
		return -1;

	printf("Now checking for error conditions\n");

	if (verify_mbuf_check_panics(NULL)) {
		printf("Error with NULL mbuf test\n");
		return -1;
	}

	badbuf = *buf;
	badbuf.pool = NULL;
	if (verify_mbuf_check_panics(&badbuf)) {
		printf("Error with bad-pool mbuf test\n");
		return -1;
	}

	badbuf = *buf;
	badbuf.buf_iova = 0;
	if (verify_mbuf_check_panics(&badbuf)) {
		printf("Error with bad-physaddr mbuf test\n");
		return -1;
	}

	badbuf = *buf;
	badbuf.buf_addr = NULL;
	if (verify_mbuf_check_panics(&badbuf)) {
		printf("Error with bad-addr mbuf test\n");
		return -1;
	}

	badbuf = *buf;
	badbuf.refcnt = 0;
	if (verify_mbuf_check_panics(&badbuf)) {
		printf("Error with bad-refcnt(0) mbuf test\n");
		return -1;
	}

	badbuf = *buf;
	badbuf.refcnt = UINT16_MAX;
	if (verify_mbuf_check_panics(&badbuf)) {
		printf("Error with bad-refcnt(MAX) mbuf test\n");
		return -1;
	}

	return 0;
}

static int
test_mbuf_linearize(struct rte_mempool *pktmbuf_pool, int pkt_len,
		    int nb_segs)
{

	struct rte_mbuf *m = NULL, *mbuf = NULL;
	uint8_t *data;
	int data_len = 0;
	int remain;
	int seg, seg_len;
	int i;

	if (pkt_len < 1) {
		printf("Packet size must be 1 or more (is %d)\n", pkt_len);
		return -1;
	}

	if (nb_segs < 1) {
		printf("Number of segments must be 1 or more (is %d)\n",
				nb_segs);
		return -1;
	}

	seg_len = pkt_len / nb_segs;
	if (seg_len == 0)
		seg_len = 1;

	remain = pkt_len;

	/* Create chained mbuf_src and fill it generated data */
	for (seg = 0; remain > 0; seg++) {

		m = rte_pktmbuf_alloc(pktmbuf_pool);
		if (m == NULL) {
			printf("Cannot create segment for source mbuf");
			goto fail;
		}

		/* Make sure if tailroom is zeroed */
		memset(rte_pktmbuf_mtod(m, uint8_t *), 0,
				rte_pktmbuf_tailroom(m));

		data_len = remain;
		if (data_len > seg_len)
			data_len = seg_len;

		data = (uint8_t *)rte_pktmbuf_append(m, data_len);
		if (data == NULL) {
			printf("Cannot append %d bytes to the mbuf\n",
					data_len);
			goto fail;
		}

		for (i = 0; i < data_len; i++)
			data[i] = (seg * seg_len + i) % 0x0ff;

		if (seg == 0)
			mbuf = m;
		else
			rte_pktmbuf_chain(mbuf, m);

		remain -= data_len;
	}

	/* Create destination buffer to store coalesced data */
	if (rte_pktmbuf_linearize(mbuf)) {
		printf("Mbuf linearization failed\n");
		goto fail;
	}

	if (!rte_pktmbuf_is_contiguous(mbuf)) {
		printf("Source buffer should be contiguous after "
				"linearization\n");
		goto fail;
	}

	data = rte_pktmbuf_mtod(mbuf, uint8_t *);

	for (i = 0; i < pkt_len; i++)
		if (data[i] != (i % 0x0ff)) {
			printf("Incorrect data in linearized mbuf\n");
			goto fail;
		}

	rte_pktmbuf_free(mbuf);
	return 0;

fail:
	if (mbuf)
		rte_pktmbuf_free(mbuf);
	return -1;
}

static int
test_mbuf_linearize_check(struct rte_mempool *pktmbuf_pool)
{
	struct test_mbuf_array {
		int size;
		int nb_segs;
	} mbuf_array[] = {
			{ 128, 1 },
			{ 64, 64 },
			{ 512, 10 },
			{ 250, 11 },
			{ 123, 8 },
	};
	unsigned int i;

	printf("Test mbuf linearize API\n");

	for (i = 0; i < RTE_DIM(mbuf_array); i++)
		if (test_mbuf_linearize(pktmbuf_pool, mbuf_array[i].size,
				mbuf_array[i].nb_segs)) {
			printf("Test failed for %d, %d\n", mbuf_array[i].size,
					mbuf_array[i].nb_segs);
			return -1;
		}

	return 0;
}

/*
 * Helper function for test_tx_ofload
 */
static inline void
set_tx_offload(struct rte_mbuf *mb, uint64_t il2, uint64_t il3, uint64_t il4,
	uint64_t tso, uint64_t ol3, uint64_t ol2)
{
	mb->l2_len = il2;
	mb->l3_len = il3;
	mb->l4_len = il4;
	mb->tso_segsz = tso;
	mb->outer_l3_len = ol3;
	mb->outer_l2_len = ol2;
}

static int
test_tx_offload(void)
{
	struct rte_mbuf *mb;
	uint64_t tm, v1, v2;
	size_t sz;
	uint32_t i;

	static volatile struct {
		uint16_t l2;
		uint16_t l3;
		uint16_t l4;
		uint16_t tso;
	} txof;

	const uint32_t num = 0x10000;

	txof.l2 = rte_rand() % (1 <<  RTE_MBUF_L2_LEN_BITS);
	txof.l3 = rte_rand() % (1 <<  RTE_MBUF_L3_LEN_BITS);
	txof.l4 = rte_rand() % (1 <<  RTE_MBUF_L4_LEN_BITS);
	txof.tso = rte_rand() % (1 <<   RTE_MBUF_TSO_SEGSZ_BITS);

	printf("%s started, tx_offload = {\n"
		"\tl2_len=%#hx,\n"
		"\tl3_len=%#hx,\n"
		"\tl4_len=%#hx,\n"
		"\ttso_segsz=%#hx,\n"
		"\touter_l3_len=%#x,\n"
		"\touter_l2_len=%#x,\n"
		"};\n",
		__func__,
		txof.l2, txof.l3, txof.l4, txof.tso, txof.l3, txof.l2);

	sz = sizeof(*mb) * num;
	mb = rte_zmalloc(NULL, sz, RTE_CACHE_LINE_SIZE);
	if (mb == NULL) {
		printf("%s failed, out of memory\n", __func__);
		return -ENOMEM;
	}

	memset(mb, 0, sz);
	tm = rte_rdtsc_precise();

	for (i = 0; i != num; i++)
		set_tx_offload(mb + i, txof.l2, txof.l3, txof.l4,
			txof.tso, txof.l3, txof.l2);

	tm = rte_rdtsc_precise() - tm;
	printf("%s set tx_offload by bit-fields: %u iterations, %"
		PRIu64 " cycles, %#Lf cycles/iter\n",
		__func__, num, tm, (long double)tm / num);

	v1 = mb[rte_rand() % num].tx_offload;

	memset(mb, 0, sz);
	tm = rte_rdtsc_precise();

	for (i = 0; i != num; i++)
		mb[i].tx_offload = rte_mbuf_tx_offload(txof.l2, txof.l3,
			txof.l4, txof.tso, txof.l3, txof.l2, 0);

	tm = rte_rdtsc_precise() - tm;
	printf("%s set raw tx_offload: %u iterations, %"
		PRIu64 " cycles, %#Lf cycles/iter\n",
		__func__, num, tm, (long double)tm / num);

	v2 = mb[rte_rand() % num].tx_offload;

	rte_free(mb);

	printf("%s finished\n"
		"expected tx_offload value: 0x%" PRIx64 ";\n"
		"rte_mbuf_tx_offload value: 0x%" PRIx64 ";\n",
		__func__, v1, v2);

	return (v1 == v2) ? 0 : -EINVAL;
}

static int
test_mbuf(void)
{
	int ret = -1;
	struct rte_mempool *pktmbuf_pool = NULL;
	struct rte_mempool *pktmbuf_pool2 = NULL;


	RTE_BUILD_BUG_ON(sizeof(struct rte_mbuf) != RTE_CACHE_LINE_MIN_SIZE * 2);

	/* create pktmbuf pool if it does not exist */
	pktmbuf_pool = rte_pktmbuf_pool_create("test_pktmbuf_pool",
			NB_MBUF, 32, 0, MBUF_DATA_SIZE, SOCKET_ID_ANY);

	if (pktmbuf_pool == NULL) {
		printf("cannot allocate mbuf pool\n");
		goto err;
	}

	/* create a specific pktmbuf pool with a priv_size != 0 and no data
	 * room size */
	pktmbuf_pool2 = rte_pktmbuf_pool_create("test_pktmbuf_pool2",
			NB_MBUF, 32, MBUF2_PRIV_SIZE, 0, SOCKET_ID_ANY);

	if (pktmbuf_pool2 == NULL) {
		printf("cannot allocate mbuf pool\n");
		goto err;
	}

	/* test multiple mbuf alloc */
	if (test_pktmbuf_pool(pktmbuf_pool) < 0) {
		printf("test_mbuf_pool() failed\n");
		goto err;
	}

	/* do it another time to check that all mbufs were freed */
	if (test_pktmbuf_pool(pktmbuf_pool) < 0) {
		printf("test_mbuf_pool() failed (2)\n");
		goto err;
	}

	/* test bulk mbuf alloc and free */
	if (test_pktmbuf_pool_bulk() < 0) {
		printf("test_pktmbuf_pool_bulk() failed\n");
		goto err;
	}

	/* test that the pointer to the data on a packet mbuf is set properly */
	if (test_pktmbuf_pool_ptr(pktmbuf_pool) < 0) {
		printf("test_pktmbuf_pool_ptr() failed\n");
		goto err;
	}

	/* test data manipulation in mbuf */
	if (test_one_pktmbuf(pktmbuf_pool) < 0) {
		printf("test_one_mbuf() failed\n");
		goto err;
	}


	/*
	 * do it another time, to check that allocation reinitialize
	 * the mbuf correctly
	 */
	if (test_one_pktmbuf(pktmbuf_pool) < 0) {
		printf("test_one_mbuf() failed (2)\n");
		goto err;
	}

	if (test_pktmbuf_with_non_ascii_data(pktmbuf_pool) < 0) {
		printf("test_pktmbuf_with_non_ascii_data() failed\n");
		goto err;
	}

	/* test free pktmbuf segment one by one */
	if (test_pktmbuf_free_segment(pktmbuf_pool) < 0) {
		printf("test_pktmbuf_free_segment() failed.\n");
		goto err;
	}

	if (testclone_testupdate_testdetach(pktmbuf_pool) < 0) {
		printf("testclone_and_testupdate() failed \n");
		goto err;
	}

	if (test_pktmbuf_copy(pktmbuf_pool) < 0) {
		printf("test_pktmbuf_copy() failed\n");
		goto err;
	}

	if (test_attach_from_different_pool(pktmbuf_pool, pktmbuf_pool2) < 0) {
		printf("test_attach_from_different_pool() failed\n");
		goto err;
	}

	if (test_refcnt_mbuf() < 0) {
		printf("test_refcnt_mbuf() failed \n");
		goto err;
	}

	if (test_failing_mbuf_sanity_check(pktmbuf_pool) < 0) {
		printf("test_failing_mbuf_sanity_check() failed\n");
		goto err;
	}

	if (test_mbuf_linearize_check(pktmbuf_pool) < 0) {
		printf("test_mbuf_linearize_check() failed\n");
		goto err;
	}

	if (test_tx_offload() < 0) {
		printf("test_tx_offload() failed\n");
		goto err;
	}

	ret = 0;
err:
	rte_mempool_free(pktmbuf_pool);
	rte_mempool_free(pktmbuf_pool2);
	return ret;
}

REGISTER_TEST_COMMAND(mbuf_autotest, test_mbuf);
