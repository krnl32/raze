#ifndef _RAZE_BUFFER_H
#define _RAZE_BUFFER_H
#include <stddef.h>

#define RAZE_BUFFER_DEFAULT_CAPACITY 4096

struct raze_buffer {
	char *data;
	size_t size;
	size_t capacity;
};

struct raze_buffer *raze_buffer_create(void);
void raze_buffer_destroy(struct raze_buffer *buffer);
void raze_buffer_append(struct raze_buffer *buffer, const char *data, size_t len);

#endif
