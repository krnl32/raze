#ifndef _RAZE_HTTP_REQUEST_H
#define _RAZE_HTTP_REQUEST_H

#include "raze/http/http_protocol.h"

#include <stddef.h>
#include <stdbool.h>

#define RAZE_HTTP_REQUEST_HEADERS_MAX 64
#define RAZE_HTTP_REQUEST_BODY_MAX (1 * 1024 * 1024) // 1mb

struct raze_http_request_header {
	const char *key;
	size_t key_len;

	const char *value;
	size_t value_len;
};

struct raze_http_request {
	enum raze_http_method method;
	enum raze_http_version version;

	const char *uri;
	size_t uri_len;

	struct raze_http_request_header headers[RAZE_HTTP_REQUEST_HEADERS_MAX];
	size_t header_count;

	const char *body;
	size_t body_len;

	// flags
	bool keep_alive;
};

const struct raze_http_request_header *raze_http_request_get_header(const struct raze_http_request *request, const char *key);
bool raze_http_request_keep_alive(const struct raze_http_request *request);

#endif
