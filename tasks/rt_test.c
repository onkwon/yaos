#include <foundation.h>
#include <kernel/sched.h>

#include <kernel/mm.h>
static void rt_task1()
{
	while (1) {
		printf("REALTIME START\n");
	show_free_list(NULL);
		sleep(3);
		printf("REALTIME END\n");
		reset_task_state(current, TASK_RUNNING);
		schedule();
	}
}

REGISTER_TASK(rt_task1, DEFAULT_STACK_SIZE, RT_LEAST_PRIORITY);
