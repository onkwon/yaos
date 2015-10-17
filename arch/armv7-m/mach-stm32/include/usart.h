#ifndef __STM32_USART_H__
#define __STM32_USART_H__

#include <io.h>

#define USART_CHANNEL_MAX	4

struct usart {
	/*unsigned int sr;*/
	/*unsigned int dr;*/
	unsigned int brr;
	unsigned int cr1;
	unsigned int cr2;
	unsigned int cr3;
	unsigned int gtpr;
};

/* 8N1 57600, make a default set of USART registers. */
	//.brr = (39 << 4) + 1,	/* 57600, PCLK1 36MHz */
#define USART_DEFAULT_SET()	((struct usart) {			\
	.brr = (78 << 4) + 2,						\
	.gtpr = 0,							\
	.cr3 = 0,							\
	.cr2 = 0,							\
	.cr1 = (1 << 13) 	/* UE    : USART enable */ 		\
		| (1 << 5)	/* RXNEIE: RXNE interrupt enable */	\
		| (1 << 3)	/* TE    : Transmitter enable */	\
		| (1 << 2)	/* RE    : Receiver enable */		\
})

#define __get_usart_active_irq()	(get_active_irq() - 53)

int  __usart_open(unsigned int channel, unsigned int baudrate);
void __usart_close(unsigned int channel);
void __usart_putc(unsigned int channel, int c);
int __usart_getc(unsigned int channel);
int __usart_check_rx(unsigned int channel);
int __usart_check_tx(unsigned int channel);
void __usart_tx_irq_raise(unsigned int channel);
void __usart_tx_irq_reset(unsigned int channel);
void __usart_flush(unsigned int channel);

#endif /* __STM32_USART_H__ */
