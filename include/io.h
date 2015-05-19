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
#define local_irq_disable()		cli()
#define local_irq_enable()		sei()
#define dmb()				__dmb()
#define dsb()				__dsb()
#define isb()				__isb()
#define register_isr(nirq, func)	__register_isr(nirq, func)

#define set_task_sp(sp)			__setusp(sp)
#define set_kernel_sp(sp)		__setksp(sp)

#ifdef MACHINE
#include <asm/io.h>
#endif

#include <syscall.h>

extern int stdin, stdout, stderr;

#define O_RDONLY	1
#define O_WRONLY	2
#define O_RDWR		3
#define O_NONBLOCK	4

extern int printf(const char *format, ...);
extern int printk(const char *format, ...);
extern void putc(int c);
extern int getc();
extern void puts(const char *s);

#endif /* __IO_H__ */
