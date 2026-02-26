#include "raze/logger.h"

#include <stdio.h>
#include <stdlib.h>

static const char *raze_logger_level_strings[] = { "INFO", "DEBUG", "WARN", "ERROR", "FATAL", "TRACE" };

static void raze_log_stdout(struct raze_logger *logger);

void raze_log(enum raze_logger_level level, const char *file, int line, const char *fmt, ...)
{
#ifdef NDEBUG
	if (level == RAZE_LOGGER_DEBUG)
		return;
#endif

	time_t t = time(NULL);

	struct raze_logger logger;
	logger.level = level;
	logger.file = file;
	logger.line = line;
	logger.fmt = fmt;
	logger.time = localtime(&t);
	logger.stream = stderr;

	va_start(logger.ap, fmt);
	raze_log_stdout(&logger);
	va_end(logger.ap);

	if (level == RAZE_LOGGER_FATAL) {
		abort();
	}
}

static void raze_log_stdout(struct raze_logger *logger)
{
	char time_str[32];
	size_t ret = strftime(time_str, sizeof(time_str), "%H:%M:%S", logger->time);
	time_str[ret] = 0;

#ifdef NDEBUG
	fprintf(logger->stream, "%s %-5s ", time_str, raze_logger_level_strings[logger->level]);
#else
	fprintf(logger->stream, "%s %-5s %s:%d  ", time_str, raze_logger_level_strings[logger->level], logger->file, logger->line);
#endif

	vfprintf(logger->stream, logger->fmt, logger->ap);
	fprintf(logger->stream, "\n");
	fflush(logger->stream);
}
