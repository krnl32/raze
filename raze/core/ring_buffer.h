#ifndef _RAZE_RING_BUFFER_H
#define _RAZE_RING_BUFFER_H

#define RAZE_RING_BUFFER_DEFAULT_CAPACITY 16384

#include <stddef.h>
#include <stdint.h>

struct raze_ring_buffer {
	uint8_t *data;
	size_t capacity;
	size_t mask;
	size_t head;
	size_t tail;
};

int raze_ring_buffer_init(struct raze_ring_buffer *buffer, size_t capacity);
void raze_ring_buffer_deinit(struct raze_ring_buffer *buffer);
size_t raze_ring_buffer_size(struct raze_ring_buffer *buffer);
size_t raze_ring_buffer_space(struct raze_ring_buffer *buffer);
size_t raze_ring_buffer_write(struct raze_ring_buffer *buffer, uint8_t **ptr);
size_t raze_ring_buffer_read(struct raze_ring_buffer *buffer, uint8_t **ptr);
void raze_ring_buffer_consume(struct raze_ring_buffer *buffer, size_t count);

#endif
