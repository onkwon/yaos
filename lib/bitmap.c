#include "bitmap.h"
#include <errno.h>

int bitmap_get(bitmap_t * const bitmap, const unsigned int pos)
{
	if (bitmap->max < (pos + 1))
		return -ERANGE;

	if (!bitmap->arr)
		return -ENOENT;

	uintptr_t val = bitmap->arr[pos / sizeof(uintptr_t)]
		& (1UL << (pos % sizeof(uintptr_t)));

	return (int)(val != 0);
}

int bitmap_set(bitmap_t * const bitmap, const unsigned int pos, const bool torf)
{
	if (bitmap->max < (pos + 1))
		return -ERANGE;

	if (!bitmap->arr)
		return -EINVAL;

	bitmap->arr[pos / sizeof(uintptr_t)]
		&= ~(1UL << (pos % sizeof(uintptr_t)));
	bitmap->arr[pos / sizeof(uintptr_t)]
		|= (1UL * torf) << (pos % sizeof(uintptr_t));

	return 0;
}

int bitmap_init_static(bitmap_t * const bitmap, uintptr_t * const arr,
		const unsigned int bitmax, const bool val)
{
	if (!arr)
		return -EINVAL;

	bitmap->max = bitmax;
	bitmap->arr = arr;

	int nword = bitmax / sizeof(uintptr_t);
	int remained = bitmax % sizeof(uintptr_t);

	for (int i = 0; i < nword; i++)
		bitmap->arr[i] = (-1UL * val);

	if (remained) {
		uintptr_t t = 0;

		for (int i = 0; i < remained; i++)
			t = (t << 1UL) | (1UL * val);

		bitmap->arr[nword] = t;
	}

	return 0;
}
