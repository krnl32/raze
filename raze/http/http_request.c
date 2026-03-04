#include "raze/http/http_request.h"
#include "raze/core/utility.h"

#include <string.h>

const struct raze_http_request_header *raze_http_request_get_header(const struct raze_http_request *request, const char *key)
{
	size_t key_len = strlen(key);

	for (size_t i = 0; i < request->header_count; i++) {
		if (request->headers[i].key_len == key_len && !raze_strncasecmp(request->headers[i].key, key, key_len)) {
			return &request->headers[i];
		}
	}

	return NULL;
}

bool raze_http_request_keep_alive(const struct raze_http_request *request)
{
	const struct raze_http_request_header *connection = raze_http_request_get_header(request, "Connection");

	if (request->version == RAZE_HTTP_1_1) {
		if (connection && connection->value_len == 5 && raze_strncasecmp(connection->value, "close", 5) == 0) {
			return false;
		}
		return true;
	}

	if (request->version == RAZE_HTTP_1_0) {
		if (connection && connection->value_len == 10 && raze_strncasecmp(connection->value, "keep-alive", 10) == 0) {
			return true;
		}
		return false;
	}

	return false;
}
