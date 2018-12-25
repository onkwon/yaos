#ifndef __YAOS_HASH_H__
#define __YAOS_HASH_H__

#include <stdint.h>

/* [djb2](http://www.cse.yorku.ca/~oz/hash.html) by Dan Bernstein */
static inline unsigned long hashstr(const void *s)
{
	const unsigned char *str = s;
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

/* http://burtleburtle.net/bob/hash/integer.html */
static inline uint32_t hash32(uint32_t a)
{
	a = (a ^ 61) ^ (a >> 16);
	a = a + (a << 3);
	a = a ^ (a >> 4);
	a = a * 0x27d4eb2d;
	a = a ^ (a >> 15);

	return a;
}

static inline uint32_t hash(uint32_t x)
{
	return hash32(x);
}

#endif /* __YAOS_HASH_H__ */
