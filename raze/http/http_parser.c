#include "raze/http/http_parser.h"
#include "raze/core/logger.h"
#include "raze/core/utility.h"

#include <stdlib.h>
#include <string.h>

static const char *find_crlf(const char *data, size_t len);
static enum http_parser_result raze_http_parser_parse_request_line(struct raze_http_parser *parser, const char *data, size_t len);
static enum http_parser_result raze_http_parser_parse_headers(struct raze_http_parser *parser, const char *data, size_t len);
static enum http_parser_result raze_http_parser_parse_body(struct raze_http_parser *parser, const char *data, size_t len);
static enum http_parser_result raze_http_parser_parse_chunk_size(struct raze_http_parser *parser, const char *data, size_t len);
static enum http_parser_result raze_http_parser_parse_chunk_data(struct raze_http_parser *parser, const char *data, size_t len);

int raze_http_parser_init(struct raze_http_parser *parser)
{
	raze_http_parser_reset(parser);
	return 0;
}

void raze_http_parser_deinit(struct raze_http_parser *parser)
{
	(void)parser;
}

void raze_http_parser_reset(struct raze_http_parser *parser)
{
	parser->state = RAZE_HTTP_STATE_REQUEST_LINE;
	parser->buffer_pos = 0;
	parser->content_length = 0;
	parser->chunk_size = 0;
	parser->chunked_encoding = false;

	parser->request.uri = NULL;
	parser->request.uri_len = 0;
	parser->request.header_count = 0;
	parser->request.body = NULL;
	parser->request.body_len = 0;
	parser->request.keep_alive = false;
}

enum http_parser_result raze_http_parser_parse(struct raze_http_parser *parser, const char *data, size_t len)
{
	while (1) {
		enum http_parser_result res = RAZE_HTTP_RESULT_ERROR;

		switch (parser->state) {
			case RAZE_HTTP_STATE_REQUEST_LINE: {
				res = raze_http_parser_parse_request_line(parser, data, len);
				break;
			}
			case RAZE_HTTP_STATE_HEADERS: {
				res = raze_http_parser_parse_headers(parser, data, len);
				break;
			}
			case RAZE_HTTP_STATE_BODY: {
				res = raze_http_parser_parse_body(parser, data, len);
				break;
			}
			case RAZE_HTTP_STATE_CHUNK_SIZE: {
				res = raze_http_parser_parse_chunk_size(parser, data, len);
				break;
			}
			case RAZE_HTTP_STATE_CHUNK_DATA: {
				res = raze_http_parser_parse_chunk_data(parser, data, len);
				break;
			}
			case RAZE_HTTP_STATE_COMPLETE: {
				return RAZE_HTTP_RESULT_COMPLETE;
			}
			default: {
				raze_error("raze_http_parser_parse bad state");
				return RAZE_HTTP_RESULT_ERROR;
			}
		}

		if (res == RAZE_HTTP_RESULT_ERROR || res == RAZE_HTTP_RESULT_INCOMPLETE) {
			return res;
		}
	}
}

static const char *find_crlf(const char *data, size_t len)
{
	for (size_t i = 0; i + 1 < len; i++) {
		if (data[i] == '\r' && data[i + 1] == '\n') {
			return (data + i);
		}
	}

	return NULL;
}

static enum http_parser_result raze_http_parser_parse_request_line(struct raze_http_parser *parser, const char *data, size_t len)
{
	if (parser->buffer_pos > len) {
		return RAZE_HTTP_RESULT_ERROR;
	}

	// Check for a complete request line
	const char *start = data + parser->buffer_pos;
	size_t remaining = len - parser->buffer_pos;

	const char *crlf = find_crlf(start, remaining);
	if (!crlf) {
		return RAZE_HTTP_RESULT_INCOMPLETE;
	}

	// Parse Method
	size_t req_line_len = (size_t)(crlf - start);

	const char *sp1 = memchr(start, ' ', req_line_len);
	if (!sp1) {
		raze_error("bad http method");
		return RAZE_HTTP_RESULT_ERROR;
	}

	size_t method_len = (size_t)(sp1 - start);
	if (method_len == 3 && !memcmp(start, "GET", 3)) {
		parser->request.method = RAZE_HTTP_GET;
	} else if (method_len == 4 && !memcmp(start, "HEAD", 4)) {
		parser->request.method = RAZE_HTTP_HEAD;
	} else if (method_len == 4 && !memcmp(start, "POST", 4)) {
		parser->request.method = RAZE_HTTP_POST;
	} else if (method_len == 3 && !memcmp(start, "PUT", 3)) {
		parser->request.method = RAZE_HTTP_PUT;
	} else if (method_len == 6 && !memcmp(start, "DELETE", 6)) {
		parser->request.method = RAZE_HTTP_DELETE;
	} else if (method_len == 7 && !memcmp(start, "CONNECT", 7)) {
		parser->request.method = RAZE_HTTP_CONNECT;
	} else if (method_len == 7 && !memcmp(start, "OPTIONS", 7)) {
		parser->request.method = RAZE_HTTP_OPTIONS;
	} else if (method_len == 5 && !memcmp(start, "TRACE", 5)) {
		parser->request.method = RAZE_HTTP_TRACE;
	} else {
		raze_error("unsupported http method");
		return RAZE_HTTP_RESULT_ERROR;
	}

	// Parse URI
	const char *uri_start = (sp1 + 1);
	size_t req_line_len_from_uri = req_line_len - (size_t)(uri_start - start);

	const char *sp2 = memchr(uri_start, ' ', req_line_len_from_uri);
	if (!sp2) {
		raze_error("bad http uri");
		return RAZE_HTTP_RESULT_ERROR;
	}

	parser->request.uri = uri_start;
	parser->request.uri_len = (size_t)(sp2 - uri_start);

	// Parse HTTP Version
	const char *version_start = sp2 + 1;
	size_t version_len = (size_t)(crlf - version_start);

	if (version_len == 8 && !memcmp(version_start, "HTTP/1.1", 8)) {
		parser->request.version = RAZE_HTTP_1_1;
	} else if (version_len == 8 && !memcmp(version_start, "HTTP/1.0", 8)) {
		parser->request.version = RAZE_HTTP_1_0;
	} else {
		raze_error("unsupported http version");
		return RAZE_HTTP_RESULT_ERROR;
	}

	parser->buffer_pos += (req_line_len + 2);
	parser->state = RAZE_HTTP_STATE_HEADERS;
	return RAZE_HTTP_RESULT_COMPLETE;
}

static enum http_parser_result raze_http_parser_parse_headers(struct raze_http_parser *parser, const char *data, size_t len)
{
	if (parser->buffer_pos > len) {
		return RAZE_HTTP_RESULT_ERROR;
	}

	while (1) {
		const char *start = data + parser->buffer_pos;
		size_t remaining = len - parser->buffer_pos;

		// Check for a complete header line
		const char *crlf = find_crlf(start, remaining);
		if (!crlf) {
			return RAZE_HTTP_RESULT_INCOMPLETE;
		}

		// End of Headers
		if (crlf == start) {
			parser->request.keep_alive = raze_http_request_keep_alive(&parser->request);

			if (parser->chunked_encoding) {
				parser->state = RAZE_HTTP_STATE_CHUNK_SIZE;
			} else if (parser->content_length > 0) {
				parser->state = RAZE_HTTP_STATE_BODY;
			} else {
				parser->state = RAZE_HTTP_STATE_COMPLETE;
			}

			parser->buffer_pos += 2;
			return RAZE_HTTP_RESULT_COMPLETE;
		}

		if (parser->request.header_count >= RAZE_HTTP_REQUEST_HEADERS_MAX) {
			raze_error("bad http header count");
			return RAZE_HTTP_RESULT_ERROR;
		}

		// Parse Header Line
		struct raze_http_request_header *current_header = &parser->request.headers[parser->request.header_count];
		size_t header_line_len = (size_t)(crlf - start);

		// Parse Key
		const char *colon = memchr(start, ':', header_line_len);
		if (!colon) {
			raze_error("bad http header line");
			return RAZE_HTTP_RESULT_ERROR;
		}

		current_header->key = start;
		current_header->key_len = (size_t)(colon - start);

		// Parse Value
		const char *value = colon + 1;
		while (*value == ' ' || *value == '\t') {
			value++;
		}

		current_header->value = value;
		current_header->value_len = (size_t)(crlf - value);

		parser->request.header_count++;

		// MISC Hold Content-Length
		if (current_header->key_len == 14 && !raze_strncasecmp(current_header->key, "Content-Length", 14)) {
			parser->content_length = strtoul(value, NULL, 10);

			if (parser->content_length > RAZE_HTTP_REQUEST_BODY_MAX) {
				raze_error("bad http content length");
				return RAZE_HTTP_RESULT_ERROR;
			}
		}

		// Handle Encoding Type
		if (current_header->key_len == 17 && !raze_strncasecmp(current_header->key, "Transfer-Encoding", 17) && current_header->value_len == 7 && !raze_strncasecmp(value, "chunked", 7)) {
			parser->chunked_encoding = true;
		}

		parser->buffer_pos += (header_line_len + 2);
	}
}

static enum http_parser_result raze_http_parser_parse_body(struct raze_http_parser *parser, const char *data, size_t len)
{
	if (parser->buffer_pos > len) {
		return RAZE_HTTP_RESULT_ERROR;
	}

	size_t remaining = len - parser->buffer_pos;
	if (remaining < parser->content_length) {
		return RAZE_HTTP_RESULT_INCOMPLETE;
	}

	parser->request.body = data + parser->buffer_pos;
	parser->request.body_len = parser->content_length;

	parser->buffer_pos += parser->content_length;
	parser->state = RAZE_HTTP_STATE_COMPLETE;
	return RAZE_HTTP_RESULT_COMPLETE;
}

static enum http_parser_result raze_http_parser_parse_chunk_size(struct raze_http_parser *parser, const char *data, size_t len)
{
	if (parser->buffer_pos > len) {
		return RAZE_HTTP_RESULT_ERROR;
	}

	const char *start = data + parser->buffer_pos;
	size_t remaining = len - parser->buffer_pos;

	const char *crlf = find_crlf(start, remaining);
	if (!crlf) {
		return RAZE_HTTP_RESULT_INCOMPLETE;
	}

	size_t chunk_line_len = (size_t)(crlf - start);
	parser->chunk_size = strtoul(start, NULL, 16);
	parser->buffer_pos += (chunk_line_len + 2);

	if (parser->chunk_size == 0) {
		parser->state = RAZE_HTTP_STATE_COMPLETE;
		return RAZE_HTTP_RESULT_COMPLETE;
	}

	parser->state = RAZE_HTTP_STATE_CHUNK_DATA;
	return RAZE_HTTP_RESULT_COMPLETE;
}

static enum http_parser_result raze_http_parser_parse_chunk_data(struct raze_http_parser *parser, const char *data, size_t len)
{
	if (parser->buffer_pos > len) {
		return RAZE_HTTP_RESULT_ERROR;
	}

	size_t remaining = len - parser->buffer_pos;
	if (remaining < parser->chunk_size + 2) {
		return RAZE_HTTP_RESULT_INCOMPLETE;
	}

	if (!parser->request.body) {
		parser->request.body = data + parser->buffer_pos;
	}

	parser->request.body_len += parser->chunk_size;

	if (data[parser->buffer_pos + parser->chunk_size] != '\r' || data[parser->buffer_pos + parser->chunk_size + 1] != '\n') {
		return RAZE_HTTP_RESULT_ERROR;
	}

	parser->buffer_pos += parser->chunk_size + 2;
	parser->state = RAZE_HTTP_STATE_CHUNK_SIZE;
	return RAZE_HTTP_RESULT_COMPLETE;
}
