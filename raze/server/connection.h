#ifndef _RAZE_CONNECTION_H
#define _RAZE_CONNECTION_H

#include "raze/core/buffer.h"
#include "raze/core/ring_buffer.h"
#include "raze/http/http_response.h"
#include "raze/http/http_parser.h"

enum raze_connection_state {
	RAZE_CONNECTION_STATE_CLOSE,
	RAZE_CONNECTION_STATE_READ,
	RAZE_CONNECTION_STATE_WRITE,
	RAZE_CONNECTION_STATE_PARSE,
	RAZE_CONNECTION_STATE_PROCESS
};

struct raze_connection {
	int fd;
	enum raze_connection_state state;
	struct raze_ring_buffer read_buffer;
	struct raze_buffer write_buffer;
	struct raze_http_parser parser;

	struct raze_http_response *response;
};

int raze_connection_init(struct raze_connection *connection, int fd);
void raze_connection_deinit(struct raze_connection *connection);
int raze_connection_handle(struct raze_connection *connection);
int raze_connection_handle_read(struct raze_connection *connection);
int raze_connection_handle_write(struct raze_connection *connection);

#endif
