#ifndef __YAOS_BITMAP_H__
#define __YAOS_BITMAP_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	unsigned int max;
	uintptr_t *arr;
} bitmap_t;

/** Get a value of the bit in the bitmap array
 *
 * @param bitmap a pointer to `bitmap_t`
 * @param pos bit position to set or clear
 * @return negative on failure otherwise 1 or 0
 */
int bitmap_get(bitmap_t * const bitmap, const unsigned int pos);
/** Set or clear a bit in the bitmap array
 *
 * @param bitmap a pointer to `bitmap_t`
 * @param pos bit position to set or clear
 * @param torf true or false to set or clear
 * @return 0 on success
 */
int bitmap_set(bitmap_t * const bitmap,
		const unsigned int pos,
		const bool torf);
/** Initialize bitmap
 *
 * @param bitmap a pointer to `bitmap_t`
 * @param arr `uintptr_t` type array. It must be big enough to contain :c:data:`bitmax` number of bits
 * @param bitmax number of bits to allocate in the array
 * @param val initial value in the array
 * @return 0 on success
 */
int bitmap_init_static(bitmap_t * const bitmap,
		uintptr_t * const arr,
		const unsigned int bitmax,
		const bool val);
#if 0 // TODO: implement dynamic allocation for bitmap array
int bitmap_init(bitmap_t *const bitmap,
		const unsigned int bitmax,
		const bool val);
#endif

#endif /* __YAOS_BITMAP_H__ */
