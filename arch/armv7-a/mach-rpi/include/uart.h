#ifndef __RPI_USART_H__
#define __RPI_USART_H__

#include <io.h>

#define USART_CHANNEL_MAX		1
#define __get_uart_active_irq()	0

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

#endif /* __RPI_USART_H__ */
