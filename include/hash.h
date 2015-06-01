#ifndef __HASH_H__
#define __HASH_H__

#define HASH_CONSTANT		0x9e370001UL

static inline unsigned int hash(unsigned int val, unsigned int bits)
{
	unsigned int hash = val * HASH_CONSTANT;
	return hash >> (WORD_BITS - bits);
}

#endif /* __HASH_H__ */
