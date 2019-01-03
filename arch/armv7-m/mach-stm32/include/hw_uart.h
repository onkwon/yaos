#ifndef __YAOS_STM32_UART_H__
#define __YAOS_STM32_UART_H__

#include "drivers/uart.h"

#define hw_uart_get_event_source(nvec)	\
	(((nvec) < 68U) ? (nvec) - 53 : \
	 ((nvec) < 87U) ? (nvec) - 68 + 3 : \
	 ((nvec) < 98U) ? (nvec) - 87 + 5 : \
	 (nvec) - 98 + 6)

int hw_uart_open(const int channel, struct uart conf);
void hw_uart_close(const int channel);
int hw_uart_putb(const int channel, const uint8_t byte);
int hw_uart_getb(const int channel, uint8_t * const byte);
void hw_uart_flush(const int channel);
int hw_uart_baudrate_set(const int channel, uintptr_t baudrate);
uintptr_t hw_uart_baudrate_get(const int channel);
bool hw_uart_has_received(const int channel);
bool hw_uart_is_tx_ready(const int channel);
void hw_uart_disable_tx_interrupt(const int channel);
void hw_uart_raise_tx_interrupt(const int channel);

#endif /* __YAOS_STM32_UART_H__ */
