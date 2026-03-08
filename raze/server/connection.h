#ifndef _RAZE_CONNECTION_H
#define _RAZE_CONNECTION_H

#include "raze/core/buffer.h"
#include "raze/core/ring_buffer.h"
#include "raze/http/http_router.h"
#include "raze/http/http_response.h"
#include "raze/http/http_parser.h"

enum raze_connection_state {
	RAZE_CONNECTION_STATE_READ,
	RAZE_CONNECTION_STATE_PARSE,
	RAZE_CONNECTION_STATE_PROCESS,
	RAZE_CONNECTION_STATE_WRITE,
	RAZE_CONNECTION_STATE_CLOSE
};

struct raze_connection {
	int fd;
	size_t write_offset;
	enum raze_connection_state state;
	struct raze_ring_buffer read_buffer;
	struct raze_buffer write_buffer;
	struct raze_http_parser parser;

	const struct raze_http_router *router;
	struct raze_http_response *response;
};

struct raze_connection *raze_connection_create(int fd, const struct raze_http_router *router);
void raze_connection_destroy(struct raze_connection *connection);
int raze_connection_on_read(struct raze_connection *connection);
int raze_connection_on_write(struct raze_connection *connection);

#endif
