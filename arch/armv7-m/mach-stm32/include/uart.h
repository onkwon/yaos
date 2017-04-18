#ifndef __STM32_UART_H__
#define __STM32_UART_H__

#include <io.h>
#include <drivers/uart.h>

#define __get_uart_channel_active(nvec)			\
	((nvec < (53 + 12))? nvec - 53 : nvec - 53 - 12)

int  __uart_open(int channel, struct uart conf);
void __uart_close(int channel);
int __uart_putc(int channel, int c);
int __uart_getc(int channel);
bool __uart_has_rx(int channel);
bool __uart_has_tx(int channel);
void __uart_tx_irq_raise(int channel);
void __uart_tx_irq_reset(int channel);
void __uart_flush(int channel);
unsigned int __uart_get_baudrate(int channel);
int __uart_set_baudrate(int channel, unsigned int baudrate);

#endif /* __STM32_UART_H__ */
