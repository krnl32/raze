#ifndef _RAZE_LOGGER_H
#define _RAZE_LOGGER_H

#include <stdarg.h>
#include <time.h>

enum raze_logger_level {
	RAZE_LOGGER_INFO,
	RAZE_LOGGER_DEBUG,
	RAZE_LOGGER_WARN,
	RAZE_LOGGER_ERROR,
	RAZE_LOGGER_FATAL,
	RAZE_LOGGER_TRACE
};

struct raze_logger {
	enum raze_logger_level level;
	const char *file;
	int line;
	const char *fmt;
	va_list ap;
	struct tm *time;
	void *stream;
};

#define raze_info(...) raze_log(RAZE_LOGGER_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define raze_debug(...) raze_log(RAZE_LOGGER_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define raze_warn(...) raze_log(RAZE_LOGGER_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define raze_error(...) raze_log(RAZE_LOGGER_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define raze_fatal(...) raze_log(RAZE_LOGGER_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#define raze_trace(...) raze_log(RAZE_LOGGER_TRACE, __FILE__, __LINE__, __VA_ARGS__)

void raze_log(enum raze_logger_level level, const char *file, int line, const char *fmt, ...);

#endif
