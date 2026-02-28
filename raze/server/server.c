#include "raze/server/server.h"
#include "raze/server/connection.h"
#include "raze/core/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct raze_server *raze_server_create(const struct raze_socket *sockconfig)
{
	struct raze_server *server = malloc(sizeof(struct raze_server));
	if (!server) {
		perror("malloc");
		return NULL;
	}

	server->sockfd = socket(sockconfig->domain, sockconfig->type, sockconfig->protocol);
	if (server->sockfd == -1) {
		perror("socket");
		free(server);
		return NULL;
	}

	int opt = 1;
	if (setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		perror("setsockopt");
		close(server->sockfd);
		free(server);
		return NULL;
	}

	struct sockaddr_in sa = { 0 };
	sa.sin_family = sockconfig->domain;
	sa.sin_addr.s_addr = htonl(sockconfig->host);
	sa.sin_port = htons(sockconfig->port);

	if (bind(server->sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
		perror("bind");
		close(server->sockfd);
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

		struct sockaddr_in client_sa = { 0 };
		socklen_t client_len = sizeof(struct sockaddr_in);

		int clientfd = accept(server->sockfd, (struct sockaddr *)&client_sa, (socklen_t *)&client_len);
		if (clientfd == -1) {
			perror("accept");
			continue;
		}

		raze_info("connection accepted: %d", clientfd);

		struct raze_connection connection;
		raze_connection_init(&connection, clientfd);

		if (raze_connection_handle(&connection) == -1) {
			raze_error("raze_connection_handle failed");
		}

		raze_connection_deinit(&connection);
	}

	return 0;
}
