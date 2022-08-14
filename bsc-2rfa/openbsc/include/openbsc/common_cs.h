#pragma once

#include <stdint.h>

struct msgb;
struct gsm_network;

typedef int (*mncc_recv_cb_t)(struct gsm_network *, struct msgb *);

struct vty;

struct gsm_network *gsm_network_init(void *ctx,
				     uint16_t country_code,
				     uint16_t network_code,
				     mncc_recv_cb_t mncc_recv);

int common_cs_vty_init(struct gsm_network *network,
                 int (* config_write_net )(struct vty *));
struct gsm_network *gsmnet_from_vty(struct vty *v);
