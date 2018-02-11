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

static void ISR_ir(int nvector)
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

	fifo_putw(&ir_queue, elapsed);

	elapsed = stamp;
	siglevel ^= HIGH;

	raise_softirq(nsoftirq, NULL);
}

static size_t ir_read(struct file *file, void *buf, size_t len)
{
	int i, *data;

	for (i = 0, data = (int *)buf; i < len; i++) {
		data[i] = fifo_getw(&ir_queue);

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
	int vector;

	if ((buf = kmalloc(QUEUE_SIZE)) == NULL)
		return -ENOMEM;

	fifo_init(&ir_queue, buf, QUEUE_SIZE);

	vector = gpio_init(PIN_IR, GPIO_MODE_INPUT | GPIO_CONF_PULLUP |
			GPIO_INT_FALLING | GPIO_INT_RISING);

	register_isr(vector, ISR_ir);

	if ((dev = mkdev(0, 0, &ops, "ir"))) {
		if ((nsoftirq = request_softirq(ird, HIGHEST_PRIORITY))
				>= SOFTIRQ_MAX) {
			kfree(buf);
			gpio_close(PIN_IR);
			remove_device(dev);

			return -ERANGE;
		}
	}

	if (gpio_get(PIN_IR))
		siglevel = HIGH;
	else
		siglevel = LOW;

	return 0;
}
MODULE_INIT(ir_init);
