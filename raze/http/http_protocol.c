#include "raze/http/http_protocol.h"

const char *raze_http_version_to_string(enum raze_http_version version)
{
	switch (version) {
		case RAZE_HTTP_0_9:
			return "HTTP/0.9";
		case RAZE_HTTP_1_0:
			return "HTTP/1.0";
		case RAZE_HTTP_1_1:
			return "HTTP/1.1";
		case RAZE_HTTP_2:
			return "HTTP/2";
		case RAZE_HTTP_3:
			return "HTTP/3";
		default:
			return "HTTP/1.1";
	}
}

const char *raze_http_status_code_to_string(enum raze_http_status_code status_code)
{
	switch (status_code) {
		// 1xx
		case RAZE_HTTP_CONTINUE:
			return "Continue";
		case RAZE_HTTP_SWITCH_PROTOCOL:
			return "Switching Protocols";
		case RAZE_HTTP_PROCESSING:
			return "Processing";
		case RAZE_HTTP_EARLY_HINTS:
			return "Early Hints";

		// 2xx
		case RAZE_HTTP_OK:
			return "OK";
		case RAZE_HTTP_CREATED:
			return "Created";
		case RAZE_HTTP_ACCEPTED:
			return "Accepted";
		case RAZE_HTTP_NON_AUTHORITATIVE_INFORMATION:
			return "Non-Authoritative Information";
		case RAZE_HTTP_NO_CONTENT:
			return "No Content";
		case RAZE_HTTP_RESET_CONTENT:
			return "Reset Content";
		case RAZE_HTTP_PARTIAL_CONTENT:
			return "Partial Content";

		// 3xx
		case RAZE_HTTP_MULTIPLE_CHOICES:
			return "Multiple Choices";
		case RAZE_HTTP_MOVED_PERMANENTLY:
			return "Moved Permanently";
		case RAZE_HTTP_FOUND:
			return "Found";
		case RAZE_HTTP_SEE_OTHER:
			return "See Other";
		case RAZE_HTTP_NOT_MODIFIED:
			return "Not Modified";
		case RAZE_HTTP_TEMPORARY_REDIRECT:
			return "Temporary Redirect";
		case RAZE_HTTP_PERMANENT_REDIRECT:
			return "Permanent Redirect";

		// 4xx
		case RAZE_HTTP_BAD_REQUEST:
			return "Bad Request";
		case RAZE_HTTP_UNAUTHORIZED:
			return "Unauthorized";
		case RAZE_HTTP_PAYMENT_REQUIRED:
			return "Payment Required";
		case RAZE_HTTP_FORBIDDEN:
			return "Forbidden";
		case RAZE_HTTP_NOT_FOUND:
			return "Not Found";
		case RAZE_HTTP_METHOD_NOT_ALLOWED:
			return "Method Not Allowed";
		case RAZE_HTTP_NOT_ACCEPTABLE:
			return "Not Acceptable";
		case RAZE_HTTP_PROXY_AUTHENTICATION_REQUIRED:
			return "Proxy Authentication Required";
		case RAZE_HTTP_REQUEST_TIMEOUT:
			return "Request Timeout";
		case RAZE_HTTP_CONFLICT:
			return "Conflict";
		case RAZE_HTTP_GONE:
			return "Gone";
		case RAZE_HTTP_LENGTH_REQUIRED:
			return "Length Required";
		case RAZE_HTTP_PRECONDITION_FAILED:
			return "Precondition Failed";
		case RAZE_HTTP_PAYLOAD_TOO_LARGE:
			return "Payload Too Large";
		case RAZE_HTTP_URI_TOO_LONG:
			return "URI Too Long";
		case RAZE_HTTP_UNSUPPORTED_MEDIA_TYPE:
			return "Unsupported Media Type";
		case RAZE_HTTP_RANGE_NOT_SATISFIABLE:
			return "Range Not Satisfiable";
		case RAZE_HTTP_EXPECTATION_FAILED:
			return "Expectation Failed";
		case RAZE_HTTP_IM_A_TEAPOT:
			return "I'm a teapot";
		case RAZE_HTTP_UNPROCESSABLE_ENTITY:
			return "Unprocessable Entity";
		case RAZE_HTTP_TOO_MANY_REQUESTS:
			return "Too Many Requests";

		// 5xx
		case RAZE_HTTP_INTERNAL_SERVER_ERROR:
			return "Internal Server Error";
		case RAZE_HTTP_NOT_IMPLEMENTED:
			return "Not Implemented";
		case RAZE_HTTP_BAD_GATEWAY:
			return "Bad Gateway";
		case RAZE_HTTP_SERVICE_UNAVAILABLE:
			return "Service Unavailable";
		case RAZE_HTTP_GATEWAY_TIMEOUT:
			return "Gateway Timeout";
		case RAZE_HTTP_HTTP_VERSION_NOT_SUPPORTED:
			return "HTTP Version Not Supported";

		default:
			return "Unknown";
	}
}
