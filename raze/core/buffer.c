#include "raze/core/buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct raze_buffer *raze_buffer_create(void)
{
	struct raze_buffer *buffer = malloc(sizeof(struct raze_buffer));
	if (!buffer) {
		perror("malloc");
		return NULL;
	}

	buffer->data = malloc(RAZE_BUFFER_DEFAULT_CAPACITY);
	if (!buffer->data) {
		perror("malloc");
		free(buffer);
		return NULL;
	}

	buffer->size = 0;
	buffer->capacity = RAZE_BUFFER_DEFAULT_CAPACITY;
	return buffer;
}

void raze_buffer_destroy(struct raze_buffer *buffer)
{
	if (buffer) {
		free(buffer->data);
		free(buffer);
	}
}

void raze_buffer_append(struct raze_buffer *buffer, const void *data, size_t size)
{
	if ((buffer->size + size) > buffer->capacity) {
		size_t new_capacity = buffer->capacity > 0 ? buffer->capacity : RAZE_BUFFER_DEFAULT_CAPACITY;

		while (buffer->size + size > new_capacity) {
			new_capacity *= 2;
		}

		void *tmp = realloc(buffer->data, new_capacity);
		if (!tmp) {
			perror("realloc");
			return;
		}

		buffer->data = tmp;
		buffer->capacity = new_capacity;
	}

	memcpy(&buffer->data[buffer->size], data, size);
	buffer->size += size;
}

void raze_buffer_clear(struct raze_buffer *buffer)
{
	buffer->size = 0;
}
