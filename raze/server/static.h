#ifndef _RAZE_STATIC_H
#define _RAZE_STATIC_H

#include "raze/http/http_request.h"
#include "raze/http/http_response.h"

struct raze_static {
	const char *root;
	const char *mount;
};

int raze_static_handle(const struct raze_static *static_cfg, const struct raze_http_request *req, struct raze_http_response *res);

#endif
