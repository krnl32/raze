#include "raze/server/connection.h"
#include "raze/core/logger.h"
#include "raze/core/buffer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sendfile.h>

static int raze_connection_handle_read(struct raze_connection *connection);
static int raze_connection_handle_parse(struct raze_connection *connection);
static int raze_connection_handle_process(struct raze_connection *connection);
static int raze_connection_handle_write(struct raze_connection *connection);

struct raze_connection *raze_connection_create(int fd, const struct raze_static *static_cfg, const struct raze_http_router *router)
{
	struct raze_connection *connection = malloc(sizeof(struct raze_connection));
	if (!connection) {
		perror("malloc");
		return NULL;
	}

	if (raze_ring_buffer_init(&connection->read_buffer, RAZE_RING_BUFFER_DEFAULT_CAPACITY) == -1) {
		raze_error("raze_ring_buffer_init failed");
		free(connection);
		return NULL;
	}

	if (raze_buffer_init(&connection->write_buffer) == -1) {
		raze_error("raze_buffer_init failed");
		raze_ring_buffer_deinit(&connection->read_buffer);
		free(connection);
		return NULL;
	}

	if (raze_http_parser_init(&connection->parser) == -1) {
		raze_error("raze_http_parser_init failed");
		raze_buffer_deinit(&connection->write_buffer);
		raze_ring_buffer_deinit(&connection->read_buffer);
		free(connection);
		return NULL;
	}

	connection->fd = fd;
	connection->write_offset = 0;
	connection->state = RAZE_CONNECTION_STATE_READ;
	connection->response = NULL;

	connection->file_fd = -1;
	connection->file_size = 0;
	connection->file_offset = 0;

	connection->static_cfg = static_cfg;
	connection->router = router;
	return connection;
}

void raze_connection_destroy(struct raze_connection *connection)
{
	if (connection) {
		raze_http_parser_deinit(&connection->parser);
		raze_buffer_deinit(&connection->write_buffer);
		raze_ring_buffer_deinit(&connection->read_buffer);
		close(connection->file_fd);
		close(connection->fd);
		free(connection);
	}
}

int raze_connection_on_read(struct raze_connection *connection)
{
	while (1) {
		switch (connection->state) {
			case RAZE_CONNECTION_STATE_READ: {
				if (raze_connection_handle_read(connection) == -1) {
					raze_error("raze_connection_handle_read failed");
					return -1;
				}
				break;
			}
			case RAZE_CONNECTION_STATE_PARSE: {
				if (raze_connection_handle_parse(connection) == -1) {
					raze_error("raze_connection_handle_parse failed");
					return -1;
				}
				break;
			}
			case RAZE_CONNECTION_STATE_PROCESS: {
				if (raze_connection_handle_process(connection) == -1) {
					raze_error("raze_connection_handle_process failed");
					return -1;
				}
				break;
			}
			case RAZE_CONNECTION_STATE_WRITE: {
				return 0;
			}
			case RAZE_CONNECTION_STATE_CLOSE: {
				return -1;
			}
			default: {
				return 0;
			}
		}
	}
}

int raze_connection_on_write(struct raze_connection *connection)
{
	if (connection->state != RAZE_CONNECTION_STATE_WRITE) {
		return 0;
	}

	int res = raze_connection_handle_write(connection);
	if (res == -1) {
		raze_error("raze_connection_handle_write failed");
		return -1;
	}

	if (res == 0) {
		return 0;
	}

	connection->state = (connection->response->keep_alive) ? RAZE_CONNECTION_STATE_READ : RAZE_CONNECTION_STATE_CLOSE;

	raze_http_response_destroy(connection->response);

	// Pipelining
	raze_ring_buffer_consume(&connection->read_buffer, connection->parser.buffer_pos);
	raze_http_parser_reset(&connection->parser);
	return 0;
}

static int raze_connection_handle_read(struct raze_connection *connection)
{
	struct raze_ring_buffer *read_buffer = &connection->read_buffer;
	uint8_t *ptr;

	size_t writable = raze_ring_buffer_write(read_buffer, &ptr);
	if (!writable) {
		raze_error("read buffer full");
		return -1;
	}

	ssize_t bytes = recv(connection->fd, ptr, writable, 0);
	if (bytes < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}

		perror("recv");
		return -1;
	}

	if (bytes == 0) {
		connection->state = RAZE_CONNECTION_STATE_CLOSE;
		return 0;
	}

	read_buffer->head += (size_t)bytes;
	connection->state = RAZE_CONNECTION_STATE_PARSE;
	return 0;
}

static int raze_connection_handle_parse(struct raze_connection *connection)
{
	uint8_t *data;
	size_t data_size = raze_ring_buffer_read(&connection->read_buffer, &data);
	if (data_size == 0) {
		connection->state = RAZE_CONNECTION_STATE_READ;
		return 0;
	}

	enum http_parser_result res = raze_http_parser_parse(&connection->parser, (const char *)data, data_size);
	if (res == RAZE_HTTP_RESULT_ERROR) {
		raze_error("raze_http_parser_parse failed");
		return -1;
	}

	if (res == RAZE_HTTP_RESULT_INCOMPLETE) {
		connection->state = RAZE_CONNECTION_STATE_READ;
		return 0;
	}

	connection->state = RAZE_CONNECTION_STATE_PROCESS;
	return 0;
}

static int raze_connection_handle_process(struct raze_connection *connection)
{
	connection->response = raze_http_response_create();
	if (!connection->response) {
		raze_error("raze_http_response_create failed");
		return -1;
	}

	int res = raze_static_handle(connection->static_cfg, &connection->parser.request, connection->response);
	if (res == -1) {
		raze_error("raze_static_handle failed");
		raze_http_response_destroy(connection->response);
		return -1;
	}

	if (res == 0) {
		connection->file_fd = connection->response->file_fd;
		connection->file_size = connection->response->file_size;
		connection->file_offset = 0;

		raze_buffer_clear(&connection->write_buffer);

		if (raze_http_response_build(connection->response, &connection->write_buffer) == -1) {
			raze_error("raze_http_response_build failed");
			raze_http_response_destroy(connection->response);
			return -1;
		}

		connection->write_offset = 0;
		connection->state = RAZE_CONNECTION_STATE_WRITE;
		return 0;
	}

	raze_http_router_route(connection->router, &connection->parser.request, connection->response);

	raze_buffer_clear(&connection->write_buffer);

	if (raze_http_response_build(connection->response, &connection->write_buffer) == -1) {
		raze_error("raze_http_response_build failed");
		raze_http_response_destroy(connection->response);
		return -1;
	}

	connection->write_offset = 0;
	connection->state = RAZE_CONNECTION_STATE_WRITE;
	return 0;
}

static int raze_connection_handle_write(struct raze_connection *connection)
{
	// send headers/body from buffer
	uint8_t *data = connection->write_buffer.data;
	size_t data_size = connection->write_buffer.size;

	while (connection->write_offset < data_size) {
		ssize_t send_len = send(connection->fd, data + connection->write_offset, data_size - connection->write_offset, 0);
		if (send_len < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return 0;
			}

			return -1;
		}

		connection->write_offset += (size_t)send_len;
	}

	// send file
	if (connection->file_fd != -1) {
		while (connection->file_offset < connection->file_size) {
			ssize_t send_len = sendfile(connection->fd, connection->file_fd, &connection->file_offset, (size_t)(connection->file_size - connection->file_offset));
			if (send_len < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					return 0;
				}

				perror("sendfile");
				return -1;
			}
		}

		close(connection->file_fd);
		connection->file_fd = -1;
	}

	return 1;
}
