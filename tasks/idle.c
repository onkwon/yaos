#include <foundation.h>

extern int get_shared();
#include <driver/usart.h>
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
	SET_PORT_CLOCK(ENABLE, PORTD);
	SET_PORT_PIN(PORTD, 2, PIN_OUTPUT_50MHZ);
	while (1) {
		if (usart_kbhit(USART1)) {
			PUT_PORT(PORTD, GET_PORT(PORTD) ^ 4);
			usart_getc(USART1);
			//printf("idle() %c = %d\n", usart_getc(USART1), get_shared());
		}
		//printf("idle()\n");
		//mdelay(500);
	}
}

#include <task.h>
REGISTER_TASK(idle, STACK_SIZE_DEFAULT, NORMAL_PRIORITY);
