#ifndef _RAZE_HTTP_PARSER_H
#define _RAZE_HTTP_PARSER_H

#include "raze/http/http_request.h"

enum http_parser_result {
	RAZE_HTTP_RESULT_ERROR = -1,
	RAZE_HTTP_RESULT_COMPLETE = 0,
	RAZE_HTTP_RESULT_INCOMPLETE = 1
};

enum raze_http_parser_state {
	RAZE_HTTP_STATE_REQUEST_LINE,
	RAZE_HTTP_STATE_HEADERS,
	RAZE_HTTP_STATE_BODY,
	RAZE_HTTP_STATE_CHUNK_SIZE,
	RAZE_HTTP_STATE_CHUNK_DATA,
	RAZE_HTTP_STATE_COMPLETE
};

struct raze_http_parser {
	enum raze_http_parser_state state;
	size_t buffer_pos;
	size_t content_length;
	size_t chunk_size;
	bool chunked_encoding;

	struct raze_http_request request;
};

int raze_http_parser_init(struct raze_http_parser *parser);
void raze_http_parser_deinit(struct raze_http_parser *parser);
void raze_http_parser_reset(struct raze_http_parser *parser);
enum http_parser_result raze_http_parser_parse(struct raze_http_parser *parser, const char *data, size_t len);

#endif
