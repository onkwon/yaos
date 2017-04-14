#include <kernel/module.h>
#include <kernel/page.h>
#include <kernel/softirq.h>
#include <asm/pinmap.h>
#include <foundation.h>
#include <kernel/systick.h>

#define QUEUE_SIZE	(128 * WORD_SIZE) /* 128 entries of WORD_SIZE */

static struct fifo ir_queue;
static unsigned int nsoftirq;
static int siglevel;

static void isr_ir(int nvector)
{
	static unsigned int elapsed = 0;
	unsigned int stamp, ir_count_max;

	ir_count_max = get_sysclk_max();
	stamp = get_sysclk();

	if (stamp > elapsed)
		elapsed += ir_count_max - stamp;
	else
		elapsed -= stamp;

	/* make it micro second time base */
	elapsed /= (ir_count_max * sysfreq) / MHZ;

	fifo_put(&ir_queue, elapsed, sizeof(elapsed));

	elapsed = stamp;
	siglevel ^= HIGH;

	raise_softirq(nsoftirq, NULL);

	ret_from_gpio_int(gpio2exti(PIN_IR));
}

static size_t ir_read(struct file *file, void *buf, size_t len)
{
	int i, *data;
	struct device *dev = getdev(file->inode->dev);

	for (i = 0, data = (int *)buf; i < len; i++) {
		spin_lock(&dev->mutex.count);
		data[i] = fifo_get(&ir_queue, sizeof(int));
		spin_unlock(&dev->mutex.count);

		if (data[i] == -1)
			break;
	}

	return i;
}

static struct file_operations ops = {
	.open  = NULL,
	.read  = ir_read,
	.write = NULL,
	.close = NULL,
	.seek  = NULL,
	.ioctl = NULL,
};

static DEFINE_WAIT_HEAD(wq);

static void ird()
{
	unsigned int data, len, i = 0;
	while (1) {
continue;
		len = ir_read(0, &data, 1);
		if (len != 1) break;
		printf("[%02d] %d\n", i, data);
		i++;
	}

	//wq_wake(&wq, WQ_EXCLUSIVE);
}

#include <kernel/init.h>

static int __init ir_init()
{
	struct device *dev;
	void *buf;
	int vector_nr;

	if ((buf = kmalloc(QUEUE_SIZE)) == NULL)
		return ENOMEM;

	fifo_init(&ir_queue, buf, QUEUE_SIZE);

	vector_nr = gpio_init(PIN_IR, GPIO_MODE_INPUT | GPIO_CONF_PULLUP |
			GPIO_INT_FALLING | GPIO_INT_RISING);

	register_isr(vector_nr, isr_ir);

	if ((dev = mkdev(0, 0, &ops, "ir"))) {
		if ((nsoftirq = request_softirq(ird, HIGHEST_PRIORITY))
				>= SOFTIRQ_MAX) {
			kfree(buf);
			gpio_close(PIN_IR);
			remove_device(dev);

			return ERANGE;
		}
	}

	if (gpio_get(PIN_IR))
		siglevel = HIGH;
	else
		siglevel = LOW;

	return 0;
}
MODULE_INIT(ir_init);
