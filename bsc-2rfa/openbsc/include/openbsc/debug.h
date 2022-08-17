#pragma once

#include <stdio.h>
#include <osmocom/core/linuxlist.h>

#define DEBUG
#include <osmocom/core/logging.h>

/* Debug Areas of the code */
enum {
	DRLL,
	DCC,
	DMM,
	DRR,
	DRSL,
	DNM,
	DMNCC,
	DPAG,
	DMEAS,
	DSCCP,
	DMSC,
	DMGCP,
	DHO,
	DDB,
	DREF,
	DGPRS,
	DNS,
	DBSSGP,
	DLLC,
	DSNDCP,
	DSLHC,
	DNAT,
	DCTRL,
	DSMPP,
	DFILTER,
	DGTPHUB,
	DRANAP,
	DSUA,
	DV42BIS,
	DPCU,
	Debug_LastEntry,
};

struct gsm_subscriber;

void log_set_filter_vlr_subscr(struct log_target *target,
			       struct gsm_subscriber *vlr_subscr);

extern const struct log_info log_info;
