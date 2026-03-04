#ifndef _RAZE_CONNECTION_H
#define _RAZE_CONNECTION_H

#include "raze/core/buffer.h"
#include "raze/http/http_response.h"
#include "raze/http/http_parser.h"

struct raze_connection {
	int fd;
	struct raze_buffer read_buffer;
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
