#include "raze/http/http_router.h"

#include <string.h>

void raze_http_router_route(const struct raze_http_router *router, const struct raze_http_request *req, struct raze_http_response *res)
{
	res->version = req->version;
	res->status_code = RAZE_HTTP_NOT_FOUND;
	res->keep_alive = req->keep_alive;

	for (size_t i = 0; i < router->route_count; i++) {
		struct raze_http_route *route = &router->routes[i];

		if (req->method == route->method && req->uri_len == strlen(route->path) && !strncmp(req->uri, route->path, req->uri_len)) {
			route->handler(req, res);
			return;
		}
	}

	// Fallback
	const char *body = "404 Not Found";
	raze_http_response_set_body(res, body, 13);
	raze_http_response_add_header(res, "Content-Type", "text/plain");
}
