#ifndef _RAZE_HTTP_RESPONSE_H
#define _RAZE_HTTP_RESPONSE_H

#include "raze/http/http_protocol.h"
#include "raze/core/buffer.h"

#include <stddef.h>
#include <stdbool.h>

#define HTTP_HEADER_SIZE 1024
#define HTTP_HEADER_FIELD_SIZE 1024
#define HTTP_BODY_SIZE 4096

struct raze_http_response_header {
	char key[HTTP_HEADER_FIELD_SIZE];
	char value[HTTP_HEADER_FIELD_SIZE];
};

struct raze_http_response {
	enum raze_http_version version;
	enum raze_http_status_code status_code;
	struct raze_http_response_header headers[HTTP_HEADER_SIZE];
	size_t header_count;
	char body[HTTP_BODY_SIZE];
	size_t body_len;

	// flags
	bool keep_alive;
};

struct raze_http_response *raze_http_response_create(void);
void raze_http_response_destroy(struct raze_http_response *response);
int raze_http_response_add_header(struct raze_http_response *response, const char *key, const char *value);
int raze_http_response_set_body(struct raze_http_response *response, const char *body, size_t len);
int raze_http_response_build(struct raze_http_response *response, struct raze_buffer *buffer);

#endif
