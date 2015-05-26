#ifndef __HASH_H__
#define __HASH_H__

#include <foundation.h>

static inline unsigned int hash(unsigned int val, unsigned int bits)
{
	unsigned int hash = val * HASH_CONSTANT;
	return hash >> (WORD_BITS - bits);
}

#endif /* __HASH_H__ */
