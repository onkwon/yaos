#include <foundation.h>
#include <stdlib.h>
#include <gpio.h>

#define QUEUE_SIZE		128
#define MHZ			1000000

#define GPIO_PIN_INDEX		7

static struct fifo_t irc_queue;
static spinlock_t irc_lock;
static int siglevel = HIGH;

static void isr_irc()
{
	static unsigned int elapsed = 0;
	unsigned int stamp, ir_count_max;
	unsigned int irq_flag;

	ir_count_max = get_systick_max();
	stamp = get_systick();

	if (stamp > elapsed)
		elapsed += ir_count_max - stamp;
	else
		elapsed -= stamp;

	/* make it micro second time base */
	elapsed /= (ir_count_max * HZ) / MHZ;

	spin_lock_irqsave(irc_lock, irq_flag);
	fifo_put(&irc_queue, elapsed, sizeof(elapsed));
	spin_unlock_irqrestore(irc_lock, irq_flag);

	elapsed = stamp;
	siglevel ^= HIGH;

	ret_from_gpio_int(GPIO_PIN_INDEX);
}

static size_t irc_read(int id, void *buf, size_t len)
{
	int i, *data;
	unsigned int irq_flag;

	spin_lock_irqsave(irc_lock, irq_flag);

	for (i = 0, data = (int *)buf; i < len; i++) {
		data[i] = fifo_get(&irc_queue, sizeof(int));

		if (data[i] == -1)
			break;
	}

	spin_unlock_irqrestore(irc_lock, irq_flag);

	return i;
}

static struct device_interface_t ops = {
	.open  = NULL,
	.read  = irc_read,
	.write = NULL,
	.close = NULL,
};

static int irc_init()
{
	void *buf;
	int vector_nr;

	if ((buf = kmalloc(QUEUE_SIZE)) == NULL)
		return -ERR_ALLOC;

	fifo_init(&irc_queue, buf, QUEUE_SIZE);
	spinlock_init(irc_lock);

	vector_nr = gpio_init(GPIO_PIN_INDEX, GPIO_MODE_INPUT | GPIO_CONF_PULL |
			GPIO_INT_FALLING | GPIO_INT_RISING);

	register_isr(vector_nr, isr_irc);

	return register_device(dev_get_newid(), &ops, "irc");
}

module_init(irc_init);
