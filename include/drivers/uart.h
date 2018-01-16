#ifndef __UART_H__
#define __UART_H__

#include <types.h>

enum uart_parity {
	UART_PARITY_NONE	= 0,
	UART_PARITY_EVEN	= 1,
	UART_PARITY_ODD		= 2,
};

struct uart {
	int npin_rx, npin_tx;
	bool rx, tx;
	size_t rxbuf, txbuf;

	int npin_rts;
	int npin_cts;
	bool flow;

	enum uart_parity parity;
	int baudrate;
};

#endif /* __UART_H__ */
