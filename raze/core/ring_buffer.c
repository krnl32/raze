#define _POSIX_C_SOURCE 199309L

#include "raze/core/ring_buffer.h"
#include "raze/core/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

int raze_ring_buffer_init(struct raze_ring_buffer *buffer, size_t capacity)
{
	size_t page_size = (size_t)(sysconf(_SC_PAGESIZE));
	if ((capacity & (capacity - 1)) != 0 || (capacity % page_size) != 0) {
		raze_error("Capacity must be power of 2 and multiple of page size");
		return -1;
	}

	int fd = shm_open("/raze_rb1", O_RDWR | O_CREAT | O_EXCL, 0600);
	if (fd == -1) {
		perror("shm_open");
		return -1;
	}
	shm_unlink("/raze_rb1");

	if (ftruncate(fd, (off_t)capacity) == -1) {
		perror("ftruncate");
		close(fd);
		return -1;
	}

	uint8_t *addr = mmap(NULL, capacity * 2, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (addr == MAP_FAILED) {
		perror("mmap reserve");
		close(fd);
		return -1;
	}

	if (mmap(addr, capacity, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0) == MAP_FAILED) {
		perror("mmap first half");
		close(fd);
		return -1;
	}

	if (mmap(addr + capacity, capacity, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0) == MAP_FAILED) {
		perror("mmap second half");
		close(fd);
		return -1;
	}

	close(fd);

	buffer->data = addr;
	buffer->capacity = capacity;
	buffer->mask = capacity - 1;
	buffer->head = 0;
	buffer->tail = 0;
	return 0;
}

void raze_ring_buffer_deinit(struct raze_ring_buffer *buffer)
{
	if (buffer && buffer->data) {
		munmap(buffer->data, buffer->capacity * 2);
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
	*ptr = buffer->data + head;
	return raze_ring_buffer_space(buffer);
}

size_t raze_ring_buffer_read(struct raze_ring_buffer *buffer, uint8_t **ptr)
{
	size_t tail = buffer->tail & buffer->mask;
	*ptr = buffer->data + tail;
	return raze_ring_buffer_size(buffer);
}

void raze_ring_buffer_consume(struct raze_ring_buffer *buffer, size_t count)
{
	buffer->tail += count;
	if (buffer->tail >= buffer->capacity && buffer->head >= buffer->capacity) {
		buffer->head &= buffer->mask;
		buffer->tail &= buffer->mask;
	}
}
