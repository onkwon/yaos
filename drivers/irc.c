#include <kernel/module.h>
#include <kernel/page.h>
#include <kernel/softirq.h>
#include <foundation.h>

#define QUEUE_SIZE	(128 * sizeof(int)) /* 128 entries of WORD_SIZE */
#define MHZ		1000000

#define GPIO_PIN_INDEX	7

static struct fifo_t irc_queue;
static unsigned int nr_softirq;
static int siglevel;
static DEFINE_SPINLOCK(irc_lock);

static void isr_irc()
{
	static unsigned int elapsed = 0;
	unsigned int stamp, ir_count_max;

	ir_count_max = get_systick_max();
	stamp = get_systick();

	if (stamp > elapsed)
		elapsed += ir_count_max - stamp;
	else
		elapsed -= stamp;

	/* make it micro second time base */
	elapsed /= (ir_count_max * HZ) / MHZ;

	spin_lock(irc_lock);
	fifo_put(&irc_queue, elapsed, sizeof(elapsed));
	spin_unlock(irc_lock);

	elapsed = stamp;
	siglevel ^= HIGH;

	raise_softirq(nr_softirq);

	ret_from_gpio_int(GPIO_PIN_INDEX);
}

static size_t irc_read(int id, void *buf, size_t len)
{
	int i, *data;
	unsigned int irqflag;

	spin_lock_irqsave(irc_lock, irqflag);
	for (i = 0, data = (int *)buf; i < len; i++) {
		data[i] = fifo_get(&irc_queue, sizeof(int));

		if (data[i] == -1)
			break;
	}
	spin_unlock_irqrestore(irc_lock, irqflag);

	return i;
}

static struct device_interface_t ops = {
	.open  = NULL,
	.read  = irc_read,
	.write = NULL,
	.close = NULL,
};

static DEFINE_WAIT_HEAD(wq);

static void daemon()
{
	unsigned int data, len, i = 0;
	while (1) {
		len = irc_read(0, &data, 1);
		if (len != 1) break;
		printf("[%02d] %d\n", i, data);
		i++;
	}

	//wq_wake(&wq, WQ_EXCLUSIVE);
}

#include <kernel/init.h>

static int __init irc_init()
{
	void *buf;
	int vector_nr, result;

	if ((buf = kmalloc(QUEUE_SIZE)) == NULL)
		return -ERR_ALLOC;
	fifo_init(&irc_queue, buf, QUEUE_SIZE);

	vector_nr = gpio_init(GPIO_PIN_INDEX, GPIO_MODE_INPUT | GPIO_CONF_PULL |
			GPIO_INT_FALLING | GPIO_INT_RISING);

	register_isr(vector_nr, isr_irc);

	if (!(result = register_dev(mkdev(), &ops, "irc"))) {
		if ((nr_softirq = register_softirq(daemon)) >= SOFTIRQ_MAX) {
			kfree(buf);
			gpio_close(GPIO_PIN_INDEX);

			return -ERR_RANGE;
		}
	}

	if (gpio_get(GPIO_PIN_INDEX))
		siglevel = HIGH;
	else
		siglevel = LOW;

	return result;
}
MODULE_INIT(irc_init);
