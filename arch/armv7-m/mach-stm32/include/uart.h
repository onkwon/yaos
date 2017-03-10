#ifndef __STM32_USART_H__
#define __STM32_USART_H__

#include <io.h>

#define __get_uart_active_irq()					\
	((get_active_irq() < (53 + 12))? get_active_irq() - 53	\
		: get_active_irq() - 53 - 12)

int  __uart_open(unsigned int channel, unsigned int baudrate);
void __uart_close(unsigned int channel);
int __uart_putc(unsigned int channel, int c);
int __uart_getc(unsigned int channel);
int __uart_check_rx(unsigned int channel);
int __uart_check_tx(unsigned int channel);
void __uart_tx_irq_raise(unsigned int channel);
void __uart_tx_irq_reset(unsigned int channel);
void __uart_flush(unsigned int channel);
unsigned int __uart_get_baudrate(unsigned int channel);
int __uart_set_baudrate(unsigned int channel, unsigned int baudrate);

#endif /* __STM32_USART_H__ */
