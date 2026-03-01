#include "raze/server/connection.h"
#include "raze/core/logger.h"
#include "raze/core/buffer.h"
#include "raze/http/http_router.h"

#include <stdio.h>
#include <stdlib.h>
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

	while (keep_alive) {
		raze_buffer_clear(connection->read_buffer);
		raze_buffer_clear(connection->write_buffer);

		if (raze_connection_handle_read(connection) == -1) {
			perror("raze_connection_handle_read failed");
			return -1;
		}

		connection->request = raze_http_request_create(connection->read_buffer->data, connection->read_buffer->size);
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

		if (raze_http_response_build(connection->response, connection->write_buffer) == -1) {
			raze_error("raze_http_response_build failed");
			return -1;
		}

		if (raze_connection_handle_write(connection) == -1) {
			perror("raze_connection_handle_write failed");
			return -1;
		}

		keep_alive = connection->response->keep_alive;

		raze_http_response_destroy(connection->response);
		raze_http_request_destroy(connection->request);
	}

	return 0;
}

int raze_connection_handle_read(struct raze_connection *connection)
{
	char buff[4096];

	ssize_t bytes = recv(connection->fd, buff, sizeof(buff), 0);
	if (bytes < 0) {
		perror("recv");
		return -1;
	}

	if (bytes == 0) {
		raze_error("connection closed");
		return -1;
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
