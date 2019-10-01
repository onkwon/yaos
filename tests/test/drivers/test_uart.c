#include "unity.h"
#include "drivers/uart.h"
#include "queue.h"
#include "kernel/interrupt.h"
#include <stdint.h>
#include <errno.h>

#include "mock_hw_uart.h"
#include "mock_hw_interrupt.h"
#include "mock_hw_io.h"
#include "mock_lock.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_uart1(void)
{
	hw_uart_open_IgnoreAndReturn(0);
	hw_uart_close_Ignore();

	int rxbuf;
	uart_t uart1 = uart_new(UART1);
	uart1.conf = (struct uart_conf) {
		.rx = UART_INTERRUPT | UART_NONBLOCK,
		.tx = UART_POLLING,
		.flow = UART_FLOW_NONE,
		.parity = UART_PARITY_NONE,
		.cts = false,
		.rts = false,
		.baudrate = 115200UL };
	uart1.open_static(&uart1, &rxbuf, sizeof(rxbuf), NULL, 0);
	uart1.close(&uart1);
}

void test_uart_open_static(void)
{
	int rxbuf, txbuf;
	uart_t uart;

	// Return -EINVAL when channel is not supported
	uart = uart_new(UART_MAX_CHANNEL);
	TEST_ASSERT_EQUAL(-EINVAL, uart.open_static(&uart, &rxbuf, sizeof(rxbuf), NULL, 0));
	uart = uart_new(-1);
	TEST_ASSERT_EQUAL(-EINVAL, uart.open_static(&uart, &rxbuf, sizeof(rxbuf), NULL, 0));

	// Return -EINVAL when NULL
	TEST_ASSERT_EQUAL(-EINVAL, uart.open_static(NULL, &rxbuf, sizeof(rxbuf), NULL, 0));

	uart = uart_new(UART1);
	// Return -EINVAL when rx interrupt is enabled and rxbuf is null
	TEST_ASSERT_EQUAL(-EINVAL, uart.open_static(&uart, NULL, sizeof(rxbuf), NULL, 0));
	// Return -EINVAL when rx interrupt is enabled and rxbufsize is 0 or less
	TEST_ASSERT_EQUAL(-EINVAL, uart.open_static(&uart, &rxbuf, 0, NULL, 0));
	TEST_ASSERT_EQUAL(-EINVAL, uart.open_static(&uart, &rxbuf, -1, NULL, 0));
	// Return -EINVAL when tx interrupt is enabled and txbuf is null
	uart.conf.tx = UART_INTERRUPT;
	TEST_ASSERT_EQUAL(-EINVAL, uart.open_static(&uart, &rxbuf, sizeof(rxbuf), NULL, 0));
	// Return -EINVAL when tx interrupt is enabled and txbufsize is 0 or less
	TEST_ASSERT_EQUAL(-EINVAL, uart.open_static(&uart, &rxbuf, sizeof(rxbuf), &txbuf, 0));
	TEST_ASSERT_EQUAL(-EINVAL, uart.open_static(&uart, &rxbuf, sizeof(rxbuf), &txbuf, -1));
	hw_uart_open_IgnoreAndReturn(0);
	TEST_ASSERT_EQUAL(0, uart.open_static(&uart, &rxbuf, sizeof(rxbuf), &txbuf, sizeof(txbuf)));
	TEST_ASSERT_EQUAL(-EEXIST, uart.open_static(&uart, &rxbuf, sizeof(rxbuf), &txbuf, sizeof(txbuf)));
	hw_uart_close_Ignore();
	uart.close(&uart);
	// Return 0 when set properly in rx only interrupt mode
	for (int i = UART1; i < UART_MAX_CHANNEL; i++) {
		uart = uart_new(i);
		hw_uart_open_IgnoreAndReturn(0);
		TEST_ASSERT_EQUAL(0, uart.open_static(&uart, &rxbuf, sizeof(rxbuf), NULL, 0));
		hw_uart_close_Ignore();
		uart.close(&uart);
	}
	// Return 0 when set properly in rx/tx interrupt mode
	uart.conf.tx = UART_INTERRUPT;
	// Return 0 when set properly in rx only polling mode
	uart.conf.rx = UART_POLLING;
	// Return 0 when set properly in rx/tx polling mode
	uart.conf.rx = UART_POLLING;
	uart.conf.tx = UART_POLLING;
	// Return 0 when set properly in nonblocking interrupt mode
	uart.conf.rx = UART_INTERRUPT | UART_NONBLOCK;
	uart.conf.tx = UART_INTERRUPT | UART_NONBLOCK;
	// Return 0 when set properly in nonblocking polling mode
	uart.conf.rx = UART_POLLING | UART_NONBLOCK;
	uart.conf.tx = UART_POLLING | UART_NONBLOCK;
}

void test_uart_read(void)
{
	TEST_IGNORE_MESSAGE("Implement me!");
}

void test_uart_write(void)
{
	TEST_IGNORE_MESSAGE("Implement me!");
}

void test_uart_open(void)
{
	TEST_IGNORE_MESSAGE("Implement me!");
}
