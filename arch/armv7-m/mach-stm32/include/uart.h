#ifndef __STM32_UART_H__
#define __STM32_UART_H__

#include <io.h>

#define __get_uart_channel_active(nvec)			\
	((nvec < (53 + 12))? nvec - 53 : nvec - 53 - 12)

int  __uart_open(unsigned int channel, unsigned int baudrate);
void __uart_close(unsigned int channel);
int __uart_putc(unsigned int channel, int c);
int __uart_getc(unsigned int channel);
int __uart_has_rx(unsigned int channel);
int __uart_has_tx(unsigned int channel);
void __uart_tx_irq_raise(unsigned int channel);
void __uart_tx_irq_reset(unsigned int channel);
void __uart_flush(unsigned int channel);
unsigned int __uart_get_baudrate(unsigned int channel);
int __uart_set_baudrate(unsigned int channel, unsigned int baudrate);

#endif /* __STM32_UART_H__ */
