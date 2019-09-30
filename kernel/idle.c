#include "kernel/task.h"
#include "kernel/systick.h"
#include "syslog.h"

void idle_task(void)
{
	set_task_pri(current, TASK_PRIORITY_LOWEST);

	while (1) {
		debug("%lx: %lu", get_systick(), (uint32_t)current->sum_exec_runtime);
		mdelay(1000);
	}
}
