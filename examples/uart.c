#include "drivers/uart.h"

#include "syslog.h"
#include "kernel/task.h"

static void uart_test(void)
{
	uint8_t rxbuf[32], txbuf[1];
	int rc;

	uart_t uart1 = uart_new(UART1);
	rc = uart1.open_static(&uart1, rxbuf, 32, txbuf, 1);
	if (rc != 0)
		debug("err opening uart %x", rc);

	uint8_t c;

	while (1) {
		if (uart1.readb(&uart1, &c) == 0)
			uart1.writeb(&uart1, c);
	}
}
REGISTER_TASK(uart_test, TASK_KERNEL, 0, STACK_SIZE_MIN);
