#ifndef _RAZE_CONNECTION_H
#define _RAZE_CONNECTION_H

#include "raze/core/buffer.h"
#include "raze/http/http_request.h"
#include "raze/http/http_response.h"

struct raze_connection {
	int fd;
	struct raze_buffer *read_buffer;
	struct raze_buffer *write_buffer;
	struct raze_http_request *request;
	struct raze_http_response *response;
};

struct raze_connection *raze_connection_create(int fd);
void raze_connection_destroy(struct raze_connection *connection);
int raze_connection_init(struct raze_connection *connection, int fd);
void raze_connection_deinit(struct raze_connection *connection);
int raze_connection_handle(struct raze_connection *connection);
int raze_connection_handle_read(struct raze_connection *connection);
int raze_connection_handle_write(struct raze_connection *connection);

#endif
