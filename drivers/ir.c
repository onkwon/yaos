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

#include <foundation.h>
#include <kernel/systick.h>
#include <kernel/module.h>

#define QUEUE_SIZE	(128 * WORD_SIZE) /* 128 entries of WORD_SIZE */

static DEFINE_WAIT_HEAD(wq);
static struct fifo ir_queue;
static unsigned int major;

static void ISR_ir(int nvector)
{
	static unsigned int last, elapsed;
	unsigned int stamp, ir_count_max, t;

	stamp = get_sysclk();
	ir_count_max = get_sysclk_max();
	t = *(unsigned int *)&systick;
	elapsed += (ir_count_max * (t - last)) - stamp;
	last = t;

	/* make it micro second time base */
	elapsed /= (ir_count_max * sysfreq) / MHZ;

	if (elapsed < 100000) { // 100ms
		fifo_putw(&ir_queue, elapsed);
		wq_wake(&wq, WQ_EXCLUSIVE);
	}

	elapsed = stamp;

	(void)nvector;
}

static size_t ir_read_core(void *buf, size_t len)
{
	int i, *data = (int *)buf;

	for (i = 0; i < len; i++) {
		data[i] = fifo_getw(&ir_queue);

		if (data[i] == -ENOENT)
			break;
	}

	return i;
}

static size_t do_ir_read(struct file *file, void *buf, size_t len)
{
	if (file->flags & O_NONBLOCK)
		return ir_read_core(buf, len);

	size_t total, gotten;

	for (total = 0; total < len;) {
		if ((gotten = ir_read_core(buf + (total*sizeof(int)), len - total)) <= 0) {
			wq_wait(&wq);
			continue;
		}
		total += gotten;
	}

	return total;
}

static size_t ir_read(struct file *file, void *buf, size_t len)
{
	syscall_delegate_atomic(do_ir_read, &current->mm.sp, &current->flags);
	return 0;

	(void)file;
	(void)(int)buf;
	(void)len;
}

static inline int ir_init()
{
	void *buf;
	int vector;

	if ((buf = kmalloc(QUEUE_SIZE)) == NULL)
		return -ENOMEM;

	/* permission check here */
	fifo_init(&ir_queue, buf, QUEUE_SIZE);
	vector = gpio_init(PIN_IR, GPIO_MODE_INPUT | GPIO_CONF_PULLUP |
			GPIO_INT_FALLING | GPIO_INT_RISING);
	register_isr(vector, ISR_ir);

	WQ_INIT(wq);

	return 0;
}

static int ir_open(struct inode *inode, struct file *file)
{
	struct device *dev = getdev(file->inode->dev);
	(void)inode;

	if (dev == NULL)
		return -EFAULT;

	spin_lock(&dev->mutex.counter);

	if (dev->refcount == 0) {
		if (ir_init()) {
			spin_unlock(&dev->mutex.counter);
			return -ENOMEM;
		}
	}

	dev->refcount++;

	spin_unlock(&dev->mutex.counter);

	return 0;
}

static struct file_operations ops = {
	.open  = ir_open,
	.read  = ir_read,
	.write = NULL,
	.close = NULL,
	.seek  = NULL,
	.ioctl = NULL,
};

void register_ir(const char *name, int minor)
{
	macro_register_device(name, major, minor, &ops);
}
