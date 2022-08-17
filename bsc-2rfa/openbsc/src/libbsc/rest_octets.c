/* GSM Mobile Radio Interface Layer 3 messages on the A-bis interface,
 * rest octet handling according to
 * 3GPP TS 04.08 version 7.21.0 Release 1998 / ETSI TS 100 940 V7.21.0 */

/* (C) 2009 by Harald Welte <laforge@gnumonks.org>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#include <openbsc/debug.h>
#include <openbsc/gsm_data.h>
#include <osmocom/core/bitvec.h>
#include <osmocom/gsm/bitvec_gsm.h>
#include <openbsc/rest_octets.h>
#include <openbsc/arfcn_range_encode.h>
#include <openbsc/system_information.h>

/* generate SI1 rest octets */
int rest_octets_si1(uint8_t *data, uint8_t *nch_pos, int is1800_net)
{
	struct bitvec bv;

	memset(&bv, 0, sizeof(bv));
	bv.data = data;
	bv.data_len = 1;

	if (nch_pos) {
		bitvec_set_bit(&bv, H);
		bitvec_set_uint(&bv, *nch_pos, 5);
	} else
		bitvec_set_bit(&bv, L);

	if (is1800_net)
		bitvec_set_bit(&bv, L);
	else
		bitvec_set_bit(&bv, H);

	bitvec_spare_padding(&bv, 6);
	return bv.data_len;
}

/* Append Repeated E-UTRAN Neighbour Cell to bitvec: see 3GPP TS 44.018 Table 10.5.2.33b.1 */
static inline void append_eutran_neib_cell(struct bitvec *bv, struct gsm_bts *bts, uint8_t budget)
{
	const struct osmo_earfcn_si2q *e = &bts->si_common.si2quater_neigh_list;
	unsigned i, skip = 0;
	size_t offset = bts->e_offset;
	uint8_t rem = budget - 6, earfcn_budget; /* account for mandatory stop bit and THRESH_E-UTRAN_high */

	if (budget <= 6)
		return;

	OSMO_ASSERT(budget <= SI2Q_MAX_LEN);

	/* first we have to properly adjust budget requirements */
	if (e->prio_valid) /* E-UTRAN_PRIORITY: 3GPP TS 45.008*/
		rem -= 4;
	else
		rem--;

	if (e->thresh_lo_valid) /* THRESH_E-UTRAN_low: */
		rem -= 6;
	else
		rem--;

	if (e->qrxlm_valid) /* E-UTRAN_QRXLEVMIN: */
		rem -= 6;
	else
		rem--;

	/* now we can proceed with actually adding EARFCNs within adjusted budget limit */
	for (i = 0; i < e->length; i++) {
		if (e->arfcn[i] != OSMO_EARFCN_INVALID) {
			if (skip < offset) {
				skip++; /* ignore EARFCNs added on previous calls */
			} else {
				earfcn_budget = 17; /* compute budget per-EARFCN */
				if (OSMO_EARFCN_MEAS_INVALID == e->meas_bw[i])
					earfcn_budget++;
				else
					earfcn_budget += 4;

				if (rem - earfcn_budget < 0)
					break;
				else {
					bts->e_offset++;
					rem -= earfcn_budget;
					bitvec_set_bit(bv, 1); /* EARFCN: */
					bitvec_set_uint(bv, e->arfcn[i], 16);

					if (OSMO_EARFCN_MEAS_INVALID == e->meas_bw[i])
						bitvec_set_bit(bv, 0);
					else { /* Measurement Bandwidth: 9.1.54 */
						bitvec_set_bit(bv, 1);
						bitvec_set_uint(bv, e->meas_bw[i], 3);
					}
				}
			}
		}
	}

	/* stop bit - end of EARFCN + Measurement Bandwidth sequence */
	bitvec_set_bit(bv, 0);

	/* Note: we don't support different EARFCN arrays each with different priority, threshold etc. */

	if (e->prio_valid) {
		/* E-UTRAN_PRIORITY: 3GPP TS 45.008*/
		bitvec_set_bit(bv, 1);
		bitvec_set_uint(bv, e->prio, 3);
	} else
		bitvec_set_bit(bv, 0);

	/* THRESH_E-UTRAN_high */
	bitvec_set_uint(bv, e->thresh_hi, 5);

	if (e->thresh_lo_valid) {
		/* THRESH_E-UTRAN_low: */
		bitvec_set_bit(bv, 1);
		bitvec_set_uint(bv, e->thresh_lo, 5);
	} else
		bitvec_set_bit(bv, 0);

	if (e->qrxlm_valid) {
		/* E-UTRAN_QRXLEVMIN: */
		bitvec_set_bit(bv, 1);
		bitvec_set_uint(bv, e->qrxlm, 5);
	} else
		bitvec_set_bit(bv, 0);
}

static inline void append_earfcn(struct bitvec *bv, struct gsm_bts *bts, uint8_t budget)
{
	int rem = budget - 25;
	if (rem <= 0)
		return;

	OSMO_ASSERT(budget <= SI2Q_MAX_LEN);

	/* Additions in Rel-5: */
	bitvec_set_bit(bv, H);
	/* No 3G Additional Measurement Param. Descr. */
	bitvec_set_bit(bv, 0);
	/* No 3G ADDITIONAL MEASUREMENT Param. Descr. 2 */
	bitvec_set_bit(bv, 0);
	/* Additions in Rel-6: */
	bitvec_set_bit(bv, H);
	/* 3G_CCN_ACTIVE */
	bitvec_set_bit(bv, 0);
	/* Additions in Rel-7: */
	bitvec_set_bit(bv, H);
	/* No 700_REPORTING_OFFSET */
	bitvec_set_bit(bv, 0);
	/* No 810_REPORTING_OFFSET */
	bitvec_set_bit(bv, 0);
	/* Additions in Rel-8: */
	bitvec_set_bit(bv, H);

	/* Priority and E-UTRAN Parameters Description */
	bitvec_set_bit(bv, 1);

	/* No Serving Cell Priority Parameters Descr. */
	bitvec_set_bit(bv, 0);
	/* No 3G Priority Parameters Description */
	bitvec_set_bit(bv, 0);
	/* E-UTRAN Parameters Description */
	bitvec_set_bit(bv, 1);

	/* E-UTRAN_CCN_ACTIVE */
	bitvec_set_bit(bv, 0);
	/* E-UTRAN_Start: 9.1.54 */
	bitvec_set_bit(bv, 1);
	/* E-UTRAN_Stop: 9.1.54 */
	bitvec_set_bit(bv, 1);

	/* No E-UTRAN Measurement Parameters Descr. */
	bitvec_set_bit(bv, 0);
	/* No GPRS E-UTRAN Measurement Param. Descr. */
	bitvec_set_bit(bv, 0);

	/* Note: each of next 3 "repeated" structures might be repeated any
	   (0, 1, 2...) times - we only support 1 and 0 */

	/* Repeated E-UTRAN Neighbour Cells */
	bitvec_set_bit(bv, 1);

	/* N. B: 25 bits are set in append_earfcn() - keep it in sync with budget adjustment below: */
	append_eutran_neib_cell(bv, bts, rem);

	/* stop bit - end of Repeated E-UTRAN Neighbour Cells sequence: */
	bitvec_set_bit(bv, 0);

	/* Note: following 2 repeated structs are not supported ATM */
	/* stop bit - end of Repeated E-UTRAN Not Allowed Cells sequence: */
	bitvec_set_bit(bv, 0);
	/* stop bit - end of Repeated E-UTRAN PCID to TA mapping sequence: */
	bitvec_set_bit(bv, 0);

	/* Priority and E-UTRAN Parameters Description ends here */
	/* No 3G CSG Description */
	bitvec_set_bit(bv, 0);
	/* No E-UTRAN CSG Description */
	bitvec_set_bit(bv, 0);
	/* No Additions in Rel-9: */
	bitvec_set_bit(bv, L);
}

static inline int f0_helper(int *sc, size_t length, uint8_t *chan_list)
{
	int w[RANGE_ENC_MAX_ARFCNS] = { 0 };

	return range_encode(ARFCN_RANGE_1024, sc, length, w, 0, chan_list);
}

/* Estimate how many bits it'll take to append single FDD UARFCN */
static inline int append_utran_fdd_length(uint16_t u, int *sc, size_t sc_len, size_t length)
{
	uint8_t chan_list[16] = { 0 };
	int tmp[sc_len], f0;

	memcpy(tmp, sc, sizeof(tmp));

	f0 = f0_helper(tmp, length, chan_list);
	if (f0 < 0)
		return f0;

	return 21 + range1024_p(length);
}

/* Append single FDD UARFCN */
static inline int append_utran_fdd(struct bitvec *bv, uint16_t u, int *sc, size_t length)
{
	uint8_t chan_list[16] = { 0 };
	int f0 = f0_helper(sc, length, chan_list);

	if (f0 < 0)
		return f0;

	/* Repeated UTRAN FDD Neighbour Cells */
	bitvec_set_bit(bv, 1);

	/* FDD-ARFCN */
	bitvec_set_bit(bv, 0);
	bitvec_set_uint(bv, u, 14);

	/* FDD_Indic0: parameter value '0000000000' is a member of the set? */
	bitvec_set_bit(bv, f0);
	/* NR_OF_FDD_CELLS */
	bitvec_set_uint(bv, length, 5);

	f0 = bv->cur_bit;
	bitvec_add_range1024(bv, (struct gsm48_range_1024 *)chan_list);
	bv->cur_bit = f0 + range1024_p(length);

	return 21 + range1024_p(length);
}

/* Append multiple FDD UARFCNs */
static inline int append_uarfcns(struct bitvec *bv, struct gsm_bts *bts, uint8_t budget)
{
	const uint16_t *u = bts->si_common.data.uarfcn_list, *sc = bts->si_common.data.scramble_list;
	int i, j, k, rc, st = 0, a[bts->si_common.uarfcn_length];
	uint16_t cu = u[bts->u_offset]; /* caller ensures that length is positive */
	uint8_t rem = budget - 7, offset_diff; /* account for constant bits right away */

	OSMO_ASSERT(budget <= SI2Q_MAX_LEN);

	if (budget <= 7)
		return -ENOMEM;

	/* 3G Neighbour Cell Description */
	bitvec_set_bit(bv, 1);
	/* No Index_Start_3G */
	bitvec_set_bit(bv, 0);
	/* No Absolute_Index_Start_EMR */
	bitvec_set_bit(bv, 0);

	/* UTRAN FDD Description */
	bitvec_set_bit(bv, 1);
	/* No Bandwidth_FDD */
	bitvec_set_bit(bv, 0);

	for (i = bts->u_offset; i < bts->si_common.uarfcn_length; i++) {
		offset_diff = 0;
		for (j = st, k = 0; j < i; j++) {
			a[k++] = sc[j]; /* copy corresponding SCs */
			offset_diff++; /* compute proper offset step */
		}
		if (u[i] != cu) { /* we've reached new UARFCN */
			rc = append_utran_fdd_length(cu, a, bts->si_common.uarfcn_length, k);
			if (rc < 0) { /* estimate bit length requirements */
				return rc;
			}

			if (rem - rc <= 0)
				break; /* we have ran out of budget in current SI2q */
			else {
				rem -= append_utran_fdd(bv, cu, a, k);
				bts->u_offset += offset_diff;
			}
			cu = u[i];
			st = i; /* update start position */
		}
	}

	if (rem > 22) {	/* add last UARFCN not covered by previous cycle if it could possibly fit into budget */
		offset_diff = 0;
		for (i = st, k = 0; i < bts->si_common.uarfcn_length; i++) {
			a[k++] = sc[i];
			offset_diff++;
		}
		rc = append_utran_fdd_length(cu, a, bts->si_common.uarfcn_length, k);
		if (rc < 0) {
			return rc;
		}

		if (rem - rc >= 0) {
			rem -= append_utran_fdd(bv, cu, a, k);
			bts->u_offset += offset_diff;
		}
	}

	/* stop bit - end of Repeated UTRAN FDD Neighbour Cells */
	bitvec_set_bit(bv, 0);

	/* UTRAN TDD Description */
	bitvec_set_bit(bv, 0);

	return 0;
}

/* generate SI2quater rest octets: 3GPP TS 44.018 § 10.5.2.33b */
int rest_octets_si2quater(uint8_t *data, struct gsm_bts *bts)
{
	int rc;
	struct bitvec bv;

	if (bts->si2q_count < bts->si2q_index)
		return -EINVAL;

	bv.data = data;
	bv.data_len = 20;
	bitvec_zero(&bv);

	/* BA_IND: Set to '0' as that's what we use for SI2xxx type,
	 * whereas '1' is used for SI5xxx type messages. The point here
	 * is to be able to correlate whether a given MS measurement
	 * report was using the neighbor cells advertised in SI2 or in
	 * SI5, as those two could very well be different */
	bitvec_set_bit(&bv, 0);
	/* 3G_BA_IND */
	bitvec_set_bit(&bv, 1);
	/* MP_CHANGE_MARK */
	bitvec_set_bit(&bv, 0);

	/* SI2quater_INDEX */
	bitvec_set_uint(&bv, bts->si2q_index, 4);
	/* SI2quater_COUNT */
	bitvec_set_uint(&bv, bts->si2q_count, 4);

	/* No Measurement_Parameters Description */
	bitvec_set_bit(&bv, 0);
	/* No GPRS_Real Time Difference Description */
	bitvec_set_bit(&bv, 0);
	/* No GPRS_BSIC Description */
	bitvec_set_bit(&bv, 0);
	/* No GPRS_REPORT PRIORITY Description */
	bitvec_set_bit(&bv, 0);
	/* No GPRS_MEASUREMENT_Parameters Description */
	bitvec_set_bit(&bv, 0);
	/* No NC Measurement Parameters */
	bitvec_set_bit(&bv, 0);
	/* No extension (length) */
	bitvec_set_bit(&bv, 0);

	rc = SI2Q_MAX_LEN - (bv.cur_bit + 3);
	if (rc > 0 && bts->si_common.uarfcn_length - bts->u_offset > 0) {
		rc = append_uarfcns(&bv, bts, rc);
		if (rc < 0) {
			LOGP(DRR, LOGL_ERROR, "SI2quater [%u/%u]: failed to append %zu UARFCNs due to range encoding "
			     "failure: %s\n",
			     bts->si2q_index, bts->si2q_count, bts->si_common.uarfcn_length, strerror(-rc));
			return rc;
		}
	} else /* No 3G Neighbour Cell Description */
		bitvec_set_bit(&bv, 0);

	/* No 3G Measurement Parameters Description */
	bitvec_set_bit(&bv, 0);
	/* No GPRS_3G_MEASUREMENT Parameters Descr. */
	bitvec_set_bit(&bv, 0);

	rc = SI2Q_MAX_LEN - bv.cur_bit;
	if (rc > 0 && si2q_earfcn_count(&bts->si_common.si2quater_neigh_list) - bts->e_offset > 0)
		append_earfcn(&bv, bts, rc);
	else /* No Additions in Rel-5: */
		bitvec_set_bit(&bv, L);

	bitvec_spare_padding(&bv, (bv.data_len * 8) - 1);
	return bv.data_len;
}

/* Append selection parameters to bitvec */
static void append_selection_params(struct bitvec *bv,
				    const struct gsm48_si_selection_params *sp)
{
	if (sp->present) {
		bitvec_set_bit(bv, H);
		bitvec_set_bit(bv, sp->cbq);
		bitvec_set_uint(bv, sp->cell_resel_off, 6);
		bitvec_set_uint(bv, sp->temp_offs, 3);
		bitvec_set_uint(bv, sp->penalty_time, 5);
	} else
		bitvec_set_bit(bv, L);
}

/* Append power offset to bitvec */
static void append_power_offset(struct bitvec *bv,
				const struct gsm48_si_power_offset *po)
{
	if (po->present) {
		bitvec_set_bit(bv, H);
		bitvec_set_uint(bv, po->power_offset, 2);
	} else
		bitvec_set_bit(bv, L);
}

/* Append GPRS indicator to bitvec */
static void append_gprs_ind(struct bitvec *bv,
			    const struct gsm48_si3_gprs_ind *gi)
{
	if (gi->present) {
		bitvec_set_bit(bv, H);
		bitvec_set_uint(bv, gi->ra_colour, 3);
		/* 0 == SI13 in BCCH Norm, 1 == SI13 sent on BCCH Ext */
		bitvec_set_bit(bv, gi->si13_position);
	} else
		bitvec_set_bit(bv, L);
}


/* Generate SI3 Rest Octests (Chapter 10.5.2.34 / Table 10.4.72) */
int rest_octets_si3(uint8_t *data, const struct gsm48_si_ro_info *si3)
{
	struct bitvec bv;

	memset(&bv, 0, sizeof(bv));
	bv.data = data;
	bv.data_len = 4;

	/* Optional Selection Parameters */
	append_selection_params(&bv, &si3->selection_params);

	/* Optional Power Offset */
	append_power_offset(&bv, &si3->power_offset);

	/* Do we have a SI2ter on the BCCH? */
	if (si3->si2ter_indicator)
		bitvec_set_bit(&bv, H);
	else
		bitvec_set_bit(&bv, L);

	/* Early Classmark Sending Control */
	if (si3->early_cm_ctrl)
		bitvec_set_bit(&bv, H);
	else
		bitvec_set_bit(&bv, L);

	/* Do we have a SI Type 9 on the BCCH? */
	if (si3->scheduling.present) {
		bitvec_set_bit(&bv, H);
		bitvec_set_uint(&bv, si3->scheduling.where, 3);
	} else
		bitvec_set_bit(&bv, L);

	/* GPRS Indicator */
	append_gprs_ind(&bv, &si3->gprs_ind);

	/* 3G Early Classmark Sending Restriction. If H, then controlled by
	 * early_cm_ctrl above */
	if (si3->early_cm_restrict_3g)
		bitvec_set_bit(&bv, L);
	else
		bitvec_set_bit(&bv, H);

	if (si3->si2quater_indicator) {
		bitvec_set_bit(&bv, H); /* indicator struct present */
		bitvec_set_uint(&bv, 0, 1); /* message is sent on BCCH Norm */
	}

	bitvec_spare_padding(&bv, (bv.data_len*8)-1);
	return bv.data_len;
}

static int append_lsa_params(struct bitvec *bv,
			     const struct gsm48_lsa_params *lsa_params)
{
	/* FIXME */
	return -1;
}

/* Generate SI4 Rest Octets (Chapter 10.5.2.35) */
int rest_octets_si4(uint8_t *data, const struct gsm48_si_ro_info *si4, int len)
{
	struct bitvec bv;

	memset(&bv, 0, sizeof(bv));
	bv.data = data;
	bv.data_len = len;

	/* SI4 Rest Octets O */
	append_selection_params(&bv, &si4->selection_params);
	append_power_offset(&bv, &si4->power_offset);
	append_gprs_ind(&bv, &si4->gprs_ind);

	if (0 /* FIXME */) {
		/* H and SI4 Rest Octets S */
		bitvec_set_bit(&bv, H);

		/* LSA Parameters */
		if (si4->lsa_params.present) {
			bitvec_set_bit(&bv, H);
			append_lsa_params(&bv, &si4->lsa_params);
		} else
			bitvec_set_bit(&bv, L);

		/* Cell Identity */
		if (1) {
			bitvec_set_bit(&bv, H);
			bitvec_set_uint(&bv, si4->cell_id, 16);
		} else
			bitvec_set_bit(&bv, L);

		/* LSA ID Information */
		if (0) {
			bitvec_set_bit(&bv, H);
			/* FIXME */
		} else
			bitvec_set_bit(&bv, L);
	} else {
		/* L and break indicator */
		bitvec_set_bit(&bv, L);
		bitvec_set_bit(&bv, si4->break_ind ? H : L);
	}

	return bv.data_len;
}


/* GSM 04.18 ETSI TS 101 503 V8.27.0 (2006-05)

<SI6 rest octets> ::=
{L | H <PCH and NCH info>}
{L | H <VBS/VGCS options : bit(2)>}
{ < DTM_support : bit == L > I < DTM_support : bit == H >
< RAC : bit (8) >
< MAX_LAPDm : bit (3) > }
< Band indicator >
{ L | H < GPRS_MS_TXPWR_MAX_CCH : bit (5) > }
<implicit spare >;
*/
int rest_octets_si6(uint8_t *data, bool is1800_net)
{
	struct bitvec bv;

	memset(&bv, 0, sizeof(bv));
	bv.data = data;
	bv.data_len = 1;

	/* no PCH/NCH info */
	bitvec_set_bit(&bv, L);
	/* no VBS/VGCS options */
	bitvec_set_bit(&bv, L);
	/* no DTM_support */
	bitvec_set_bit(&bv, L);
	/* band indicator */
	if (is1800_net)
		bitvec_set_bit(&bv, L);
	else
		bitvec_set_bit(&bv, H);
	/* no GPRS_MS_TXPWR_MAX_CCH */
	bitvec_set_bit(&bv, L);

	bitvec_spare_padding(&bv, (bv.data_len * 8) - 1);
	return bv.data_len;
}

/* GPRS Mobile Allocation as per TS 04.60 Chapter 12.10a:
   < GPRS Mobile Allocation IE > ::=
     < HSN : bit (6) >
     { 0 | 1 < RFL number list : < RFL number list struct > > }
     { 0 < MA_LENGTH : bit (6) >
         < MA_BITMAP: bit (val(MA_LENGTH) + 1) >
     | 1 { 0 | 1 <ARFCN index list : < ARFCN index list struct > > } } ;

     < RFL number list struct > :: =
       < RFL_NUMBER : bit (4) >
       { 0 | 1 < RFL number list struct > } ;
     < ARFCN index list struct > ::=
       < ARFCN_INDEX : bit(6) >
       { 0 | 1 < ARFCN index list struct > } ;
 */
static int append_gprs_mobile_alloc(struct bitvec *bv)
{
	/* Hopping Sequence Number */
	bitvec_set_uint(bv, 0, 6);

	if (0) {
		/* We want to use a RFL number list */
		bitvec_set_bit(bv, 1);
		/* FIXME: RFL number list */
	} else
		bitvec_set_bit(bv, 0);

	if (0) {
		/* We want to use a MA_BITMAP */
		bitvec_set_bit(bv, 0);
		/* FIXME: MA_LENGTH, MA_BITMAP, ... */
	} else {
		bitvec_set_bit(bv, 1);
		if (0) {
			/* We want to provide an ARFCN index list */
			bitvec_set_bit(bv, 1);
			/* FIXME */
		} else
			bitvec_set_bit(bv, 0);
	}
	return 0;
}

static int encode_t3192(unsigned int t3192)
{
	/* See also 3GPP TS 44.060
	   Table 12.24.2: GPRS Cell Options information element details */
	if (t3192 == 0)
		return 3;
	else if (t3192 <= 80)
		return 4;
	else if (t3192 <= 120)
		return 5;
	else if (t3192 <= 160)
		return 6;
	else if (t3192 <= 200)
		return 7;
	else if (t3192 <= 500)
		return 0;
	else if (t3192 <= 1000)
		return 1;
	else if (t3192 <= 1500)
		return 2;
	else
		return -EINVAL;
}

static int encode_drx_timer(unsigned int drx)
{
	if (drx == 0)
		return 0;
	else if (drx == 1)
		return 1;
	else if (drx == 2)
		return 2;
	else if (drx <= 4)
		return 3;
	else if (drx <= 8)
		return 4;
	else if (drx <= 16)
		return 5;
	else if (drx <= 32)
		return 6;
	else if (drx <= 64)
		return 7;
	else
		return -EINVAL;
}

/* GPRS Cell Options as per TS 04.60 Chapter 12.24
	< GPRS Cell Options IE > ::=
		< NMO : bit(2) >
		< T3168 : bit(3) >
		< T3192 : bit(3) >
		< DRX_TIMER_MAX: bit(3) >
		< ACCESS_BURST_TYPE: bit >
		< CONTROL_ACK_TYPE : bit >
		< BS_CV_MAX: bit(4) >
		{ 0 | 1 < PAN_DEC : bit(3) >
			< PAN_INC : bit(3) >
			< PAN_MAX : bit(3) >
		{ 0 | 1 < Extension Length : bit(6) >
			< bit (val(Extension Length) + 1
			& { < Extension Information > ! { bit ** = <no string> } } ;
	< Extension Information > ::=
		{ 0 | 1 < EGPRS_PACKET_CHANNEL_REQUEST : bit >
			< BEP_PERIOD : bit(4) > }
		< PFC_FEATURE_MODE : bit >
		< DTM_SUPPORT : bit >
		<BSS_PAGING_COORDINATION: bit >
		<spare bit > ** ;
 */
static int append_gprs_cell_opt(struct bitvec *bv,
				const struct gprs_cell_options *gco)
{
	int t3192, drx_timer_max;

	t3192 = encode_t3192(gco->t3192);
	if (t3192 < 0)
		return t3192;

	drx_timer_max = encode_drx_timer(gco->drx_timer_max);
	if (drx_timer_max < 0)
		return drx_timer_max;

	bitvec_set_uint(bv, gco->nmo, 2);

	/* See also 3GPP TS 44.060
	   Table 12.24.2: GPRS Cell Options information element details */
	bitvec_set_uint(bv, gco->t3168 / 500 - 1, 3);

	bitvec_set_uint(bv, t3192, 3);
	bitvec_set_uint(bv, drx_timer_max, 3);
	/* ACCESS_BURST_TYPE: Hard-code 8bit */
	bitvec_set_bit(bv, 0);
	/* CONTROL_ACK_TYPE: */
	bitvec_set_bit(bv, gco->ctrl_ack_type_use_block);
	bitvec_set_uint(bv, gco->bs_cv_max, 4);

	if (0) {
		/* hard-code no PAN_{DEC,INC,MAX} */
		bitvec_set_bit(bv, 0);
	} else {
		/* copied from ip.access BSC protocol trace */
		bitvec_set_bit(bv, 1);
		bitvec_set_uint(bv, 1, 3);	/* DEC */
		bitvec_set_uint(bv, 1, 3);	/* INC */
		bitvec_set_uint(bv, 15, 3);	/* MAX */
	}

	if (!gco->ext_info_present) {
		/* no extension information */
		bitvec_set_bit(bv, 0);
	} else {
		/* extension information */
		bitvec_set_bit(bv, 1);
		if (!gco->ext_info.egprs_supported) {
			/* 6bit length of extension */
			bitvec_set_uint(bv, (1 + 3)-1, 6);
			/* EGPRS supported in the cell */
			bitvec_set_bit(bv, 0);
		} else {
			/* 6bit length of extension */
			bitvec_set_uint(bv, (1 + 5 + 3)-1, 6);
			/* EGPRS supported in the cell */
			bitvec_set_bit(bv, 1);

			/* 1bit EGPRS PACKET CHANNEL REQUEST */
			if (gco->supports_egprs_11bit_rach == 0) {
				bitvec_set_bit(bv,
					gco->ext_info.use_egprs_p_ch_req);
			} else {
				bitvec_set_bit(bv, 0);
			}

			/* 4bit BEP PERIOD */
			bitvec_set_uint(bv, gco->ext_info.bep_period, 4);
		}
		bitvec_set_bit(bv, gco->ext_info.pfc_supported);
		bitvec_set_bit(bv, gco->ext_info.dtm_supported);
		bitvec_set_bit(bv, gco->ext_info.bss_paging_coordination);
	}

	return 0;
}

static void append_gprs_pwr_ctrl_pars(struct bitvec *bv,
				      const struct gprs_power_ctrl_pars *pcp)
{
	bitvec_set_uint(bv, pcp->alpha, 4);
	bitvec_set_uint(bv, pcp->t_avg_w, 5);
	bitvec_set_uint(bv, pcp->t_avg_t, 5);
	bitvec_set_uint(bv, pcp->pc_meas_chan, 1);
	bitvec_set_uint(bv, pcp->n_avg_i, 4);
}

/* Generate SI13 Rest Octests (04.08 Chapter 10.5.2.37b) */
int rest_octets_si13(uint8_t *data, const struct gsm48_si13_info *si13)
{
	struct bitvec bv;

	memset(&bv, 0, sizeof(bv));
	bv.data = data;
	bv.data_len = 20;

	if (0) {
		/* No rest octets */
		bitvec_set_bit(&bv, L);
	} else {
		bitvec_set_bit(&bv, H);
		bitvec_set_uint(&bv, si13->bcch_change_mark, 3);
		bitvec_set_uint(&bv, si13->si_change_field, 4);
		if (1) {
			bitvec_set_bit(&bv, 0);
		} else {
			bitvec_set_bit(&bv, 1);
			bitvec_set_uint(&bv, si13->bcch_change_mark, 2);
			append_gprs_mobile_alloc(&bv);
		}
		/* PBCCH not present in cell:
		   it shall never be indicated according to 3GPP TS 44.018 Table 10.5.2.37b.1 */
		bitvec_set_bit(&bv, 0);
		bitvec_set_uint(&bv, si13->rac, 8);
		bitvec_set_bit(&bv, si13->spgc_ccch_sup);
		bitvec_set_uint(&bv, si13->prio_acc_thr, 3);
		bitvec_set_uint(&bv, si13->net_ctrl_ord, 2);
		append_gprs_cell_opt(&bv, &si13->cell_opts);
		append_gprs_pwr_ctrl_pars(&bv, &si13->pwr_ctrl_pars);

		/* 3GPP TS 44.018 Release 6 / 10.5.2.37b */
		bitvec_set_bit(&bv, H);	/* added Release 99 */
		/* claim our SGSN is compatible with Release 99, as EDGE and EGPRS
		 * was only added in this Release */
		bitvec_set_bit(&bv, 1);
	}
	bitvec_spare_padding(&bv, (bv.data_len*8)-1);
	return bv.data_len;
}
