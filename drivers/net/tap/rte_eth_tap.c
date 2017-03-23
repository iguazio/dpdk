/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2016 Intel Corporation. All rights reserved.
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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <rte_atomic.h>
#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>
#include <rte_vdev.h>
#include <rte_kvargs.h>
#include <rte_net.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/if_ether.h>
#include <fcntl.h>

#include <rte_eth_tap.h>
#include <tap_flow.h>

/* Linux based path to the TUN device */
#define TUN_TAP_DEV_PATH        "/dev/net/tun"
#define DEFAULT_TAP_NAME        "dtap"

#define ETH_TAP_IFACE_ARG       "iface"
#define ETH_TAP_SPEED_ARG       "speed"

#ifdef IFF_MULTI_QUEUE
#define RTE_PMD_TAP_MAX_QUEUES	16
#else
#define RTE_PMD_TAP_MAX_QUEUES	1
#endif

static struct rte_vdev_driver pmd_tap_drv;

static const char *valid_arguments[] = {
	ETH_TAP_IFACE_ARG,
	ETH_TAP_SPEED_ARG,
	NULL
};

static int tap_unit;

static volatile uint32_t tap_trigger;	/* Rx trigger */

static struct rte_eth_link pmd_link = {
	.link_speed = ETH_SPEED_NUM_10G,
	.link_duplex = ETH_LINK_FULL_DUPLEX,
	.link_status = ETH_LINK_DOWN,
	.link_autoneg = ETH_LINK_SPEED_AUTONEG
};

static void
tap_trigger_cb(int sig __rte_unused)
{
	/* Valid trigger values are nonzero */
	tap_trigger = (tap_trigger + 1) | 0x80000000;
}

static int
tap_ioctl(struct pmd_internals *pmd, unsigned long request,
	  struct ifreq *ifr, int set);

/* Tun/Tap allocation routine
 *
 * name is the number of the interface to use, unless NULL to take the host
 * supplied name.
 */
static int
tun_alloc(struct pmd_internals *pmd, uint16_t qid)
{
	struct ifreq ifr;
#ifdef IFF_MULTI_QUEUE
	unsigned int features;
#endif
	int fd;

	memset(&ifr, 0, sizeof(struct ifreq));

	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	snprintf(ifr.ifr_name, IFNAMSIZ, "%s", pmd->name);

	RTE_LOG(DEBUG, PMD, "ifr_name '%s'\n", ifr.ifr_name);

	fd = open(TUN_TAP_DEV_PATH, O_RDWR);
	if (fd < 0) {
		RTE_LOG(ERR, PMD, "Unable to create TAP interface");
		goto error;
	}

#ifdef IFF_MULTI_QUEUE
	/* Grab the TUN features to verify we can work multi-queue */
	if (ioctl(fd, TUNGETFEATURES, &features) < 0) {
		RTE_LOG(ERR, PMD, "TAP unable to get TUN/TAP features\n");
		goto error;
	}
	RTE_LOG(DEBUG, PMD, "  TAP Features %08x\n", features);

	if (features & IFF_MULTI_QUEUE) {
		RTE_LOG(DEBUG, PMD, "  Multi-queue support for %d queues\n",
			RTE_PMD_TAP_MAX_QUEUES);
		ifr.ifr_flags |= IFF_MULTI_QUEUE;
	} else
#endif
	{
		ifr.ifr_flags |= IFF_ONE_QUEUE;
		RTE_LOG(DEBUG, PMD, "  Single queue only support\n");
	}

	/* Set the TUN/TAP configuration and set the name if needed */
	if (ioctl(fd, TUNSETIFF, (void *)&ifr) < 0) {
		RTE_LOG(WARNING, PMD,
			"Unable to set TUNSETIFF for %s\n",
			ifr.ifr_name);
		perror("TUNSETIFF");
		goto error;
	}

	/* Always set the file descriptor to non-blocking */
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
		RTE_LOG(WARNING, PMD,
			"Unable to set %s to nonblocking\n",
			ifr.ifr_name);
		perror("F_SETFL, NONBLOCK");
		goto error;
	}

	/* Set up trigger to optimize empty Rx bursts */
	errno = 0;
	do {
		struct sigaction sa;
		int flags = fcntl(fd, F_GETFL);

		if (flags == -1 || sigaction(SIGIO, NULL, &sa) == -1)
			break;
		if (sa.sa_handler != tap_trigger_cb) {
			/*
			 * Make sure SIGIO is not already taken. This is done
			 * as late as possible to leave the application a
			 * chance to set up its own signal handler first.
			 */
			if (sa.sa_handler != SIG_IGN &&
			    sa.sa_handler != SIG_DFL) {
				errno = EBUSY;
				break;
			}
			sa = (struct sigaction){
				.sa_flags = SA_RESTART,
				.sa_handler = tap_trigger_cb,
			};
			if (sigaction(SIGIO, &sa, NULL) == -1)
				break;
		}
		/* Enable SIGIO on file descriptor */
		fcntl(fd, F_SETFL, flags | O_ASYNC);
		fcntl(fd, F_SETOWN, getpid());
	} while (0);
	if (errno) {
		/* Disable trigger globally in case of error */
		tap_trigger = 0;
		RTE_LOG(WARNING, PMD, "Rx trigger disabled: %s\n",
			strerror(errno));
	}

	if (qid == 0) {
		struct ifreq ifr;

		if (tap_ioctl(pmd, SIOCGIFHWADDR, &ifr, 0) < 0)
			goto error;
		rte_memcpy(&pmd->eth_addr, ifr.ifr_hwaddr.sa_data,
			   ETHER_ADDR_LEN);
	}

	return fd;

error:
	if (fd > 0)
		close(fd);
	return -1;
}

/* Callback to handle the rx burst of packets to the correct interface and
 * file descriptor(s) in a multi-queue setup.
 */
static uint16_t
pmd_rx_burst(void *queue, struct rte_mbuf **bufs, uint16_t nb_pkts)
{
	int len;
	struct rte_mbuf *mbuf;
	struct rx_queue *rxq = queue;
	uint16_t num_rx;
	unsigned long num_rx_bytes = 0;
	uint32_t trigger = tap_trigger;

	if (trigger == rxq->trigger_seen)
		return 0;
	if (trigger)
		rxq->trigger_seen = trigger;
	rte_compiler_barrier();
	for (num_rx = 0; num_rx < nb_pkts; ) {
		/* allocate the next mbuf */
		mbuf = rte_pktmbuf_alloc(rxq->mp);
		if (unlikely(!mbuf)) {
			RTE_LOG(WARNING, PMD, "TAP unable to allocate mbuf\n");
			break;
		}

		len = read(rxq->fd, rte_pktmbuf_mtod(mbuf, char *),
			   rte_pktmbuf_tailroom(mbuf));
		if (len <= 0) {
			rte_pktmbuf_free(mbuf);
			break;
		}

		mbuf->data_len = len;
		mbuf->pkt_len = len;
		mbuf->port = rxq->in_port;
		mbuf->packet_type = rte_net_get_ptype(mbuf, NULL,
						      RTE_PTYPE_ALL_MASK);

		/* account for the receive frame */
		bufs[num_rx++] = mbuf;
		num_rx_bytes += mbuf->pkt_len;
	}
	rxq->stats.ipackets += num_rx;
	rxq->stats.ibytes += num_rx_bytes;

	return num_rx;
}

/* Callback to handle sending packets from the tap interface
 */
static uint16_t
pmd_tx_burst(void *queue, struct rte_mbuf **bufs, uint16_t nb_pkts)
{
	struct rte_mbuf *mbuf;
	struct tx_queue *txq = queue;
	uint16_t num_tx = 0;
	unsigned long num_tx_bytes = 0;
	int i, n;

	if (unlikely(nb_pkts == 0))
		return 0;

	for (i = 0; i < nb_pkts; i++) {
		/* copy the tx frame data */
		mbuf = bufs[num_tx];
		n = write(txq->fd,
			  rte_pktmbuf_mtod(mbuf, void *),
			  rte_pktmbuf_pkt_len(mbuf));
		if (n <= 0)
			break;

		num_tx++;
		num_tx_bytes += mbuf->pkt_len;
		rte_pktmbuf_free(mbuf);
	}

	txq->stats.opackets += num_tx;
	txq->stats.errs += nb_pkts - num_tx;
	txq->stats.obytes += num_tx_bytes;

	return num_tx;
}

static int
tap_ioctl(struct pmd_internals *pmd, unsigned long request,
	  struct ifreq *ifr, int set)
{
	short req_flags = ifr->ifr_flags;

	snprintf(ifr->ifr_name, IFNAMSIZ, "%s", pmd->name);
	switch (request) {
	case SIOCSIFFLAGS:
		/* fetch current flags to leave other flags untouched */
		if (ioctl(pmd->ioctl_sock, SIOCGIFFLAGS, ifr) < 0)
			goto error;
		if (set)
			ifr->ifr_flags |= req_flags;
		else
			ifr->ifr_flags &= ~req_flags;
		break;
	case SIOCGIFHWADDR:
	case SIOCSIFHWADDR:
	case SIOCSIFMTU:
		break;
	default:
		RTE_LOG(WARNING, PMD, "%s: ioctl() called with wrong arg\n",
			pmd->name);
		return -EINVAL;
	}
	if (ioctl(pmd->ioctl_sock, request, ifr) < 0)
		goto error;
	return 0;

error:
	RTE_LOG(ERR, PMD, "%s: ioctl(%lu) failed with error: %s\n",
		ifr->ifr_name, request, strerror(errno));
	return -errno;
}

static int
tap_link_set_down(struct rte_eth_dev *dev)
{
	struct pmd_internals *pmd = dev->data->dev_private;
	struct ifreq ifr = { .ifr_flags = IFF_UP };

	dev->data->dev_link.link_status = ETH_LINK_DOWN;
	return tap_ioctl(pmd, SIOCSIFFLAGS, &ifr, 0);
}

static int
tap_link_set_up(struct rte_eth_dev *dev)
{
	struct pmd_internals *pmd = dev->data->dev_private;
	struct ifreq ifr = { .ifr_flags = IFF_UP };

	dev->data->dev_link.link_status = ETH_LINK_UP;
	return tap_ioctl(pmd, SIOCSIFFLAGS, &ifr, 1);
}

static int
tap_dev_start(struct rte_eth_dev *dev)
{
	return tap_link_set_up(dev);
}

/* This function gets called when the current port gets stopped.
 */
static void
tap_dev_stop(struct rte_eth_dev *dev)
{
	tap_link_set_down(dev);
}

static int
tap_dev_configure(struct rte_eth_dev *dev __rte_unused)
{
	return 0;
}

static uint32_t
tap_dev_speed_capa(void)
{
	uint32_t speed = pmd_link.link_speed;
	uint32_t capa = 0;

	if (speed >= ETH_SPEED_NUM_10M)
		capa |= ETH_LINK_SPEED_10M;
	if (speed >= ETH_SPEED_NUM_100M)
		capa |= ETH_LINK_SPEED_100M;
	if (speed >= ETH_SPEED_NUM_1G)
		capa |= ETH_LINK_SPEED_1G;
	if (speed >= ETH_SPEED_NUM_5G)
		capa |= ETH_LINK_SPEED_2_5G;
	if (speed >= ETH_SPEED_NUM_5G)
		capa |= ETH_LINK_SPEED_5G;
	if (speed >= ETH_SPEED_NUM_10G)
		capa |= ETH_LINK_SPEED_10G;
	if (speed >= ETH_SPEED_NUM_20G)
		capa |= ETH_LINK_SPEED_20G;
	if (speed >= ETH_SPEED_NUM_25G)
		capa |= ETH_LINK_SPEED_25G;
	if (speed >= ETH_SPEED_NUM_40G)
		capa |= ETH_LINK_SPEED_40G;
	if (speed >= ETH_SPEED_NUM_50G)
		capa |= ETH_LINK_SPEED_50G;
	if (speed >= ETH_SPEED_NUM_56G)
		capa |= ETH_LINK_SPEED_56G;
	if (speed >= ETH_SPEED_NUM_100G)
		capa |= ETH_LINK_SPEED_100G;

	return capa;
}

static void
tap_dev_info(struct rte_eth_dev *dev, struct rte_eth_dev_info *dev_info)
{
	struct pmd_internals *internals = dev->data->dev_private;

	dev_info->if_index = internals->if_index;
	dev_info->max_mac_addrs = 1;
	dev_info->max_rx_pktlen = (uint32_t)ETHER_MAX_VLAN_FRAME_LEN;
	dev_info->max_rx_queues = internals->nb_queues;
	dev_info->max_tx_queues = internals->nb_queues;
	dev_info->min_rx_bufsize = 0;
	dev_info->pci_dev = NULL;
	dev_info->speed_capa = tap_dev_speed_capa();
}

static void
tap_stats_get(struct rte_eth_dev *dev, struct rte_eth_stats *tap_stats)
{
	unsigned int i, imax;
	unsigned long rx_total = 0, tx_total = 0, tx_err_total = 0;
	unsigned long rx_bytes_total = 0, tx_bytes_total = 0;
	const struct pmd_internals *pmd = dev->data->dev_private;

	imax = (pmd->nb_queues < RTE_ETHDEV_QUEUE_STAT_CNTRS) ?
		pmd->nb_queues : RTE_ETHDEV_QUEUE_STAT_CNTRS;

	for (i = 0; i < imax; i++) {
		tap_stats->q_ipackets[i] = pmd->rxq[i].stats.ipackets;
		tap_stats->q_ibytes[i] = pmd->rxq[i].stats.ibytes;
		rx_total += tap_stats->q_ipackets[i];
		rx_bytes_total += tap_stats->q_ibytes[i];

		tap_stats->q_opackets[i] = pmd->txq[i].stats.opackets;
		tap_stats->q_errors[i] = pmd->txq[i].stats.errs;
		tap_stats->q_obytes[i] = pmd->txq[i].stats.obytes;
		tx_total += tap_stats->q_opackets[i];
		tx_err_total += tap_stats->q_errors[i];
		tx_bytes_total += tap_stats->q_obytes[i];
	}

	tap_stats->ipackets = rx_total;
	tap_stats->ibytes = rx_bytes_total;
	tap_stats->opackets = tx_total;
	tap_stats->oerrors = tx_err_total;
	tap_stats->obytes = tx_bytes_total;
}

static void
tap_stats_reset(struct rte_eth_dev *dev)
{
	int i;
	struct pmd_internals *pmd = dev->data->dev_private;

	for (i = 0; i < pmd->nb_queues; i++) {
		pmd->rxq[i].stats.ipackets = 0;
		pmd->rxq[i].stats.ibytes = 0;

		pmd->txq[i].stats.opackets = 0;
		pmd->txq[i].stats.errs = 0;
		pmd->txq[i].stats.obytes = 0;
	}
}

static void
tap_dev_close(struct rte_eth_dev *dev __rte_unused)
{
	int i;
	struct pmd_internals *internals = dev->data->dev_private;

	tap_link_set_down(dev);
	tap_flow_flush(dev, NULL);

	for (i = 0; i < internals->nb_queues; i++) {
		if (internals->rxq[i].fd != -1)
			close(internals->rxq[i].fd);
		internals->rxq[i].fd = -1;
		internals->txq[i].fd = -1;
	}
}

static void
tap_rx_queue_release(void *queue)
{
	struct rx_queue *rxq = queue;

	if (rxq && (rxq->fd > 0)) {
		close(rxq->fd);
		rxq->fd = -1;
	}
}

static void
tap_tx_queue_release(void *queue)
{
	struct tx_queue *txq = queue;

	if (txq && (txq->fd > 0)) {
		close(txq->fd);
		txq->fd = -1;
	}
}

static int
tap_link_update(struct rte_eth_dev *dev __rte_unused,
		int wait_to_complete __rte_unused)
{
	return 0;
}

static void
tap_promisc_enable(struct rte_eth_dev *dev)
{
	struct pmd_internals *pmd = dev->data->dev_private;
	struct ifreq ifr = { .ifr_flags = IFF_PROMISC };

	dev->data->promiscuous = 1;
	tap_ioctl(pmd, SIOCSIFFLAGS, &ifr, 1);
}

static void
tap_promisc_disable(struct rte_eth_dev *dev)
{
	struct pmd_internals *pmd = dev->data->dev_private;
	struct ifreq ifr = { .ifr_flags = IFF_PROMISC };

	dev->data->promiscuous = 0;
	tap_ioctl(pmd, SIOCSIFFLAGS, &ifr, 0);
}

static void
tap_allmulti_enable(struct rte_eth_dev *dev)
{
	struct pmd_internals *pmd = dev->data->dev_private;
	struct ifreq ifr = { .ifr_flags = IFF_ALLMULTI };

	dev->data->all_multicast = 1;
	tap_ioctl(pmd, SIOCSIFFLAGS, &ifr, 1);
}

static void
tap_allmulti_disable(struct rte_eth_dev *dev)
{
	struct pmd_internals *pmd = dev->data->dev_private;
	struct ifreq ifr = { .ifr_flags = IFF_ALLMULTI };

	dev->data->all_multicast = 0;
	tap_ioctl(pmd, SIOCSIFFLAGS, &ifr, 0);
}


static void
tap_mac_set(struct rte_eth_dev *dev, struct ether_addr *mac_addr)
{
	struct pmd_internals *pmd = dev->data->dev_private;
	struct ifreq ifr;

	if (is_zero_ether_addr(mac_addr)) {
		RTE_LOG(ERR, PMD, "%s: can't set an empty MAC address\n",
			dev->data->name);
		return;
	}

	ifr.ifr_hwaddr.sa_family = AF_LOCAL;
	rte_memcpy(ifr.ifr_hwaddr.sa_data, mac_addr, ETHER_ADDR_LEN);
	if (tap_ioctl(pmd, SIOCSIFHWADDR, &ifr, 1) < 0)
		return;
	rte_memcpy(&pmd->eth_addr, mac_addr, ETHER_ADDR_LEN);
}

static int
tap_setup_queue(struct rte_eth_dev *dev,
		struct pmd_internals *internals,
		uint16_t qid)
{
	struct pmd_internals *pmd = dev->data->dev_private;
	struct rx_queue *rx = &internals->rxq[qid];
	struct tx_queue *tx = &internals->txq[qid];
	int fd;

	fd = rx->fd;
	if (fd < 0) {
		fd = tx->fd;
		if (fd < 0) {
			RTE_LOG(INFO, PMD, "Add queue to TAP %s for qid %d\n",
				pmd->name, qid);
			fd = tun_alloc(pmd, qid);
			if (fd < 0) {
				RTE_LOG(ERR, PMD, "tun_alloc(%s, %d) failed\n",
					pmd->name, qid);
				return -1;
			}
			if (qid == 0) {
				struct ifreq ifr;

				ifr.ifr_mtu = dev->data->mtu;
				if (tap_ioctl(pmd, SIOCSIFMTU, &ifr, 1) < 0) {
					close(fd);
					return -1;
				}
			}
		}
	}

	rx->fd = fd;
	tx->fd = fd;

	return fd;
}

static int
rx_setup_queue(struct rte_eth_dev *dev,
		struct pmd_internals *internals,
		uint16_t qid)
{
	dev->data->rx_queues[qid] = &internals->rxq[qid];

	return tap_setup_queue(dev, internals, qid);
}

static int
tx_setup_queue(struct rte_eth_dev *dev,
		struct pmd_internals *internals,
		uint16_t qid)
{
	dev->data->tx_queues[qid] = &internals->txq[qid];

	return tap_setup_queue(dev, internals, qid);
}

static int
tap_rx_queue_setup(struct rte_eth_dev *dev,
		   uint16_t rx_queue_id,
		   uint16_t nb_rx_desc __rte_unused,
		   unsigned int socket_id __rte_unused,
		   const struct rte_eth_rxconf *rx_conf __rte_unused,
		   struct rte_mempool *mp)
{
	struct pmd_internals *internals = dev->data->dev_private;
	uint16_t buf_size;
	int fd;

	if ((rx_queue_id >= internals->nb_queues) || !mp) {
		RTE_LOG(WARNING, PMD,
			"nb_queues %d too small or mempool NULL\n",
			internals->nb_queues);
		return -1;
	}

	internals->rxq[rx_queue_id].mp = mp;
	internals->rxq[rx_queue_id].trigger_seen = 1; /* force initial burst */
	internals->rxq[rx_queue_id].in_port = dev->data->port_id;

	/* Now get the space available for data in the mbuf */
	buf_size = (uint16_t)(rte_pktmbuf_data_room_size(mp) -
				RTE_PKTMBUF_HEADROOM);

	if (buf_size < ETH_FRAME_LEN) {
		RTE_LOG(WARNING, PMD,
			"%s: %d bytes will not fit in mbuf (%d bytes)\n",
			dev->data->name, ETH_FRAME_LEN, buf_size);
		return -ENOMEM;
	}

	fd = rx_setup_queue(dev, internals, rx_queue_id);
	if (fd == -1)
		return -1;

	RTE_LOG(DEBUG, PMD, "  RX TAP device name %s, qid %d on fd %d\n",
		internals->name, rx_queue_id, internals->rxq[rx_queue_id].fd);

	return 0;
}

static int
tap_tx_queue_setup(struct rte_eth_dev *dev,
		   uint16_t tx_queue_id,
		   uint16_t nb_tx_desc __rte_unused,
		   unsigned int socket_id __rte_unused,
		   const struct rte_eth_txconf *tx_conf __rte_unused)
{
	struct pmd_internals *internals = dev->data->dev_private;
	int ret;

	if (tx_queue_id >= internals->nb_queues)
		return -1;

	ret = tx_setup_queue(dev, internals, tx_queue_id);
	if (ret == -1)
		return -1;

	RTE_LOG(DEBUG, PMD, "  TX TAP device name %s, qid %d on fd %d\n",
		internals->name, tx_queue_id, internals->txq[tx_queue_id].fd);

	return 0;
}

static int
tap_mtu_set(struct rte_eth_dev *dev, uint16_t mtu)
{
	struct pmd_internals *pmd = dev->data->dev_private;
	struct ifreq ifr = { .ifr_mtu = mtu };
	int err = 0;

	err = tap_ioctl(pmd, SIOCSIFMTU, &ifr, 1);
	if (!err)
		dev->data->mtu = mtu;

	return err;
}

static int
tap_set_mc_addr_list(struct rte_eth_dev *dev __rte_unused,
		     struct ether_addr *mc_addr_set __rte_unused,
		     uint32_t nb_mc_addr __rte_unused)
{
	/*
	 * Nothing to do actually: the tap has no filtering whatsoever, every
	 * packet is received.
	 */
	return 0;
}

static const uint32_t*
tap_dev_supported_ptypes_get(struct rte_eth_dev *dev __rte_unused)
{
	static const uint32_t ptypes[] = {
		RTE_PTYPE_INNER_L2_ETHER,
		RTE_PTYPE_INNER_L2_ETHER_VLAN,
		RTE_PTYPE_INNER_L2_ETHER_QINQ,
		RTE_PTYPE_INNER_L3_IPV4,
		RTE_PTYPE_INNER_L3_IPV4_EXT,
		RTE_PTYPE_INNER_L3_IPV6,
		RTE_PTYPE_INNER_L3_IPV6_EXT,
		RTE_PTYPE_INNER_L4_FRAG,
		RTE_PTYPE_INNER_L4_UDP,
		RTE_PTYPE_INNER_L4_TCP,
		RTE_PTYPE_INNER_L4_SCTP,
		RTE_PTYPE_L2_ETHER,
		RTE_PTYPE_L2_ETHER_VLAN,
		RTE_PTYPE_L2_ETHER_QINQ,
		RTE_PTYPE_L3_IPV4,
		RTE_PTYPE_L3_IPV4_EXT,
		RTE_PTYPE_L3_IPV6_EXT,
		RTE_PTYPE_L3_IPV6,
		RTE_PTYPE_L4_FRAG,
		RTE_PTYPE_L4_UDP,
		RTE_PTYPE_L4_TCP,
		RTE_PTYPE_L4_SCTP,
	};

	return ptypes;
}

static int
tap_flow_ctrl_get(struct rte_eth_dev *dev __rte_unused,
		  struct rte_eth_fc_conf *fc_conf)
{
	fc_conf->mode = RTE_FC_NONE;
	return 0;
}

static int
tap_flow_ctrl_set(struct rte_eth_dev *dev __rte_unused,
		  struct rte_eth_fc_conf *fc_conf)
{
	if (fc_conf->mode != RTE_FC_NONE)
		return -ENOTSUP;
	return 0;
}

static const struct eth_dev_ops ops = {
	.dev_start              = tap_dev_start,
	.dev_stop               = tap_dev_stop,
	.dev_close              = tap_dev_close,
	.dev_configure          = tap_dev_configure,
	.dev_infos_get          = tap_dev_info,
	.rx_queue_setup         = tap_rx_queue_setup,
	.tx_queue_setup         = tap_tx_queue_setup,
	.rx_queue_release       = tap_rx_queue_release,
	.tx_queue_release       = tap_tx_queue_release,
	.flow_ctrl_get          = tap_flow_ctrl_get,
	.flow_ctrl_set          = tap_flow_ctrl_set,
	.link_update            = tap_link_update,
	.dev_set_link_up        = tap_link_set_up,
	.dev_set_link_down      = tap_link_set_down,
	.promiscuous_enable     = tap_promisc_enable,
	.promiscuous_disable    = tap_promisc_disable,
	.allmulticast_enable    = tap_allmulti_enable,
	.allmulticast_disable   = tap_allmulti_disable,
	.mac_addr_set           = tap_mac_set,
	.mtu_set                = tap_mtu_set,
	.set_mc_addr_list       = tap_set_mc_addr_list,
	.stats_get              = tap_stats_get,
	.stats_reset            = tap_stats_reset,
	.dev_supported_ptypes_get = tap_dev_supported_ptypes_get,
	.filter_ctrl            = tap_dev_filter_ctrl,
};

static int
eth_dev_tap_create(const char *name, char *tap_name)
{
	int numa_node = rte_socket_id();
	struct rte_eth_dev *dev = NULL;
	struct pmd_internals *pmd = NULL;
	struct rte_eth_dev_data *data = NULL;
	int i;

	RTE_LOG(DEBUG, PMD, "  TAP device on numa %u\n", rte_socket_id());

	data = rte_zmalloc_socket(tap_name, sizeof(*data), 0, numa_node);
	if (!data) {
		RTE_LOG(ERR, PMD, "TAP Failed to allocate data\n");
		goto error_exit;
	}

	pmd = rte_zmalloc_socket(tap_name, sizeof(*pmd), 0, numa_node);
	if (!pmd) {
		RTE_LOG(ERR, PMD, "TAP Unable to allocate internal struct\n");
		goto error_exit;
	}

	/* name in allocation and data->name must be consistent */
	snprintf(data->name, sizeof(data->name), "%s", name);
	dev = rte_eth_dev_allocate(name);
	if (!dev) {
		RTE_LOG(ERR, PMD, "TAP Unable to allocate device struct\n");
		goto error_exit;
	}

	snprintf(pmd->name, sizeof(pmd->name), "%s", tap_name);

	pmd->nb_queues = RTE_PMD_TAP_MAX_QUEUES;

	pmd->ioctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (pmd->ioctl_sock == -1) {
		RTE_LOG(ERR, PMD,
			"TAP Unable to get a socket for management: %s\n",
			strerror(errno));
		goto error_exit;
	}

	/* Setup some default values */
	data->dev_private = pmd;
	data->port_id = dev->data->port_id;
	data->mtu = dev->data->mtu;
	data->dev_flags = RTE_ETH_DEV_DETACHABLE;
	data->kdrv = RTE_KDRV_NONE;
	data->drv_name = pmd_tap_drv.driver.name;
	data->numa_node = numa_node;

	data->dev_link = pmd_link;
	data->mac_addrs = &pmd->eth_addr;
	data->nb_rx_queues = pmd->nb_queues;
	data->nb_tx_queues = pmd->nb_queues;

	dev->data = data;
	dev->dev_ops = &ops;
	dev->driver = NULL;
	dev->rx_pkt_burst = pmd_rx_burst;
	dev->tx_pkt_burst = pmd_tx_burst;

	/* Presetup the fds to -1 as being not valid */
	for (i = 0; i < RTE_PMD_TAP_MAX_QUEUES; i++) {
		pmd->rxq[i].fd = -1;
		pmd->txq[i].fd = -1;
	}

	LIST_INIT(&pmd->flows);

	return 0;

error_exit:
	RTE_LOG(DEBUG, PMD, "TAP Unable to initialize %s\n", name);

	rte_free(data);
	rte_free(pmd);

	rte_eth_dev_release_port(dev);

	return -EINVAL;
}

static int
set_interface_name(const char *key __rte_unused,
		   const char *value,
		   void *extra_args)
{
	char *name = (char *)extra_args;

	if (value)
		snprintf(name, RTE_ETH_NAME_MAX_LEN - 1, "%s", value);
	else
		snprintf(name, RTE_ETH_NAME_MAX_LEN - 1, "%s%d",
			 DEFAULT_TAP_NAME, (tap_unit - 1));

	return 0;
}

static int
set_interface_speed(const char *key __rte_unused,
		    const char *value,
		    void *extra_args)
{
	*(int *)extra_args = (value) ? atoi(value) : ETH_SPEED_NUM_10G;

	return 0;
}

/* Open a TAP interface device.
 */
static int
rte_pmd_tap_probe(const char *name, const char *params)
{
	int ret;
	struct rte_kvargs *kvlist = NULL;
	int speed;
	char tap_name[RTE_ETH_NAME_MAX_LEN];

	speed = ETH_SPEED_NUM_10G;
	snprintf(tap_name, sizeof(tap_name), "%s%d",
		 DEFAULT_TAP_NAME, tap_unit++);

	if (params && (params[0] != '\0')) {
		RTE_LOG(DEBUG, PMD, "paramaters (%s)\n", params);

		kvlist = rte_kvargs_parse(params, valid_arguments);
		if (kvlist) {
			if (rte_kvargs_count(kvlist, ETH_TAP_SPEED_ARG) == 1) {
				ret = rte_kvargs_process(kvlist,
							 ETH_TAP_SPEED_ARG,
							 &set_interface_speed,
							 &speed);
				if (ret == -1)
					goto leave;
			}

			if (rte_kvargs_count(kvlist, ETH_TAP_IFACE_ARG) == 1) {
				ret = rte_kvargs_process(kvlist,
							 ETH_TAP_IFACE_ARG,
							 &set_interface_name,
							 tap_name);
				if (ret == -1)
					goto leave;
			}
		}
	}
	pmd_link.link_speed = speed;

	RTE_LOG(NOTICE, PMD, "Initializing pmd_tap for %s as %s\n",
		name, tap_name);

	ret = eth_dev_tap_create(name, tap_name);

leave:
	if (ret == -1) {
		RTE_LOG(ERR, PMD, "Failed to create pmd for %s as %s\n",
			name, tap_name);
		tap_unit--;		/* Restore the unit number */
	}
	rte_kvargs_free(kvlist);

	return ret;
}

/* detach a TAP device.
 */
static int
rte_pmd_tap_remove(const char *name)
{
	struct rte_eth_dev *eth_dev = NULL;
	struct pmd_internals *internals;
	int i;

	RTE_LOG(DEBUG, PMD, "Closing TUN/TAP Ethernet device on numa %u\n",
		rte_socket_id());

	/* find the ethdev entry */
	eth_dev = rte_eth_dev_allocated(name);
	if (!eth_dev)
		return 0;

	internals = eth_dev->data->dev_private;
	tap_flow_flush(eth_dev, NULL);
	for (i = 0; i < internals->nb_queues; i++)
		if (internals->rxq[i].fd != -1)
			close(internals->rxq[i].fd);

	close(internals->ioctl_sock);
	rte_free(eth_dev->data->dev_private);
	rte_free(eth_dev->data);

	rte_eth_dev_release_port(eth_dev);

	return 0;
}

static struct rte_vdev_driver pmd_tap_drv = {
	.probe = rte_pmd_tap_probe,
	.remove = rte_pmd_tap_remove,
};
RTE_PMD_REGISTER_VDEV(net_tap, pmd_tap_drv);
RTE_PMD_REGISTER_ALIAS(net_tap, eth_tap);
RTE_PMD_REGISTER_PARAM_STRING(net_tap, "iface=<string>,speed=N");
