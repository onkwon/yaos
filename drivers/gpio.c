#include <error.h>
#include <gpio.h>
#include <kernel/module.h>
#include <kernel/softirq.h>
#include <asm/pinmap.h>

static unsigned int major;
static DEFINE_LINK_HEAD(q_user_isr);
static DEFINE_MUTEX(q_lock);
static int nsoftirq;

struct uisr {
	struct link list; /* keep it in the first place */
	unsigned int id;
	int vector;
	int (*func)(void *arg);
	struct task *task;
};

static void ISR_gpio(int nvector)
{
#ifndef CONFIG_COMMON_IRQ_FRAMEWORK
	nvector = get_active_irq();
#endif
	raise_softirq(nsoftirq, (void *)nvector);
}

static size_t gpio_read(struct file *file, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	*p = gpio_get(MINOR(file->inode->dev));

	return 1;
}

static size_t gpio_write(struct file *file, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	gpio_put(MINOR(file->inode->dev), *p);

	return 1;
}

static inline int getmode(int flags)
{
	switch (flags & O_RDWR) {
	case O_RDONLY:
		return GPIO_MODE_INPUT;
	case O_WRONLY:
		return GPIO_MODE_OUTPUT;
	default:
		break;
	}

	return EFAULT;
}

static inline int getconf(int opt)
{
	return opt & (GPIO_CONF_MASK | GPIO_INT_MASK | GPIO_SPD_MASK);
}

static int gpio_open(struct inode *inode, struct file *file)
{
	struct device *dev = getdev(file->inode->dev);
	int vector, mode = 0;

	if (dev == NULL)
		return EFAULT;

	mutex_lock(&dev->mutex);

	if (dev->refcount == 0) {
#if 0
		if (!(get_task_flags(current->parent) & TF_PRIVILEGED)) {
			mode = EPERM;
			goto out_unlock;
		}
#endif

		if ((mode = getmode(file->flags)) < 0)
			goto out_unlock;

		mode |= getconf((int)file->option);

		if ((vector = gpio_init(MINOR(file->inode->dev), mode)) < 0) {
			mode = vector;
			goto out_unlock;
		}

		if (vector > 0) {
			register_isr(vector, ISR_gpio);
			/* we do not use file->offset nor seek() for gpio
			 * driver so safe to keep the vector number in it */
			file->offset = vector;
		}
	}

	dev->refcount++;

out_unlock:
	mutex_unlock(&dev->mutex);

	return mode;
}

static int gpio_close_core(struct file *file)
{
	struct link *curr, *prev;
	struct uisr *uisr;
	int ret = 0;

	mutex_lock(&q_lock);

	prev = &q_user_isr;
	for (curr = prev->next; curr; curr = curr->next) {
		uisr = (struct uisr *)curr;

		if (uisr->id != (unsigned int)file) {
			prev = curr;
			continue;
		}

		link_del(curr, prev);
		kfree(uisr);
		prev = curr;
	}

	mutex_unlock(&q_lock);

	struct device *dev = getdev(file->inode->dev);

	mutex_lock(&dev->mutex);

	if (--dev->refcount == 0) {
		gpio_reset(MINOR(file->inode->dev));

		assert(!link_empty(&q_user_isr));
	}

	mutex_unlock(&dev->mutex);

	syscall_delegate_return(current->parent, ret);

	return ret;
}

static int gpio_close(struct file *file)
{
	struct task *thread;
	struct device *dev = getdev(file->inode->dev);

	if (dev == NULL)
		return EFAULT;

#if 0
	if (!(get_task_flags(current) & TF_PRIVILEGED))
		return EPERM;
#endif

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					gpio_close_core, current)) == NULL)
		return ENOMEM;

	syscall_put_arguments(thread, file, NULL, NULL, NULL);
	syscall_delegate(current, thread);

	return 0;
}

static int gpio_isr_add(struct file *file, void *data)
{
	struct uisr *uisr;
	int ret = 0;

	if (!(uisr = kmalloc(sizeof(*uisr))))
		return ENOMEM;

	/* save the file descriptor's address for later identification when
	 * close() to remove the isr and free memory. And keep track on the
	 * owner task's privilege level to run user-define-isr in its own
	 * context in secure. */
	uisr->id = (unsigned int)file;
	uisr->vector = file->offset;
	uisr->func = data;
	uisr->task = current->parent;

	mutex_lock(&q_lock);
	link_add(&uisr->list, &q_user_isr);
	mutex_unlock(&q_lock);

	syscall_delegate_return(current->parent, ret);

	return ret;
}

static int gpio_ioctl(struct file *file, int request, void *data)
{
	struct task *thread;

#if 0
	if (!(get_task_flags(current) & TF_PRIVILEGED))
		return EPERM;
#endif

	if (request != C_EVENT)
		return EFAULT;

	if (getmode(file->flags) != GPIO_MODE_INPUT)
		return EINVAL;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					gpio_isr_add, current)) == NULL)
		return ENOMEM;

	syscall_put_arguments(thread, file, data, NULL, NULL);
	syscall_delegate(current, thread);

	return 0;
}

static struct file_operations ops = {
	.open  = gpio_open,
	.read  = gpio_read,
	.write = gpio_write,
	.close = gpio_close,
	.seek  = NULL,
	.ioctl = gpio_ioctl,
};

void register_gpio(const char *name, int minor)
{
	macro_register_device(name, major, minor, &ops);
}

static void gpiod(void *args)
{
	struct link *curr;
	struct uisr *uisr;
	struct task *thread;
	unsigned int mode;
	int vector;

	vector = (int)args;

	mutex_lock(&q_lock);

	for (curr = q_user_isr.next; curr; curr = curr->next) {
		uisr = (struct uisr *)curr;

		if (uisr->vector != vector)
			continue;

		mode = (get_task_flags(uisr->task) & TASK_PRIVILEGED) |
			STACK_SHARED;

		if ((thread = make(mode, STACK_SIZE_DEFAULT, uisr->func,
						uisr->task)))
			go_run(thread);
	}

	mutex_unlock(&q_lock);
}

#include <kernel/init.h>

static void __init module_gpio_init()
{
	if ((nsoftirq = request_softirq(gpiod, HIGHEST_PRIORITY)) >= SOFTIRQ_MAX)
		error("full of softirq!");
}
MODULE_INIT(module_gpio_init);
