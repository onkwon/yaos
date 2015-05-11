#include <foundation.h>
#include <stdlib.h>
#include <kernel/device.h>
#include <asm/usart.h>

#define BUF_SIZE	PAGE_SIZE

static struct fifo_t rxq, txq;
static spinlock_t rx_lock, tx_lock;

static int usart_close(int id)
{
	struct device_t *dev = getdev(id);

	if (--dev->count == 0) {
		__usart_close();
		free(rxq.buf);
		free(txq.buf);
	}

	return dev->count;
}

static size_t usart_read(int id, void *buf, size_t size)
{
	int data;
	char *c = (char *)buf;
	unsigned long irq_flag;

	spinlock_irqsave(rx_lock, irq_flag);
	data = fifo_get(&rxq, 1);
	spinlock_irqrestore(rx_lock, irq_flag);

	if (data == -1)
		return 0;

	if (c)
		*c = data & 0xff;

	return 1;
}

static size_t usart_write_int(int id, void *buf, size_t size)
{
	char c = *(char *)buf;

	unsigned long irq_flag;
	int err;

	spinlock_irqsave(tx_lock, irq_flag);
	err = fifo_put(&txq, c, 1);
	spinlock_irqrestore(tx_lock, irq_flag);

	__usart_tx_irq_raise();

	if (err == -1) return 0;

	return 1;
}

static size_t usart_write_polling(int id, void *buf, size_t size)
{
	char c = *(char *)buf;

	__usart_putc(c);

	return 1;
}

/* well, think about how to deliver this kind of functions to user
static int kbhit()
{
	return (rxq.front != rxq.rear);
}

static void fflush()
{
	unsigned long irq_flag;

	spinlock_irqsave(rx_lock, irq_flag);
	fifo_flush(&rxq);
	spinlock_irqrestore(rx_lock, irq_flag);
}
*/

static void isr_usart()
{
	int c;
	unsigned long irq_flag;

	if (__usart_check_rx()) {
		c = __usart_getc();

		spinlock_irqsave(rx_lock, irq_flag);
		c = fifo_put(&rxq, c, 1);
		spinlock_irqrestore(rx_lock, irq_flag);

		if (c == -1) {
			/* overflow */
		}
	}

	if (__usart_check_tx()) {
		spinlock_irqsave(tx_lock, irq_flag);
		c = fifo_get(&txq, 1);
		spinlock_irqrestore(tx_lock, irq_flag);

		if (c == -1)
			__usart_tx_irq_reset();
		else
			__usart_putc(c);
	}
}

static int usart_open(int id, int mode)
{
	struct device_t *dev = getdev(id);

	if (!(dev->count++)) { /* if it's the first access */
		int nr_irq = __usart_open(115200);

		if (nr_irq > 0) {
			if (mode & O_RDONLY) {
				fifo_init(&rxq, kmalloc(BUF_SIZE), BUF_SIZE);
				spinlock_init(rx_lock);
			}

			if (mode & O_WRONLY) {
				fifo_init(&txq, kmalloc(BUF_SIZE), BUF_SIZE);
				spinlock_init(tx_lock);
			}

			if (mode & O_NONBLOCK)
				dev->ops->write = usart_write_polling;
			else
				dev->ops->write = usart_write_int;

			register_isr(nr_irq, isr_usart);
		}
	}

	return dev->count;
}

static struct driver_operations ops = {
	.open  = usart_open,
	.read  = usart_read,
	.write = usart_write_int,
	.close = usart_close,
};
REGISTER_DEVICE(USART, &ops);
