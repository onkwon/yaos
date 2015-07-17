#include <foundation.h>
#include <kernel/jiffies.h>

void sleep(unsigned int sec)
{
	unsigned int timeout = jiffies + sec_to_jiffies(sec);

	while (time_before(timeout, jiffies)) {
		/* sleep. Implement cascaded timer wheel */
	}
}

void msleep(unsigned int ms)
{
	unsigned int timeout = jiffies + msec_to_jiffies(ms);

	while (time_before(timeout, jiffies)) {
		/* sleep. Implement cascaded timer wheel */
	}
}

void set_timeout(unsigned int *tv, unsigned int ms)
{
	*tv = jiffies + msec_to_jiffies(ms);
}

int is_timeout(unsigned int goal)
{
	if (time_after(goal, jiffies))
		return 1;

	return 0;
}
