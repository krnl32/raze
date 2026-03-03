#include "raze/core/logger.h"
#include "raze/server/server.h"

int main(void)
{
	struct raze_socket sockconfig;
	sockconfig.domain = AF_INET;
	sockconfig.type = SOCK_STREAM;
	sockconfig.protocol = 0;
	sockconfig.host = INADDR_ANY;
	sockconfig.port = 8080;
	sockconfig.backlog = 255;

	struct raze_server *server = raze_server_create(&sockconfig);
	if (!server) {
		raze_error("raze_server_create failed");
		return -1;
	}

	if (raze_server_run(server) == -1) {
		raze_server_destroy(server);
		return -1;
	}

	raze_server_destroy(server);
	return 0;
}
