#ifndef __USART_H__
#define __USART_H__

#include "io.h"

struct usart_t {
	/*unsigned sr;*/
	/*unsigned dr;*/
	unsigned brr;
	unsigned cr1;
	unsigned cr2;
	unsigned cr3;
	unsigned gtpr;
};

/* 8N1 57600, make a default set of USART registers. */
	//.brr = (39 << 4) + 1,	/* 57600, PCLK1 36MHz */
#define USART_DEFAULT_SET()	((struct usart_t) {			\
	.brr = (78 << 4) + 2,						\
	.gtpr = 0,							\
	.cr3 = 0,							\
	.cr2 = 0,							\
	.cr1 = (1 << 13) 	/* UE    : USART enable */ 		\
		| (1 << 5)	/* RXNEIE: RXNE interrupt enable */	\
		| (1 << 3)	/* TE    : Transmitter enable */	\
		| (1 << 2)	/* RE    : Receiver enable */		\
})

void usart_open  (unsigned channel, struct usart_t arg);
void usart_close (unsigned channel);
void usart_putc  (unsigned channel, int c);
void __usart_putc(unsigned channel, int c);
int  usart_getc  (unsigned channel);
int  usart_kbhit (unsigned channel);
void usart_fflush(unsigned channel);
unsigned brr2reg (unsigned baudrate, unsigned clk);

#endif /* __USART_H__ */
