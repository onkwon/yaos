#include <types.h>

void *memcpy(void *dst, const void *src, size_t len)
{
	const char *s = src;
	char *d = dst;

	while (len--) *d++ = *s++;

	return dst;
}
