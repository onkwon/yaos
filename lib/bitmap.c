// Implement lock-free
#include "bitmap.h"
#include <errno.h>

unsigned int bitmap_count(const bitmap_t * const bitmap, const unsigned bits)
{
	unsigned int cnt = 0;
	int nword = bits / BITMAP_UNIT;
	int remain = bits % BITMAP_UNIT;

	for (int i = 0; i < nword; i++) {
		for (int j = 0; bitmap[i] >> j; j++)
			if ((bitmap[i] >> j) & 1U)
				cnt++;
	}

	for (int i = 0; (i < remain) && (bitmap[nword] >> i); i++) {
		if ((bitmap[nword] >> i) & 1U)
			cnt++;
	}

	return cnt;
}

bool bitmap_get(const bitmap_t * const bitmap, const unsigned int pos)
{
	bitmap_t val = bitmap[pos / BITMAP_UNIT]
		& (1UL << (pos % BITMAP_UNIT));

	return val != 0;
}

void bitmap_set(bitmap_t * const bitmap, const unsigned int pos, const bool zerone)
{
	bitmap[pos / BITMAP_UNIT]
		&= ~(1UL << (pos % BITMAP_UNIT));
	bitmap[pos / BITMAP_UNIT]
		|= (1UL * zerone) << (pos % BITMAP_UNIT);
}

void bitmap_init(bitmap_t * const bitmap, const unsigned int bitmax, const bool val)
{
	int nword = bitmax / BITMAP_UNIT;
	int remained = bitmax % BITMAP_UNIT;

	for (int i = 0; i < nword; i++)
		bitmap[i] = (-1UL * val);

	if (remained) {
		bitmap_t t = 0;

		for (int i = 0; i < remained; i++)
			t = (t << 1UL) | (1UL * val);

		bitmap[nword] = t;
	}
}
