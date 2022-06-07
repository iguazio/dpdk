.. SPDX-License-Identifier: BSD-3-Clause
   Copyright 2019 The DPDK contributors

.. include:: <isonum.txt>

DPDK Release 20.02
==================

.. **Read this first.**

   The text in the sections below explains how to update the release notes.

   Use proper spelling, capitalization and punctuation in all sections.

   Variable and config names should be quoted as fixed width text:
   ``LIKE_THIS``.

   Build the docs and view the output file to ensure the changes are correct::

      make doc-guides-html

      xdg-open build/doc/html/guides/rel_notes/release_20_02.html


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

     Suggested order in release notes items:
     * Core libs (EAL, mempool, ring, mbuf, buses)
     * Device abstraction libs and PMDs
       - ethdev (lib, PMDs)
       - cryptodev (lib, PMDs)
       - eventdev (lib, PMDs)
       - etc
     * Other libs
     * Apps, Examples, Tools (if significant)

     This section is a comment. Do not overwrite or remove it.
     Also, make sure to start the actual text at the margin.
     =========================================================

* **Added Wait Until Equal API.**

  A new API has been added to wait for a memory location to be updated with a
  16-bit, 32-bit, 64-bit value.

* **Added rte_ring_xxx_elem APIs.**

  New APIs have been added to support rings with custom element size.

* **Updated rte_flow api to support L2TPv3 over IP flows.**

  Added support for new flow item to handle L2TPv3 over IP rte_flow patterns.

* **Added IONIC net PMD.**

  Added the new ``ionic`` net driver for Pensando Ethernet Network Adapters.
  See the :doc:`../nics/ionic` NIC guide for more details on this new driver.

* **Updated Broadcom bnxt driver**

  Updated Broadcom bnxt driver with new features and improvements, including:

  * Added support for MARK action.

* **Updated Hisilicon hns3 driver.**

  Updated Hisilicon hns3 driver with new features and improvements, including:

  * Added support for Rx interrupt.
  * Added support setting VF MAC address by PF driver.

* **Updated the Intel ice driver.**

  Updated the Intel ice driver with new features and improvements, including:

  * Added support for MAC rules on specific port.
  * Added support for MAC/VLAN with TCP/UDP in switch rule.
  * Added support for 1/10G device.
  * Added support for API rte_eth_tx_done_cleanup.

* **Updated Intel iavf driver.**

  Updated iavf PMD with new features and improvements, including:

  * Added more supported device IDs.
  * Updated virtual channel to latest AVF spec.

* **Updated the Intel ixgbe driver.**

  * Added support for API rte_eth_tx_done_cleanup.
  * Added support setting VF MAC address by PF driver.
  * Added support for setting the link to specific speed.

* **Updated Intel i40e driver.**

  Updated i40e PMD with new features and improvements, including:

  * Added support for L2TPv3 over IP profiles which can be programmed by the
    dynamic device personalization (DDP) process.
  * Added support for ESP-AH profiles which can be programmed by the
    dynamic device personalization (DDP) process.
  * Added PF support Malicious Device Drive event catch and notify.
  * Added LLDP support.
  * Extended PHY access AQ cmd.
  * Added support for reading LPI counters.
  * Added support for Energy Efficient Ethernet.
  * Added support for API rte_eth_tx_done_cleanup.
  * Added support for VF multiple queues interrupt.
  * Added support for setting the link to specific speed.

* **Updated Mellanox mlx5 driver.**

  Updated Mellanox mlx5 driver with new features and improvements, including:

  * Added support for RSS using L3/L4 source/destination only.
  * Added support for matching on GTP tunnel header item.
  * Removed limitation of matching on tagged/untagged packets (when using DV flow engine).

* **Add new vDPA PMD based on Mellanox devices**

  Added a new Mellanox vDPA  (``mlx5_vdpa``) PMD.
  See the :doc:`../vdpadevs/mlx5` guide for more details on this driver.

* **Added support for virtio-PMD notification data.**

  Added support for virtio-PMD notification data so that the driver
  passes extra data (besides identifying the virtqueue) in its device
  notifications, expanding the notifications to include the avail index and
  avail wrap counter (When split ring is used, the avail wrap counter is not
  included in the notification data).

* **Updated testpmd application.**

  Added support for ESP and L2TPv3 over IP rte_flow patterns to the testpmd
  application.

* **Added algorithms to cryptodev API.**

  * ECDSA (Elliptic Curve Digital Signature Algorithm) is added to
    asymmetric crypto library specifications.
  * ECPM (Elliptic Curve Point Multiplication) is added to
    asymmetric crypto library specifications.

* **Added synchronous Crypto burst API.**

  A new API is introduced in crypto library to handle synchronous cryptographic
  operations allowing to achieve performance gain for cryptodevs which use
  CPU based acceleration, such as Intel AES-NI. An implementation for aesni_gcm
  cryptodev is provided. The IPsec example application and ipsec library itself
  were changed to allow utilization of this new feature.

* **Added handling of mixed algorithms in encrypted digest requests in QAT PMD.**

  Added handling of mixed algorithms in encrypted digest hash-cipher
  (generation) and cipher-hash (verification) requests (e.g. SNOW3G + ZUC or
  ZUC + AES CTR) in QAT PMD possible when running on GEN3 QAT hardware.
  Such algorithm combinations are not supported on GEN1/GEN2 hardware
  and executing the request returns RTE_CRYPTO_OP_STATUS_INVALID_SESSION.

* **Queue-pairs are now thread-safe on Intel QuickAssist Technology (QAT) PMD.**

  Queue-pairs are thread-safe on Intel CPUs but Queues are not (that is, within
  a single queue-pair all enqueues to the TX queue must be done from one thread
  and all dequeues from the RX queue must be done from one thread, but enqueues
  and dequeues may be done in different threads.).

* **Updated the ZUC PMD.**

  * Transistioned underlying library from libSSO ZUC to intel-ipsec-mb
    library (minimum version required 0.53).
  * Removed dynamic library limitation, so PMD can be built as a shared
    object now.

* **Updated the KASUMI PMD.**

  * Transistioned underlying library from libSSO KASUMI to intel-ipsec-mb
    library (minimum version required 0.53).

* **Updated the SNOW3G PMD.**

  * Transistioned underlying library from libSSO SNOW3G to intel-ipsec-mb
    library (minimum version required 0.53).

* **Changed armv8 crypto PMD external dependency.**

  armv8 crypto PMD now depends on Arm crypto library, and Marvell's
  armv8 crypto library is not used anymore. Library name is changed
  from armv8_crypto to AArch64crypto.

* **Added inline IPsec support to Marvell OCTEON TX2 PMD.**

  Added inline IPsec support to Marvell OCTEON TX2 PMD. With the feature,
  applications would be able to offload entire IPsec offload to the hardware.
  For the configured sessions, hardware will do the lookup and perform
  decryption and IPsec transformation. For the outbound path, application
  can submit a plain packet to the PMD, and it would be sent out on wire
  after doing encryption and IPsec transformation of the packet.

* **Added Marvell OCTEON TX2 End Point rawdev PMD.**

  Added a new OCTEON TX2 rawdev PMD for End Point mode of operation.
  See the :doc:`../rawdevs/octeontx2_ep` for more details on this new PMD.

* **Added event mode to l3fwd sample application.**

  Add event device support for ``l3fwd`` sample application. It demonstrates
  usage of poll and event mode IO mechanism under a single application.

* **Added cycle-count mode to the compression performance tool.**

  Enhanced the compression performance tool by adding a cycle-count mode
  which can be used to help measure and tune hardware and software PMDs.


Removed Items
-------------

.. This section should contain removed items in this release. Sample format:

   * Add a short 1-2 sentence description of the removed item
     in the past tense.

   This section is a comment. Do not overwrite or remove it.
   Also, make sure to start the actual text at the margin.
   =========================================================

* **Disabled building all the Linux kernel modules by default.**

  In order to remove the build time dependency with Linux kernel,
  the Technical Board decided to disable all the kernel modules
  by default from 20.02 version.

* **Removed coalescing feature from Intel QuickAssist Technology (QAT) PMD.**

  The internal tail write coalescing feature was removed as not compatible with
  dual-thread feature. It was replaced with a threshold feature. At busy times
  if only a small number of packets can be enqueued, each enqueue causes
  an expensive MMIO write. These MMIO write occurrences can be optimised by using
  the new threshold parameter on process start. Please see qat documentation for
  more details.


API Changes
-----------

.. This section should contain API changes. Sample format:

   * sample: Add a short 1-2 sentence description of the API change
     which was announced in the previous releases and made in this release.
     Start with a scope label like "ethdev:".
     Use fixed width quotes for ``function_names`` or ``struct_names``.
     Use the past tense.

   This section is a comment. Do not overwrite or remove it.
   Also, make sure to start the actual text at the margin.
   =========================================================


ABI Changes
-----------

.. This section should contain ABI changes. Sample format:

   * sample: Add a short 1-2 sentence description of the ABI change
     which was announced in the previous releases and made in this release.
     Start with a scope label like "ethdev:".
     Use fixed width quotes for ``function_names`` or ``struct_names``.
     Use the past tense.

   This section is a comment. Do not overwrite or remove it.
   Also, make sure to start the actual text at the margin.
   =========================================================

* No change, kept ABI v20. DPDK 20.02 is compatible with DPDK 19.11.


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
