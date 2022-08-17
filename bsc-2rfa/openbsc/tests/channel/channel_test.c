/*
 * (C) 2009 by Holger Hans Peter Freyther <zecke@selfish.org>
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include <osmocom/core/application.h>
#include <osmocom/core/select.h>

#include <openbsc/common_bsc.h>
#include <openbsc/abis_rsl.h>
#include <openbsc/debug.h>
#include <openbsc/gsm_subscriber.h>

static int s_end = 0;
static struct gsm_subscriber_connection s_conn;
static void *s_data;
static gsm_cbfn *s_cbfn = NULL;

/* our handler */
static int subscr_cb(unsigned int hook, unsigned int event, struct msgb *msg, void *data, void *param)
{
	assert(hook == 101);
	assert(event == 200);
	assert(msg == (void*)0x1323L);
	assert(data == &s_conn);
	assert(param == (void*)0x2342L);
	printf("Reached, didn't crash, test passed\n");
	s_end = true;
	return 0;
}

/* override, requires '-Wl,--wrap=paging_request'.
/  mock object for testing, directly invoke the cb... maybe later through the timer */
int __real_paging_request(struct gsm_bts *bts, struct bsc_subscr *bsub, int type, gsm_cbfn *cbfn, void *data);
int __wrap_paging_request(struct gsm_bts *bts, struct bsc_subscr *bsub, int type, gsm_cbfn *cbfn, void *data)
{
	s_data = data;
	s_cbfn = cbfn;

	/* claim we have patched */
	return 1;
}


void test_request_chan(struct gsm_network *net)
{
	struct gsm_bts *bts;

	printf("Testing the gsm_subscriber chan logic\n");

	bts = gsm_bts_alloc(net, 0);
	bts->location_area_code = 23;
	s_conn.network = net;

	/* Create a dummy subscriber */
	struct gsm_subscriber *subscr = subscr_alloc();
	subscr->lac = 23;
	subscr->group = net->subscr_group;

	OSMO_ASSERT(subscr->group);
	OSMO_ASSERT(subscr->group->net == net);

	/* Ask for a channel... */
	struct subscr_request *sr;
	sr = subscr_request_channel(subscr, RSL_CHANNEED_TCH_F, subscr_cb, (void*)0x2342L);
	OSMO_ASSERT(sr);
	OSMO_ASSERT(s_cbfn);
	s_cbfn(101, 200, (void*)0x1323L, &s_conn, s_data);

	OSMO_ASSERT(s_end);
}


void test_bts_debug_print(struct gsm_network *net)
{
	struct gsm_bts *bts;
	struct gsm_bts_trx *trx;

	printf("Testing the lchan printing:");

	/* Add a BTS with some reasonanbly non-zero id */
	bts = gsm_bts_alloc(net, 45);
	/* Add a second TRX to test on multiple TRXs */
	gsm_bts_trx_alloc(bts);

	llist_for_each_entry(trx, &bts->trx_list, list) {
		char *name = gsm_lchan_name(&trx->ts[3].lchan[4]);

		if (name)
			printf(" %s", name);
		else
			printf("NULL name");
	}
	printf("\n");
}


void test_dyn_ts_subslots(void)
{
	struct gsm_bts_trx_ts ts;

	printf("Testing subslot numbers for pchan types\n");

	ts.pchan = GSM_PCHAN_TCH_F;
	OSMO_ASSERT(ts_subslots(&ts) == 1);

	ts.pchan = GSM_PCHAN_TCH_H;
	OSMO_ASSERT(ts_subslots(&ts) == 2);

	ts.pchan = GSM_PCHAN_PDCH;
	OSMO_ASSERT(ts_subslots(&ts) == 0);

	ts.pchan = GSM_PCHAN_TCH_F_PDCH;
	ts.flags = 0; /* TCH_F mode */
	OSMO_ASSERT(ts_subslots(&ts) == 1);
	ts.flags = TS_F_PDCH_ACTIVE;
	OSMO_ASSERT(ts_subslots(&ts) == 0);

	ts.pchan = GSM_PCHAN_TCH_F_TCH_H_PDCH;
	ts.dyn.pchan_is = GSM_PCHAN_TCH_F;
	OSMO_ASSERT(ts_subslots(&ts) == 1);
	ts.dyn.pchan_is = GSM_PCHAN_TCH_H;
	OSMO_ASSERT(ts_subslots(&ts) == 2);
	ts.dyn.pchan_is = GSM_PCHAN_PDCH;
	OSMO_ASSERT(ts_subslots(&ts) == 0);
}

int main(int argc, char **argv)
{
	struct gsm_network *network;

	osmo_init_logging(&log_info);

	/* Create a dummy network */
	network = bsc_network_init(tall_bsc_ctx, 1, 1, NULL);
	if (!network)
		return EXIT_FAILURE;

	test_request_chan(network);
	test_dyn_ts_subslots();
	test_bts_debug_print(network);

	return EXIT_SUCCESS;
}

void sms_alloc() {}
void sms_free() {}
void gsm48_secure_channel() {}
void vty_out() {}
void switch_trau_mux() {}
void rtp_socket_free() {}

struct tlv_definition nm_att_tlvdef;

