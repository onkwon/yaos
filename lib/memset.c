#include <types.h>

void *memset(void *src, int c, size_t len)
{
	char *s = src;

	/* Truncate c to 8 bits */
	c = c & 0xFF;

	while (len--) *s++ = c;

	return src;
}
