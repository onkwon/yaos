#include "drivers/uart.h"
#include "kernel/interrupt.h"
#include "kernel/lock.h"
#include "queue.h"
#include <errno.h>
#include <assert.h>

static struct _uart {
	queue_t rxq, txq;
	uint16_t rxerr, txerr;
	uint16_t rxovf;
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
		if ((1 != hw_uart_getb(channel, &data))
				|| !queue_is_initialized(&p->rxq))
			p->rxerr++;
		else if (enqueue(&p->rxq, QUEUE_ITEM(data)))
			p->rxovf++;
	}

	if (queue_is_initialized(&p->txq) && hw_uart_is_tx_ready(channel)) {
		if (queue_empty(&p->txq)) {
			hw_uart_disable_tx_interrupt(channel);
		} else {
			if (dequeue(&p->txq, &data))
				p->txerr++;
			else if (1 != hw_uart_putb(channel, data))
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
	int vector;

	if (!self || (UART_MAX_CHANNEL <= (unsigned int)self->ch))
		goto errout;

	if ((UART_INTERRUPT == self->conf.rx)) {
		if (!rxbuf || (0 >= rxbufsize))
			goto errout;

		queue_init_static(&_uart[self->ch].rxq,
				rxbuf, rxbufsize, 1U);
	}
	if (UART_INTERRUPT == self->conf.tx) {
		if (!txbuf || (0 >= txbufsize))
			goto errout;

		queue_init_static(&_uart[self->ch].txq,
				txbuf, txbufsize, 1U);
	}

	// TODO: lock
	// mutex_lock();
	vector = hw_uart_open(self->ch, self->conf);
	// mutex_unlock();

	/* FIXME: Interrupt must be prevented before its handler registered.
	 * If an interrupt occurs first before its isr registered, the
	 * interrupt is going to be handled by the default isr doing noting on
	 * it. that means the interrupt keeps interrupting normal tasks making
	 * it never take places. so registering the actual isr below never ever
	 * happen and the whole system gets stuck. */
	register_isr(vector, ISR_uart);

	return 0;
errout:
	return -EINVAL;
}

static bool uart_kbhit(const uart_t * const self)
{
	struct _uart *p;

	if (!self || (UART_MAX_CHANNEL <= (unsigned int)self->ch))
		return false;

	p = &_uart[self->ch];

	if (!p || !queue_is_initialized(&p->rxq))
		return false;

	if (queue_empty(&p->rxq))
		return false;

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

uart_t uart_new(const enum uart_channel ch)
{
	assert(ch < UART_MAX_CHANNEL);

	return (uart_t) {
		.ch = ch,
		.conf = UART_DEFAULT_CONF(),
		.open_static = uart_open_static,
		.flush = uart_flush,
		.kbhit = uart_kbhit,
	};
}

#if 0
/* TODO: API, design the interface */
static int uart_ioctl(struct file *file, int request, void *data)
{
	unsigned int *brr;

	switch (request) {
	case C_FLUSH:
		uart_flush(file);
		return 0;
	case C_EVENT:
		*(int *)data = uart_kbhit(file)? 1 : 0;
		return 0;
	case C_FREQ:
		brr = (unsigned int *)data;
		if (*brr)
			return __uart_set_baudrate(CHANNEL(file->inode->dev),
					*brr);
		else
			*brr = __uart_get_baudrate(CHANNEL(file->inode->dev));
		return 0;
	case C_BUFSIZE:
		break;
	default:
		break;
	}

	return -ERANGE;
}

static void do_uart_close(struct file *file)
{
	struct device *dev;
	struct uart_buffer *buf;

	dev = getdev(file->inode->dev);
	buf = dev->buffer;

	mutex_lock(&dev->mutex);
	if (--dev->refcount == 0) {
		__uart_close(CHANNEL(dev->id));

		kfree(buf->rxq.buf);
		kfree(buf->txq.buf);
	}
	mutex_unlock(&dev->mutex);

#ifdef CONFIG_SYSCALL_THREAD
	syscall_delegate_return(current->parent, 0);
#endif
}

#ifdef CONFIG_SYSCALL_THREAD
static int uart_close(struct file *file)
{
	struct task *thread;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					do_uart_close, current)) == NULL)
		return -ENOMEM;

	syscall_put_arguments(thread, file, NULL, NULL, NULL);
	syscall_delegate(current, thread);

	return 0;
}
#else
static int uart_close(struct file *file)
{
	syscall_delegate_atomic(do_uart_close, &current->mm.sp, &current->flags);

	(void)file;
	return 0;
}
#endif

static inline size_t uart_read_core(struct file *file, void *buf, size_t len)
{
	struct device *dev;
	struct uart_buffer *uartq;
	char *p;
	int data;

	dev = getdev(file->inode->dev);
	uartq = dev->buffer;
	p = (char *)buf;
	data = fifo_getb(&uartq->rxq);

	if (data < 0)
		return 0;

	if (p)
		*p = data & 0xff;

	(void)len;
	return 1;
}

static size_t do_uart_read(struct file *file, void *buf, size_t len)
{
#ifndef CONFIG_SYSCALL_THREAD
	if (file->flags & O_NONBLOCK)
		return uart_read_core(file, buf, len);
#endif

	struct device *dev;
	struct uart_buffer *uartq;
	size_t total, d;

	dev = getdev(file->inode->dev);
	uartq = dev->buffer;

	for (total = 0; total < len && file->offset < file->inode->size;) {
		if ((d = uart_read_core(file, buf + total, len - total)) <= 0) {
			wq_wait(&uartq->waitq);
			continue;
		}
		total += d;
	}

#ifdef CONFIG_SYSCALL_THREAD
	syscall_delegate_return(current->parent, total);
#endif
	return total;
}

#ifdef CONFIG_SYSCALL_THREAD
static size_t uart_read(struct file *file, void *buf, size_t len)
{
	struct task *thread;

	if (file->flags & O_NONBLOCK)
		return uart_read_core(file, buf, len);

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					do_uart_read, current)) == NULL)
		return -ENOMEM;

	syscall_put_arguments(thread, file, buf, len, NULL);
	syscall_delegate(current, thread);

	return 0;
}
#else
size_t uart_read(struct file *file, void *buf, size_t len)
{
	syscall_delegate_atomic(do_uart_read, &current->mm.sp, &current->flags);

	(void)file;
	(void)(int)buf;
	(void)len;
	return 0;
}
#endif

static size_t uart_write_int(struct file *file, void *data)
{
	struct device *dev;
	struct uart_buffer *buf;
	char c;

	dev = getdev(file->inode->dev);
	buf = dev->buffer;
	c = *(char *)data;

	/* ring buffer: if full, throw the oldest one for new one */
	while (fifo_putb(&buf->txq, c) == -ENOSPC)
		fifo_getb(&buf->txq);

	__uart_tx_irq_raise(CHANNEL(file->inode->dev));

	return 1;
}

static size_t uart_write_polling(struct file *file, void *data)
{
	int res;

	char c = *(char *)data;

	do {
		res = __uart_putc(CHANNEL(file->inode->dev), c);
	} while (!res);

	return res;
}

static size_t do_uart_write(struct file *file, void *buf, size_t len)
{
	size_t written, (*f)(struct file *file, void *data);

	if (file->flags & O_NONBLOCK)
		f = uart_write_polling;
	else
		f = uart_write_int;

	for (written = 0; written < len && file->offset < file->inode->size;)
		if (f(file, buf + written) >= 1)
			written++;

#ifdef CONFIG_SYSCALL_THREAD
	syscall_delegate_return(current->parent, written);
#endif
	return written;
}

#ifdef CONFIG_SYSCALL_THREAD
static size_t uart_write(struct file *file, void *buf, size_t len)
{
	struct task *thread;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					do_uart_write, current)) == NULL)
		return -ENOMEM;

	syscall_put_arguments(thread, file, buf, len, NULL);
	syscall_delegate(current, thread);

	return 0;
}
#else
size_t uart_write(struct file *file, void *buf, size_t len)
{
	syscall_delegate_atomic(do_uart_write, &current->mm.sp, &current->flags);

	(void)file;
	(void)(int)buf;
	(void)len;
	return 0;
}
#endif
#endif
