#include "foundation.h"

#include "driver/usart.h"
static void idle()
{
	/*
	extern int _user_task_list;
	struct task_t *task = (struct task_t *)&_user_task_list;

	while (task->stack) {
		printf("%x\n", task->flags);
		task++;
	}
	*/
	while (1) {
		if (usart_kbhit(USART1)) {
			printf("%c", usart_getc(USART1));
		}
		//printf("idle()\n");
		//mdelay(500);
	}
}

#include "task.h"
REGISTER_TASK(idle, STACK_SIZE_DEFAULT, NORMAL_PRIORITY);
