#include "raze/core/utility.h"

#include <ctype.h>

int raze_strcasecmp(const char *s1, const char *s2)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;

	while (*p1 != '\0' && (tolower(*p1) == tolower(*p2))) {
		p1++;
		p2++;
	}

	return tolower(*p1) - tolower(*p2);
}

int raze_strncasecmp(const char *s1, const char *s2, size_t n)
{
	if (n == 0) {
		return 0;
	}

	while (n-- > 0 && tolower((unsigned char)*s1) == tolower((unsigned char)*s2)) {
		if (n == 0 || *s1 == '\0') {
			break;
		}

		s1++;
		s2++;
	}

	return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}
