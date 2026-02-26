#include "logger.h"
#include "server.h"
#include "raze/http_request.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
	const char *simple_request = "POST /cgi-bin/process.cgi HTTP/1.1\r\n"
								 "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n"
								 "Host: www.tutorialspoint.com\r\n"
								 "Content-Type: text/xml; charset=utf-8\r\n"
								 "Content-Length: 95\r\n" // Updated to reflect the XML body length
								 "Accept-Language: en-us\r\n"
								 "Accept-Encoding: gzip, deflate\r\n"
								 "Connection: Keep-Alive\r\n"
								 "\r\n" // Critical empty line between headers and body
								 "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
								 "<string xmlns=\"http://clearforest.com/\">string</string>";

	struct raze_http_request *request = raze_http_request_create(simple_request, strlen(simple_request));
	if (!request) {
		return -1;
	}

	raze_info("METHOD: %d, URI: %s, VERSION: %d", request->method, request->uri, request->version);
	printf("Body: %s\n", request->body);

	// struct raze_socket sockconfig;
	// sockconfig.domain = AF_INET;
	// sockconfig.type = SOCK_STREAM;
	// sockconfig.protocol = 0;
	// sockconfig.host = INADDR_ANY;
	// sockconfig.port = 53422;
	// sockconfig.backlog = 255;
	//
	// struct raze_server* server = raze_server_create(&sockconfig);
	// if (!server) {
	// 	raze_error("raze_server_create failed");
	// 	return -1;
	// }
	//
	// raze_server_run(server);
	// raze_server_destroy(server);
	return 0;
}
