#include <foundation.h>
#include <kernel/jiffies.h>

void sleep(unsigned long sec)
{
	unsigned long timeout = jiffies + sec_to_jiffies(sec);

	while (time_before(timeout, jiffies)) {
		/* sleep. Implement cascaded timer wheel */
	}
}

void msleep(unsigned long ms)
{
	unsigned long timeout = jiffies + msec_to_jiffies(ms);

	while (time_before(timeout, jiffies)) {
		/* sleep. Implement cascaded timer wheel */
	}
}

unsigned long set_timeout(unsigned long ms)
{
	return jiffies + msec_to_jiffies(ms);
}

int is_timeout(unsigned long goal)
{
	if (time_after(goal, jiffies))
		return 1;

	return 0;
}
