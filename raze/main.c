#include "raze/core/logger.h"
#include "raze/server/server.h"
#include "raze/server/static.h"
#include "raze/http/http_router.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

static volatile sig_atomic_t running = 1;

static void router_handle_get_home(const struct raze_http_request *req, struct raze_http_response *res);
static void router_handle_get_data(const struct raze_http_request *req, struct raze_http_response *res);
static void router_handle_get_junk(const struct raze_http_request *req, struct raze_http_response *res);
static void router_handle_post_data(const struct raze_http_request *req, struct raze_http_response *res);

static void raze_sigint_handle(int sig)
{
	(void)sig;
	running = 0;
}

static void *raze_thread_task(void *arg)
{
	struct raze_server *server = arg;

	if (raze_server_run(server) == -1) {
		raze_error("raze_server_run failed");
	}

	return NULL;
}

int main(void)
{
	signal(SIGINT, raze_sigint_handle);

	int success = 0;

	struct raze_static static_cfg = {
		.root = "/var/www/html",
		.mount = "/",
	};

	struct raze_http_route routes[] = {
		{ RAZE_HTTP_GET, "/home", router_handle_get_home },
		{ RAZE_HTTP_GET, "/data", router_handle_get_data },
		{ RAZE_HTTP_GET, "/junk", router_handle_get_junk },
		{ RAZE_HTTP_POST, "/data", router_handle_post_data },
	};

	struct raze_http_router router;
	router.routes = routes;
	router.route_count = sizeof(routes) / sizeof(routes[0]);

	struct raze_socket sockconfig;
	sockconfig.domain = AF_INET;
	sockconfig.type = SOCK_STREAM;
	sockconfig.protocol = 0;
	sockconfig.host = "127.0.0.1";
	sockconfig.port = "8080";
	sockconfig.backlog = 255;

	// Setup Threads
	long proc_count = sysconf(_SC_NPROCESSORS_ONLN);
	size_t thread_count = (proc_count > 0) ? (size_t)proc_count : 1;
	raze_info("threadcount: %zu\n", thread_count);

	struct raze_server **servers = malloc(sizeof(*servers) * thread_count);
	if (!servers) {
		perror("malloc");
		return -1;
	}

	size_t started_threads = 0;
	pthread_t *threads = malloc(sizeof(*threads) * thread_count);
	if (!threads) {
		perror("malloc");
		free(servers);
		return -1;
	}

	memset(servers, 0, sizeof(*servers) * thread_count);

	for (size_t i = 0; i < thread_count; i++) {
		servers[i] = raze_server_create(&sockconfig, &static_cfg, &router);
		if (!servers[i]) {
			raze_error("raze_server_create failed");
			success = -1;
			goto cleanup_error;
		}
	}

	for (size_t i = 0; i < thread_count; i++) {
		if (pthread_create(&threads[i], NULL, raze_thread_task, servers[i]) != 0) {
			raze_error("pthread_create failed");
			success = -1;
			goto cleanup_error;
		}

		started_threads++;
	}

	while (running) {
		pause();
	}

cleanup_error:
	for (size_t i = 0; i < started_threads; i++) {
		raze_server_stop(servers[i]);
	}

	for (size_t i = 0; i < started_threads; i++) {
		pthread_join(threads[i], NULL);
	}

	for (size_t i = 0; i < thread_count; i++) {
		raze_server_destroy(servers[i]);
	}

	free(threads);
	free(servers);
	return success;
}

static void router_handle_get_home(const struct raze_http_request *req, struct raze_http_response *res)
{
	(void)req;
	res->status_code = RAZE_HTTP_OK;
	raze_http_response_add_header(res, "Content-Type", "text/html");
	const char *html_body = "<h1>Home</h1>";
	raze_http_response_set_body(res, html_body, 13);
}

static void router_handle_get_data(const struct raze_http_request *req, struct raze_http_response *res)
{
	(void)req;
	res->status_code = RAZE_HTTP_OK;
	raze_http_response_add_header(res, "Content-Type", "application/json");
	const char *json_body = "{\"message\": \"This is the home data\"}";
	raze_http_response_set_body(res, json_body, 36);
}

static void router_handle_get_junk(const struct raze_http_request *req, struct raze_http_response *res)
{
	(void)req;
	res->status_code = RAZE_HTTP_OK;
	raze_http_response_add_header(res, "Content-Type", "text/plain");
	const char *body = "junk";
	raze_http_response_set_body(res, body, 5);
}

static void router_handle_post_data(const struct raze_http_request *req, struct raze_http_response *res)
{
	(void)res;
	printf("Recieved Data: %.*s\n", (int)req->body_len, req->body);
}
