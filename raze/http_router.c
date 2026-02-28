#include "raze/http_router.h"

#include <string.h>

void http_router_route(const struct raze_http_request *req, struct raze_http_response *res)
{
	if (req->method == RAZE_HTTP_GET && req->uri_len == 1 && req->uri[0] == '/') {
		res->version = req->version;
		res->status_code = RAZE_HTTP_OK;

		raze_http_response_add_header(res, "Content-Type", "text/html");

		const char *body = "<h1>Home</h1>";
		size_t body_len = strlen(body);

		memcpy(res->body, body, body_len);
		res->body_len = body_len;
	} else if (req->method == RAZE_HTTP_GET && req->uri_len == 6 && !memcmp(req->uri, "/hello", 6)) {
		res->version = req->version;
		res->status_code = RAZE_HTTP_OK;

		raze_http_response_add_header(res, "Content-Type", "text/html");

		const char *body = "<h1>Hello Raze</h1>";
		size_t body_len = strlen(body);

		memcpy(res->body, body, body_len);
		res->body_len = body_len;
	} else {
		res->version = req->version;
		res->status_code = RAZE_HTTP_NOT_FOUND;

		raze_http_response_add_header(res, "Content-Type", "text/plain");

		const char *body = "404 Not Found";
		size_t body_len = strlen(body);

		memcpy(res->body, body, body_len);
		res->body_len = body_len;
	}
}
