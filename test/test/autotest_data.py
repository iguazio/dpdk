# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2010-2014 Intel Corporation

# Test data for autotests

from autotest_test_funcs import *

# groups of tests that can be run in parallel
# the grouping has been found largely empirically
parallel_test_list = [
    {
        "Name":    "Cycles autotest",
        "Command": "cycles_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Timer autotest",
        "Command": "timer_autotest",
        "Func":    timer_autotest,
        "Report":   None,
    },
    {
        "Name":    "Debug autotest",
        "Command": "debug_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Errno autotest",
        "Command": "errno_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Meter autotest",
        "Command": "meter_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Common autotest",
        "Command": "common_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Resource autotest",
        "Command": "resource_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Memory autotest",
        "Command": "memory_autotest",
        "Func":    memory_autotest,
        "Report":  None,
    },
    {
        "Name":    "Read/write lock autotest",
        "Command": "rwlock_autotest",
        "Func":    rwlock_autotest,
        "Report":  None,
    },
    {
        "Name":    "Logs autotest",
        "Command": "logs_autotest",
        "Func":    logs_autotest,
        "Report":  None,
    },
    {
        "Name":    "CPU flags autotest",
        "Command": "cpuflags_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Version autotest",
        "Command": "version_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "EAL filesystem autotest",
        "Command": "eal_fs_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "EAL flags autotest",
        "Command": "eal_flags_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Hash autotest",
        "Command": "hash_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "LPM autotest",
        "Command": "lpm_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "LPM6 autotest",
        "Command": "lpm6_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Memcpy autotest",
        "Command": "memcpy_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Memzone autotest",
        "Command": "memzone_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "String autotest",
        "Command": "string_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Alarm autotest",
        "Command": "alarm_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Malloc autotest",
        "Command": "malloc_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Multi-process autotest",
        "Command": "multiprocess_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Mbuf autotest",
        "Command": "mbuf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Per-lcore autotest",
        "Command": "per_lcore_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Ring autotest",
        "Command": "ring_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Spinlock autotest",
        "Command": "spinlock_autotest",
        "Func":    spinlock_autotest,
        "Report":  None,
    },
    {
        "Name":    "Byte order autotest",
        "Command": "byteorder_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "TAILQ autotest",
        "Command": "tailq_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Command-line autotest",
        "Command": "cmdline_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Interrupts autotest",
        "Command": "interrupt_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Function reentrancy autotest",
        "Command": "func_reentrancy_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Mempool autotest",
        "Command": "mempool_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Atomics autotest",
        "Command": "atomic_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Prefetch autotest",
        "Command": "prefetch_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Red autotest",
        "Command": "red_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "PMD ring autotest",
        "Command": "ring_pmd_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Access list control autotest",
        "Command": "acl_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Sched autotest",
        "Command": "sched_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Eventdev selftest octeontx",
        "Command": "eventdev_selftest_octeontx",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Event ring autotest",
        "Command": "event_ring_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Table autotest",
        "Command": "table_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Flow classify autotest",
        "Command": "flow_classify_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Event eth rx adapter autotest",
        "Command": "event_eth_rx_adapter_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "User delay",
        "Command": "user_delay_us",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Sleep delay",
        "Command": "delay_us_sleep_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Rawdev autotest",
        "Command": "rawdev_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Kvargs autotest",
        "Command": "kvargs_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Devargs autotest",
        "Command": "devargs_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Link bonding autotest",
        "Command": "link_bonding_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Link bonding mode4 autotest",
        "Command": "link_bonding_mode4_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Link bonding rssconf autotest",
        "Command": "link_bonding_rssconf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Crc autotest",
        "Command": "crc_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Distributor autotest",
        "Command": "distributor_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Reorder autotest",
        "Command": "reorder_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Barrier autotest",
        "Command": "barrier_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Bitmap test",
        "Command": "bitmap_test",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Hash multiwriter autotest",
        "Command": "hash_multiwriter_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Service autotest",
        "Command": "service_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Timer racecond autotest",
        "Command": "timer_racecond_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Member autotest",
        "Command": "member_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":   "Efd_autotest",
        "Command": "efd_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Thash autotest",
        "Command": "thash_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Hash function autotest",
        "Command": "hash_functions_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev sw mrvl autotest",
        "Command": "cryptodev_sw_mrvl_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev dpaa2 sec autotest",
        "Command": "cryptodev_dpaa2_sec_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev dpaa sec autotest",
        "Command": "cryptodev_dpaa_sec_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev qat autotest",
        "Command": "cryptodev_qat_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev aesni mb autotest",
        "Command": "cryptodev_aesni_mb_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev openssl autotest",
        "Command": "cryptodev_openssl_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev scheduler autotest",
        "Command": "cryptodev_scheduler_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev aesni gcm autotest",
        "Command": "cryptodev_aesni_gcm_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev null autotest",
        "Command": "cryptodev_null_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev sw snow3g autotest",
        "Command": "cryptodev_sw_snow3g_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev sw kasumi autotest",
        "Command": "cryptodev_sw_kasumi_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Cryptodev_sw_zuc_autotest",
        "Command": "cryptodev_sw_zuc_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Reciprocal division",
        "Command": "reciprocal_division",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Red all",
        "Command": "red_all",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
	"Name":    "Fbarray autotest",
	"Command": "fbarray_autotest",
	"Func":    default_autotest,
	"Report":  None,
    },
    {
	"Name":    "External memory autotest",
	"Command": "external_mem_autotest",
	"Func":    default_autotest,
	"Report":  None,
    },
    {
        "Name":    "Metrics autotest",
        "Command": "metrics_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    #
    #Please always keep all dump tests at the end and together!
    #
    {
        "Name":    "Dump physmem",
        "Command": "dump_physmem",
        "Func":    dump_autotest,
        "Report":  None,
    },
    {
        "Name":    "Dump memzone",
        "Command": "dump_memzone",
        "Func":    dump_autotest,
        "Report":  None,
    },
    {
        "Name":    "Dump struct sizes",
        "Command": "dump_struct_sizes",
        "Func":    dump_autotest,
        "Report":  None,
    },
    {
        "Name":    "Dump mempool",
        "Command": "dump_mempool",
        "Func":    dump_autotest,
        "Report":  None,
    },
    {
        "Name":    "Dump malloc stats",
        "Command": "dump_malloc_stats",
        "Func":    dump_autotest,
        "Report":  None,
    },
    {
        "Name":    "Dump devargs",
        "Command": "dump_devargs",
        "Func":    dump_autotest,
        "Report":  None,
    },
    {
        "Name":    "Dump log types",
        "Command": "dump_log_types",
        "Func":    dump_autotest,
        "Report":  None,
    },
    {
        "Name":    "Dump_ring",
        "Command": "dump_ring",
        "Func":    dump_autotest,
        "Report":  None,
    },
]

# tests that should not be run when any other tests are running
non_parallel_test_list = [
    {
        "Name":    "Eventdev common autotest",
        "Command": "eventdev_common_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Eventdev selftest sw",
        "Command": "eventdev_selftest_sw",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "KNI autotest",
        "Command": "kni_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Mempool performance autotest",
        "Command": "mempool_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Memcpy performance autotest",
        "Command": "memcpy_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Hash performance autotest",
        "Command": "hash_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Hash read-write concurrency autotest",
        "Command": "hash_readwrite_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Hash read-write lock-free concurrency autotest",
        "Command": "hash_readwrite_lf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":       "Power autotest",
        "Command":    "power_autotest",
        "Func":       default_autotest,
        "Report":      None,
    },
    {
        "Name":       "Power ACPI cpufreq autotest",
        "Command":    "power_acpi_cpufreq_autotest",
        "Func":       default_autotest,
        "Report":     None,
    },
    {
        "Name":       "Power KVM VM  autotest",
        "Command":    "power_kvm_vm_autotest",
        "Func":       default_autotest,
        "Report":     None,
    },
    {
        "Name":    "Timer performance autotest",
        "Command": "timer_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {

        "Name":    "Pmd perf autotest",
        "Command": "pmd_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Ring pmd perf autotest",
        "Command": "ring_pmd_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Distributor perf autotest",
        "Command": "distributor_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Red_perf",
        "Command": "red_perf",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Lpm6 perf autotest",
        "Command": "lpm6_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Lpm perf autotest",
        "Command": "lpm_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
         "Name":    "Efd perf autotest",
         "Command": "efd_perf_autotest",
         "Func":    default_autotest,
         "Report":  None,
    },
    {
        "Name":    "Member perf autotest",
        "Command": "member_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
    {
        "Name":    "Reciprocal division perf",
        "Command": "reciprocal_division_perf",
        "Func":    default_autotest,
        "Report":  None,
    },
    #
    # Please always make sure that ring_perf is the last test!
    #
    {
        "Name":    "Ring performance autotest",
        "Command": "ring_perf_autotest",
        "Func":    default_autotest,
        "Report":  None,
    },
]
