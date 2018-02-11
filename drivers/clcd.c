/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include <error.h>
#include <kernel/module.h>
#include <kernel/timer.h>
#include <kernel/page.h>
#include <kernel/systick.h>
#include <drivers/gpio.h>
#include <asm/pinmap.h>

#define set_cs()	gpio_put(PIN_CLCD_E, 1)
#define clr_cs()	gpio_put(PIN_CLCD_E, 0)
#define set_rs()	gpio_put(PIN_CLCD_RS, 1)
#define clr_rs()	gpio_put(PIN_CLCD_RS, 0)
#define set_rw()	gpio_put(PIN_CLCD_RW, 1)
#define clr_rw()	gpio_put(PIN_CLCD_RW, 0)

#define OUTPUT		0
#define INPUT		1

#define QSIZE		33 /* 16x2 + 1 */

static inline void raise()
{
	udelay(1); /* >= 60ns after RS or RW transition */
	set_cs();
	udelay(1); /* >= 450ns enable hold time */
	clr_cs();
	udelay(1); /* >= 20ns before RS or RW transition */
}

static inline void clcd_put_nibble(unsigned char nibble)
{
	gpio_put(PIN_CLCD_DB7, nibble >> 3);
	gpio_put(PIN_CLCD_DB6, nibble >> 2);
	gpio_put(PIN_CLCD_DB5, nibble >> 1);
	gpio_put(PIN_CLCD_DB4, nibble >> 0);
	raise();
}

static inline void clcd_put(unsigned char v)
{
	clcd_put_nibble(v >> 4);
	clcd_put_nibble(v);
}

static inline void clcd_dir(unsigned int dir)
{
	if (dir == INPUT)
		gpio_init(PIN_CLCD_DB7, GPIO_MODE_INPUT | GPIO_CONF_PULLDOWN);
	else
		gpio_init(PIN_CLCD_DB7, GPIO_MODE_OUTPUT);

}

static inline int get_busy_flag()
{
	return gpio_get(PIN_CLCD_DB7);
}

static inline void wait_while_busy()
{
	unsigned int busy, tout;

	set_timeout(&tout, msec_to_ticks(100)); /* 100ms */

	clcd_dir(INPUT);
	set_rw();
	udelay(1); /* >= 60ns after RS or RW transition */

	do {
		if (is_timeout(tout)) {
			debug("timeout!");
			break;
		}

		set_cs();
		udelay(1); /* >= 450ns enable hold time */
		busy = get_busy_flag();
		clr_cs();

		raise(); /* discard alternate nibbles */
	} while (busy);

	clr_rw();
	clcd_dir(OUTPUT);
}

static void clcd_cmd(unsigned char cmd)
{
	wait_while_busy();
	clcd_put(cmd);
}

static void clcd_putc(unsigned char c)
{
	wait_while_busy();
	set_rs();
	clcd_put(c);
	clr_rs();
}

static void clcd_clear()			{ clcd_cmd(0x01); }
static void clcd_home()				{ clcd_cmd(0x02); }
static void clcd_mode_set(unsigned int flags)	{ clcd_cmd(0x04 | flags); }
static void clcd_disp_set(unsigned int flags)	{ clcd_cmd(0x08 | flags); }
static void clcd_pos_set(unsigned int flags)	{ clcd_cmd(0x10 | flags); }
static void clcd_func_set(unsigned int flags)	{ clcd_cmd(0x20 | flags); }
static void clcd_addr_set(unsigned int addr) 	{ clcd_cmd(0x80 | addr); }
static void clcd_addr_set_cg(unsigned int addr)	{ clcd_cmd(0x40 | addr); }

static void clcd_reset()
{
	mdelay(100);
	clcd_put(0x30);
	mdelay(4);
	clcd_put(0x30);
	udelay(100);
	clcd_put(0x30);
	udelay(100);

	/* change interface to 4-bit */
	clcd_put(0x20);
	udelay(100);

	/* busy flag works now */
	clcd_func_set(8); /* 2 lines, 5x7 */
	clcd_disp_set(0); /* display on/off */
	clcd_clear();
	clcd_mode_set(2); /* increment */
}

static void __clcd_open(unsigned int mode)
{
	clcd_reset();

	clcd_disp_set(4); /* display on */
}

static struct fifo queue;

/* FIXME: do not access fifo directly but use fifo_xxx functions
 * I don't remember why I have done like this. feeling like wanting to redo
 * whole code. *shy* */
static inline void clcd_print()
{
	unsigned int i, idx;
	char *buf = queue.buf;

	clcd_clear();
	clcd_addr_set(0);
	for (i = 0; i < 16; i++) {
		idx = (queue.front + i) % QSIZE;
		if (idx == queue.rear)
			return;
		clcd_putc(buf[idx]);
	}
	clcd_addr_set(0x40);
	for (i = 16; i < 32; i++) {
		idx = (queue.front + i) % QSIZE;
		if (idx == queue.rear)
			return;
		clcd_putc(buf[idx]);
	}
}

static size_t clcd_write_core(struct file *file, void *buf, size_t len)
{
	char *src = buf;
	unsigned int i;

	for (i = 0; i < len; i++) {
		while (fifo_putb(&queue, src[i]) < 0)
			fifo_getb(&queue);
		clcd_print();
	}

	return i;
}

static size_t clcd_write(struct file *file, void *buf, size_t len)
{
	struct task *parent;
	int tid;

	parent = current;
	tid = clone(TASK_HANDLER | STACK_SHARED, &init);

	if (tid == 0) { /* parent */
		sys_yield(); /* it goes sleep as soon as exiting from system
				call to wait for its child's job to be done
				that returns the result. */
		return 0;
	} else if (tid < 0) { /* error */
		/* use errno */
		error("failed cloning");
		return -EAGAIN;
	}

	int ret;
	unsigned int irqflag;

	/* child takes place from here turning to kernel task,
	 * nomore in handler mode */
	ret = clcd_write_core(file, buf, len);

	spin_lock_irqsave(&parent->lock, irqflag);

	__set_retval(parent, ret);
	sum_curr_stat(parent);

	if (get_task_state(parent)) {
		set_task_state(parent, TASK_RUNNING);
		runqueue_add_core(parent);
	}

	spin_unlock_irqrestore(&parent->lock, irqflag);

	sys_kill(current);
	freeze(); /* never reaches here */

	return ret;
}

static int clcd_open(struct inode *inode, struct file *file)
{
	struct device *dev = getdev(file->inode->dev);
	int err = 0;

	if (dev == NULL)
		return -EFAULT;

	mutex_lock(&dev->mutex);

	if (dev->refcount++ == 0) {
		void *buf;

		if ((buf = kmalloc(QSIZE)) == NULL) {
			err = -ENOMEM;
			goto out;
		}
		fifo_init(&queue, buf, QSIZE);

		gpio_init(PIN_CLCD_DB7, GPIO_MODE_OUTPUT);
		gpio_init(PIN_CLCD_DB6, GPIO_MODE_OUTPUT);
		gpio_init(PIN_CLCD_DB5, GPIO_MODE_OUTPUT);
		gpio_init(PIN_CLCD_DB4, GPIO_MODE_OUTPUT);
		gpio_init(PIN_CLCD_E,   GPIO_MODE_OUTPUT);
		gpio_init(PIN_CLCD_RW,  GPIO_MODE_OUTPUT);
		gpio_init(PIN_CLCD_RS,  GPIO_MODE_OUTPUT);

		__clcd_open(file->flags & 0x1c);
	}

out:
	mutex_unlock(&dev->mutex);

	return err;
}

static int clcd_close(struct file *file)
{
	struct device *dev = getdev(file->inode->dev);

	if (dev == NULL)
		return -EFAULT;

	mutex_lock(&dev->mutex);

	if (--dev->refcount == 0) {
		kfree(queue.buf);
	}

	mutex_unlock(&dev->mutex);

	return 0;
}

static struct file_operations ops = {
	.open  = clcd_open,
	.read  = NULL,
	.write = clcd_write,
	.close = clcd_close,
	.seek  = NULL,
	.ioctl = NULL,
};

#include <kernel/init.h>

static void __init clcd_init()
{
	struct device *dev;

	if (!(dev = mkdev(0, 0, &ops, "clcd")))
		return;
}
MODULE_INIT(clcd_init);
