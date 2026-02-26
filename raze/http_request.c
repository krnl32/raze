#include "raze/http_request.h"
#include "raze/logger.h"
#include "raze/utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *raze_http_request_parse_request_line(struct raze_http_request *request, const char *req_str, size_t req_len);
static const char *raze_http_request_parse_headers(struct raze_http_request *request, const char *header_start, const char *req_end);

struct raze_http_request *raze_http_request_create(const char *req_str, size_t req_len)
{
	struct raze_http_request *request = malloc(sizeof(struct raze_http_request));
	if (!request) {
		perror("malloc");
		return NULL;
	}

	memset(request, 0, sizeof(*request));

	const char *header_start = raze_http_request_parse_request_line(request, req_str, req_len);
	if (!header_start) {
		raze_error("bad http request line");
		free(request);
		return NULL;
	}

	const char *body_start = raze_http_request_parse_headers(request, header_start, req_str + req_len);
	if (!body_start) {
		raze_error("bad http headers");
		free(request);
		return NULL;
	}

	request->body = body_start;
	request->body_len = (size_t)((req_str + req_len) - body_start);

	const char *content_len_str = raze_http_request_get_header(request, "Content-Length");
	if (content_len_str) {
		size_t content_len = strtoul(content_len_str, NULL, 10);
		if (request->body_len < content_len) {
			raze_error("bad http body length");
			raze_error("http partial reading not supported");
			free(request);
			return NULL;
		}
	}

	return request;
}

void raze_http_request_destroy(struct raze_http_request *request)
{
	if (request) {
		free(request);
	}
}

const char *raze_http_request_get_header(const struct raze_http_request *request, const char *key)
{
	for (size_t i = 0; i < request->header_count; i++) {
		if (!raze_strcasecmp(request->headers[i].key, key)) {
			return request->headers[i].value;
		}
	}

	return NULL;
}

static const char *raze_http_request_parse_request_line(struct raze_http_request *request, const char *req_str, size_t req_len)
{
	// Parse Method
	const char *sp1 = memchr(req_str, ' ', req_len);
	if (!sp1) {
		raze_error("bad http request method");
		return NULL;
	}

	size_t method_len = (size_t)(sp1 - req_str);
	if (method_len == 3 && !memcmp(req_str, "GET", 3)) {
		request->method = RAZE_HTTP_GET;
	} else if (method_len == 4 && !memcmp(req_str, "HEAD", 4)) {
		request->method = RAZE_HTTP_HEAD;
	} else if (method_len == 4 && !memcmp(req_str, "POST", 4)) {
		request->method = RAZE_HTTP_POST;
	} else if (method_len == 3 && !memcmp(req_str, "PUT", 3)) {
		request->method = RAZE_HTTP_PUT;
	} else if (method_len == 6 && !memcmp(req_str, "DELETE", 6)) {
		request->method = RAZE_HTTP_DELETE;
	} else if (method_len == 7 && !memcmp(req_str, "CONNECT", 7)) {
		request->method = RAZE_HTTP_CONNECT;
	} else if (method_len == 7 && !memcmp(req_str, "OPTIONS", 7)) {
		request->method = RAZE_HTTP_OPTIONS;
	} else if (method_len == 5 && !memcmp(req_str, "TRACE", 5)) {
		request->method = RAZE_HTTP_TRACE;
	} else {
		raze_error("unsupported http request method");
		return NULL;
	}

	// Parse URI
	const char *uri_start = (sp1 + 1);
	size_t req_len_from_uri = req_len - (size_t)(uri_start - req_str);

	const char *sp2 = memchr(uri_start, ' ', req_len_from_uri);
	if (!sp2) {
		raze_error("bad http request uri");
		return NULL;
	}

	size_t uri_len = (size_t)(sp2 - uri_start);
	if (uri_len >= HTTP_URI_SIZE) {
		raze_error("bad http request uri size");
		return NULL;
	}

	memcpy(request->uri, uri_start, uri_len);
	request->uri[uri_len] = 0;

	// Parse HTTP Version
	const char *version_start = (sp2 + 1);

	size_t remaining_len = (req_len - (size_t)(version_start - req_str));
	if (remaining_len < 8) {
		raze_error("bad http request version");
		return NULL;
	}

	const char *header_start = NULL;

	if (remaining_len >= 10 && !memcmp(version_start, "HTTP/0.9\r\n", 10)) {
		request->version = RAZE_HTTP_0_9;
		header_start = version_start + 10;
	} else if (remaining_len >= 10 && !memcmp(version_start, "HTTP/1.0\r\n", 10)) {
		request->version = RAZE_HTTP_1_0;
		header_start = version_start + 10;
	} else if (remaining_len >= 10 && !memcmp(version_start, "HTTP/1.1\r\n", 10)) {
		request->version = RAZE_HTTP_1_1;
		header_start = version_start + 10;
	} else if (remaining_len >= 8 && !memcmp(version_start, "HTTP/2\r\n", 8)) {
		request->version = RAZE_HTTP_2;
		header_start = version_start + 8;
	} else if (remaining_len >= 8 && !memcmp(version_start, "HTTP/3\r\n", 8)) {
		request->version = RAZE_HTTP_3;
		header_start = version_start + 8;
	} else {
		raze_error("bad http request version");
		return NULL;
	}

	return header_start;
}

const char *raze_http_request_parse_headers(struct raze_http_request *request, const char *header_start, const char *req_end)
{
	const char *current_line = header_start;
	uint32_t current_header = request->header_count;

	while (current_line + 1 < req_end) {
		if (current_line[0] == '\r' && current_line[1] == '\n') {
			request->header_count = current_header;
			return current_line + 2;
		}

		if (current_header >= HTTP_HEADER_SIZE) {
			raze_error("bad http header count");
			return NULL;
		}

		const char *header_end = memchr(current_line, '\r', (size_t)(req_end - current_line));
		if (!header_end || (header_end + 1 >= req_end) || header_end[1] != '\n') {
			raze_error("bad http header end");
			return NULL;
		}

		// Parse Key
		const char *colon = memchr(current_line, ':', (size_t)(header_end - current_line));
		if (!colon) {
			raze_error("bad http header field");
			return NULL;
		}

		size_t key_len = (size_t)(colon - current_line);
		if (key_len >= HTTP_HEADER_FIELD_SIZE) {
			raze_error("bad http header key size");
			return NULL;
		}

		memcpy(request->headers[current_header].key, current_line, key_len);
		request->headers[current_header].key[key_len] = 0;

		// Parse Value
		const char *value_start = colon + 1;
		while (value_start < header_end && (*value_start == ' ' || *value_start == '\t')) {
			value_start++;
		}

		size_t value_len = (size_t)(header_end - value_start);
		if (value_len >= HTTP_HEADER_FIELD_SIZE) {
			raze_error("bad http header value size");
			return NULL;
		}

		memcpy(request->headers[current_header].value, value_start, value_len);
		request->headers[current_header].value[value_len] = 0;

		current_header++;
		current_line = header_end + 2;
	}

	return NULL;
}
