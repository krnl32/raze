#include "raze/server/connection.h"
#include "raze/core/logger.h"
#include "raze/core/buffer.h"
#include "raze/http/http_router.h"
#include "raze/http/http_utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

int raze_connection_init(struct raze_connection *connection, int fd)
{
	if (raze_buffer_init(&connection->read_buffer) == -1) {
		raze_error("raze_buffer_init failed");
		return -1;
	}

	if (raze_buffer_init(&connection->write_buffer) == -1) {
		raze_error("raze_buffer_init failed");
		raze_buffer_deinit(&connection->read_buffer);
		return -1;
	}

	if (raze_http_parser_init(&connection->parser) == -1) {
		raze_error("raze_http_parser_init failed");
		raze_buffer_deinit(&connection->write_buffer);
		raze_buffer_deinit(&connection->read_buffer);
		return -1;
	}

	connection->fd = fd;
	connection->response = NULL;
	return 0;
}

void raze_connection_deinit(struct raze_connection *connection)
{
	if (connection) {
		raze_http_parser_deinit(&connection->parser);
		raze_buffer_deinit(&connection->write_buffer);
		raze_buffer_deinit(&connection->read_buffer);
		shutdown(connection->fd, SHUT_RDWR);
		close(connection->fd);
	}
}

int raze_connection_handle(struct raze_connection *connection)
{
	bool keep_alive = true;
	bool connection_closed = false;

	while (keep_alive) {
		int read_status = raze_connection_handle_read(connection);
		if (read_status == -1) {
			raze_error("raze_connection_handle_read failed");
			return -1;
		}

		if (read_status == 1) {
			connection_closed = true;
		}

		while (1) {
			uint8_t *data = connection->read_buffer.data;
			size_t data_size = connection->read_buffer.size;

			enum http_parser_result res = raze_http_parser_parse(&connection->parser, (const char *)data, data_size);
			if (res == RAZE_HTTP_RESULT_ERROR) {
				raze_error("raze_http_parser_parse failed");
				return -1;
			}

			if (res == RAZE_HTTP_RESULT_INCOMPLETE) {
				break;
			}

			connection->response = raze_http_response_create();
			if (!connection->response) {
				raze_error("raze_http_response_create failed");
				return -1;
			}

			http_router_route(&connection->parser.request, connection->response);
			raze_buffer_clear(&connection->write_buffer);

			if (raze_http_response_build(connection->response, &connection->write_buffer) == -1) {
				raze_error("raze_http_response_build failed");
				return -1;
			}

			if (raze_connection_handle_write(connection) == -1) {
				raze_error("raze_connection_handle_write failed");
				return -1;
			}

			keep_alive = connection->response->keep_alive;

			raze_http_response_destroy(connection->response);

			// Pipelining
			size_t consumed = connection->parser.buffer_pos;
			size_t remaining = data_size - consumed;
			if (remaining > 0) {
				memmove(connection->read_buffer.data, connection->read_buffer.data + consumed, remaining);
			}

			connection->read_buffer.size = remaining;
			raze_http_parser_reset(&connection->parser);

			if (!keep_alive) {
				break;
			}
		}

		if (connection_closed) {
			break;
		}
	}

	return 0;
}

int raze_connection_handle_read(struct raze_connection *connection)
{
	struct raze_buffer *read_buffer = &connection->read_buffer;
	if (raze_buffer_reserve(read_buffer, 4096) == -1) {
		return -1;
	}

	size_t writable = read_buffer->capacity - read_buffer->size;

	ssize_t bytes = recv(connection->fd, read_buffer->data + read_buffer->size, writable, 0);
	if (bytes < 0) {
		perror("recv");
		return -1;
	}

	if (bytes == 0) {
		raze_debug("connection closed");
		return 1;
	}

	read_buffer->size += (size_t)bytes;
	return 0;
}

int raze_connection_handle_write(struct raze_connection *connection)
{
	size_t total_sent = 0;
	size_t send_size = connection->write_buffer.size;

	while (total_sent < send_size) {
		ssize_t send_len = send(connection->fd, connection->write_buffer.data + total_sent, send_size - total_sent, 0);
		if (send_len <= 0) {
			perror("send");
			return -1;
		}

		total_sent += (size_t)send_len;
	}

	return 0;
}
