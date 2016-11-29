/*
 * Copyright (c) 2009-2016 Solarflare Communications Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the FreeBSD Project.
 */

#include "efx.h"
#include "efx_impl.h"

#if EFSYS_OPT_SIENA

static			void
siena_phy_decode_cap(
	__in		uint32_t mcdi_cap,
	__out		uint32_t *maskp)
{
	uint32_t mask;

	mask = 0;
	if (mcdi_cap & (1 << MC_CMD_PHY_CAP_10HDX_LBN))
		mask |= (1 << EFX_PHY_CAP_10HDX);
	if (mcdi_cap & (1 << MC_CMD_PHY_CAP_10FDX_LBN))
		mask |= (1 << EFX_PHY_CAP_10FDX);
	if (mcdi_cap & (1 << MC_CMD_PHY_CAP_100HDX_LBN))
		mask |= (1 << EFX_PHY_CAP_100HDX);
	if (mcdi_cap & (1 << MC_CMD_PHY_CAP_100FDX_LBN))
		mask |= (1 << EFX_PHY_CAP_100FDX);
	if (mcdi_cap & (1 << MC_CMD_PHY_CAP_1000HDX_LBN))
		mask |= (1 << EFX_PHY_CAP_1000HDX);
	if (mcdi_cap & (1 << MC_CMD_PHY_CAP_1000FDX_LBN))
		mask |= (1 << EFX_PHY_CAP_1000FDX);
	if (mcdi_cap & (1 << MC_CMD_PHY_CAP_10000FDX_LBN))
		mask |= (1 << EFX_PHY_CAP_10000FDX);
	if (mcdi_cap & (1 << MC_CMD_PHY_CAP_PAUSE_LBN))
		mask |= (1 << EFX_PHY_CAP_PAUSE);
	if (mcdi_cap & (1 << MC_CMD_PHY_CAP_ASYM_LBN))
		mask |= (1 << EFX_PHY_CAP_ASYM);
	if (mcdi_cap & (1 << MC_CMD_PHY_CAP_AN_LBN))
		mask |= (1 << EFX_PHY_CAP_AN);

	*maskp = mask;
}

static			void
siena_phy_decode_link_mode(
	__in		efx_nic_t *enp,
	__in		uint32_t link_flags,
	__in		unsigned int speed,
	__in		unsigned int fcntl,
	__out		efx_link_mode_t *link_modep,
	__out		unsigned int *fcntlp)
{
	boolean_t fd = !!(link_flags &
		    (1 << MC_CMD_GET_LINK_OUT_FULL_DUPLEX_LBN));
	boolean_t up = !!(link_flags &
		    (1 << MC_CMD_GET_LINK_OUT_LINK_UP_LBN));

	_NOTE(ARGUNUSED(enp))

	if (!up)
		*link_modep = EFX_LINK_DOWN;
	else if (speed == 10000 && fd)
		*link_modep = EFX_LINK_10000FDX;
	else if (speed == 1000)
		*link_modep = fd ? EFX_LINK_1000FDX : EFX_LINK_1000HDX;
	else if (speed == 100)
		*link_modep = fd ? EFX_LINK_100FDX : EFX_LINK_100HDX;
	else if (speed == 10)
		*link_modep = fd ? EFX_LINK_10FDX : EFX_LINK_10HDX;
	else
		*link_modep = EFX_LINK_UNKNOWN;

	if (fcntl == MC_CMD_FCNTL_OFF)
		*fcntlp = 0;
	else if (fcntl == MC_CMD_FCNTL_RESPOND)
		*fcntlp = EFX_FCNTL_RESPOND;
	else if (fcntl == MC_CMD_FCNTL_BIDIR)
		*fcntlp = EFX_FCNTL_RESPOND | EFX_FCNTL_GENERATE;
	else {
		EFSYS_PROBE1(mc_pcol_error, int, fcntl);
		*fcntlp = 0;
	}
}

			void
siena_phy_link_ev(
	__in		efx_nic_t *enp,
	__in		efx_qword_t *eqp,
	__out		efx_link_mode_t *link_modep)
{
	efx_port_t *epp = &(enp->en_port);
	unsigned int link_flags;
	unsigned int speed;
	unsigned int fcntl;
	efx_link_mode_t link_mode;
	uint32_t lp_cap_mask;

	/*
	 * Convert the LINKCHANGE speed enumeration into mbit/s, in the
	 * same way as GET_LINK encodes the speed
	 */
	switch (MCDI_EV_FIELD(eqp, LINKCHANGE_SPEED)) {
	case MCDI_EVENT_LINKCHANGE_SPEED_100M:
		speed = 100;
		break;
	case MCDI_EVENT_LINKCHANGE_SPEED_1G:
		speed = 1000;
		break;
	case MCDI_EVENT_LINKCHANGE_SPEED_10G:
		speed = 10000;
		break;
	default:
		speed = 0;
		break;
	}

	link_flags = MCDI_EV_FIELD(eqp, LINKCHANGE_LINK_FLAGS);
	siena_phy_decode_link_mode(enp, link_flags, speed,
				    MCDI_EV_FIELD(eqp, LINKCHANGE_FCNTL),
				    &link_mode, &fcntl);
	siena_phy_decode_cap(MCDI_EV_FIELD(eqp, LINKCHANGE_LP_CAP),
			    &lp_cap_mask);

	/*
	 * It's safe to update ep_lp_cap_mask without the driver's port lock
	 * because presumably any concurrently running efx_port_poll() is
	 * only going to arrive at the same value.
	 *
	 * ep_fcntl has two meanings. It's either the link common fcntl
	 * (if the PHY supports AN), or it's the forced link state. If
	 * the former, it's safe to update the value for the same reason as
	 * for ep_lp_cap_mask. If the latter, then just ignore the value,
	 * because we can race with efx_mac_fcntl_set().
	 */
	epp->ep_lp_cap_mask = lp_cap_mask;
	if (epp->ep_phy_cap_mask & (1 << EFX_PHY_CAP_AN))
		epp->ep_fcntl = fcntl;

	*link_modep = link_mode;
}

	__checkReturn	efx_rc_t
siena_phy_power(
	__in		efx_nic_t *enp,
	__in		boolean_t power)
{
	efx_rc_t rc;

	if (!power)
		return (0);

	/* Check if the PHY is a zombie */
	if ((rc = siena_phy_verify(enp)) != 0)
		goto fail1;

	enp->en_reset_flags |= EFX_RESET_PHY;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn	efx_rc_t
siena_phy_get_link(
	__in		efx_nic_t *enp,
	__out		siena_link_state_t *slsp)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_GET_LINK_IN_LEN,
			    MC_CMD_GET_LINK_OUT_LEN)];
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_GET_LINK;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_GET_LINK_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_GET_LINK_OUT_LEN;

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (req.emr_out_length_used < MC_CMD_GET_LINK_OUT_LEN) {
		rc = EMSGSIZE;
		goto fail2;
	}

	siena_phy_decode_cap(MCDI_OUT_DWORD(req, GET_LINK_OUT_CAP),
			    &slsp->sls_adv_cap_mask);
	siena_phy_decode_cap(MCDI_OUT_DWORD(req, GET_LINK_OUT_LP_CAP),
			    &slsp->sls_lp_cap_mask);

	siena_phy_decode_link_mode(enp, MCDI_OUT_DWORD(req, GET_LINK_OUT_FLAGS),
			    MCDI_OUT_DWORD(req, GET_LINK_OUT_LINK_SPEED),
			    MCDI_OUT_DWORD(req, GET_LINK_OUT_FCNTL),
			    &slsp->sls_link_mode, &slsp->sls_fcntl);

	slsp->sls_mac_up = MCDI_OUT_DWORD(req, GET_LINK_OUT_MAC_FAULT) == 0;

	return (0);

fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn	efx_rc_t
siena_phy_reconfigure(
	__in		efx_nic_t *enp)
{
	efx_port_t *epp = &(enp->en_port);
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MAX(MC_CMD_SET_ID_LED_IN_LEN,
				MC_CMD_SET_ID_LED_OUT_LEN),
			    MAX(MC_CMD_SET_LINK_IN_LEN,
				MC_CMD_SET_LINK_OUT_LEN))];
	uint32_t cap_mask;
	unsigned int led_mode;
	unsigned int speed;
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_SET_LINK;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_SET_LINK_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_SET_LINK_OUT_LEN;

	cap_mask = epp->ep_adv_cap_mask;
	MCDI_IN_POPULATE_DWORD_10(req, SET_LINK_IN_CAP,
		PHY_CAP_10HDX, (cap_mask >> EFX_PHY_CAP_10HDX) & 0x1,
		PHY_CAP_10FDX, (cap_mask >> EFX_PHY_CAP_10FDX) & 0x1,
		PHY_CAP_100HDX, (cap_mask >> EFX_PHY_CAP_100HDX) & 0x1,
		PHY_CAP_100FDX, (cap_mask >> EFX_PHY_CAP_100FDX) & 0x1,
		PHY_CAP_1000HDX, (cap_mask >> EFX_PHY_CAP_1000HDX) & 0x1,
		PHY_CAP_1000FDX, (cap_mask >> EFX_PHY_CAP_1000FDX) & 0x1,
		PHY_CAP_10000FDX, (cap_mask >> EFX_PHY_CAP_10000FDX) & 0x1,
		PHY_CAP_PAUSE, (cap_mask >> EFX_PHY_CAP_PAUSE) & 0x1,
		PHY_CAP_ASYM, (cap_mask >> EFX_PHY_CAP_ASYM) & 0x1,
		PHY_CAP_AN, (cap_mask >> EFX_PHY_CAP_AN) & 0x1);

	MCDI_IN_SET_DWORD(req, SET_LINK_IN_LOOPBACK_MODE, MC_CMD_LOOPBACK_NONE);
	speed = 0;
	MCDI_IN_SET_DWORD(req, SET_LINK_IN_LOOPBACK_SPEED, speed);

#if EFSYS_OPT_PHY_FLAGS
	MCDI_IN_SET_DWORD(req, SET_LINK_IN_FLAGS, epp->ep_phy_flags);
#else
	MCDI_IN_SET_DWORD(req, SET_LINK_IN_FLAGS, 0);
#endif	/* EFSYS_OPT_PHY_FLAGS */

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	/* And set the blink mode */
	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_SET_ID_LED;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_SET_ID_LED_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_SET_ID_LED_OUT_LEN;

	MCDI_IN_SET_DWORD(req, SET_ID_LED_IN_STATE, MC_CMD_LED_DEFAULT);

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail2;
	}

	return (0);

fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn	efx_rc_t
siena_phy_verify(
	__in		efx_nic_t *enp)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_GET_PHY_STATE_IN_LEN,
			    MC_CMD_GET_PHY_STATE_OUT_LEN)];
	uint32_t state;
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_GET_PHY_STATE;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_GET_PHY_STATE_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_GET_PHY_STATE_OUT_LEN;

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (req.emr_out_length_used < MC_CMD_GET_PHY_STATE_OUT_LEN) {
		rc = EMSGSIZE;
		goto fail2;
	}

	state = MCDI_OUT_DWORD(req, GET_PHY_STATE_OUT_STATE);
	if (state != MC_CMD_PHY_STATE_OK) {
		if (state != MC_CMD_PHY_STATE_ZOMBIE)
			EFSYS_PROBE1(mc_pcol_error, int, state);
		rc = ENOTACTIVE;
		goto fail3;
	}

	return (0);

fail3:
	EFSYS_PROBE(fail3);
fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn	efx_rc_t
siena_phy_oui_get(
	__in		efx_nic_t *enp,
	__out		uint32_t *ouip)
{
	_NOTE(ARGUNUSED(enp, ouip))

	return (ENOTSUP);
}

#if EFSYS_OPT_BIST

	__checkReturn		efx_rc_t
siena_phy_bist_start(
	__in			efx_nic_t *enp,
	__in			efx_bist_type_t type)
{
	efx_rc_t rc;

	if ((rc = efx_mcdi_bist_start(enp, type)) != 0)
		goto fail1;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

static	__checkReturn		unsigned long
siena_phy_sft9001_bist_status(
	__in			uint16_t code)
{
	switch (code) {
	case MC_CMD_POLL_BIST_SFT9001_PAIR_BUSY:
		return (EFX_PHY_CABLE_STATUS_BUSY);
	case MC_CMD_POLL_BIST_SFT9001_INTER_PAIR_SHORT:
		return (EFX_PHY_CABLE_STATUS_INTERPAIRSHORT);
	case MC_CMD_POLL_BIST_SFT9001_INTRA_PAIR_SHORT:
		return (EFX_PHY_CABLE_STATUS_INTRAPAIRSHORT);
	case MC_CMD_POLL_BIST_SFT9001_PAIR_OPEN:
		return (EFX_PHY_CABLE_STATUS_OPEN);
	case MC_CMD_POLL_BIST_SFT9001_PAIR_OK:
		return (EFX_PHY_CABLE_STATUS_OK);
	default:
		return (EFX_PHY_CABLE_STATUS_INVALID);
	}
}

	__checkReturn		efx_rc_t
siena_phy_bist_poll(
	__in			efx_nic_t *enp,
	__in			efx_bist_type_t type,
	__out			efx_bist_result_t *resultp,
	__out_opt __drv_when(count > 0, __notnull)
	uint32_t *value_maskp,
	__out_ecount_opt(count)	__drv_when(count > 0, __notnull)
	unsigned long *valuesp,
	__in			size_t count)
{
	efx_nic_cfg_t *encp = &(enp->en_nic_cfg);
	uint8_t payload[MAX(MC_CMD_POLL_BIST_IN_LEN,
			    MCDI_CTL_SDU_LEN_MAX)];
	uint32_t value_mask = 0;
	efx_mcdi_req_t req;
	uint32_t result;
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_POLL_BIST;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_POLL_BIST_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MCDI_CTL_SDU_LEN_MAX;

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (req.emr_out_length_used < MC_CMD_POLL_BIST_OUT_RESULT_OFST + 4) {
		rc = EMSGSIZE;
		goto fail2;
	}

	if (count > 0)
		(void) memset(valuesp, '\0', count * sizeof (unsigned long));

	result = MCDI_OUT_DWORD(req, POLL_BIST_OUT_RESULT);

	/* Extract PHY specific results */
	if (result == MC_CMD_POLL_BIST_PASSED &&
	    encp->enc_phy_type == EFX_PHY_SFT9001B &&
	    req.emr_out_length_used >= MC_CMD_POLL_BIST_OUT_SFT9001_LEN &&
	    (type == EFX_BIST_TYPE_PHY_CABLE_SHORT ||
	    type == EFX_BIST_TYPE_PHY_CABLE_LONG)) {
		uint16_t word;

		if (count > EFX_BIST_PHY_CABLE_LENGTH_A) {
			if (valuesp != NULL)
				valuesp[EFX_BIST_PHY_CABLE_LENGTH_A] =
				    MCDI_OUT_DWORD(req,
				    POLL_BIST_OUT_SFT9001_CABLE_LENGTH_A);
			value_mask |= (1 << EFX_BIST_PHY_CABLE_LENGTH_A);
		}

		if (count > EFX_BIST_PHY_CABLE_LENGTH_B) {
			if (valuesp != NULL)
				valuesp[EFX_BIST_PHY_CABLE_LENGTH_B] =
				    MCDI_OUT_DWORD(req,
				    POLL_BIST_OUT_SFT9001_CABLE_LENGTH_B);
			value_mask |= (1 << EFX_BIST_PHY_CABLE_LENGTH_B);
		}

		if (count > EFX_BIST_PHY_CABLE_LENGTH_C) {
			if (valuesp != NULL)
				valuesp[EFX_BIST_PHY_CABLE_LENGTH_C] =
				    MCDI_OUT_DWORD(req,
				    POLL_BIST_OUT_SFT9001_CABLE_LENGTH_C);
			value_mask |= (1 << EFX_BIST_PHY_CABLE_LENGTH_C);
		}

		if (count > EFX_BIST_PHY_CABLE_LENGTH_D) {
			if (valuesp != NULL)
				valuesp[EFX_BIST_PHY_CABLE_LENGTH_D] =
				    MCDI_OUT_DWORD(req,
				    POLL_BIST_OUT_SFT9001_CABLE_LENGTH_D);
			value_mask |= (1 << EFX_BIST_PHY_CABLE_LENGTH_D);
		}

		if (count > EFX_BIST_PHY_CABLE_STATUS_A) {
			if (valuesp != NULL) {
				word = MCDI_OUT_WORD(req,
				    POLL_BIST_OUT_SFT9001_CABLE_STATUS_A);
				valuesp[EFX_BIST_PHY_CABLE_STATUS_A] =
				    siena_phy_sft9001_bist_status(word);
			}
			value_mask |= (1 << EFX_BIST_PHY_CABLE_STATUS_A);
		}

		if (count > EFX_BIST_PHY_CABLE_STATUS_B) {
			if (valuesp != NULL) {
				word = MCDI_OUT_WORD(req,
				    POLL_BIST_OUT_SFT9001_CABLE_STATUS_B);
				valuesp[EFX_BIST_PHY_CABLE_STATUS_B] =
				    siena_phy_sft9001_bist_status(word);
			}
			value_mask |= (1 << EFX_BIST_PHY_CABLE_STATUS_B);
		}

		if (count > EFX_BIST_PHY_CABLE_STATUS_C) {
			if (valuesp != NULL) {
				word = MCDI_OUT_WORD(req,
				    POLL_BIST_OUT_SFT9001_CABLE_STATUS_C);
				valuesp[EFX_BIST_PHY_CABLE_STATUS_C] =
				    siena_phy_sft9001_bist_status(word);
			}
			value_mask |= (1 << EFX_BIST_PHY_CABLE_STATUS_C);
		}

		if (count > EFX_BIST_PHY_CABLE_STATUS_D) {
			if (valuesp != NULL) {
				word = MCDI_OUT_WORD(req,
				    POLL_BIST_OUT_SFT9001_CABLE_STATUS_D);
				valuesp[EFX_BIST_PHY_CABLE_STATUS_D] =
				    siena_phy_sft9001_bist_status(word);
			}
			value_mask |= (1 << EFX_BIST_PHY_CABLE_STATUS_D);
		}

	} else if (result == MC_CMD_POLL_BIST_FAILED &&
		    encp->enc_phy_type == EFX_PHY_QLX111V &&
		    req.emr_out_length >= MC_CMD_POLL_BIST_OUT_MRSFP_LEN &&
		    count > EFX_BIST_FAULT_CODE) {
		if (valuesp != NULL)
			valuesp[EFX_BIST_FAULT_CODE] =
			    MCDI_OUT_DWORD(req, POLL_BIST_OUT_MRSFP_TEST);
		value_mask |= 1 << EFX_BIST_FAULT_CODE;
	}

	if (value_maskp != NULL)
		*value_maskp = value_mask;

	EFSYS_ASSERT(resultp != NULL);
	if (result == MC_CMD_POLL_BIST_RUNNING)
		*resultp = EFX_BIST_RESULT_RUNNING;
	else if (result == MC_CMD_POLL_BIST_PASSED)
		*resultp = EFX_BIST_RESULT_PASSED;
	else
		*resultp = EFX_BIST_RESULT_FAILED;

	return (0);

fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

			void
siena_phy_bist_stop(
	__in		efx_nic_t *enp,
	__in		efx_bist_type_t type)
{
	/* There is no way to stop BIST on Siena */
	_NOTE(ARGUNUSED(enp, type))
}

#endif	/* EFSYS_OPT_BIST */

#endif	/* EFSYS_OPT_SIENA */
