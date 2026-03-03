#include "raze/http/http_response.h"
#include "raze/core/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct raze_http_response *raze_http_response_create(void)
{
	struct raze_http_response *response = malloc(sizeof(struct raze_http_response));
	if (!response) {
		perror("malloc");
		return NULL;
	}

	response->version = 0;
	response->status_code = 0;
	response->header_count = 0;
	response->body_len = 0;
	return response;
}

void raze_http_response_destroy(struct raze_http_response *response)
{
	if (response) {
		free(response);
	}
}

int raze_http_response_add_header(struct raze_http_response *response, const char *key, const char *value)
{
	if (response->header_count >= HTTP_RESPONSE_HEADER_SIZE) {
		raze_error("bad http header count");
		return -1;
	}

	struct raze_http_response_header *current_header = &response->headers[response->header_count++];
	strncpy(current_header->key, key, HTTP_HEADER_KEY_SIZE - 1);
	current_header->key[HTTP_HEADER_KEY_SIZE - 1] = 0;

	strncpy(current_header->value, value, HTTP_HEADER_VALUE_SIZE - 1);
	current_header->value[HTTP_HEADER_VALUE_SIZE - 1] = 0;
	return 0;
}

int raze_http_response_set_body(struct raze_http_response *response, const char *body, size_t len)
{
	if (len > HTTP_BODY_SIZE) {
		raze_error("bad http body len");
		return -1;
	}

	memcpy(response->body, body, len);
	response->body_len = len;
	return 0;
}

int raze_http_response_build(struct raze_http_response *response, struct raze_buffer *buffer)
{
	char tmp[4096] = { 0 };

	// Build Status Line
	const char *http_version = raze_http_version_to_string(response->version);
	const char *status_code = raze_http_status_code_to_string(response->status_code);
	int status_code_num = (int)response->status_code;

	int len = snprintf(tmp, sizeof(tmp), "%s %d %s\r\n", http_version, status_code_num, status_code);
	if (len < 0 || (size_t)len >= sizeof(tmp)) {
		raze_error("snprintf failed");
		return -1;
	}

	raze_buffer_append(buffer, tmp, (size_t)len);

	// Build Headers
	const char *connection = response->keep_alive ? "keep-alive" : "close";

	len = snprintf(tmp, sizeof(tmp), "Content-Length: %zu\r\nConnection: %s\r\n", response->body_len, connection);
	if (len < 0 || (size_t)len >= sizeof(tmp)) {
		raze_error("snprintf failed");
		return -1;
	}

	raze_buffer_append(buffer, tmp, (size_t)len);

	for (size_t i = 0; i < response->header_count; i++) {
		struct raze_http_response_header *header = &response->headers[i];

		len = snprintf(tmp, sizeof(tmp), "%s: %s\r\n", header->key, header->value);
		if (len < 0 || (size_t)len >= sizeof(tmp)) {
			raze_error("snprintf failed");
			return -1;
		}

		raze_buffer_append(buffer, tmp, (size_t)len);
	}

	raze_buffer_append(buffer, "\r\n", 2);

	// Build Body
	if (response->body_len > 0) {
		raze_buffer_append(buffer, response->body, response->body_len);
	}

	return 0;
}
