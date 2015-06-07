#include <kernel/module.h>
#include <kernel/page.h>
#include <kernel/softirq.h>
#include <foundation.h>

#define QUEUE_SIZE	(128 * WORD_SIZE) /* 128 entries of WORD_SIZE */
#define MHZ		1000000

#define GPIO_PIN_INDEX	7

static struct fifo ir_queue;
static unsigned int nr_softirq;
static int siglevel;

static void isr_ir()
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

	fifo_put(&ir_queue, elapsed, sizeof(elapsed));

	elapsed = stamp;
	siglevel ^= HIGH;

	raise_softirq(nr_softirq);

	ret_from_gpio_int(GPIO_PIN_INDEX);
}

static size_t ir_read(struct file *file, void *buf, size_t len)
{
	int i, *data;

	for (i = 0, data = (int *)buf; i < len; i++) {
		data[i] = fifo_get(&ir_queue, sizeof(int));

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
};

static DEFINE_WAIT_HEAD(wq);

static void daemon()
{
	unsigned int data, len, i = 0;
	while (1) {
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
		return -ERR_ALLOC;

	fifo_init(&ir_queue, buf, QUEUE_SIZE);

	vector_nr = gpio_init(GPIO_PIN_INDEX, GPIO_MODE_INPUT | GPIO_CONF_PULL |
			GPIO_INT_FALLING | GPIO_INT_RISING);

	register_isr(vector_nr, isr_ir);

	if ((dev = mkdev(0, 0, &ops, "ir"))) {
		if ((nr_softirq = request_softirq(daemon)) >= SOFTIRQ_MAX) {
			kfree(buf);
			gpio_close(GPIO_PIN_INDEX);
			remove_device(dev);

			return -ERR_RANGE;
		}
	}

	if (gpio_get(GPIO_PIN_INDEX))
		siglevel = HIGH;
	else
		siglevel = LOW;

	return 0;
}
MODULE_INIT(ir_init);
