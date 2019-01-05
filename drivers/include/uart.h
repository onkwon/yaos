#ifndef __YAOS_UART_H__
#define __YAOS_UART_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

enum uart_channel {
	UART1			= 0,
	UART2,
	UART3,
	UART4,
	UART5,
	UART6,
	UART7,
	UART8,
	UART_MAX_CHANNEL,
};

enum uart_mode {
	UART_DISABLED		= 0x00,
	UART_INTERRUPT		= 0x01,
	UART_POLLING		= 0x02,
	UART_NONBLOCK		= 0x04,
};

enum uart_flow {
	UART_FLOW_NONE		= 0,
	UART_FLOW_HARD,
	UART_FLOW_SOFT,
};

enum uart_parity {
	UART_PARITY_NONE	= 0,
	UART_PARITY_EVEN,
	UART_PARITY_ODD,
};

struct uart_conf {
	enum uart_mode rx, tx;
	enum uart_flow flow;
	enum uart_parity parity;
	bool rts, cts;
	uint32_t baudrate;
};

typedef struct uart_t {
	enum uart_channel ch;
	struct uart_conf conf;

	int (*open_static)(const struct uart_t * const self,
			void *rxbuf, int rxbufsize,
			void *txbuf, int txbufsize);
	int (*open)(const struct uart_t * const self,
			int rxbufsize, int txbufsize);
	int (*writeb)(const struct uart_t * const self, const uint8_t byte);
	long (*write)(const struct uart_t * const self,
			const void * const data, size_t len);
	/** Read a byte
	 *
	 * @param self A pointer to an instance of :c:data:`uart_t`
	 * @param byte A buffer to store byte string
	 * @return 0 on success
	 */
	int (*readb)(const struct uart_t * const self, void * const byte);
	long (*read)(const struct uart_t * const self,
			void * const buf, size_t len);
	void (*flush)(const struct uart_t * const self);
	bool (*kbhit)(const struct uart_t * const self);
} uart_t;

#define UART_DEFAULT_CONF() {				\
	.rx = UART_INTERRUPT | UART_NONBLOCK,		\
	.tx = UART_POLLING,				\
	.flow = UART_FLOW_NONE,				\
	.parity = UART_PARITY_NONE,			\
	.cts = false,					\
	.rts = false,					\
	.baudrate = 115200UL,				\
}

uart_t uart_new(const enum uart_channel ch);

#include "arch/mach/hw_uart.h"

#endif /* __YAOS_UART_H__ */
