#include <usart.h>
#include <stdlib.h>
#include <foundation.h>
#include <gpio.h>
#include <error.h>

#define SYSTEM_CLOCK		500000000 /* 500MHz */

int __usart_open(unsigned int channel, unsigned int baudrate)
{
	if (channel)
		return -ERR_RANGE;

	struct aux *aux = (struct aux *)AUX_BASE;
	struct mini_uart *uart = (struct mini_uart *)UART_BASE;
	struct gpio *gpio = (struct gpio *)GPIO_BASE;
	unsigned int i;

	aux->enable = 1; /* enable mini-uart */
	uart->cntl  = 0;
	uart->lcr   = 3; /* 8-bit data */
	uart->iir   = 0xc6;
	uart->baudrate = (SYSTEM_CLOCK / (baudrate * 8)) - 1;

	SET_GPIO_FS(14, GPIO_ALT5);
	SET_GPIO_FS(15, GPIO_ALT5);
	gpio->pud = 0; /* 0: off, 1: pull down, 2: pull up */
	for (i = 0; i < 150; i++) __nop(); /* wait 150 cycles for set-up time */
	gpio->pudclk0 = 1 << 14 | 1 << 15;
	for (i = 0; i < 150; i++) __nop(); /* hold time */
	gpio->pudclk0 = 0; /* clean */

	uart->cntl = 2;

	return 1;
}

void __usart_close(unsigned int channel)
{
}

void __usart_putc(unsigned int channel, int c)
{
	struct mini_uart *uart = (struct mini_uart *)UART_BASE;

	while (!(uart->lsr & 0x20)) ;
	uart->dr = c;
}

int __usart_check_rx(unsigned int channel)
{
	return 0;
}

int __usart_check_tx(unsigned int channel)
{
	return 0;
}

int __usart_getc(unsigned int channel)
{
	return 0;
}

void __usart_tx_irq_reset(unsigned int channel)
{
}

void __usart_tx_irq_raise(unsigned int channel)
{
}

void __putc_debug(int c)
{
	__usart_putc(0, c);

	if (c == '\n')
		__usart_putc(0, '\r');
}

void __usart_flush(unsigned int channel)
{
}
