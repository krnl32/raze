#include "raze/core/logger.h"
#include "raze/server/server.h"
#include "raze/server/static.h"
#include "raze/http/http_router.h"

#include <stdio.h>

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

int main(void)
{
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

	struct raze_server *server = raze_server_create(&sockconfig, &static_cfg, &router);
	if (!server) {
		raze_error("raze_server_create failed");
		return -1;
	}

	if (raze_server_run(server) == -1) {
		raze_server_destroy(server);
		return -1;
	}

	raze_server_destroy(server);
	return 0;
}
