#include <foundation.h>
#include <kernel/sched.h>
#include <kernel/task.h>
#include <kernel/page.h>
#include <kernel/module.h>
#include <kernel/syscall.h>
#include <lib/firstfit.h>

static void rt_task1()
{
	while (1) {
		printf("REALTIME START\n");
#ifdef CONFIG_PAGING
	show_buddy(NULL);
#endif
	show_freelist(current->mm.heap);
#ifdef CONFIG_SYSCALL
	display_devtab();
#endif
		sleep(3);
		printf("REALTIME END\n");
		yield();
	}
}
REGISTER_TASK(rt_task1, TASK_USER, RT_LEAST_PRIORITY);
