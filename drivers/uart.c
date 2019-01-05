#include "drivers/uart.h"
#include "kernel/interrupt.h"
#include "kernel/lock.h"
#include "queue.h"
#include <errno.h>
#include <assert.h>
#include <string.h>

static struct _uart {
	queue_t rxq, txq;
	uint16_t rxerr, txerr;
	uint16_t rxovf;
	bool active;
} _uart[UART_MAX_CHANNEL];

static void ISR_uart(int vector)
{
	struct _uart *p;
	int channel;
	uint8_t data;

	vector = get_active_irq_from_isr(vector);
	channel = hw_uart_get_event_source((unsigned int)vector);
	p = &_uart[channel];

	assert(channel >= UART1 && channel < UART_MAX_CHANNEL);
	assert(p);

	if (hw_uart_has_received(channel)) {
		if (hw_uart_readb(channel, &data)
				|| !queue_is_initialized(&p->rxq))
			p->rxerr++;
		else if (enqueue(&p->rxq, &data))
			p->rxovf++;
	}

	if (queue_is_initialized(&p->txq) && hw_uart_is_tx_ready(channel)) {
		if (queue_empty(&p->txq)) {
			hw_uart_disable_tx_interrupt(channel);
		} else {
			if (dequeue(&p->txq, &data))
				p->txerr++;
			else if (hw_uart_writeb(channel, data))
				p->txerr++;
		}
	}
}

/* TODO: Implement
static int uart_open(const uart_t * const self, int rxbufsize, int txbufsize)
*/

static int uart_open_static(const uart_t * const self,
		void *rxbuf, int rxbufsize,
		void *txbuf, int txbufsize)
{
	struct _uart *p;
	int vector;
	int rc = -EINVAL;

	if (!self || (UART_MAX_CHANNEL <= (unsigned int)self->ch)
			|| !(p = &_uart[self->ch]))
		goto out;

	if (p->active) {
		rc = -EEXIST;
		goto out;
	}

	if ((UART_INTERRUPT & self->conf.rx)) {
		if (!rxbuf || (0 >= rxbufsize))
			goto out;

		queue_init_static(&p->rxq,
				rxbuf, rxbufsize, 1U);
	}
	if (UART_INTERRUPT & self->conf.tx) {
		if (!txbuf || (0 >= txbufsize))
			goto out;

		queue_init_static(&p->txq,
				txbuf, txbufsize, 1U);
	}

	// FIXME: lock
	// mutex_lock();
	p->active = true;
	vector = hw_uart_open(self->ch, self->conf);
	// mutex_unlock();

	/* FIXME: Interrupt must be prevented before its handler registered.
	 * If an interrupt occurs first before its isr registered, the
	 * interrupt is going to be handled by the default isr doing noting on
	 * it. that means the interrupt keeps interrupting normal tasks making
	 * it never take places. so registering the actual isr below never ever
	 * happen and the whole system gets stuck. */
	if (vector > 0)
		register_isr(vector, ISR_uart);

	rc = 0;
out:
	return rc;
}

static bool uart_kbhit(const uart_t * const self)
{
	struct _uart *p;

	if (!self || (UART_MAX_CHANNEL <= (unsigned int)self->ch))
		return false;

	if (UART_INTERRUPT & self->conf.rx) {
		p = &_uart[self->ch];

		if (!p || !queue_is_initialized(&p->rxq))
			return false;

		if (queue_empty(&p->rxq))
			return false;
	} else if (UART_POLLING & self->conf.rx) {
		if (!hw_uart_has_received(self->ch))
			return false;
	}

	return true;
}

static void uart_flush(const uart_t * const self)
{
	struct _uart *p;

	if (!self || (UART_MAX_CHANNEL <= (unsigned int)self->ch))
		return;

	if (!(p = &_uart[self->ch]))
		return;

	// rx flush
	if (queue_is_initialized(&p->rxq))
		queue_flush(&p->rxq);

	// tx flush
	hw_uart_flush(self->ch);
	if (queue_is_initialized(&p->txq))
		queue_flush(&p->txq);
}

static int uart_writeb(const uart_t * const self, const uint8_t byte)
{
	int rc = -EINVAL;

	if (!self || (UART_MAX_CHANNEL <= (unsigned int)self->ch))
		goto out;

	if (UART_INTERRUPT & self->conf.tx) {
		struct _uart *p = &_uart[self->ch];
		if (!p || !p->active)
			goto out;

		do {
			rc = enqueue(&p->txq, &byte);
		} while (rc && !(UART_NONBLOCK & self->conf.tx));
	} else if (UART_POLLING & self->conf.tx) {
		do {
			// FIXME: Lock!
			rc = hw_uart_writeb(self->ch, byte);
		} while (rc && !(UART_NONBLOCK & self->conf.tx));
	}

out:
	return rc;
}

static long uart_write(const uart_t * const self,
		const void * const data, size_t len)
{
	size_t i = 0;
	const uint8_t *byte = data;

	if (!byte)
		goto out;

	for (i = 0; i < len; i++) {
		if (uart_writeb(self, byte[i]))
			break;
	}
out:
	return i;
}

static int uart_readb(const uart_t * const self, void *ptrb)
{
	int rc = -EINVAL;

	if (!self || (UART_MAX_CHANNEL <= (unsigned int)self->ch))
		goto out;

	if (UART_INTERRUPT & self->conf.rx) {
		struct _uart *p = &_uart[self->ch];
		if (!p || !p->active)
			goto out;

		do {
			rc = dequeue(&p->rxq, ptrb);
		} while (rc && !(UART_NONBLOCK & self->conf.rx));
	} else if (UART_POLLING & self->conf.rx) {
		do {
			// FIXME: Lock!
			rc = hw_uart_readb(self->ch, ptrb);
		} while (rc && !(UART_NONBLOCK & self->conf.rx));
	}

out:
	return rc;
}

static long uart_read(const uart_t * const self, void * const buf, size_t len)
{
	size_t i = 0;
	uint8_t *byte = buf;

	if (!byte)
		goto out;

	for (i = 0; i < len; i++) {
		while (uart_readb(self, &byte[i]));
	}
out:
	return i;
}

static void uart_close(const uart_t * const self)
{
	struct _uart *p;

	if (!self || (UART_MAX_CHANNEL <= (unsigned int)self->ch))
		return;

	if (!(p = &_uart[self->ch]) || !p->active)
		return;

	// FIXME: Lock!
	hw_uart_close(self->ch);
	memset(p, 0, sizeof(*p));
}

uart_t uart_new(const enum uart_channel ch)
{
	assert(ch < UART_MAX_CHANNEL);

	return (uart_t) {
		.ch = ch,
		.conf = UART_DEFAULT_CONF(),
		.open_static = uart_open_static,
		.flush = uart_flush,
		.kbhit = uart_kbhit,
		.writeb = uart_writeb,
		.write = uart_write,
		.readb = uart_readb,
		.read = uart_read,
		.close = uart_close,
	};
}
