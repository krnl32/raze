#ifndef _RAZE_SERVER_H
#define _RAZE_SERVER_H

#include "raze/http/http_router.h"

#include <netinet/in.h>

struct raze_socket {
	uint16_t domain;
	uint16_t type;
	uint16_t protocol;
	uint32_t host;
	uint16_t port;
	uint16_t backlog;
};

struct raze_server {
	int sockfd;
	const struct raze_http_router *router;
};

struct raze_server *raze_server_create(const struct raze_socket *sockconfig, const struct raze_http_router *router);
void raze_server_destroy(struct raze_server *server);
int raze_server_run(struct raze_server *server);

#endif
