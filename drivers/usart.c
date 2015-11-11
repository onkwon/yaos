#include <kernel/module.h>
#include <kernel/page.h>
#include <asm/usart.h>
#include <error.h>

#define BUF_SIZE		PAGE_SIZE

#ifndef USART_CHANNEL_MAX
#define USART_CHANNEL_MAX	1
#endif

#define CHANNEL(n)		(MINOR(n) - 1)

static struct fifo rxq[USART_CHANNEL_MAX], txq[USART_CHANNEL_MAX];
static lock_t rx_lock[USART_CHANNEL_MAX], tx_lock[USART_CHANNEL_MAX];
static struct waitqueue_head wq[USART_CHANNEL_MAX];

static int usart_close(struct file *file)
{
	struct device *dev = getdev(file->inode->dev);

	if (dev == NULL)
		return -ERR_UNDEF;

	if (--dev->count == 0) {
		__usart_close(CHANNEL(dev->id));

		kfree(rxq[CHANNEL(dev->id)].buf);
		kfree(txq[CHANNEL(dev->id)].buf);
	}

	return 0;
}

static size_t usart_read_core(struct file *file, void *buf, size_t len)
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

static size_t usart_read(struct file *file, void *buf, size_t len)
{
	if (file->flags & O_NONBLOCK)
		return usart_read_core(file, buf, len);

	struct task *parent;
	size_t retval, cnt;
	int tid;

	parent = current;
	tid = clone(TASK_HANDLER | TASK_KERNEL | STACK_SHARED, &init);

	if (tid == 0) { /* parent */
		/* it goes TASK_WAITING state as soon as exiting from system
		 * call to wait for its child's job to be done that returns the
		 * result. */
		set_task_state(current, TASK_WAITING);
		resched();
		return 0;
	} else if (tid < 0) { /* error */
		/* use errno */
		debug(MSG_DEBUG, "failed cloning");
		return -ERR_RETRY;
	}

	/* child takes place from here turning to kernel task,
	 * nomore in handler mode */
	for (retval = 0; retval < len && file->offset < file->inode->size;) {
		if ((cnt = usart_read_core(file, buf + retval, len - retval))
				<= 0) {
			cnt = 0;
			wq_wait(&wq[CHANNEL(file->inode->dev)]);
		}
		retval += cnt;
	}

	__set_retval(parent, retval);
	sum_curr_stat(parent);

	if (get_task_state(parent)) {
		set_task_state(parent, TASK_RUNNING);
		runqueue_add(parent);
	}

	sys_kill((unsigned int)current);
	freeze(); /* never reaches here */

	return -ERR_UNDEF;
}

static size_t usart_write_int(struct file *file, void *data)
{
	char c = *(char *)data;

	unsigned int irqflag;
	int err;

	spin_lock_irqsave(tx_lock[CHANNEL(file->inode->dev)], irqflag);
	err = fifo_put(&txq[CHANNEL(file->inode->dev)], c, 1);
	spin_unlock_irqrestore(tx_lock[CHANNEL(file->inode->dev)], irqflag);

	if (err == -1) return 0;

	__usart_tx_irq_raise(CHANNEL(file->inode->dev));

	return 1;
}

static size_t usart_write_polling(struct file *file, void *data)
{
	char c = *(char *)data;

	unsigned int irqflag;
	int res;

	do {
		spin_lock_irqsave(tx_lock[CHANNEL(file->inode->dev)], irqflag);
		res = __usart_putc(CHANNEL(file->inode->dev), c);
		spin_unlock_irqrestore(tx_lock[CHANNEL(file->inode->dev)], irqflag);
	} while (!res);

	return res;
}

static size_t usart_write(struct file *file, void *buf, size_t len)
{
	struct task *parent;
	size_t retval;
	int tid;

	parent = current;
	tid = clone(TASK_HANDLER | TASK_KERNEL | STACK_SHARED, &init);

	if (tid == 0) { /* parent */
		/* it goes TASK_WAITING state as soon as exiting from system
		 * call to wait for its child's job to be done that returns the
		 * result. */
		set_task_state(current, TASK_WAITING);
		resched();
		return 0;
	} else if (tid < 0) { /* error */
		/* use errno */
		debug(MSG_DEBUG, "failed cloning");
		return -ERR_RETRY;
	}

	size_t (*f)(struct file *file, void *data);
	if (file->flags & O_NONBLOCK)
		f = usart_write_polling;
	else
		f = usart_write_int;

	/* child takes place from here turning to kernel task,
	 * nomore in handler mode */
	for (retval = 0; retval < len && file->offset < file->inode->size;)
		if (f(file, buf + retval) >= 1) retval++;

	__set_retval(parent, retval);
	sum_curr_stat(parent);

	if (get_task_state(parent)) {
		set_task_state(parent, TASK_RUNNING);
		runqueue_add(parent);
	}

	sys_kill((unsigned int)current);
	freeze(); /* never reaches here */

	return -ERR_UNDEF;
}

static void isr_usart()
{
	int c;
	unsigned int channel;

	if ((channel = __get_usart_active_irq()) >= USART_CHANNEL_MAX)
		return;

	if (__usart_check_rx(channel)) {
		c = __usart_getc(channel);

		spin_lock(rx_lock[channel]);
		c = fifo_put(&rxq[channel], c, 1);
		spin_unlock(rx_lock[channel]);

		if (c == -1) {
			/* overflow */
		}

		wq_wake(&wq[channel], WQ_ALL);
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
		unsigned int baudrate = 115200;

		if (file->flags & O_9600) baudrate = 9600;

		if (CHANNEL(dev->id) >= USART_CHANNEL_MAX) {
			err = -ERR_RANGE;
			goto out;
		}

		if ((nr_irq = __usart_open(CHANNEL(dev->id), baudrate)) <= 0) {
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
		lock_init(&rx_lock[CHANNEL(dev->id)]);

		/* write */
		if ((buf = kmalloc(BUF_SIZE)) == NULL) {
			__usart_close(CHANNEL(dev->id));
			err = -ERR_ALLOC;
			goto out;
		}
		fifo_init(&txq[CHANNEL(dev->id)], buf, BUF_SIZE);
		lock_init(&tx_lock[CHANNEL(dev->id)]);

		WQ_INIT(wq[CHANNEL(dev->id)]);

		register_isr(nr_irq, isr_usart);
	}

out:
	spin_unlock(dev->lock.count);
	return err;
}

static struct file_operations ops = {
	.open  = usart_open,
	.read  = usart_read,
	.write = usart_write,
	.close = usart_close,
	.seek  = NULL,
};

#include <kernel/init.h>

static void __init usart_init()
{
	struct device *dev;
	unsigned int major, i;

	for (major = i = 0; i < USART_CHANNEL_MAX; i++) {
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

static void flush()
{
	unsigned int irqflag;

	spin_lock_irqsave(rx_lock, irqflag);
	fifo_flush(&rxq);
	spin_unlock_irqrestore(rx_lock, irqflag);

	__usart_flush();
}
*/
