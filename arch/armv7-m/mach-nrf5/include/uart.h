#ifndef __NRF5_UART_H__
#define __NRF5_UART_H__

#include <error.h>
/* to not include nrf_assert.h */
#define NRF_ASSERT_H_
#define ASSERT		assert
#define APP_ERROR_H__

#define __get_uart_channel_active(nvec)		nvec

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

#endif /* __NRF5_H__ */
