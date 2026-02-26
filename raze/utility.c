#include "raze/utility.h"

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
