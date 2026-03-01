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

struct raze_connection *raze_connection_create(int fd)
{
	struct raze_connection *connection = malloc(sizeof(struct raze_connection));
	if (!connection) {
		perror("malloc");
		return NULL;
	}

	connection->read_buffer = raze_buffer_create();
	if (!connection->read_buffer) {
		raze_error("raze_buffer_create failed");
		free(connection);
		return NULL;
	}

	connection->write_buffer = raze_buffer_create();
	if (!connection->write_buffer) {
		raze_error("raze_buffer_create failed");
		raze_buffer_destroy(connection->read_buffer);
		free(connection);
		return NULL;
	}

	connection->fd = fd;
	connection->request = NULL;
	connection->response = NULL;
	return connection;
}

void raze_connection_destroy(struct raze_connection *connection)
{
	if (connection) {
		raze_buffer_destroy(connection->write_buffer);
		raze_buffer_destroy(connection->read_buffer);
		shutdown(connection->fd, SHUT_RDWR);
		close(connection->fd);
		free(connection);
	}
}

int raze_connection_init(struct raze_connection *connection, int fd)
{
	connection->read_buffer = raze_buffer_create();
	if (!connection->read_buffer) {
		raze_error("raze_buffer_create failed");
		free(connection);
		return -1;
	}

	connection->write_buffer = raze_buffer_create();
	if (!connection->write_buffer) {
		raze_error("raze_buffer_create failed");
		raze_buffer_destroy(connection->read_buffer);
		free(connection);
		return -1;
	}

	connection->fd = fd;
	connection->request = NULL;
	connection->response = NULL;
	return 0;
}

void raze_connection_deinit(struct raze_connection *connection)
{
	if (connection) {
		raze_buffer_destroy(connection->write_buffer);
		raze_buffer_destroy(connection->read_buffer);
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
			uint8_t *data = connection->read_buffer->data;
			size_t data_size = connection->read_buffer->size;

			// Read Full Request
			ssize_t header_end = raze_http_utility_find_header_end(data, data_size);
			if (header_end == -1) {
				break;
			}

			size_t content_len = raze_http_utility_extract_content_length(data, (size_t)header_end);

			// Read Request Body
			size_t total_required = (size_t)header_end + content_len;
			if (data_size < total_required) {
				break;
			}

			connection->request = raze_http_request_create((const char *)connection->read_buffer->data, total_required);
			if (!connection->request) {
				raze_error("http request parser failed");
				return -1;
			}

			connection->response = raze_http_response_create();
			if (!connection->response) {
				raze_error("raze_http_response_create failed");
				return -1;
			}

			http_router_route(connection->request, connection->response);
			raze_buffer_clear(connection->write_buffer);

			if (raze_http_response_build(connection->response, connection->write_buffer) == -1) {
				raze_error("raze_http_response_build failed");
				return -1;
			}

			if (raze_connection_handle_write(connection) == -1) {
				raze_error("raze_connection_handle_write failed");
				return -1;
			}

			keep_alive = connection->response->keep_alive;

			raze_http_response_destroy(connection->response);
			raze_http_request_destroy(connection->request);

			// Pipelining
			size_t remaining = data_size - total_required;
			if (remaining > 0) {
				memmove(connection->read_buffer->data, connection->read_buffer->data + total_required, remaining);
			}

			connection->read_buffer->size = remaining;

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
	uint8_t buff[4096];

	ssize_t bytes = recv(connection->fd, buff, sizeof(buff), 0);
	if (bytes < 0) {
		perror("recv");
		return -1;
	}

	if (bytes == 0) {
		raze_debug("connection closed");
		return 1;
	}

	raze_buffer_append(connection->read_buffer, buff, (size_t)bytes);
	return 0;
}

int raze_connection_handle_write(struct raze_connection *connection)
{
	size_t total_sent = 0;
	size_t send_size = connection->write_buffer->size;

	while (total_sent < send_size) {
		ssize_t send_len = send(connection->fd, connection->write_buffer->data + total_sent, send_size - total_sent, 0);
		if (send_len <= 0) {
			perror("send");
			return -1;
		}

		total_sent += (size_t)send_len;
	}

	return 0;
}
