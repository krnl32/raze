#define _GNU_SOURCE

#include "raze/server/static.h"
#include "raze/core/logger.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

static int raze_path_check(const char *root, const char *path);
static const char *raze_get_mime_from_path(const char *path);

int raze_static_handle(const struct raze_static *static_cfg, const struct raze_http_request *req, struct raze_http_response *res)
{
	size_t mount_len = strlen(static_cfg->mount);

	if (req->uri_len < mount_len || strncmp(req->uri, static_cfg->mount, mount_len) != 0) {
		return 1;
	}

	char requested_path[PATH_MAX];
	int ret = snprintf(requested_path, sizeof(requested_path), "%s/%.*s", static_cfg->root, (int)(req->uri_len - mount_len), req->uri + mount_len);
	if (ret < 0 || ret >= (int)sizeof(requested_path)) {
		raze_error("snprintf failed");
		res->status_code = RAZE_HTTP_INTERNAL_SERVER_ERROR;
		return -1;
	}

	char resolved_path[PATH_MAX];
	if (!realpath(requested_path, resolved_path)) {
		return 1;
	}

	if (!raze_path_check(static_cfg->root, resolved_path)) {
		res->status_code = RAZE_HTTP_FORBIDDEN;
		return -1;
	}

	char file_path[PATH_MAX];
	ret = snprintf(file_path, sizeof(file_path), "%s", resolved_path);
	if (ret < 0 || ret >= (int)sizeof(file_path)) {
		raze_error("snprintf failed");
		res->status_code = RAZE_HTTP_INTERNAL_SERVER_ERROR;
		return -1;
	}

	int fd = open(file_path, O_RDONLY);
	if (fd == -1) {
		return 1;
	}

	struct stat st;
	if (fstat(fd, &st) == -1) {
		perror("fstat");
		close(fd);
		res->status_code = RAZE_HTTP_INTERNAL_SERVER_ERROR;
		return -1;
	}

	if (S_ISDIR(st.st_mode)) {
		close(fd);

		ret = snprintf(file_path, sizeof(file_path), "%s/index.html", resolved_path);
		if (ret < 0 || ret >= (int)sizeof(file_path)) {
			raze_error("snprintf failed");
			res->status_code = RAZE_HTTP_INTERNAL_SERVER_ERROR;
			return -1;
		}

		fd = open(file_path, O_RDONLY);
		if (fd == -1) {
			return 1;
		}

		if (fstat(fd, &st) == -1) {
			perror("fstat");
			close(fd);
			res->status_code = RAZE_HTTP_INTERNAL_SERVER_ERROR;
			return -1;
		}
	}

	if (!S_ISREG(st.st_mode)) {
		close(fd);
		res->status_code = RAZE_HTTP_FORBIDDEN;
		return -1;
	}

	res->status_code = RAZE_HTTP_OK;

	const char *mime = raze_get_mime_from_path(file_path);
	raze_http_response_add_header(res, "Content-Type", mime);

	res->file_fd = fd;
	res->file_size = st.st_size;

	return 0;
}

static int raze_path_check(const char *root, const char *path)
{
	size_t len = strlen(root);
	return !strncmp(root, path, len) && (path[len] == '/' || path[len] == '\0');
}

static const char *raze_get_mime_from_path(const char *path)
{
	const char *ext = strrchr(path, '.');
	if (!ext) {
		return "application/octet-stream";
	}

	ext++;

	if (!strcmp(ext, "html")) {
		return "text/html";
	}

	if (!strcmp(ext, "htm")) {
		return "text/html";
	}

	if (!strcmp(ext, "css")) {
		return "text/css";
	}

	if (!strcmp(ext, "js")) {
		return "application/javascript";
	}

	if (!strcmp(ext, "json")) {
		return "application/json";
	}

	if (!strcmp(ext, "png")) {
		return "image/png";
	}

	if (!strcmp(ext, "jpg")) {
		return "image/jpeg";
	}

	if (!strcmp(ext, "jpeg")) {
		return "image/jpeg";
	}

	if (!strcmp(ext, "gif")) {
		return "image/gif";
	}

	if (!strcmp(ext, "svg")) {
		return "image/svg+xml";
	}

	if (!strcmp(ext, "txt")) {
		return "text/plain";
	}

	if (!strcmp(ext, "ico")) {
		return "image/x-icon";
	}

	return "application/octet-stream";
}
