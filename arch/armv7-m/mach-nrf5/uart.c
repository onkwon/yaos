#include <uart.h>
#include "app_uart.h"

int __uart_open(unsigned int channel, unsigned int baudrate)
{
#if 0
	const app_uart_comm_params_t comm_params = {
		29, // RX
		30, // TX
		0, // RTS
		0, // CTS
		APP_UART_FLOW_CONTROL_DISABLE, // flow
		false, // parity
		UART_BAUDRATE_BAUDRATE_Baud115200 };

	APP_UART_FIFO_INIT(&comm_params,
			UART_RX_BUF_SIZE,
			UART_TX_BUF_SIZE,
			error_handler,
			IRQ_PRIORITY_DEFAULT,
			errcode);
#endif

	return 0;
}

void __uart_close(unsigned int channel)
{
}

int __uart_putc(unsigned int channel, int c)
{
	return 0;
}

int __uart_has_rx(unsigned int channel)
{
	return 0;
}

int __uart_has_tx(unsigned int channel)
{
	return 0;
}

int __uart_getc(unsigned int channel)
{
	return 0;
}

void __uart_tx_irq_reset(unsigned int channel)
{
}

void __uart_tx_irq_raise(unsigned int channel)
{
}

void __uart_flush(unsigned int channel)
{
}

unsigned int __uart_get_baudrate(unsigned int channel)
{
	return 0;
}

int __uart_set_baudrate(unsigned int channel, unsigned int baudrate)
{
	return 0;
}
