#ifndef _RAZE_HTTP_ROUTER_H
#define _RAZE_HTTP_ROUTER_H

#include "raze/http/http_request.h"
#include "raze/http/http_response.h"

void http_router_route(const struct raze_http_request *req, struct raze_http_response *res);

#endif
