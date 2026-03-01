#ifndef _RAZE_HTTP_UTILITY_H
#define _RAZE_HTTP_UTILITY_H

#include <stdint.h>
#include <unistd.h>

ssize_t raze_http_utility_find_header_end(const uint8_t* data, size_t size);
size_t raze_http_utility_extract_content_length(const uint8_t* data, size_t size);

#endif
