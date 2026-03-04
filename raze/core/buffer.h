#ifndef _RAZE_BUFFER_H
#define _RAZE_BUFFER_H

#include <stdint.h>
#include <stddef.h>

#define RAZE_BUFFER_DEFAULT_CAPACITY 4096

struct raze_buffer {
	uint8_t *data;
	size_t size;
	size_t capacity;
};

int raze_buffer_init(struct raze_buffer *buffer);
void raze_buffer_deinit(struct raze_buffer *buffer);
int raze_buffer_append(struct raze_buffer *buffer, const void *data, size_t size);
void raze_buffer_clear(struct raze_buffer *buffer);
int raze_buffer_reserve(struct raze_buffer *buffer, size_t size);

#endif
