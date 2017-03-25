/*
 *   BSD LICENSE
 *
 *   Copyright(c) 2017 Cavium, Inc.. All rights reserved.
 *   All rights reserved.
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
 *     * Neither the name of Cavium, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER(S) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_malloc.h>

#include "lio_logs.h"
#include "lio_struct.h"
#include "lio_ethdev.h"
#include "lio_rxtx.h"

static void
lio_dma_zone_free(struct lio_device *lio_dev, const struct rte_memzone *mz)
{
	const struct rte_memzone *mz_tmp;
	int ret = 0;

	if (mz == NULL) {
		lio_dev_err(lio_dev, "Memzone NULL\n");
		return;
	}

	mz_tmp = rte_memzone_lookup(mz->name);
	if (mz_tmp == NULL) {
		lio_dev_err(lio_dev, "Memzone %s Not Found\n", mz->name);
		return;
	}

	ret = rte_memzone_free(mz);
	if (ret)
		lio_dev_err(lio_dev, "Memzone free Failed ret %d\n", ret);
}

/**
 *  lio_init_instr_queue()
 *  @param lio_dev	- pointer to the lio device structure.
 *  @param txpciq	- queue to be initialized.
 *
 *  Called at driver init time for each input queue. iq_conf has the
 *  configuration parameters for the queue.
 *
 *  @return  Success: 0	Failure: -1
 */
static int
lio_init_instr_queue(struct lio_device *lio_dev,
		     union octeon_txpciq txpciq,
		     uint32_t num_descs, unsigned int socket_id)
{
	uint32_t iq_no = (uint32_t)txpciq.s.q_no;
	struct lio_instr_queue *iq;
	uint32_t instr_type;
	uint32_t q_size;

	instr_type = LIO_IQ_INSTR_TYPE(lio_dev);

	q_size = instr_type * num_descs;
	iq = lio_dev->instr_queue[iq_no];
	iq->iq_mz = rte_eth_dma_zone_reserve(lio_dev->eth_dev,
					     "instr_queue", iq_no, q_size,
					     RTE_CACHE_LINE_SIZE,
					     socket_id);
	if (iq->iq_mz == NULL) {
		lio_dev_err(lio_dev, "Cannot allocate memory for instr queue %d\n",
			    iq_no);
		return -1;
	}

	iq->base_addr_dma = iq->iq_mz->phys_addr;
	iq->base_addr = (uint8_t *)iq->iq_mz->addr;

	iq->max_count = num_descs;

	/* Initialize a list to holds requests that have been posted to Octeon
	 * but has yet to be fetched by octeon
	 */
	iq->request_list = rte_zmalloc_socket("request_list",
					      sizeof(*iq->request_list) *
							num_descs,
					      RTE_CACHE_LINE_SIZE,
					      socket_id);
	if (iq->request_list == NULL) {
		lio_dev_err(lio_dev, "Alloc failed for IQ[%d] nr free list\n",
			    iq_no);
		lio_dma_zone_free(lio_dev, iq->iq_mz);
		return -1;
	}

	lio_dev_dbg(lio_dev, "IQ[%d]: base: %p basedma: %lx count: %d\n",
		    iq_no, iq->base_addr, (unsigned long)iq->base_addr_dma,
		    iq->max_count);

	iq->lio_dev = lio_dev;
	iq->txpciq.txpciq64 = txpciq.txpciq64;
	iq->fill_cnt = 0;
	iq->host_write_index = 0;
	iq->lio_read_index = 0;
	iq->flush_index = 0;

	rte_atomic64_set(&iq->instr_pending, 0);

	/* Initialize the spinlock for this instruction queue */
	rte_spinlock_init(&iq->lock);
	rte_spinlock_init(&iq->post_lock);

	rte_atomic64_clear(&iq->iq_flush_running);

	lio_dev->io_qmask.iq |= (1ULL << iq_no);

	/* Set the 32B/64B mode for each input queue */
	lio_dev->io_qmask.iq64B |= ((instr_type == 64) << iq_no);
	iq->iqcmd_64B = (instr_type == 64);

	lio_dev->fn_list.setup_iq_regs(lio_dev, iq_no);

	return 0;
}

int
lio_setup_instr_queue0(struct lio_device *lio_dev)
{
	union octeon_txpciq txpciq;
	uint32_t num_descs = 0;
	uint32_t iq_no = 0;

	num_descs = LIO_NUM_DEF_TX_DESCS_CFG(lio_dev);

	lio_dev->num_iqs = 0;

	lio_dev->instr_queue[0] = rte_zmalloc(NULL,
					sizeof(struct lio_instr_queue), 0);
	if (lio_dev->instr_queue[0] == NULL)
		return -ENOMEM;

	lio_dev->instr_queue[0]->q_index = 0;
	lio_dev->instr_queue[0]->app_ctx = (void *)(size_t)0;
	txpciq.txpciq64 = 0;
	txpciq.s.q_no = iq_no;
	txpciq.s.pkind = lio_dev->pfvf_hsword.pkind;
	txpciq.s.use_qpg = 0;
	txpciq.s.qpg = 0;
	if (lio_init_instr_queue(lio_dev, txpciq, num_descs, SOCKET_ID_ANY)) {
		rte_free(lio_dev->instr_queue[0]);
		lio_dev->instr_queue[0] = NULL;
		return -1;
	}

	lio_dev->num_iqs++;

	return 0;
}

/**
 *  lio_delete_instr_queue()
 *  @param lio_dev	- pointer to the lio device structure.
 *  @param iq_no	- queue to be deleted.
 *
 *  Called at driver unload time for each input queue. Deletes all
 *  allocated resources for the input queue.
 */
static void
lio_delete_instr_queue(struct lio_device *lio_dev, uint32_t iq_no)
{
	struct lio_instr_queue *iq = lio_dev->instr_queue[iq_no];

	rte_free(iq->request_list);
	iq->request_list = NULL;
	lio_dma_zone_free(lio_dev, iq->iq_mz);
}

void
lio_free_instr_queue0(struct lio_device *lio_dev)
{
	lio_delete_instr_queue(lio_dev, 0);
	rte_free(lio_dev->instr_queue[0]);
	lio_dev->instr_queue[0] = NULL;
	lio_dev->num_iqs--;
}

static inline void
lio_ring_doorbell(struct lio_device *lio_dev,
		  struct lio_instr_queue *iq)
{
	if (rte_atomic64_read(&lio_dev->status) == LIO_DEV_RUNNING) {
		rte_write32(iq->fill_cnt, iq->doorbell_reg);
		/* make sure doorbell write goes through */
		rte_wmb();
		iq->fill_cnt = 0;
	}
}

static inline void
copy_cmd_into_iq(struct lio_instr_queue *iq, uint8_t *cmd)
{
	uint8_t *iqptr, cmdsize;

	cmdsize = ((iq->iqcmd_64B) ? 64 : 32);
	iqptr = iq->base_addr + (cmdsize * iq->host_write_index);

	rte_memcpy(iqptr, cmd, cmdsize);
}

static inline struct lio_iq_post_status
post_command2(struct lio_instr_queue *iq, uint8_t *cmd)
{
	struct lio_iq_post_status st;

	st.status = LIO_IQ_SEND_OK;

	/* This ensures that the read index does not wrap around to the same
	 * position if queue gets full before Octeon could fetch any instr.
	 */
	if (rte_atomic64_read(&iq->instr_pending) >=
			(int32_t)(iq->max_count - 1)) {
		st.status = LIO_IQ_SEND_FAILED;
		st.index = -1;
		return st;
	}

	if (rte_atomic64_read(&iq->instr_pending) >=
			(int32_t)(iq->max_count - 2))
		st.status = LIO_IQ_SEND_STOP;

	copy_cmd_into_iq(iq, cmd);

	/* "index" is returned, host_write_index is modified. */
	st.index = iq->host_write_index;
	iq->host_write_index = lio_incr_index(iq->host_write_index, 1,
					      iq->max_count);
	iq->fill_cnt++;

	/* Flush the command into memory. We need to be sure the data is in
	 * memory before indicating that the instruction is pending.
	 */
	rte_wmb();

	rte_atomic64_inc(&iq->instr_pending);

	return st;
}

static inline void
lio_add_to_request_list(struct lio_instr_queue *iq,
			int idx, void *buf, int reqtype)
{
	iq->request_list[idx].buf = buf;
	iq->request_list[idx].reqtype = reqtype;
}

static int
lio_send_command(struct lio_device *lio_dev, uint32_t iq_no, void *cmd,
		 void *buf, uint32_t datasize __rte_unused, uint32_t reqtype)
{
	struct lio_instr_queue *iq = lio_dev->instr_queue[iq_no];
	struct lio_iq_post_status st;

	rte_spinlock_lock(&iq->post_lock);

	st = post_command2(iq, cmd);

	if (st.status != LIO_IQ_SEND_FAILED) {
		lio_add_to_request_list(iq, st.index, buf, reqtype);
		lio_ring_doorbell(lio_dev, iq);
	}

	rte_spinlock_unlock(&iq->post_lock);

	return st.status;
}

void
lio_prepare_soft_command(struct lio_device *lio_dev,
			 struct lio_soft_command *sc, uint8_t opcode,
			 uint8_t subcode, uint32_t irh_ossp, uint64_t ossp0,
			 uint64_t ossp1)
{
	struct octeon_instr_pki_ih3 *pki_ih3;
	struct octeon_instr_ih3 *ih3;
	struct octeon_instr_irh *irh;
	struct octeon_instr_rdp *rdp;

	RTE_ASSERT(opcode <= 15);
	RTE_ASSERT(subcode <= 127);

	ih3	  = (struct octeon_instr_ih3 *)&sc->cmd.cmd3.ih3;

	ih3->pkind = lio_dev->instr_queue[sc->iq_no]->txpciq.s.pkind;

	pki_ih3 = (struct octeon_instr_pki_ih3 *)&sc->cmd.cmd3.pki_ih3;

	pki_ih3->w	= 1;
	pki_ih3->raw	= 1;
	pki_ih3->utag	= 1;
	pki_ih3->uqpg	= lio_dev->instr_queue[sc->iq_no]->txpciq.s.use_qpg;
	pki_ih3->utt	= 1;

	pki_ih3->tag	= LIO_CONTROL;
	pki_ih3->tagtype = OCTEON_ATOMIC_TAG;
	pki_ih3->qpg	= lio_dev->instr_queue[sc->iq_no]->txpciq.s.qpg;
	pki_ih3->pm	= 0x7;
	pki_ih3->sl	= 8;

	if (sc->datasize)
		ih3->dlengsz = sc->datasize;

	irh		= (struct octeon_instr_irh *)&sc->cmd.cmd3.irh;
	irh->opcode	= opcode;
	irh->subcode	= subcode;

	/* opcode/subcode specific parameters (ossp) */
	irh->ossp = irh_ossp;
	sc->cmd.cmd3.ossp[0] = ossp0;
	sc->cmd.cmd3.ossp[1] = ossp1;

	if (sc->rdatasize) {
		rdp = (struct octeon_instr_rdp *)&sc->cmd.cmd3.rdp;
		rdp->pcie_port = lio_dev->pcie_port;
		rdp->rlen      = sc->rdatasize;
		irh->rflag = 1;
		/* PKI IH3 */
		ih3->fsz    = OCTEON_SOFT_CMD_RESP_IH3;
	} else {
		irh->rflag = 0;
		/* PKI IH3 */
		ih3->fsz    = OCTEON_PCI_CMD_O3;
	}
}

int
lio_send_soft_command(struct lio_device *lio_dev,
		      struct lio_soft_command *sc)
{
	struct octeon_instr_ih3 *ih3;
	struct octeon_instr_irh *irh;
	uint32_t len = 0;

	ih3 = (struct octeon_instr_ih3 *)&sc->cmd.cmd3.ih3;
	if (ih3->dlengsz) {
		RTE_ASSERT(sc->dmadptr);
		sc->cmd.cmd3.dptr = sc->dmadptr;
	}

	irh = (struct octeon_instr_irh *)&sc->cmd.cmd3.irh;
	if (irh->rflag) {
		RTE_ASSERT(sc->dmarptr);
		RTE_ASSERT(sc->status_word != NULL);
		*sc->status_word = LIO_COMPLETION_WORD_INIT;
		sc->cmd.cmd3.rptr = sc->dmarptr;
	}

	len = (uint32_t)ih3->dlengsz;

	if (sc->wait_time)
		sc->timeout = lio_uptime + sc->wait_time;

	return lio_send_command(lio_dev, sc->iq_no, &sc->cmd, sc, len,
				LIO_REQTYPE_SOFT_COMMAND);
}

int
lio_setup_sc_buffer_pool(struct lio_device *lio_dev)
{
	char sc_pool_name[RTE_MEMPOOL_NAMESIZE];
	uint16_t buf_size;

	buf_size = LIO_SOFT_COMMAND_BUFFER_SIZE + RTE_PKTMBUF_HEADROOM;
	snprintf(sc_pool_name, sizeof(sc_pool_name),
		 "lio_sc_pool_%u", lio_dev->port_id);
	lio_dev->sc_buf_pool = rte_pktmbuf_pool_create(sc_pool_name,
						LIO_MAX_SOFT_COMMAND_BUFFERS,
						0, 0, buf_size, SOCKET_ID_ANY);
	return 0;
}

void
lio_free_sc_buffer_pool(struct lio_device *lio_dev)
{
	rte_mempool_free(lio_dev->sc_buf_pool);
}

struct lio_soft_command *
lio_alloc_soft_command(struct lio_device *lio_dev, uint32_t datasize,
		       uint32_t rdatasize, uint32_t ctxsize)
{
	uint32_t offset = sizeof(struct lio_soft_command);
	struct lio_soft_command *sc;
	struct rte_mbuf *m;
	uint64_t dma_addr;

	RTE_ASSERT((offset + datasize + rdatasize + ctxsize) <=
		   LIO_SOFT_COMMAND_BUFFER_SIZE);

	m = rte_pktmbuf_alloc(lio_dev->sc_buf_pool);
	if (m == NULL) {
		lio_dev_err(lio_dev, "Cannot allocate mbuf for sc\n");
		return NULL;
	}

	/* set rte_mbuf data size and there is only 1 segment */
	m->pkt_len = LIO_SOFT_COMMAND_BUFFER_SIZE;
	m->data_len = LIO_SOFT_COMMAND_BUFFER_SIZE;

	/* use rte_mbuf buffer for soft command */
	sc = rte_pktmbuf_mtod(m, struct lio_soft_command *);
	memset(sc, 0, LIO_SOFT_COMMAND_BUFFER_SIZE);
	sc->size = LIO_SOFT_COMMAND_BUFFER_SIZE;
	sc->dma_addr = rte_mbuf_data_dma_addr(m);
	sc->mbuf = m;

	dma_addr = sc->dma_addr;

	if (ctxsize) {
		sc->ctxptr = (uint8_t *)sc + offset;
		sc->ctxsize = ctxsize;
	}

	/* Start data at 128 byte boundary */
	offset = (offset + ctxsize + 127) & 0xffffff80;

	if (datasize) {
		sc->virtdptr = (uint8_t *)sc + offset;
		sc->dmadptr = dma_addr + offset;
		sc->datasize = datasize;
	}

	/* Start rdata at 128 byte boundary */
	offset = (offset + datasize + 127) & 0xffffff80;

	if (rdatasize) {
		RTE_ASSERT(rdatasize >= 16);
		sc->virtrptr = (uint8_t *)sc + offset;
		sc->dmarptr = dma_addr + offset;
		sc->rdatasize = rdatasize;
		sc->status_word = (uint64_t *)((uint8_t *)(sc->virtrptr) +
					       rdatasize - 8);
	}

	return sc;
}

void
lio_free_soft_command(struct lio_soft_command *sc)
{
	rte_pktmbuf_free(sc->mbuf);
}

void
lio_setup_response_list(struct lio_device *lio_dev)
{
	STAILQ_INIT(&lio_dev->response_list.head);
	rte_spinlock_init(&lio_dev->response_list.lock);
	rte_atomic64_set(&lio_dev->response_list.pending_req_count, 0);
}

int
lio_process_ordered_list(struct lio_device *lio_dev)
{
	int resp_to_process = LIO_MAX_ORD_REQS_TO_PROCESS;
	struct lio_response_list *ordered_sc_list;
	struct lio_soft_command *sc;
	int request_complete = 0;
	uint64_t status64;
	uint32_t status;

	ordered_sc_list = &lio_dev->response_list;

	do {
		rte_spinlock_lock(&ordered_sc_list->lock);

		if (STAILQ_EMPTY(&ordered_sc_list->head)) {
			/* ordered_sc_list is empty; there is
			 * nothing to process
			 */
			rte_spinlock_unlock(&ordered_sc_list->lock);
			return -1;
		}

		sc = LIO_STQUEUE_FIRST_ENTRY(&ordered_sc_list->head,
					     struct lio_soft_command, node);

		status = LIO_REQUEST_PENDING;

		/* check if octeon has finished DMA'ing a response
		 * to where rptr is pointing to
		 */
		status64 = *sc->status_word;

		if (status64 != LIO_COMPLETION_WORD_INIT) {
			/* This logic ensures that all 64b have been written.
			 * 1. check byte 0 for non-FF
			 * 2. if non-FF, then swap result from BE to host order
			 * 3. check byte 7 (swapped to 0) for non-FF
			 * 4. if non-FF, use the low 32-bit status code
			 * 5. if either byte 0 or byte 7 is FF, don't use status
			 */
			if ((status64 & 0xff) != 0xff) {
				lio_swap_8B_data(&status64, 1);
				if (((status64 & 0xff) != 0xff)) {
					/* retrieve 16-bit firmware status */
					status = (uint32_t)(status64 &
							    0xffffULL);
					if (status) {
						status =
						LIO_FIRMWARE_STATUS_CODE(
									status);
					} else {
						/* i.e. no error */
						status = LIO_REQUEST_DONE;
					}
				}
			}
		} else if ((sc->timeout && lio_check_timeout(lio_uptime,
							     sc->timeout))) {
			lio_dev_err(lio_dev,
				    "cmd failed, timeout (%ld, %ld)\n",
				    (long)lio_uptime, (long)sc->timeout);
			status = LIO_REQUEST_TIMEOUT;
		}

		if (status != LIO_REQUEST_PENDING) {
			/* we have received a response or we have timed out.
			 * remove node from linked list
			 */
			STAILQ_REMOVE(&ordered_sc_list->head,
				      &sc->node, lio_stailq_node, entries);
			rte_atomic64_dec(
			    &lio_dev->response_list.pending_req_count);
			rte_spinlock_unlock(&ordered_sc_list->lock);

			if (sc->callback)
				sc->callback(status, sc->callback_arg);

			request_complete++;
		} else {
			/* no response yet */
			request_complete = 0;
			rte_spinlock_unlock(&ordered_sc_list->lock);
		}

		/* If we hit the Max Ordered requests to process every loop,
		 * we quit and let this function be invoked the next time
		 * the poll thread runs to process the remaining requests.
		 * This function can take up the entire CPU if there is
		 * no upper limit to the requests processed.
		 */
		if (request_complete >= resp_to_process)
			break;
	} while (request_complete);

	return 0;
}
