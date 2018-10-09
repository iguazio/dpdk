/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Cavium, Inc
 */

#include <rte_alarm.h>
#include <rte_bus_pci.h>
#include <rte_cryptodev.h>
#include <rte_cryptodev_pmd.h>
#include <rte_malloc.h>

#include "cpt_pmd_logs.h"
#include "cpt_pmd_ops_helper.h"

#include "otx_cryptodev.h"
#include "otx_cryptodev_capabilities.h"
#include "otx_cryptodev_hw_access.h"
#include "otx_cryptodev_ops.h"

static int otx_cryptodev_probe_count;
static rte_spinlock_t otx_probe_count_lock = RTE_SPINLOCK_INITIALIZER;

static struct rte_mempool *otx_cpt_meta_pool;
static int otx_cpt_op_mlen;
static int otx_cpt_op_sb_mlen;

/*
 * Initializes global variables used by fast-path code
 *
 * @return
 *   - 0 on success, errcode on error
 */
static int
init_global_resources(void)
{
	/* Get meta len for scatter gather mode */
	otx_cpt_op_mlen = cpt_pmd_ops_helper_get_mlen_sg_mode();

	/* Extra 4B saved for future considerations */
	otx_cpt_op_mlen += 4 * sizeof(uint64_t);

	otx_cpt_meta_pool = rte_mempool_create("cpt_metabuf-pool", 4096 * 16,
					       otx_cpt_op_mlen, 512, 0,
					       NULL, NULL, NULL, NULL,
					       SOCKET_ID_ANY, 0);
	if (!otx_cpt_meta_pool) {
		CPT_LOG_ERR("cpt metabuf pool not created");
		return -ENOMEM;
	}

	/* Get meta len for direct mode */
	otx_cpt_op_sb_mlen = cpt_pmd_ops_helper_get_mlen_direct_mode();

	/* Extra 4B saved for future considerations */
	otx_cpt_op_sb_mlen += 4 * sizeof(uint64_t);

	return 0;
}

void
cleanup_global_resources(void)
{
	/* Take lock */
	rte_spinlock_lock(&otx_probe_count_lock);

	/* Decrement the cryptodev count */
	otx_cryptodev_probe_count--;

	/* Free buffers */
	if (otx_cpt_meta_pool && otx_cryptodev_probe_count == 0)
		rte_mempool_free(otx_cpt_meta_pool);

	/* Free lock */
	rte_spinlock_unlock(&otx_probe_count_lock);
}

/* Alarm routines */

static void
otx_cpt_alarm_cb(void *arg)
{
	struct cpt_vf *cptvf = arg;
	otx_cpt_poll_misc(cptvf);
	rte_eal_alarm_set(CPT_INTR_POLL_INTERVAL_MS * 1000,
			  otx_cpt_alarm_cb, cptvf);
}

static int
otx_cpt_periodic_alarm_start(void *arg)
{
	return rte_eal_alarm_set(CPT_INTR_POLL_INTERVAL_MS * 1000,
				 otx_cpt_alarm_cb, arg);
}

static int
otx_cpt_periodic_alarm_stop(void *arg)
{
	return rte_eal_alarm_cancel(otx_cpt_alarm_cb, arg);
}

/* PMD ops */

static int
otx_cpt_dev_config(struct rte_cryptodev *dev __rte_unused,
		   struct rte_cryptodev_config *config __rte_unused)
{
	CPT_PMD_INIT_FUNC_TRACE();
	return 0;
}

static int
otx_cpt_dev_start(struct rte_cryptodev *c_dev)
{
	void *cptvf = c_dev->data->dev_private;

	CPT_PMD_INIT_FUNC_TRACE();

	return otx_cpt_start_device(cptvf);
}

static void
otx_cpt_dev_stop(struct rte_cryptodev *c_dev)
{
	void *cptvf = c_dev->data->dev_private;

	CPT_PMD_INIT_FUNC_TRACE();

	otx_cpt_stop_device(cptvf);
}

static int
otx_cpt_dev_close(struct rte_cryptodev *c_dev)
{
	void *cptvf = c_dev->data->dev_private;

	CPT_PMD_INIT_FUNC_TRACE();

	otx_cpt_periodic_alarm_stop(cptvf);
	otx_cpt_deinit_device(cptvf);

	return 0;
}

static void
otx_cpt_dev_info_get(struct rte_cryptodev *dev, struct rte_cryptodev_info *info)
{
	CPT_PMD_INIT_FUNC_TRACE();
	if (info != NULL) {
		info->max_nb_queue_pairs = CPT_NUM_QS_PER_VF;
		info->feature_flags = dev->feature_flags;
		info->capabilities = otx_get_capabilities();
		info->sym.max_nb_sessions = 0;
		info->driver_id = otx_cryptodev_driver_id;
		info->min_mbuf_headroom_req = OTX_CPT_MIN_HEADROOM_REQ;
		info->min_mbuf_tailroom_req = OTX_CPT_MIN_TAILROOM_REQ;
	}
}

static void
otx_cpt_stats_get(struct rte_cryptodev *dev __rte_unused,
		  struct rte_cryptodev_stats *stats __rte_unused)
{
	CPT_PMD_INIT_FUNC_TRACE();
}

static void
otx_cpt_stats_reset(struct rte_cryptodev *dev __rte_unused)
{
	CPT_PMD_INIT_FUNC_TRACE();
}

static struct rte_cryptodev_ops cptvf_ops = {
	/* Device related operations */
	.dev_configure = otx_cpt_dev_config,
	.dev_start = otx_cpt_dev_start,
	.dev_stop = otx_cpt_dev_stop,
	.dev_close = otx_cpt_dev_close,
	.dev_infos_get = otx_cpt_dev_info_get,

	.stats_get = otx_cpt_stats_get,
	.stats_reset = otx_cpt_stats_reset,
	.queue_pair_setup = NULL,
	.queue_pair_release = NULL,
	.queue_pair_count = NULL,

	/* Crypto related operations */
	.sym_session_get_size = NULL,
	.sym_session_configure = NULL,
	.sym_session_clear = NULL
};

static void
otx_cpt_common_vars_init(struct cpt_vf *cptvf)
{
	cptvf->meta_info.cptvf_meta_pool = otx_cpt_meta_pool;
	cptvf->meta_info.cptvf_op_mlen = otx_cpt_op_mlen;
	cptvf->meta_info.cptvf_op_sb_mlen = otx_cpt_op_sb_mlen;
}

int
otx_cpt_dev_create(struct rte_cryptodev *c_dev)
{
	struct rte_pci_device *pdev = RTE_DEV_TO_PCI(c_dev->device);
	struct cpt_vf *cptvf = NULL;
	void *reg_base;
	char dev_name[32];
	int ret;

	if (pdev->mem_resource[0].phys_addr == 0ULL)
		return -EIO;

	/* for secondary processes, we don't initialise any further as primary
	 * has already done this work.
	 */
	if (rte_eal_process_type() != RTE_PROC_PRIMARY)
		return 0;

	cptvf = rte_zmalloc_socket("otx_cryptodev_private_mem",
			sizeof(struct cpt_vf), RTE_CACHE_LINE_SIZE,
			rte_socket_id());

	if (cptvf == NULL) {
		CPT_LOG_ERR("Cannot allocate memory for device private data");
		return -ENOMEM;
	}

	snprintf(dev_name, 32, "%02x:%02x.%x",
			pdev->addr.bus, pdev->addr.devid, pdev->addr.function);

	reg_base = pdev->mem_resource[0].addr;
	if (!reg_base) {
		CPT_LOG_ERR("Failed to map BAR0 of %s", dev_name);
		ret = -ENODEV;
		goto fail;
	}

	ret = otx_cpt_hw_init(cptvf, pdev, reg_base, dev_name);
	if (ret) {
		CPT_LOG_ERR("Failed to init cptvf %s", dev_name);
		ret = -EIO;
		goto fail;
	}

	/* Start off timer for mailbox interrupts */
	otx_cpt_periodic_alarm_start(cptvf);

	rte_spinlock_lock(&otx_probe_count_lock);
	if (!otx_cryptodev_probe_count) {
		ret = init_global_resources();
		if (ret) {
			rte_spinlock_unlock(&otx_probe_count_lock);
			goto init_fail;
		}
	}
	otx_cryptodev_probe_count++;
	rte_spinlock_unlock(&otx_probe_count_lock);

	/* Initialize data path variables used by common code */
	otx_cpt_common_vars_init(cptvf);

	c_dev->dev_ops = &cptvf_ops;

	c_dev->enqueue_burst = NULL;
	c_dev->dequeue_burst = NULL;

	c_dev->feature_flags = RTE_CRYPTODEV_FF_SYMMETRIC_CRYPTO |
			RTE_CRYPTODEV_FF_HW_ACCELERATED |
			RTE_CRYPTODEV_FF_SYM_OPERATION_CHAINING |
			RTE_CRYPTODEV_FF_IN_PLACE_SGL |
			RTE_CRYPTODEV_FF_OOP_SGL_IN_LB_OUT |
			RTE_CRYPTODEV_FF_OOP_SGL_IN_SGL_OUT;

	/* Save dev private data */
	c_dev->data->dev_private = cptvf;

	return 0;

init_fail:
	otx_cpt_periodic_alarm_stop(cptvf);
	otx_cpt_deinit_device(cptvf);

fail:
	if (cptvf) {
		/* Free private data allocated */
		rte_free(cptvf);
	}

	return ret;
}
