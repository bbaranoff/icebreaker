#ifndef _OPENBSC_OSMUX_H_
#define _OPENBSC_OSMUX_H_

#include <osmocom/netif/osmux.h>

#define OSMUX_PORT	1984

enum {
	OSMUX_ROLE_BSC = 0,
	OSMUX_ROLE_BSC_NAT,
};

int osmux_init(int role, struct mgcp_config *cfg);
int osmux_enable_endpoint(struct mgcp_endpoint *endp, struct in_addr *addr, uint16_t port);
void osmux_disable_endpoint(struct mgcp_endpoint *endp);
void osmux_allocate_cid(struct mgcp_endpoint *endp);
void osmux_release_cid(struct mgcp_endpoint *endp);

int osmux_xfrm_to_rtp(struct mgcp_endpoint *endp, int type, char *buf, int rc);
int osmux_xfrm_to_osmux(int type, char *buf, int rc, struct mgcp_endpoint *endp);

int osmux_send_dummy(struct mgcp_endpoint *endp);

int osmux_get_cid(void);
void osmux_put_cid(uint8_t osmux_cid);
int osmux_used_cid(void);

enum osmux_state {
	OSMUX_STATE_DISABLED = 0, /* Osmux not being currently used by endp */
	OSMUX_STATE_NEGOTIATING,  /* Osmux was locally requested in MGCP CRCX */
	OSMUX_STATE_ACTIVATING,   /* Osmux was accepted in MGCP CRCX ACK. It can now be enabled by \ref osmux_enable_endpoint. */
	OSMUX_STATE_ENABLED,	  /* Osmux was initialized by \ref osmux_enable_endpoint and can process frames */
};

enum osmux_usage {
	OSMUX_USAGE_OFF = 0,
	OSMUX_USAGE_ON = 1,
	OSMUX_USAGE_ONLY = 2,
};

#endif
