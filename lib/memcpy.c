void *memcpy(void *dst, const void *src, int len)
{
	const char *s = src;
	char *d = dst;

	while (len--) *d++ = *s++;

	return dst;
}
