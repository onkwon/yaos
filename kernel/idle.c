#include "kernel/task.h"
#include "kernel/systick.h"
#include "syslog.h"

void idle_task(void)
{
	while (1) {
		debug("%lx: %lu", get_systick(), (uint32_t)current->sum_exec_runtime);
		mdelay(1000);
	}
}
