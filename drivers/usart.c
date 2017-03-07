#include <kernel/module.h>
#include <kernel/page.h>
#include <asm/usart.h>
#include <error.h>

#ifdef CONFIG_PAGING
#define BUFSIZE		PAGESIZE
#else
#define BUFSIZE		32
#endif

#ifndef USART_CHANNEL_MAX
#define USART_CHANNEL_MAX	1
#endif

#define CHANNEL(n)		(MINOR(n) - 1)

static unsigned int major;

static struct fifo rxq[USART_CHANNEL_MAX], txq[USART_CHANNEL_MAX];
static lock_t rx_lock[USART_CHANNEL_MAX], tx_lock[USART_CHANNEL_MAX];
static struct waitqueue_head wq[USART_CHANNEL_MAX];

static DEFINE_LINK_HEAD(wr_work_list_head);
static struct task *wr_uartd;

struct work_list {
	struct link list;
	struct task *task;
	struct file *file;
	void *buf;
	size_t size;
};

static size_t usart_write_polling(struct file *file, void *data);
static size_t usart_write_int(struct file *file, void *data);

static void uart_wr_daemon(struct file *file, void *buf, size_t len)
{
	struct work_list *work;
	struct task *task;
	size_t (*f)(struct file *file, void *data), ret;
	unsigned int irqflag;

	wr_uartd = current;

loop:
	while (!link_empty(&wr_work_list_head)) {
		work = (struct work_list *)wr_work_list_head.next;
		wr_work_list_head.next = work->list.next;

		file = work->file;
		buf = work->buf;
		len = work->size;
		task = work->task;

		kfree(work);

#if ((SOC == bcm2835) || (SOC == bcm2836))
		f = usart_write_polling;
#else
		if (file->flags & O_NONBLOCK)
			f = usart_write_polling;
		else
			f = usart_write_int;
#endif

		for (ret = 0; ret < len && file->offset < file->inode->size;)
			if (f(file, buf + ret) >= 1)
				ret++;

		__set_retval(task, ret);
		//sum_curr_stat(task); /* FIXME: it works for only clone() */

		spin_lock_irqsave(&task->lock, irqflag);
		go_run_atomic(task);
		spin_unlock_irqrestore(&task->lock, irqflag);
	}

	yield();
	goto loop;
}
REGISTER_TASK(uart_wr_daemon, TASK_KERNEL, DEFAULT_PRIORITY);

static int usart_kbhit(struct file *file)
{
	return (rxq[CHANNEL(file->inode->dev)].front
			!= rxq[CHANNEL(file->inode->dev)].rear);
}

static void usart_flush(struct file *file)
{
	unsigned int irqflag;

	spin_lock_irqsave(&rx_lock[CHANNEL(file->inode->dev)], irqflag);
	fifo_flush(&rxq[CHANNEL(file->inode->dev)]);
	spin_unlock_irqrestore(&rx_lock[CHANNEL(file->inode->dev)], irqflag);

	__usart_flush(CHANNEL(file->inode->dev));

	/* It must awaken tasks waiting in runqueue before closing the file
	 * descriptor which is in use also by other tasks. If a file descriptor
	 * is used by not only a task but also other tasks, the file descriptor
	 * might be destroyed(closed) by a task while other tasks waiting in
	 * runqueue are still referencing the file descriptor. That will cause
	 * a fault.
	 *
	 * lock or reference count in file struct should be a solution. But I
	 * rather suggest to use different file descriptor each task instead.
	 *
	 * wq_wake(&wq[channel], WQ_ALL);
	 */
}

static int usart_ioctl(struct file *file, int request, void *data)
{
	unsigned int *brr;

	switch (request) {
	case C_FLUSH:
		usart_flush(file);
		return 0;
	case C_EVENT:
		*(int *)data = usart_kbhit(file)? 1 : 0;
		return 0;
	case C_FREQ:
		brr = (unsigned int *)data;
		if (*brr)
			return __usart_set_baudrate(CHANNEL(file->inode->dev),
					*brr);
		else
			*brr = __usart_get_baudrate(CHANNEL(file->inode->dev));
		return 0;
	case C_BUFSIZE:
		break;
	default:
		break;
	}

	return -ERR_RANGE;
}

static int usart_close(struct file *file)
{
	struct device *dev = getdev(file->inode->dev);

	if (dev == NULL)
		return -ERR_UNDEF;

	spin_lock(&dev->mutex.counter);

	if (--dev->refcount == 0) {
		__usart_close(CHANNEL(dev->id));

		kfree(rxq[CHANNEL(dev->id)].buf);
		kfree(txq[CHANNEL(dev->id)].buf);
	}

	spin_unlock(&dev->mutex.counter);

	return 0;
}

static size_t usart_read_core(struct file *file, void *buf, size_t len)
{
	int data;
	char *c = (char *)buf;
	unsigned int irqflag;

	spin_lock_irqsave(&rx_lock[CHANNEL(file->inode->dev)], irqflag);
	data = fifo_get(&rxq[CHANNEL(file->inode->dev)], 1);
	spin_unlock_irqrestore(&rx_lock[CHANNEL(file->inode->dev)], irqflag);

	if (data == -1)
		return 0;

	if (c)
		*c = data & 0xff;

	return 1;
}

static void do_uart_read(struct file *file, void *buf, size_t len)
{
	struct work_list *work;
	size_t ret, cnt;

	work = current->args;
	file = work->file;
	buf = work->buf;
	len = work->size;

	kfree(work);

	for (ret = 0; ret < len && file->offset < file->inode->size;) {
		if ((cnt = usart_read_core(file, buf + ret, len - ret))
				<= 0) {
			wq_wait(&wq[CHANNEL(file->inode->dev)]);
			continue;
		}
		ret += cnt;
	}

	syscall_delegate_return(current->parent, ret);
}

static size_t usart_read(struct file *file, void *buf, size_t len)
{
	struct task *thread;
	struct work_list *work;

	if (file->flags & O_NONBLOCK)
		return usart_read_core(file, buf, len);

	if ((work = kmalloc(sizeof(*work))) == NULL)
		return -ERR_ALLOC;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, do_uart_read,
					current)) == NULL)
		return -ERR_ALLOC;

	work->task = current;
	work->file = file;
	work->buf = buf;
	work->size = len;

	thread->args = (void *)work;
	syscall_delegate(current, thread);

	return 0;
}

static size_t usart_write_int(struct file *file, void *data)
{
	unsigned int irqflag;
	char c = *(char *)data;

	spin_lock_irqsave(&tx_lock[CHANNEL(file->inode->dev)], irqflag);

	/* ring buffer: if full, throw the oldest one for new one */
	while (fifo_put(&txq[CHANNEL(file->inode->dev)], c, 1) == -1)
		fifo_get(&txq[CHANNEL(file->inode->dev)], 1);

	spin_unlock_irqrestore(&tx_lock[CHANNEL(file->inode->dev)], irqflag);

	__usart_tx_irq_raise(CHANNEL(file->inode->dev));

	return 1;
}

static size_t usart_write_polling(struct file *file, void *data)
{
	unsigned int irqflag;
	int res;

	char c = *(char *)data;

	do {
		spin_lock_irqsave(&tx_lock[CHANNEL(file->inode->dev)], irqflag);
		res = __usart_putc(CHANNEL(file->inode->dev), c);
		spin_unlock_irqrestore(&tx_lock[CHANNEL(file->inode->dev)], irqflag);
	} while (!res);

	return res;
}

static size_t usart_write(struct file *file, void *buf, size_t len)
{
	struct work_list *work;

	if ((work = kmalloc(sizeof(*work))) == NULL)
		return -ERR_ALLOC;

	work->task = current;
	work->file = file;
	work->buf = buf;
	work->size = len;

	link_add_tail(&work->list, &wr_work_list_head);
	go_run_atomic(wr_uartd);

	set_task_state(current, TASK_WAITING);
	resched();

	return 0;
}

#ifdef CONFIG_DEBUG
static unsigned int bufover[USART_CHANNEL_MAX];

unsigned int get_usart_bufover(int ch)
{
	return bufover[ch];
}
#endif

static void isr_usart()
{
	int c;
	unsigned int channel;

	if ((channel = __get_usart_active_irq()) >= USART_CHANNEL_MAX)
		return;

	if (__usart_check_rx(channel)) {
		c = __usart_getc(channel);

		spin_lock(&rx_lock[channel]);
		c = fifo_put(&rxq[channel], c, 1);
		spin_unlock(&rx_lock[channel]);

		if (c == -1) {
			/* overflow */
#ifdef CONFIG_DEBUG
			bufover[channel]++;
#endif
		}

		wq_wake(&wq[channel], WQ_EXCLUSIVE);
	}

	if (__usart_check_tx(channel)) {
		spin_lock(&tx_lock[channel]);

		c = fifo_get(&txq[channel], 1);
		if (c == -1) /* end of transmitting */
			__usart_tx_irq_reset(channel);
		else if (!__usart_putc(channel, c)) /* put it back if error */
			fifo_put(&txq[channel], c, 1);

		spin_unlock(&tx_lock[channel]);
	}
}

#include <fs/fs.h>

static int usart_open(struct inode *inode, struct file *file)
{
	struct device *dev = getdev(file->inode->dev);
	int err = 0;

	if (dev == NULL)
		return -ERR_UNDEF;

	spin_lock(&dev->mutex.counter);

	if (dev->refcount++ == 0) {
		void *buf;
		int nvector;
		unsigned int baudrate;

		baudrate = 115200; /* default */
#if 0
		/* FIXME: file->option may have any value when not passed in
		 * variable arguments list by user */
		if (file->option)
			baudrate = (unsigned int)file->option;
#endif

		debug("baudrate %d", baudrate);

		if (CHANNEL(dev->id) >= USART_CHANNEL_MAX) {
			err = -ERR_RANGE;
			goto out;
		}

		if ((nvector = __usart_open(CHANNEL(dev->id), baudrate)) <= 0) {
			err = -ERR_OPEN;
			goto out;
		}

		/* read */
		if ((buf = kmalloc(BUFSIZE)) == NULL) {
			__usart_close(CHANNEL(dev->id));
			err = -ERR_ALLOC;
			goto out;
		}
		fifo_init(&rxq[CHANNEL(dev->id)], buf, BUFSIZE);
		lock_init(&rx_lock[CHANNEL(dev->id)]);

		/* write */
		if ((buf = kmalloc(BUFSIZE)) == NULL) {
			__usart_close(CHANNEL(dev->id));
			err = -ERR_ALLOC;
			goto out;
		}
		fifo_init(&txq[CHANNEL(dev->id)], buf, BUFSIZE);
		lock_init(&tx_lock[CHANNEL(dev->id)]);

		WQ_INIT(wq[CHANNEL(dev->id)]);

		register_isr(nvector, isr_usart);

#ifdef CONFIG_DEBUG
		bufover[CHANNEL(dev->id)] = 0;
#endif
	}

out:
	spin_unlock(&dev->mutex.counter);
	return err;
}

static struct file_operations ops = {
	.open  = usart_open,
	.read  = usart_read,
	.write = usart_write,
	.close = usart_close,
	.seek  = NULL,
	.ioctl = usart_ioctl,
};

void register_usart(const char *name, int minor)
{
	macro_register_device(name, major, minor, &ops);
}

void __putc_debug(int c)
{
	struct file *file;
	int res, chan;
	unsigned int irqflag;

	if ((file = getfile(stdout)))
		chan = CHANNEL(file->inode->dev);
	else
		chan = 0;

putcr:
	do {
		spin_lock_irqsave(&tx_lock[chan], irqflag);
		res = __usart_putc(chan, c);
		spin_unlock_irqrestore(&tx_lock[chan], irqflag);
	} while (!res);

	if (c == '\n') {
		c = '\r';
		goto putcr;
	}
}
