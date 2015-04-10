void *memset(void *src, int c, int n)
{
	char *s = src;

	/* Truncate c to 8 bits */
	c = c & 0xFF;

	while (n--) *s++ = c;

	return src;
}
