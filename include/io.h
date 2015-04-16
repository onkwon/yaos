#ifndef __IO_H__
#define __IO_H__

#define MASK(v, mask)			((v) & (mask))
#define MASK_RESET(v, mask)		((v) & ~(mask))
#define MASK_SET(v, mask)		((v) | (mask))

#define sbi(v, bit)			(v |= (1 << (bit)))
#define cbi(v, bit)			(v &= ~(1 << (bit)))
#define gbi(v, bit)			(((v) >> (bit)) & 1)

#ifdef MACHINE
#include <asm/io.h>
#endif

#endif /* __IO_H__ */
