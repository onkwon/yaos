#include <error.h>
#include <kernel/module.h>
#include <kernel/gpio.h>
#include <asm/pinmap.h>

static unsigned int major;
static struct task *daemon;
static volatile bool pending;
static DEFINE_LINK_HEAD(q_user_isr);
static DEFINE_MUTEX(q_lock);

struct uisr {
	struct link list; /* keep it in the first place */
	unsigned int id;
	int vector;
	int (*func)(void *arg);
	struct task *task;
};

static void isr()
{
	if (!daemon)
		goto out;

	if (get_task_state(daemon) == TASK_SLEEPING) {
		daemon->args = (void *)get_active_irq();
		go_run_atomic_if(daemon, TASK_SLEEPING);
	} else {
		daemon->args = (void *)get_active_irq();
		pending = true;
	}

out:
	ret_from_exti(get_active_irq());
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

	return -ERR_UNDEF;
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
		return -ERR_UNDEF;

	mutex_lock(&dev->mutex);

	if (dev->refcount == 0) {
		if (!(get_task_flags(current->parent) & TF_PRIVILEGED)) {
			mode = -ERR_PERM;
			goto out_unlock;
		}

		if ((mode = getmode(file->flags)) < 0)
			goto out_unlock;

		mode |= getconf((int)file->option);

		if (!(vector = gpio_init(MINOR(file->inode->dev), mode))) {
			mode = -ERR_DUP;
			goto out_unlock;
		}

		if (vector > 0) {
			register_isr(vector, isr);
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
		return -ERR_UNDEF;

	if (!(get_task_flags(current) & TF_PRIVILEGED))
		return -ERR_PERM;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					gpio_close_core, current)) == NULL)
		return -ERR_ALLOC;

	syscall_put_arguments(thread, file, NULL, NULL, NULL);
	syscall_delegate(current, thread);

	return 0;
}

static int gpio_isr_add(struct file *file, void *data)
{
	struct uisr *uisr;
	int ret = 0;

	if (!(uisr = kmalloc(sizeof(*uisr))))
		return -ERR_ALLOC;

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

	if (!(get_task_flags(current) & TF_PRIVILEGED))
		return -ERR_PERM;

	if (request != C_EVENT)
		return -ERR_UNDEF;

	if (getmode(file->flags) != GPIO_MODE_INPUT)
		return -ERR_ATTR;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					gpio_isr_add, current)) == NULL)
		return -ERR_ALLOC;

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

static void gpio_daemon()
{
	struct link *curr;
	struct uisr *uisr;
	struct task *thread;
	unsigned int mode;
	int vector;

	/* register daemon to be notified to isr */
	daemon = current;

endless:
	vector = (int)daemon->args;

	mutex_lock(&q_lock);

	for (curr = q_user_isr.next; curr; curr = curr->next) {
		uisr = (struct uisr *)curr;

		if (uisr->vector != vector)
			continue;

		mode = (get_task_flags(uisr->task) & TASK_PRIVILEGED) |
			STACK_SHARED;

		if ((thread = make(mode, STACK_SIZE_MIN, uisr->func, uisr->task))) {
			go_run(thread);
			/* NOTE: you will not be able to service all the
			 * interrupts if you do reschedule here. even if not
			 * reschedule here, there is still chance to miss,
			 * pending more than once. */
			//resched();
		}
	}

	mutex_unlock(&q_lock);

	if (pending) {
		pending = false;
		goto endless;
	}

	yield();

	goto endless;
}
REGISTER_TASK(gpio_daemon, TASK_KERNEL, DEFAULT_PRIORITY, STACK_SIZE_MIN);
