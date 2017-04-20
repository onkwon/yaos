#include <kernel/module.h>
#include <kernel/page.h>
#include <kernel/syscall.h>
#include <asm/uart.h>
#include <error.h>
#include "worklist.h"

#define CHANNEL(n)		(MINOR(n) - 1)

static unsigned int major;

struct uart {
	size_t rx, tx;
	int baudrate;
};

struct uart_buffer {
	struct fifo rxq, txq;
	struct waitqueue_head waitq;
};

static void ISR_uart(int nvector)
{
	unsigned int channel;
	struct device *dev;
	struct uart_buffer *buf;
	int c;

#ifndef CONFIG_COMMON_IRQ_FRAMEWORK
	nvector = get_active_irq();
#endif
	channel = __get_uart_channel_active(nvector);

	if ((dev = getdev(SET_DEVID(major, channel + 1))) == NULL)
		return;

	buf = dev->buffer;

	if (__uart_has_rx(channel)) {
		c = __uart_getc(channel);

		if (fifo_put(&buf->rxq, c, 1) == -1) {
			/* TODO: make a stats counting overflow */
		}

		wq_wake(&buf->waitq, WQ_EXCLUSIVE);
	}

	if (__uart_has_tx(channel)) {
		c = fifo_get(&buf->txq, 1);

		if (c == -1) /* end of transmitting */
			__uart_tx_irq_reset(channel);
		else if (!__uart_putc(channel, c)) /* put it back if error */
			fifo_put(&buf->txq, c, 1);
	}
}

static int uart_kbhit(struct file *file)
{
	struct device *dev;
	struct uart_buffer *buf;

	dev = getdev(file->inode->dev);
	buf = dev->buffer;

	return (buf->rxq.front != buf->rxq.rear);
}

static void uart_flush(struct file *file)
{
	struct device *dev;
	struct uart_buffer *buf;
	unsigned int irqflag;

	dev = getdev(file->inode->dev);
	buf = dev->buffer;

	spin_lock_irqsave(nospin, irqflag);
	fifo_flush(&buf->rxq);
	spin_unlock_irqrestore(nospin, irqflag);

	__uart_flush(CHANNEL(file->inode->dev));
}

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

	return ERANGE;
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
		return ENOMEM;

	syscall_put_arguments(thread, file, NULL, NULL, NULL);
	syscall_delegate(current, thread);

	return 0;
}
#else
static int uart_close(struct file *file)
{
	syscall_delegate_atomic(do_uart_close, &current->mm.sp, &current->flags);

	return 0;
}
#endif

static inline size_t uart_read_core(struct file *file, void *buf, size_t len)
{
	struct device *dev;
	struct uart_buffer *uartq;
	char *c;
	int data;
	unsigned int irqflag;

	dev = getdev(file->inode->dev);
	uartq = dev->buffer;
	c = (char *)buf;

	spin_lock_irqsave(nospin, irqflag);
	data = fifo_get(&uartq->rxq, 1);
	spin_unlock_irqrestore(nospin, irqflag);

	if (data == -1)
		return 0;

	if (c)
		*c = data & 0xff;

	return 1;
}

static size_t do_uart_read(struct file *file, void *buf, size_t len)
{
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
		return ENOMEM;

	syscall_put_arguments(thread, file, buf, len, NULL);
	syscall_delegate(current, thread);

	return 0;
}
#else
size_t uart_read(struct file *file, void *buf, size_t len)
{
	syscall_delegate_atomic(do_uart_read, &current->mm.sp, &current->flags);

	return 0;
}
#endif

static size_t uart_write_int(struct file *file, void *data)
{
	struct device *dev;
	struct uart_buffer *buf;
	char c;
	unsigned int irqflag;

	dev = getdev(file->inode->dev);
	buf = dev->buffer;
	c = *(char *)data;

	spin_lock_irqsave(nospin, irqflag);

	/* ring buffer: if full, throw the oldest one for new one */
	while (fifo_put(&buf->txq, c, 1) == -1)
		fifo_get(&buf->txq, 1);

	spin_unlock_irqrestore(nospin, irqflag);

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
		return ENOMEM;

	syscall_put_arguments(thread, file, buf, len, NULL);
	syscall_delegate(current, thread);

	return 0;
}
#else
size_t uart_write(struct file *file, void *buf, size_t len)
{
	syscall_delegate_atomic(do_uart_write, &current->mm.sp, &current->flags);

	return 0;
}
#endif

#include <fs/fs.h>

static int uart_open(struct inode *inode, struct file *file)
{
	struct device *dev = getdev(file->inode->dev);
	int err = 0;
	struct uart_buffer *buf;
	void *rx, *tx;

	if (dev == NULL)
		return EFAULT;

	mutex_lock(&dev->mutex);

	if (dev->refcount++ == 0) {
		struct uart def = { 128, 128, 115200 }; /* default */
		struct uart *conf;
		int vector;

		if ((conf = (struct uart *)file->option) == NULL)
			conf = &def;

		debug("rx: %d, tx: %d, baudrate: %d",
				conf->rx, conf->tx, conf->baudrate);

		if (conf->rx < def.rx) conf->rx = def.rx;
		if (conf->tx < def.tx) conf->tx = def.tx;
		assert(conf->baudrate);

		if ((vector = __uart_open(CHANNEL(dev->id), conf->baudrate)) <= 0) {
			err = EINVAL;
			goto out;
		}

		if ((buf = kmalloc(sizeof(*buf))) == NULL)
			goto out_close;

		/* read */
		if ((rx = kmalloc(conf->rx)) == NULL)
			goto out_free_buf;

		/* write */
		if ((tx = kmalloc(conf->tx)) == NULL)
			goto out_free_rx;

		fifo_init(&buf->rxq, rx, conf->rx);
		fifo_init(&buf->txq, tx, conf->tx);
		WQ_INIT(buf->waitq);
		dev->buffer = buf;

		register_isr(vector, ISR_uart);
	}

	goto out;

out_free_rx:
	kfree(rx);
out_free_buf:
	kfree(buf);
out_close:
	__uart_close(CHANNEL(dev->id));
	err = ENOMEM;
out:
	mutex_unlock(&dev->mutex);
	return err;
}

static struct file_operations ops = {
	.open  = uart_open,
	.read  = uart_read,
	.write = uart_write,
	.close = uart_close,
	.seek  = NULL,
	.ioctl = uart_ioctl,
};

void register_uart(const char *name, int minor)
{
	macro_register_device(name, major, minor, &ops);
}

void __putc_debug(int c)
{
	struct file *file;
	int res, chan;

	if ((file = getfile(stdout)))
		chan = CHANNEL(file->inode->dev);
	else
		chan = 0;

putcr:
	do {
		res = __uart_putc(chan, c);
	} while (!res);

	if (c == '\n') {
		c = '\r';
		goto putcr;
	}
}
