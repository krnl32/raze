#ifndef _RAZE_HTTP_PARSER_H
#define _RAZE_HTTP_PARSER_H

#include <stddef.h>

#define HTTP_HEADER_SIZE 1024

enum raze_http_method {
	RAZE_HTTP_GET,
	RAZE_HTTP_HEAD,
	RAZE_HTTP_POST,
	RAZE_HTTP_PUT,
	RAZE_HTTP_DELETE,
	RAZE_HTTP_CONNECT,
	RAZE_HTTP_OPTIONS,
	RAZE_HTTP_TRACE
};

enum raze_http_version {
	RAZE_HTTP_0_9,
	RAZE_HTTP_1_0,
	RAZE_HTTP_1_1,
	RAZE_HTTP_2,
	RAZE_HTTP_3
};

struct raze_http_header {
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
	struct raze_http_header headers[HTTP_HEADER_SIZE];
	size_t header_count;
	const char *body;
	size_t body_len;
};

struct raze_http_request *raze_http_request_create(const char *req_str, size_t req_len);
void raze_http_request_destroy(struct raze_http_request *request);
const struct raze_http_header *raze_http_request_get_header(const struct raze_http_request *request, const char *key);

#endif
