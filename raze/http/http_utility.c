#include "raze/http/http_utility.h"
#include "raze/core/utility.h"

#include <stdlib.h>

ssize_t raze_http_utility_find_header_end(const uint8_t *data, size_t size)
{
	for (size_t i = 0; i + 3 < size; i++) {
		if (data[i] == '\r' && data[i + 1] == '\n' && data[i + 2] == '\r' && data[i + 3] == '\n') {
			return (ssize_t)(i + 4);
		}
	}

	return -1;
}

size_t raze_http_utility_extract_content_length(const uint8_t *data, size_t size)
{
	const char *key = "Content-Length:";
	size_t key_len = 15;

	for (size_t i = 0; i + key_len < size; i++) {
		if (!raze_strncasecmp((const char *)(data + i), key, key_len)) {
			size_t j = i + key_len;

			while (j < size && (data[j] == ' ' || data[j] == '\t')) {
				j++;
			}

			return strtoul((const char *)(data + j), NULL, 10);
		}
	}

	return 0;
}
