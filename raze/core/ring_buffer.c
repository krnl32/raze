#include "raze/core/ring_buffer.h"
#include "raze/core/logger.h"

#include <stdio.h>
#include <stdlib.h>

int raze_ring_buffer_init(struct raze_ring_buffer *buffer, size_t capacity)
{
	if ((capacity & (capacity - 1)) != 0) {
		raze_error("raze_ring_buffer_init capacity must be pow of 2");
		return -1;
	}

	buffer->data = malloc(capacity);
	if (!buffer->data) {
		perror("malloc");
		return -1;
	}

	buffer->capacity = capacity;
	buffer->mask = capacity - 1;
	buffer->head = 0;
	buffer->tail = 0;
	return 0;
}

void raze_ring_buffer_deinit(struct raze_ring_buffer *buffer)
{
	if (buffer) {
		free(buffer->data);
		buffer->data = NULL;
	}
}

size_t raze_ring_buffer_size(struct raze_ring_buffer *buffer)
{
	return buffer->head - buffer->tail;
}

size_t raze_ring_buffer_space(struct raze_ring_buffer *buffer)
{
	return buffer->capacity - (buffer->head - buffer->tail);
}

size_t raze_ring_buffer_write(struct raze_ring_buffer *buffer, uint8_t **ptr)
{
	size_t head = buffer->head & buffer->mask;
	size_t tail = buffer->tail & buffer->mask;

	*ptr = (buffer->data + head);

	size_t space = buffer->capacity - (buffer->head - buffer->tail);
	size_t contiguous = (head >= tail) ? buffer->capacity - head : tail - head;
	return contiguous < space ? contiguous : space;
}

size_t raze_ring_buffer_read(struct raze_ring_buffer *buffer, uint8_t **ptr)
{
	size_t head = buffer->head & buffer->mask;
	size_t tail = buffer->tail & buffer->mask;

	*ptr = (buffer->data + tail);

	if (tail <= head) {
		return head - tail;
	}

	return buffer->capacity - tail;
}

void raze_ring_buffer_consume(struct raze_ring_buffer *buffer, size_t count)
{
	size_t size = buffer->head - buffer->tail;
	if (count > size) {
		raze_error("ring buffer consume overflow");
		count = size;
	}

	buffer->tail += count;
}
