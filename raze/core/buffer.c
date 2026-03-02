#include "raze/core/buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	if ((buffer->size + size) > buffer->capacity) {
		size_t new_capacity = buffer->capacity > 0 ? buffer->capacity : RAZE_BUFFER_DEFAULT_CAPACITY;

		while (buffer->size + size > new_capacity) {
			new_capacity *= 2;
		}

		void *tmp = realloc(buffer->data, new_capacity);
		if (!tmp) {
			perror("realloc");
			return -1;
		}

		buffer->data = tmp;
		buffer->capacity = new_capacity;
	}

	memcpy(&buffer->data[buffer->size], data, size);
	buffer->size += size;
	return 0;
}

void raze_buffer_clear(struct raze_buffer *buffer)
{
	buffer->size = 0;
}
