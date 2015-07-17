#include <kernel/module.h>
#include <kernel/page.h>
#include <asm/usart.h>
#include <error.h>

#define BUF_SIZE	PAGE_SIZE
#define CHANNEL_MAX	3

#define CHANNEL(n)	(MINOR(n) - 1)

static struct fifo rxq[CHANNEL_MAX], txq[CHANNEL_MAX];
static lock_t rx_lock[CHANNEL_MAX], tx_lock[CHANNEL_MAX];

static int usart_close(struct file *file)
{
	struct device *dev = getdev(file->inode->dev);

	if (dev == NULL)
		return -ERR_UNDEF;

	if (--dev->count == 0) {
		__usart_close(CHANNEL(dev->id));

		kfree(rxq[CHANNEL(dev->id)].buf);
		kfree(txq[CHANNEL(dev->id)].buf);

		dev->op->read = NULL;
		dev->op->write = NULL;
		dev->op->close = NULL;
	}

	return 0;
}

static size_t usart_read(struct file *file, void *buf, size_t len)
{
	int data;
	char *c = (char *)buf;
	unsigned int irqflag;

	spin_lock_irqsave(rx_lock[CHANNEL(file->inode->dev)], irqflag);
	data = fifo_get(&rxq[CHANNEL(file->inode->dev)], 1);
	spin_unlock_irqrestore(rx_lock[CHANNEL(file->inode->dev)], irqflag);

	if (data == -1)
		return 0;

	if (c)
		*c = data & 0xff;

	return 1;
}

static size_t usart_write_int(struct file *file, void *buf, size_t len)
{
	char c = *(char *)buf;

	unsigned int irqflag;
	int err;

	spin_lock_irqsave(tx_lock[CHANNEL(file->inode->dev)], irqflag);
	err = fifo_put(&txq[CHANNEL(file->inode->dev)], c, 1);
	spin_unlock_irqrestore(tx_lock[CHANNEL(file->inode->dev)], irqflag);

	__usart_tx_irq_raise(CHANNEL(file->inode->dev));

	if (err == -1) return 0;

	return 1;
}

static size_t usart_write_polling(struct file *file, void *buf, size_t len)
{
	char c = *(char *)buf;
	unsigned int irqflag;

	spin_lock_irqsave(tx_lock[CHANNEL(file->inode->dev)], irqflag);
	__usart_putc(CHANNEL(file->inode->dev), c);
	spin_unlock_irqrestore(tx_lock[CHANNEL(file->inode->dev)], irqflag);

	return 1;
}

static void isr_usart()
{
	int c;
	unsigned int channel;

	if ((channel = __get_usart_active_irq()) >= CHANNEL_MAX)
		return;

	if (__usart_check_rx(channel)) {
		c = __usart_getc(channel);

		spin_lock(rx_lock[channel]);
		c = fifo_put(&rxq[channel], c, 1);
		spin_unlock(rx_lock[channel]);

		if (c == -1) {
			/* overflow */
		}
	}

	if (__usart_check_tx(channel)) {
		spin_lock(tx_lock[channel]);
		c = fifo_get(&txq[channel], 1);
		spin_unlock(tx_lock[channel]);

		if (c == -1)
			__usart_tx_irq_reset(channel);
		else
			__usart_putc(channel, c);
	}
}

#include <fs/fs.h>

static int usart_open(struct inode *inode, struct file *file)
{
	struct device *dev = getdev(file->inode->dev);
	int err = 0;

	if (dev == NULL)
		return -ERR_UNDEF;

	spin_lock(dev->lock.count);

	if (dev->count++ == 0) {
		void *buf;
		int nr_irq;

		if (CHANNEL(dev->id) >= CHANNEL_MAX) {
			err = -ERR_RANGE;
			goto out;
		}

		if ((nr_irq = __usart_open(CHANNEL(dev->id), 115200)) <= 0) {
			err = -ERR_UNDEF;
			goto out;
		}

		/* read */
		if ((buf = kmalloc(BUF_SIZE)) == NULL) {
			__usart_close(CHANNEL(dev->id));
			err = -ERR_ALLOC;
			goto out;
		}
		fifo_init(&rxq[CHANNEL(dev->id)], buf, BUF_SIZE);
		INIT_LOCK(rx_lock[CHANNEL(dev->id)]);

		/* write */
		if ((buf = kmalloc(BUF_SIZE)) == NULL) {
			__usart_close(CHANNEL(dev->id));
			err = -ERR_ALLOC;
			goto out;
		}
		fifo_init(&txq[CHANNEL(dev->id)], buf, BUF_SIZE);
		INIT_LOCK(tx_lock[CHANNEL(dev->id)]);

		register_isr(nr_irq, isr_usart);
	}

	if (file->flags & O_NONBLOCK)
		file->op->write = usart_write_polling;

out:
	spin_unlock(dev->lock.count);
	return err;
}

static struct file_operations ops = {
	.open  = usart_open,
	.read  = usart_read,
	.write = usart_write_int,
	.close = usart_close,
	.seek  = NULL,
};

#include <kernel/init.h>

static void __init usart_init()
{
	struct device *dev;
	unsigned int major, i;

	for (major = i = 0; i < CHANNEL_MAX; i++) {
		if ((dev = mkdev(major, i+1, &ops, "usart")))
			major = MAJOR(dev->id);
	}
}
MODULE_INIT(usart_init);

/* well, think about how to deliver this kind of functions to user
static int kbhit()
{
	return (rxq.front != rxq.rear);
}

static void fflush()
{
	unsigned int irqflag;

	spin_lock_irqsave(rx_lock, irqflag);
	fifo_flush(&rxq);
	spin_unlock_irqrestore(rx_lock, irqflag);
}
*/
