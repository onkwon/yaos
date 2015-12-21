#ifndef __IO_H__
#define __IO_H__

#define MASK(v, mask)			((v) & (mask))
#define MASK_RESET(v, mask)		((v) & ~(mask))
#define MASK_SET(v, mask)		((v) | (mask))

#define sbi(v, bit)			(v |= (1 << (bit)))
#define cbi(v, bit)			(v &= ~(1 << (bit)))
#define gbi(v, bit)			(((v) >> (bit)) & 1)

#define barrier()			__asm__ __volatile__("" ::: "memory")

#define O_RDONLY			0x01
#define O_WRONLY			0x02
#define O_RDWR				(O_RDONLY | O_WRONLY)
#define O_NONBLOCK			0x04
#define O_CREATE			0x08

#ifdef MACHINE
#include <asm/io.h>
#endif

int stdin, stdout, stderr;

#include <types.h>

extern size_t printk(const char *format, ...);
extern size_t printf(const char *format, ...);
extern void putc(int c);
extern void (*putchar)(int c);
extern int getc();
extern int (*getchar)();
extern void puts(const char *s);

#include <kernel/interrupt.h>
#include <kernel/gpio.h>

/* usart ioctl */
#define USART_FLUSH			1
#define USART_KBHIT			2
#define USART_GET_BAUDRATE		3
#define USART_SET_BAUDRATE		4

#endif /* __IO_H__ */
