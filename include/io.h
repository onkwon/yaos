#ifndef __IO_H__
#define __IO_H__

#define MASK(v, mask)			((v) & (mask))
#define MASK_RESET(v, mask)		((v) & ~(mask))
#define MASK_SET(v, mask)		((v) | (mask))

#define sbi(v, bit)			(v |= (1 << (bit)))
#define cbi(v, bit)			(v &= ~(1 << (bit)))
#define gbi(v, bit)			(((v) >> (bit)) & 1)

#define barrier()			__asm__ __volatile__("" ::: "memory")

#define O_RDONLY			1
#define O_WRONLY			2
#define O_RDWR				3
#define O_NONBLOCK			4

#ifdef MACHINE
#include <asm/io.h>
#endif

int stdin, stdout, stderr;

#include <types.h>

extern size_t printf(const char *format, ...);
extern size_t printk(const char *format, ...);
extern void (*putchar)(int c);
extern int getc();
extern void puts(const char *s);

#include <kernel/interrupt.h>
#include <kernel/gpio.h>

#endif /* __IO_H__ */
