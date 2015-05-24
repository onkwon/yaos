#include <kernel/module.h>
#include <kernel/page.h>
#include <kernel/softirq.h>
#include <foundation.h>

#define QUEUE_SIZE		128
#define MHZ			1000000

#define GPIO_PIN_INDEX		7

static struct fifo_t irc_queue;
static spinlock_t irc_lock;

static unsigned int nr_softirq;
static int siglevel;

static void isr_irc()
{
	static unsigned int elapsed = 0;
	unsigned int stamp, ir_count_max;
	unsigned int irqflag;

	ir_count_max = get_systick_max();
	stamp = get_systick();

	if (stamp > elapsed)
		elapsed += ir_count_max - stamp;
	else
		elapsed -= stamp;

	/* make it micro second time base */
	elapsed /= (ir_count_max * HZ) / MHZ;

	spin_lock_irqsave(irc_lock, irqflag);
	fifo_put(&irc_queue, elapsed, sizeof(elapsed));
	spin_unlock_irqrestore(irc_lock, irqflag);

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

static struct waitqueue_head_t wq;

static void daemon()
{
	//wq_wake(&wq, WQ_EXCLUSIVE);
}

static int irc_init()
{
	void *buf;
	int vector_nr, result;

	if ((buf = kmalloc(QUEUE_SIZE)) == NULL)
		return -ERR_ALLOC;

	fifo_init(&irc_queue, buf, QUEUE_SIZE);
	INIT_SPINLOCK(irc_lock);
	INIT_WAIT_HEAD(wq);

	vector_nr = gpio_init(GPIO_PIN_INDEX, GPIO_MODE_INPUT | GPIO_CONF_PULL |
			GPIO_INT_FALLING | GPIO_INT_RISING);

	register_isr(vector_nr, isr_irc);

	if ((result = register_device(dev_get_newid(), &ops, "irc")) == 0) {
		struct task_t *task;

		if ((task = make(TASK_KERNEL, daemon, init.mm.kernel,
						STACK_SHARE)) == NULL) {
			kfree(buf);
			gpio_close(GPIO_PIN_INDEX);

			return -ERR_ALLOC;
		}

		if ((nr_softirq = register_softirq(task)) >= SOFTIRQ_MAX) {
			kfree(buf);
			gpio_close(GPIO_PIN_INDEX);
			kill(task);

			return -ERR_RANGE;
		}
	}

	/* check pin level */
	siglevel = HIGH;

	return result;
}

module_init(irc_init);
