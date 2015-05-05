#ifndef __STM32_USART_H__
#define __STM32_USART_H__

#include <io.h>

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

int  __usart_open(unsigned baudrate);
void __usart_close();
void __usart_putc(int c);
int __usart_getc();
int __usart_check_rx();
int __usart_check_tx();
void __usart_tx_irq_raise();
void __usart_tx_irq_reset();

#endif /* __STM32_USART_H__ */
