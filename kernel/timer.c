#include <foundation.h>

void sleep(unsigned long sec)
{
	unsigned long timeout = jiffies + sec * HZ;

	while (time_before(timeout, jiffies)) {
		/* sleep. Implement cascaded timer wheel */
	}
}

void msleep(unsigned long ms)
{
	unsigned long timeout = jiffies + ms * HZ / 1000;

	while (time_before(timeout, jiffies)) {
		/* sleep. Implement cascaded timer wheel */
	}
}
