#include <kernel/module.h>
#include <kernel/page.h>
#include <error.h>
#include <fs/fs.h>
#include <asm/timer.h>
#include <timer.h>

static unsigned int major;
static int (*user_isr[NR_TIMER_MAX][TIM_CHANNEL_MAX+1])(int flags);
static int capture[NR_TIMER_MAX][TIM_CHANNEL_MAX];
static volatile int new;

static void isr_timer(int nvector)
{
	struct __timer *tim;
	unsigned int *reg;
	int flags, id, ch, i;

#ifndef CONFIG_IRQ_HIERARCHY
	nvector = get_active_irq();
#endif

	if (!(id = __get_timer_active(nvector))) {
		error("INT:unmapped irq source:%x", nvector);
		return;
	}

	tim = __timer_id2reg(id);
	flags = tim->sr;
	ch = get_timer_channel_active(flags);

	if (is_timer(flags, TIM_OVERCAPTURE_MASK)) {
		warn("INT:overcaptured!");
	}

	if (is_timer(flags, TIM_UPDATE_MASK)) {
		if (user_isr[id-1][0])
			user_isr[id-1][0](flags);
	}

	if (!ch)
		goto out;

	/* NOTE: the capture registers must be sequentially located in memory
	 * or the code below will cause undefined behavior. */
	reg = (unsigned int *)&tim->ccr1;
	for (i = 1; ch && i <= TIM_CHANNEL_MAX; i++, reg++, ch >>= 1) {
		if (ch & 1) {
			capture[id-1][i-1] = *reg;
			new |= 1 << id;

			if (user_isr[id-1][i])
				user_isr[id-1][i](flags);
		}
	}

out:
	tim->sr = 0;
}

static inline void timer_ioctl_set(int id, void *data, bool dir)
{
	timer_t *tim = (timer_t *)data;

	if (!(get_task_flags(current) & TASK_PRIVILEGED)) {
		debug("no permission");
		return;
	}

	__timer_run(id, false);

	__timer_mode_set(id, tim->mode);
	__timer_channel_set(id, tim->channel, tim->iomode, tim->pin, dir);
	__timer_match_set(id, tim->channel, tim->match);

	if (tim->prescale)
		__timer_prescale_set(id, tim->prescale);
	if (tim->period)
		__timer_period_set(id, tim->period);
	if (tim->interrupt) {
		__timer_intr_set(id, tim->channel);
		if (tim->channel < TIM_CHANNEL_MAX)
			user_isr[id-1][tim->channel] = tim->isr;
	}

	__timer_reload(id);
	__timer_run(id, true);
}

static inline void timer_ioctl_run(int id, void *data)
{
	timer_t *tim = (timer_t *)data;
	__timer_match_set(id, tim->channel, tim->match);
	/* always true when passed with timer structure becase of its address
	 * but false when passed like ioctl(fd, C_RUN, 0) */
	__timer_run(id, !!data);
}

static inline void timer_ioctl_get(int id, void *data)
{
#if 0
	tim->mode = __timer_mode_get(id);
	tim->prescale = __timer_prescale_get(id);
	tim->period = __timer_period_get(id);
	tim->match = __timer_match_get(id, tim->channel);
	tim->iomode = __timer_iomode_get(id, tim->channel);
	tim->interrupt = __timer_intr_get(id, tim->channel);
	tim->isr;
#endif
}

static void do_timer_ioctl(struct file *file, int request, void *data)
{
	struct device *dev = getdev(file->inode->dev);
	int id = MINOR(file->inode->dev);
	bool dir = !!(file->flags & O_WRONLY);
	int ret = 0;

	mutex_lock(&dev->mutex);

	switch (request) {
	case C_GET:
		timer_ioctl_get(id, data);
		break;
	case C_SET:
		timer_ioctl_set(id, data, dir);
		break;
	case C_RUN:
		timer_ioctl_run(id, data);
		break;
	case C_EVENT:
		*(int *)data = (new & (1 << id))? 1 : 0;
		break;
	default:
		ret = ERANGE;
		break;
	}

	mutex_unlock(&dev->mutex);

	syscall_delegate_return(current->parent, ret);
}

static int timer_ioctl(struct file *file, int request, void *data)
{
	struct task *thread;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					do_timer_ioctl, current)) == NULL)
		return ENOMEM;

	syscall_put_arguments(thread, file, request, data, NULL);
	syscall_delegate(current, thread);

	return 0;
}

static size_t timer_read(struct file *file, void *buf, size_t len)
{
	char *s, *d;
	unsigned int i, id;

	id = MINOR(file->inode->dev);

	if (!(new & (1 << id)))
		return 0;

	s = (char *)capture[id-1];
	d = (char *)buf;

	for (i = 0; i < len; i++)
		*d++ = *s++;

	new &= ~(1 << id);

	return i;
}

static void do_timer_close(struct file *file)
{
	struct device *dev = getdev(file->inode->dev);

	mutex_lock(&dev->mutex);

	if (--dev->refcount == 0) {
		/* TODO: reset the timer and disable the corresponding clock
		 * reset pin as well if set */
	}

	mutex_unlock(&dev->mutex);

	syscall_delegate_return(current->parent, 0);
}

static int timer_close(struct file *file)
{
	struct task *thread;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					do_timer_close, current)) == NULL)
		return ENOMEM;

	syscall_put_arguments(thread, file, NULL, NULL, NULL);
	syscall_delegate(current, thread);

	return 0;
}

static int timer_open(struct inode *inode, struct file *file)
{
	struct device *dev = getdev(file->inode->dev);
	int ret = 0;

	if (dev == NULL)
		return EFAULT;

	mutex_lock(&dev->mutex);

	if (dev->refcount == 0) {
		int nvector, dir, id;
		unsigned int hz;

		if (!(get_task_flags(current->parent) & TASK_PRIVILEGED)) {
			debug("no permission");
			mutex_unlock(&dev->mutex);
			return EPERM;
		}

		hz = (unsigned int)file->option;
		dir = file->flags & O_RDWR;
		if (dir == O_RDWR) {
			ret = EFAULT;
			goto out_unlock;
		}

		dir = dir >> 1; /* to the bool data type */
		id = MINOR(file->inode->dev);
		if ((nvector = __timer_open(id, (bool)dir, hz)) <= 0) {
			ret = EAGAIN;
			goto out_unlock;
		}

		register_isr(nvector, isr_timer);
		if (!dir || hz)
			__timer_run(id, true);
	}

	dev->refcount++;

out_unlock:
	mutex_unlock(&dev->mutex);

	return ret;
}

static struct file_operations ops = {
	.open  = timer_open,
	.read  = timer_read,
	.write = NULL,
	.close = timer_close,
	.seek  = NULL,
	.ioctl = timer_ioctl,
};

void register_timer(const char *name, int minor)
{
	macro_register_device(name, major, minor, &ops);
}
