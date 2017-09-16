/*-
 * This file is provided under a dual BSD/GPLv2 license. When using or
 * redistributing this file, you may do so under either license.
 *
 *   BSD LICENSE
 *
 * Copyright 2013-2016 Freescale Semiconductor Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of the above-listed copyright holders nor the
 * names of any contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 *   GPL LICENSE SUMMARY
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __FSL_DPCI_H
#define __FSL_DPCI_H

/* Data Path Communication Interface API
 * Contains initialization APIs and runtime control APIs for DPCI
 */

struct fsl_mc_io;

/** General DPCI macros */

/**
 * Maximum number of Tx/Rx priorities per DPCI object
 */
#define DPCI_PRIO_NUM		2

/**
 * Indicates an invalid frame queue
 */
#define DPCI_FQID_NOT_VALID	(uint32_t)(-1)

/**
 * All queues considered; see dpci_set_rx_queue()
 */
#define DPCI_ALL_QUEUES		(uint8_t)(-1)

int dpci_open(struct fsl_mc_io *mc_io,
	      uint32_t cmd_flags,
	      int dpci_id,
	      uint16_t *token);

int dpci_close(struct fsl_mc_io *mc_io,
	       uint32_t cmd_flags,
	       uint16_t token);

/**
 * Enable the Order Restoration support
 */
#define DPCI_OPT_HAS_OPR					0x000040

/**
 * Order Point Records are shared for the entire DPCI
 */
#define DPCI_OPT_OPR_SHARED					0x000080

/**
 * struct dpci_cfg - Structure representing DPCI configuration
 * @options: Any combination of the following options:
 *		DPCI_OPT_HAS_OPR
 *		DPCI_OPT_OPR_SHARED
 * @num_of_priorities:	Number of receive priorities (queues) for the DPCI;
 *			note, that the number of transmit priorities (queues)
 *			is determined by the number of receive priorities of
 *			the peer DPCI object
 */
struct dpci_cfg {
	uint32_t options;
	uint8_t num_of_priorities;
};

int dpci_create(struct fsl_mc_io *mc_io,
		uint16_t dprc_token,
		uint32_t cmd_flags,
		const struct dpci_cfg *cfg,
		uint32_t *obj_id);

int dpci_destroy(struct fsl_mc_io *mc_io,
		 uint16_t dprc_token,
		 uint32_t cmd_flags,
		 uint32_t object_id);

int dpci_enable(struct fsl_mc_io *mc_io,
		uint32_t cmd_flags,
		uint16_t token);

int dpci_disable(struct fsl_mc_io *mc_io,
		 uint32_t cmd_flags,
		 uint16_t token);

int dpci_is_enabled(struct fsl_mc_io *mc_io,
		    uint32_t cmd_flags,
		    uint16_t token,
		    int *en);

int dpci_reset(struct fsl_mc_io *mc_io,
	       uint32_t cmd_flags,
	       uint16_t token);

/**
 * struct dpci_attr - Structure representing DPCI attributes
 * @id:			DPCI object ID
 * @num_of_priorities:	Number of receive priorities
 */
struct dpci_attr {
	int id;
	uint8_t num_of_priorities;
};

int dpci_get_attributes(struct fsl_mc_io *mc_io,
			uint32_t cmd_flags,
			uint16_t token,
			struct dpci_attr *attr);

/**
 * enum dpci_dest - DPCI destination types
 * @DPCI_DEST_NONE:	Unassigned destination; The queue is set in parked mode
 *			and does not generate FQDAN notifications; user is
 *			expected to dequeue from the queue based on polling or
 *			other user-defined method
 * @DPCI_DEST_DPIO:	The queue is set in schedule mode and generates FQDAN
 *			notifications to the specified DPIO; user is expected
 *			to dequeue from the queue only after notification is
 *			received
 * @DPCI_DEST_DPCON:	The queue is set in schedule mode and does not generate
 *			FQDAN notifications, but is connected to the specified
 *			DPCON object;
 *			user is expected to dequeue from the DPCON channel
 */
enum dpci_dest {
	DPCI_DEST_NONE = 0,
	DPCI_DEST_DPIO = 1,
	DPCI_DEST_DPCON = 2
};

/**
 * struct dpci_dest_cfg - Structure representing DPCI destination configuration
 * @dest_type:	Destination type
 * @dest_id:	Either DPIO ID or DPCON ID, depending on the destination type
 * @priority:	Priority selection within the DPIO or DPCON channel; valid
 *		values are 0-1 or 0-7, depending on the number of priorities
 *		in that	channel; not relevant for 'DPCI_DEST_NONE' option
 */
struct dpci_dest_cfg {
	enum dpci_dest dest_type;
	int dest_id;
	uint8_t priority;
};

/** DPCI queue modification options */

/**
 * Select to modify the user's context associated with the queue
 */
#define DPCI_QUEUE_OPT_USER_CTX		0x00000001

/**
 * Select to modify the queue's destination
 */
#define DPCI_QUEUE_OPT_DEST		0x00000002

/**
 * struct dpci_rx_queue_cfg - Structure representing RX queue configuration
 * @options:	Flags representing the suggested modifications to the queue;
 *		Use any combination of 'DPCI_QUEUE_OPT_<X>' flags
 * @user_ctx:	User context value provided in the frame descriptor of each
 *		dequeued frame;
 *		valid only if 'DPCI_QUEUE_OPT_USER_CTX' is contained in
 *		'options'
 * @dest_cfg:	Queue destination parameters;
 *		valid only if 'DPCI_QUEUE_OPT_DEST' is contained in 'options'
 */
struct dpci_rx_queue_cfg {
	uint32_t options;
	uint64_t user_ctx;
	struct dpci_dest_cfg dest_cfg;
};

int dpci_set_rx_queue(struct fsl_mc_io *mc_io,
		      uint32_t cmd_flags,
		      uint16_t token,
		      uint8_t priority,
		      const struct dpci_rx_queue_cfg *cfg);

/**
 * struct dpci_rx_queue_attr - Structure representing Rx queue attributes
 * @user_ctx:	User context value provided in the frame descriptor of each
 *		dequeued frame
 * @dest_cfg:	Queue destination configuration
 * @fqid:	Virtual FQID value to be used for dequeue operations
 */
struct dpci_rx_queue_attr {
	uint64_t user_ctx;
	struct dpci_dest_cfg dest_cfg;
	uint32_t fqid;
};

int dpci_get_rx_queue(struct fsl_mc_io *mc_io,
		      uint32_t cmd_flags,
		      uint16_t token,
		      uint8_t priority,
		      struct dpci_rx_queue_attr	*attr);

/**
 * struct dpci_tx_queue_attr - Structure representing attributes of Tx queues
 * @fqid:	Virtual FQID to be used for sending frames to peer DPCI;
 *		returns 'DPCI_FQID_NOT_VALID' if a no peer is connected or if
 *		the selected priority exceeds the number of priorities of the
 *		peer DPCI object
 */
struct dpci_tx_queue_attr {
	uint32_t fqid;
};

int dpci_get_tx_queue(struct fsl_mc_io *mc_io,
		      uint32_t cmd_flags,
		      uint16_t token,
		      uint8_t priority,
		      struct dpci_tx_queue_attr *attr);

int dpci_get_api_version(struct fsl_mc_io *mc_io,
			 uint32_t cmd_flags,
			 uint16_t *major_ver,
			 uint16_t *minor_ver);

#endif /* __FSL_DPCI_H */
