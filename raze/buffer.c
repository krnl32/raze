#include "raze/buffer.h"

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

void raze_buffer_append(struct raze_buffer *buffer, const char *data, size_t len)
{
	if ((buffer->size + len) > buffer->capacity) {
		size_t new_capacity = buffer->capacity;

		while (buffer->size + len > new_capacity) {
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

	memcpy(&buffer->data[buffer->size], data, len);
	buffer->size += len;
}
