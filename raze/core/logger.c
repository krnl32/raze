#define _POSIX_C_SOURCE 200809L

#include "raze/core/logger.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

static const char *raze_logger_level_strings[] = { "INFO", "DEBUG", "WARN", "ERROR", "FATAL", "TRACE" };
static void raze_localtime(time_t *t, struct tm *out);

void raze_log(enum raze_logger_level level, const char *file, int line, const char *fmt, ...)
{
#ifdef NDEBUG
	if (level == RAZE_LOGGER_DEBUG || level == RAZE_LOGGER_TRACE) {
		return;
	}
#endif

	time_t now = time(NULL);
	struct tm tm;
	raze_localtime(&now, &tm);

	char timebuf[16];
	strftime(timebuf, sizeof(timebuf), "%H:%M:%S", &tm);

	char buf[1024];
	size_t offset = 0;

	int n;
#ifndef NDEBUG
	n = snprintf(buf + offset, sizeof(buf) - offset, "%s %-5s %s:%d  ", timebuf, raze_logger_level_strings[level], file, line);
#else
	n = snprintf(buf + offset, sizeof(buf) - offset, "%s %-5s ", timebuf, raze_logger_level_strings[level]);
#endif

	if (n > 0) {
		size_t written = (size_t)n;
		size_t remaining = sizeof(buf) - offset;

		if (written >= remaining) {
			offset = sizeof(buf) - 1;
		} else {
			offset += written;
		}
	}

	va_list ap;
	va_start(ap, fmt);
	n = vsnprintf(buf + offset, sizeof(buf) - offset, fmt, ap);
	va_end(ap);

	if (n > 0) {
		size_t written = (size_t)n;
		size_t remaining = sizeof(buf) - offset;

		if (written >= remaining) {
			offset = sizeof(buf) - 1;
		} else {
			offset += written;
		}
	}

	if (offset < sizeof(buf) - 1) {
		buf[offset++] = '\n';
	}

	write(STDERR_FILENO, buf, offset);

	if (level == RAZE_LOGGER_FATAL) {
		abort();
	}
}

static void raze_localtime(time_t *t, struct tm *out)
{
#if defined(_WIN32)
	localtime_s(out, t);
#elif defined(__unix__) || defined(__APPLE__)
	localtime_r(t, out);
#else
	struct tm *tmp = localtime(t);
	if (tmp) {
		*out = *tmp;
	}
#endif
}
