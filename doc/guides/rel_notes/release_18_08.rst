DPDK Release 18.08
==================

.. **Read this first.**

   The text in the sections below explains how to update the release notes.

   Use proper spelling, capitalization and punctuation in all sections.

   Variable and config names should be quoted as fixed width text:
   ``LIKE_THIS``.

   Build the docs and view the output file to ensure the changes are correct::

      make doc-guides-html

      xdg-open build/doc/html/guides/rel_notes/release_18_08.html


New Features
------------

.. This section should contain new features added in this release.
   Sample format:

   * **Add a title in the past tense with a full stop.**

     Add a short 1-2 sentence description in the past tense.
     The description should be enough to allow someone scanning
     the release notes to understand the new feature.

     If the feature adds a lot of sub-features you can use a bullet list
     like this:

     * Added feature foo to do something.
     * Enhanced feature bar to do something else.

     Refer to the previous release notes for examples.

     This section is a comment. Do not overwrite or remove it.
     Also, make sure to start the actual text at the margin.
     =========================================================

* **Added support for Hyper-V netvsc PMD.**

  The new ``netvsc`` poll mode driver provides native support for
  networking on Hyper-V. See the :doc:`../nics/netvsc` NIC driver guide
  for more details on this new driver.

* **Added Flow API support for CXGBE PMD.**

  Flow API support has been added to CXGBE Poll Mode Driver to offload
  flows to Chelsio T5/T6 NICs. Support added for:
  * Wildcard (LE-TCAM) and Exact (HASH) match filters.
  * Match items: physical ingress port, IPv4, IPv6, TCP and UDP.
  * Action items: queue, drop, count, and physical egress port redirect.

* **Added ixgbe preferred Rx/Tx parameters.**

  Rather than applications providing explicit Rx and Tx parameters such as
  queue and burst sizes, they can request that the EAL instead uses preferred
  values provided by the PMD, falling back to defaults within the EAL if the
  PMD does not provide any. The provision of such tuned values now includes
  the ixgbe PMD.

* **Added descriptor status check support for fm10k.**

  ``rte_eth_rx_descritpr_status`` and ``rte_eth_tx_descriptor_status``
  are supported by fm10K.

* **Updated the enic driver.**

  * Add support for mbuf fast free offload.
  * Add low cycle count Tx handler for no-offload Tx (except mbuf fast free).
  * Add low cycle count Rx handler for non-scattered Rx.
  * Minor performance improvements to scattered Rx handler.
  * Add handlers to add/delete VxLAN port number.
  * Add devarg to specify ingress VLAN rewrite mode.

* **Updated the AESNI MB PMD.**

  The AESNI MB PMD has been updated with additional support for:

  * 3DES for 8, 16 and 24 byte keys.

* **Added a new compression PMD using Intel's QuickAssist (QAT) device family.**

  Added the new ``QAT`` compression driver, for compression and decompression
  operations in software. See the :doc:`../compressdevs/qat_comp` compression
  driver guide for details on this new driver.

* **Updated the ISA-L PMD.**

  Added support for chained mbufs (input and output).


API Changes
-----------

.. This section should contain API changes. Sample format:

   * Add a short 1-2 sentence description of the API change.
     Use fixed width quotes for ``function_names`` or ``struct_names``.
     Use the past tense.

   This section is a comment. Do not overwrite or remove it.
   Also, make sure to start the actual text at the margin.
   =========================================================

* Path to runtime config file has changed. The new path is determined as
  follows:

  - If DPDK is running as root, ``/var/run/dpdk/<prefix>/config``
  - If DPDK is not running as root:

    * If ``$XDG_RUNTIME_DIR`` is set, ``${XDG_RUNTIME_DIR}/dpdk/<prefix>/config``
    * Otherwise, ``/tmp/dpdk/<prefix>/config``

* cryptodev: In struct ``struct rte_cryptodev_info``, field ``rte_pci_device *pci_dev``
  has been replaced with field ``struct rte_device *device``.
  Value 0 is accepted in ``sym.max_nb_sessions``, meaning that a device
  supports an unlimited number of sessions.
  Two new fields of type ``uint16_t`` have been added:
  ``min_mbuf_headroom_req`` and ``min_mbuf_tailroom_req``.
  These parameters specify the recommended headroom and tailroom for mbufs
  to be processed by the PMD.

* cryptodev: Following functions were deprecated and are removed in 18.08:

  - ``rte_cryptodev_queue_pair_start``
  - ``rte_cryptodev_queue_pair_stop``
  - ``rte_cryptodev_queue_pair_attach_sym_session``
  - ``rte_cryptodev_queue_pair_detach_sym_session``

* cryptodev: Following functions were deprecated and are replaced by
  other functions in 18.08:

  - ``rte_cryptodev_get_header_session_size`` is replaced with
    ``rte_cryptodev_sym_get_header_session_size``
  - ``rte_cryptodev_get_private_session_size`` is replaced with
    ``rte_cryptodev_sym_get_private_session_size``

* cryptodev: Feature flag ``RTE_CRYPTODEV_FF_MBUF_SCATTER_GATHER`` is
  replaced with the following more explicit flags:

  - ``RTE_CRYPTODEV_FF_IN_PLACE_SGL``
  - ``RTE_CRYPTODEV_FF_OOP_SGL_IN_SGL_OUT``
  - ``RTE_CRYPTODEV_FF_OOP_SGL_IN_LB_OUT``
  - ``RTE_CRYPTODEV_FF_OOP_LB_IN_SGL_OUT``
  - ``RTE_CRYPTODEV_FF_OOP_LB_IN_LB_OUT``

* cryptodev: Renamed cryptodev experimental APIs:

  Used user_data instead of private_data in following APIs to avoid confusion
  with the existing session parameter ``sess_private_data[]`` and related APIs.

  - ``rte_cryptodev_sym_session_set_private_data()`` changed to
    ``rte_cryptodev_sym_session_set_user_data()``
  - ``rte_cryptodev_sym_session_get_private_data()`` changed to
    ``rte_cryptodev_sym_session_get_user_data()``

* compressdev: Feature flag ``RTE_COMP_FF_MBUF_SCATTER_GATHER`` is
  replaced with the following more explicit flags:

  - ``RTE_COMP_FF_OOP_SGL_IN_SGL_OUT``
  - ``RTE_COMP_FF_OOP_SGL_IN_LB_OUT``
  - ``RTE_COMP_FF_OOP_LB_IN_SGL_OUT``


ABI Changes
-----------

.. This section should contain ABI changes. Sample format:

   * Add a short 1-2 sentence description of the ABI change
     that was announced in the previous releases and made in this release.
     Use fixed width quotes for ``function_names`` or ``struct_names``.
     Use the past tense.

   This section is a comment. Do not overwrite or remove it.
   Also, make sure to start the actual text at the margin.
   =========================================================


Removed Items
-------------

.. This section should contain removed items in this release. Sample format:

   * Add a short 1-2 sentence description of the removed item
     in the past tense.

   This section is a comment. Do not overwrite or remove it.
   Also, make sure to start the actual text at the margin.
   =========================================================


Shared Library Versions
-----------------------

.. Update any library version updated in this release
   and prepend with a ``+`` sign, like this:

     librte_acl.so.2
   + librte_cfgfile.so.2
     librte_cmdline.so.2

   This section is a comment. Do not overwrite or remove it.
   =========================================================

The libraries prepended with a plus sign were incremented in this version.

.. code-block:: diff

     librte_acl.so.2
     librte_bbdev.so.1
     librte_bitratestats.so.2
     librte_bpf.so.1
     librte_bus_dpaa.so.1
     librte_bus_fslmc.so.1
     librte_bus_pci.so.1
     librte_bus_vdev.so.1
   + librte_bus_vmbus.so.1
     librte_cfgfile.so.2
     librte_cmdline.so.2
     librte_common_octeontx.so.1
     librte_compressdev.so.1
   + librte_cryptodev.so.5
     librte_distributor.so.1
     librte_eal.so.7
     librte_ethdev.so.9
     librte_eventdev.so.4
     librte_flow_classify.so.1
     librte_gro.so.1
     librte_gso.so.1
     librte_hash.so.2
     librte_ip_frag.so.1
     librte_jobstats.so.1
     librte_kni.so.2
     librte_kvargs.so.1
     librte_latencystats.so.1
     librte_lpm.so.2
     librte_mbuf.so.4
     librte_mempool.so.4
     librte_meter.so.2
     librte_metrics.so.1
     librte_net.so.1
     librte_pci.so.1
     librte_pdump.so.2
     librte_pipeline.so.3
     librte_pmd_bnxt.so.2
     librte_pmd_bond.so.2
     librte_pmd_i40e.so.2
     librte_pmd_ixgbe.so.2
     librte_pmd_dpaa2_cmdif.so.1
     librte_pmd_dpaa2_qdma.so.1
     librte_pmd_ring.so.2
     librte_pmd_softnic.so.1
     librte_pmd_vhost.so.2
     librte_port.so.3
     librte_power.so.1
     librte_rawdev.so.1
     librte_reorder.so.1
     librte_ring.so.2
     librte_sched.so.1
     librte_security.so.1
     librte_table.so.3
     librte_timer.so.1
     librte_vhost.so.3


Known Issues
------------

.. This section should contain new known issues in this release. Sample format:

   * **Add title in present tense with full stop.**

     Add a short 1-2 sentence description of the known issue
     in the present tense. Add information on any known workarounds.

   This section is a comment. Do not overwrite or remove it.
   Also, make sure to start the actual text at the margin.
   =========================================================


Tested Platforms
----------------

.. This section should contain a list of platforms that were tested
   with this release.

   The format is:

   * <vendor> platform with <vendor> <type of devices> combinations

     * List of CPU
     * List of OS
     * List of devices
     * Other relevant details...

   This section is a comment. Do not overwrite or remove it.
   Also, make sure to start the actual text at the margin.
   =========================================================
