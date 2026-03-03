#include "raze/core/buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int raze_buffer_init(struct raze_buffer *buffer)
{
	buffer->data = malloc(RAZE_BUFFER_DEFAULT_CAPACITY);
	if (!buffer->data) {
		perror("malloc");
		return -1;
	}

	buffer->size = 0;
	buffer->capacity = RAZE_BUFFER_DEFAULT_CAPACITY;
	return 0;
}

void raze_buffer_deinit(struct raze_buffer *buffer)
{
	if (buffer) {
		free(buffer->data);
	}
}

int raze_buffer_append(struct raze_buffer *buffer, const void *data, size_t size)
{
	if (raze_buffer_reserve(buffer, size) == -1) {
		return -1;
	}

	memcpy(&buffer->data[buffer->size], data, size);
	buffer->size += size;
	return 0;
}

void raze_buffer_clear(struct raze_buffer *buffer)
{
	buffer->size = 0;
}

int raze_buffer_reserve(struct raze_buffer *buffer, size_t size)
{
	if (size > SIZE_MAX - buffer->size) {
		return -1;
	}

	size_t total_required = buffer->size + size;
	if (total_required <= buffer->capacity) {
		return 0;
	}

	size_t new_capacity = buffer->capacity > 0 ? buffer->capacity : RAZE_BUFFER_DEFAULT_CAPACITY;
	while (new_capacity < total_required) {
		if (new_capacity > SIZE_MAX / 2) {
			return -1;
		}

		new_capacity *= 2;
	}

	void *tmp = realloc(buffer->data, new_capacity);
	if (!tmp) {
		perror("realloc");
		return -1;
	}

	buffer->data = tmp;
	buffer->capacity = new_capacity;
	return 0;
}
