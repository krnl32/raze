#define _POSIX_C_SOURCE 200112L

#include "raze/server/server.h"
#include "raze/server/connection.h"
#include "raze/core/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

struct raze_server *raze_server_create(const struct raze_socket *sockconfig, const struct raze_http_router *router)
{
	struct raze_server *server = malloc(sizeof(struct raze_server));
	if (!server) {
		perror("malloc");
		return NULL;
	}

	server->sockfd = -1;
	server->router = router;

	struct addrinfo *addr_list = NULL;
	struct addrinfo hints = { 0 };
	hints.ai_family = sockconfig->domain;
	hints.ai_socktype = sockconfig->type;
	hints.ai_protocol = sockconfig->protocol;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(sockconfig->host, sockconfig->port, &hints, &addr_list) != 0) {
		raze_error("getaddrinfo");
		free(server);
		return NULL;
	}

	struct addrinfo *addr;
	for (addr = addr_list; addr != NULL; addr = addr->ai_next) {
		server->sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (server->sockfd == -1) {
			continue;
		}

		int opt = 1;
		if (setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
			perror("setsockopt");
			close(server->sockfd);
			server->sockfd = -1;
			continue;
		}

		if (bind(server->sockfd, addr->ai_addr, addr->ai_addrlen) == 0) {
			break;
		}

		close(server->sockfd);
		server->sockfd = -1;
	}

	freeaddrinfo(addr_list);

	if (server->sockfd == -1) {
		perror("bind");
		free(server);
		return NULL;
	}

	if (listen(server->sockfd, sockconfig->backlog) == -1) {
		perror("listen");
		close(server->sockfd);
		free(server);
		return NULL;
	}

	return server;
}

void raze_server_destroy(struct raze_server *server)
{
	if (server) {
		if (server->sockfd != -1) {
			shutdown(server->sockfd, SHUT_RDWR);
			close(server->sockfd);
		}

		free(server);
	}
}

int raze_server_run(struct raze_server *server)
{
	while (1) {
		raze_info("waiting for connections...");

		struct sockaddr_storage client_sa;
		socklen_t client_len = sizeof(client_sa);

		int clientfd = accept(server->sockfd, (struct sockaddr *)&client_sa, &client_len);
		if (clientfd == -1) {
			perror("accept");
			continue;
		}

		raze_info("connection accepted: %d", clientfd);

		struct raze_connection connection;
		raze_connection_init(&connection, clientfd, server->router);

		if (raze_connection_handle(&connection) == -1) {
			raze_error("raze_connection_handle failed");
		}

		raze_connection_deinit(&connection);
	}

	return 0;
}
