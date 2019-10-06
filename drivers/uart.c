#include "drivers/uart.h"
#include "kernel/interrupt.h"
#include "kernel/lock.h"
#include "queue.h"
#include <errno.h>
#include <assert.h>
#include <string.h>

static struct uart_handle {
	queue_t rxq, txq;
	uint16_t rxerr, txerr;
	uint16_t rxovf;
	bool active;
	lock_t lock;
} handles[UART_MAX_CHANNEL];

static void ISR_uart(int vector)
{
	struct uart_handle *p;
	int channel;
	uint8_t data;

	vector = get_active_irq_from_isr(vector);
	channel = hw_uart_get_event_source((unsigned int)vector);
	p = &handles[channel];

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
	struct uart_handle *p;
	int vector;
	int rc = -EINVAL;

	if (!self || ( (unsigned int)self->ch >= UART_MAX_CHANNEL)
			|| !(p = &handles[self->ch]))
		goto out;

	mutex_lock(&p->lock);

	if (p->active) {
		rc = -EEXIST;
		goto out_unlock;
	}

	if (self->conf.rx & UART_INTERRUPT) {
		if (!rxbuf || (rxbufsize <= 0))
			goto out_unlock;

		queue_init_static(&p->rxq,
				rxbuf, rxbufsize, 1U);
	}
	if (self->conf.tx & UART_INTERRUPT) {
		if (!txbuf || (txbufsize <= 0))
			goto out_unlock;

		queue_init_static(&p->txq,
				txbuf, txbufsize, 1U);
	}

	vector = hw_uart_open(self->ch, self->conf);
	/* FIXME: Interrupt must be prevented before its handler registered.
	 * If an interrupt occurs first before its isr registered, the
	 * interrupt is going to be handled by the default isr doing noting on
	 * it. that means the interrupt keeps interrupting normal tasks making
	 * it never take places. so registering the actual isr below never ever
	 * happen and the whole system gets stuck. */
	if (vector > 0)
		register_isr(vector, ISR_uart);

	p->active = true;
	rc = 0;
out_unlock:
	mutex_unlock(&p->lock);
out:
	return rc;
}

static bool uart_kbhit(const uart_t * const self)
{
	struct uart_handle *p;
	bool rc = false;

	if (!self || ((unsigned int)self->ch >= UART_MAX_CHANNEL))
		goto out;

	if (!(p = &handles[self->ch]))
		goto out;

	mutex_lock(&p->lock);

	if (!p->active)
		goto out_unlock;

	if (self->conf.rx & UART_INTERRUPT) {
		if (!queue_is_initialized(&p->rxq))
			goto out_unlock;

		if (queue_empty(&p->rxq))
			goto out_unlock;
	} else if (self->conf.rx & UART_POLLING) {
		if (!hw_uart_has_received(self->ch))
			goto out_unlock;
	}

	rc = true;
out_unlock:
	mutex_unlock(&p->lock);
out:
	return rc;
}

static void uart_flush(const uart_t * const self)
{
	struct uart_handle *p;

	if (!self || ((unsigned int)self->ch >= UART_MAX_CHANNEL))
		return;

	if (!(p = &handles[self->ch]))
		return;

	mutex_lock(&p->lock);

	if (!p->active)
		goto out_unlock;

	// rx flush
	if (queue_is_initialized(&p->rxq))
		queue_flush(&p->rxq);

	// tx flush
	hw_uart_flush(self->ch);
	if (queue_is_initialized(&p->txq))
		queue_flush(&p->txq);

out_unlock:
	mutex_unlock(&p->lock);
}

static int uart_writeb(const uart_t * const self, const uint8_t byte)
{
	struct uart_handle *p;
	int rc = -EINVAL;

	if (!self || ((unsigned int)self->ch >= UART_MAX_CHANNEL))
		goto out;

	if (!(p = &handles[self->ch]))
		goto out;

	mutex_lock(&p->lock);

	if (!p->active)
		goto out_unlock;

	if (self->conf.tx & UART_INTERRUPT) {
		do {
			rc = enqueue(&p->txq, &byte);
		} while (rc && !(self->conf.tx & UART_NONBLOCK));
	} else if (self->conf.tx & UART_POLLING) {
		do {
			/* Don't need to disable the local interrupts as the
			 * code below never gets executed when tx is set as
			 * UART_INTERRUPT. Mutual exclusive needed only for
			 * multi threads */
			rc = hw_uart_writeb(self->ch, byte);
		} while (rc && !(self->conf.tx & UART_NONBLOCK));
	}

out_unlock:
	mutex_unlock(&p->lock);
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
	struct uart_handle *p;
	int rc = -EINVAL;

	if (!self || ((unsigned int)self->ch >= UART_MAX_CHANNEL))
		goto out;

	if (!(p = &handles[self->ch]))
		goto out;

	mutex_lock(&p->lock);

	if (!p->active)
		goto out_unlock;

	if (self->conf.rx & UART_INTERRUPT) {
		do {
			rc = dequeue(&p->rxq, ptrb);
		} while (rc && !(self->conf.rx & UART_NONBLOCK));
	} else if (self->conf.rx & UART_POLLING) {
		do {
			rc = hw_uart_readb(self->ch, ptrb);
		} while (rc && !(self->conf.rx & UART_NONBLOCK));
	}

out_unlock:
	mutex_unlock(&p->lock);
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
	struct uart_handle *p;

	if (!self || ((unsigned int)self->ch >= UART_MAX_CHANNEL))
		return;

	if (!(p = &handles[self->ch]))
		return;

	mutex_lock(&p->lock);

	if (p->active)
		hw_uart_close(self->ch);

	p->active = false;

	mutex_unlock(&p->lock);

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

#include "kernel/init.h"

static void uart_init(void)
{
	for (int i = 0; i < UART_MAX_CHANNEL; i++) {
		mutex_init(&handles[i].lock);
	}
}
DRIVER_INIT(uart_init);
