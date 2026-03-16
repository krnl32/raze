#define _POSIX_C_SOURCE 200112L

#include "raze/server/server.h"
#include "raze/server/connection.h"
#include "raze/core/logger.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <asm-generic/socket.h>

static int raze_socket_set_nonblock(int fd);
static int raze_server_accept(struct raze_server *server);

struct raze_server *raze_server_create(const struct raze_socket *sockconfig, const struct raze_static *static_cfg, const struct raze_http_router *router)
{
	struct raze_server *server = malloc(sizeof(struct raze_server));
	if (!server) {
		perror("malloc");
		return NULL;
	}

	server->sockfd = -1;
	server->router = router;
	server->static_cfg = static_cfg;

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

	// Setup Socket
	struct addrinfo *addr;
	for (addr = addr_list; addr != NULL; addr = addr->ai_next) {
		server->sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (server->sockfd == -1) {
			continue;
		}

		if (raze_socket_set_nonblock(server->sockfd) == -1) {
			perror("fcntl");
			close(server->sockfd);
			server->sockfd = -1;
			continue;
		}

		int opt = 1;
		if (setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
			perror("setsockopt");
			close(server->sockfd);
			server->sockfd = -1;
			continue;
		}

		if (setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
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

	// Setup EPoll
	server->epfd = epoll_create1(0);
	if (server->epfd == -1) {
		perror("epoll_create1");
		close(server->sockfd);
		free(server);
		return NULL;
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.ptr = NULL;

	if (epoll_ctl(server->epfd, EPOLL_CTL_ADD, server->sockfd, &ev) == -1) {
		perror("epoll_ctl");
		close(server->epfd);
		close(server->sockfd);
		free(server);
		return NULL;
	}

	// Setup Eventfd
	server->evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (server->evfd == -1) {
		perror("eventfd");
		close(server->epfd);
		close(server->sockfd);
		free(server);
		return NULL;
	}

	ev.events = EPOLLIN;
	ev.data.ptr = server;

	if (epoll_ctl(server->epfd, EPOLL_CTL_ADD, server->evfd, &ev) == -1) {
		perror("epoll_ctl");
		close(server->evfd);
		close(server->epfd);
		close(server->sockfd);
		free(server);
		return NULL;
	}

	server->start = true;
	return server;
}

void raze_server_destroy(struct raze_server *server)
{
	if (server) {
		if (server->evfd != -1) {
			close(server->evfd);
		}

		if (server->epfd != -1) {
			close(server->epfd);
		}

		if (server->sockfd != -1) {
			close(server->sockfd);
		}

		free(server);
	}
}

int raze_server_run(struct raze_server *server)
{
	raze_info("waiting for connections...");
	struct epoll_event events[RAZE_EPOLL_EVENTS_MAX];

	while (__atomic_load_n(&server->start, __ATOMIC_SEQ_CST)) {
		int evr = epoll_wait(server->epfd, events, RAZE_EPOLL_EVENTS_MAX, -1);
		if (evr == -1) {
			perror("epoll_wait");
			continue;
		}

		for (int i = 0; i < evr; i++) {
			// Handle Server
			if (events[i].data.ptr == NULL) {
				if (raze_server_accept(server) == -1) {
					raze_error("raze_server_handle_accept failed");
				}
				continue;
			}

			// Handle Eventfd
			if (events[i].data.ptr == server) {
				uint64_t val;
				if (read(server->evfd, &val, sizeof(val)) == -1) {
					if (errno != EAGAIN) {
						perror("eventfd read");
					}
				}
				continue;
			}

			// Handle Clients
			struct raze_connection *connection = events[i].data.ptr;

			// Handle Errors
			if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
				if (epoll_ctl(server->epfd, EPOLL_CTL_DEL, connection->fd, NULL) == -1) {
					perror("epoll_ctl");
				}

				raze_connection_destroy(connection);
				continue;
			}

			// Handle Read
			if ((events[i].events & EPOLLIN) && connection->state == RAZE_CONNECTION_STATE_READ) {
				if (raze_connection_on_read(connection) == -1) {
					raze_error("raze_connection_on_read failed");

					if (epoll_ctl(server->epfd, EPOLL_CTL_DEL, connection->fd, NULL) == -1) {
						perror("epoll_ctl");
					}

					raze_connection_destroy(connection);
					continue;
				}
			}

			// Handle Write
			if ((events[i].events & EPOLLOUT) && connection->state == RAZE_CONNECTION_STATE_WRITE) {
				if (raze_connection_on_write(connection) == -1) {
					raze_error("raze_connection_on_write failed");

					if (epoll_ctl(server->epfd, EPOLL_CTL_DEL, connection->fd, NULL) == -1) {
						perror("epoll_ctl");
					}

					raze_connection_destroy(connection);
					continue;
				}

				if (connection->state == RAZE_CONNECTION_STATE_CLOSE) {
					if (epoll_ctl(server->epfd, EPOLL_CTL_DEL, connection->fd, NULL) == -1) {
						perror("epoll_ctl");
					}

					raze_connection_destroy(connection);
				}
			}
		}
	}

	return 0;
}

void raze_server_stop(struct raze_server *server)
{
	__atomic_store_n(&server->start, false, __ATOMIC_SEQ_CST);
	uint64_t val = 1;
	write(server->evfd, &val, sizeof(val));
}

static int raze_socket_set_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1) {
		perror("fcntl");
		return -1;
	}
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int raze_server_accept(struct raze_server *server)
{
	while (1) {
		struct sockaddr_storage client_sa;
		socklen_t client_len = sizeof(client_sa);

		int clientfd = accept(server->sockfd, (struct sockaddr *)&client_sa, &client_len);
		if (clientfd == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}

			perror("accept");
			return -1;
		}

		if (raze_socket_set_nonblock(clientfd) == -1) {
			perror("fcntl");
			close(clientfd);
			continue;
		}

		struct raze_connection *connection = raze_connection_create(clientfd, server->static_cfg, server->router);
		if (!connection) {
			raze_error("raze_connection_create failed");
			close(clientfd);
			continue;
		}

		struct epoll_event ev;
		ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
		ev.data.ptr = connection;

		if (epoll_ctl(server->epfd, EPOLL_CTL_ADD, clientfd, &ev) == -1) {
			perror("epoll_ctl");
			raze_connection_destroy(connection);
			return -1;
		}
	}

	return 0;
}
