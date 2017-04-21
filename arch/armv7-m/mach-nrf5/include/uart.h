#ifndef __NRF5_UART_H__
#define __NRF5_UART_H__

#include <error.h>
/* to not include nrf_assert.h */
#define NRF_ASSERT_H_
#define ASSERT		assert
#define APP_ERROR_H__

#include <types.h>

#ifndef __IO_H__
extern size_t printk(const char *format, ...);
#endif

#define __get_uart_channel_active(nvec)		0

#include <drivers/uart.h>

int __uart_open(int channel, struct uart conf);
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

#endif /* __NRF5_H__ */
