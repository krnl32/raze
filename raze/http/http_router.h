#ifndef _RAZE_HTTP_ROUTER_H
#define _RAZE_HTTP_ROUTER_H

#include "raze/http/http_request.h"
#include "raze/http/http_response.h"

struct raze_http_route {
	enum raze_http_method method;
	const char *path;
	void (*handler)(const struct raze_http_request *req, struct raze_http_response *res);
};

struct raze_http_router {
	struct raze_http_route *routes;
	size_t route_count;
};

void raze_http_router_route(const struct raze_http_router *router, const struct raze_http_request *req, struct raze_http_response *res);

#endif
