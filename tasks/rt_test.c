#include <foundation.h>
#include <kernel/sched.h>
#include <stdlib.h>

static void rt_task1()
{
	while (1) {
		printf("REALTIME START\n");
#ifdef CONFIG_PAGING
	show_buddy(NULL);
#endif
	show_freelist(current->stack.heap);
	display_devtab();
		sleep(3);
		printf("REALTIME END\n");
		reset_task_state(current, TASK_RUNNING);
		schedule();
	}
}
REGISTER_TASK(rt_task1, DEFAULT_STACK_SIZE, RT_LEAST_PRIORITY);
