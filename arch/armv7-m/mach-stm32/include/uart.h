#ifndef __YAOS_STM32_UART_H__
#define __YAOS_STM32_UART_H__

#define __get_uart_channel_active(nvec)	\
	((nvec < 68U) ? nvec - 53 : \
	 (nvec < 87U) ? nvec - 68 + 3 : \
	 (nvec < 98U) ? nvec - 87 + 5 : \
	 nvec - 98 + 6);

int  __uart_open(const int channel, struct uart conf);
void __uart_close(const int channel);
int __uart_putc(const int channel, const int c);
int __uart_getc(const int channel);
bool __uart_has_rx(const int channel);
bool __uart_has_tx(const int channel);
void __uart_tx_irq_raise(const int channel);
void __uart_tx_irq_reset(const int channel);
void __uart_flush(const int channel);
unsigned int __uart_get_baudrate(const int channel);
int __uart_set_baudrate(const int channel, unsigned int baudrate);

#endif /* __YAOS_STM32_UART_H__ */
