#ifndef __IO_H__
#define __IO_H__

#define MASK(v, mask)			((v) & (mask))
#define MASK_RESET(v, mask)		((v) & ~(mask))
#define MASK_SET(v, mask)		((v) | (mask))

#define sbi(v, bit)			(v |= (1 << (bit)))
#define cbi(v, bit)			(v &= ~(1 << (bit)))
#define gbi(v, bit)			(((v) >> (bit)) & 1)

#define barrier()			__asm__ __volatile__("" ::: "memory")

#include <asm/interrupt.h>

#define irq_save(flag)			__irq_save(flag)
#define irq_restore(flag)		__irq_restore(flag)
#define cli()				__cli()
#define sei()				__sei()
#define dmb()				__dmb()
#define dsb()				__dsb()
#define isb()				__isb()
#define register_isr(nirq, func)	__register_isr(nirq, func)

#ifdef MACHINE
#include <asm/io.h>
#endif

#include <syscall.h>

extern int stdin, stdout, stderr;

#define O_RDONLY	1
#define O_WRONLY	2
#define O_RDWR		3
#define O_NONBLOCK	4

#endif /* __IO_H__ */
