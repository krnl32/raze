#ifndef _RAZE_HTTP_PARSER_H
#define _RAZE_HTTP_PARSER_H

#include "raze/http_protocol.h"

#include <stddef.h>

#define HTTP_HEADER_SIZE 1024

struct raze_http_request_header {
	const char *key;
	size_t key_len;
	const char *value;
	size_t value_len;
};

struct raze_http_request {
	enum raze_http_method method;
	const char *uri;
	size_t uri_len;
	enum raze_http_version version;
	struct raze_http_request_header headers[HTTP_HEADER_SIZE];
	size_t header_count;
	const char *body;
	size_t body_len;
};

struct raze_http_request *raze_http_request_create(const char *req_str, size_t req_len);
void raze_http_request_destroy(struct raze_http_request *request);
const struct raze_http_request_header *raze_http_request_get_header(const struct raze_http_request *request, const char *key);

#endif
