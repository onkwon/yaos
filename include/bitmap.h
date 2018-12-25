#ifndef __YAOS_BITMAP_H__
#define __YAOS_BITMAP_H__

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

typedef uintptr_t bitmap_t;

/** Bitmap unit size in bytes, that shows each one in array can hold at most */
#define BITMAP_UNIT			(sizeof(bitmap_t) * CHAR_BIT)
#define BITMAP_SIZE(nbits)		((nbits) / BITMAP_UNIT)
#define BITMAP_ARRAY_SIZE(nbits)	\
	(BITMAP_SIZE(nbits) + (1 * !!((nbits) % BITMAP_UNIT)))

/** Define a bitmap
 * @param name name of bitmap
 * @param nbits number of bits to use
 */
#define DEFINE_BITMAP(name, nbits)				\
	bitmap_t name[BITMAP_ARRAY_SIZE(nbits)]

/** Get a value of bit in the bitmap array
 *
 * @param bitmap a pointer to `bitmap_t`
 * @param pos bit position to set or clear
 * @return 1 or 0
 */
bool bitmap_get(const bitmap_t * const bitmap, const unsigned int pos);
/** Set or clear a bit in the bitmap array
 *
 * @param bitmap a pointer to `bitmap_t`
 * @param pos bit position to set or clear
 * @param zerone zero or one to clear or set
 */
void bitmap_set(bitmap_t * const bitmap,
		const unsigned int pos,
		const bool zerone);
/** Count number of bits set in a bitmap
 *
 * @param bitmap a pointer to `bitmap_t`
 * @param bits number of bits used
 * @return number of bits set
 */
unsigned int bitmap_count(const bitmap_t * const bitmap, const unsigned bits);
/** Initialize bitmap
 *
 * @param bitmap a pointer to `bitmap_t`
 * @param bitmax number of bits to initialize in the array
 * @param val initial value in the array
 */
void bitmap_init(bitmap_t * const bitmap, const unsigned int bitmax, const bool val);

#endif /* __YAOS_BITMAP_H__ */
