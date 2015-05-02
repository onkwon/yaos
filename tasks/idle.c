#include <foundation.h>
#include <kernel/sched.h>

static struct task_t *get_rt_task()
{
	extern int _user_task_list;
	struct task_t *p = (struct task_t *)&_user_task_list;

	while (p->state) {
		if (IS_TASK_REALTIME(p))
			break;

		p++;
	}

	return p;
}

extern void print_rq();
extern int get_shared();
#include <driver/usart.h>
#include <kernel/mm.h>
static void idle()
{
	/*
	extern int _user_task_list;
	struct task_t *task = (struct task_t *)&_user_task_list;

	while (task->stack) {
		printf("%x\n", task->state);
		task++;
	}
	*/
	struct task_t *rt_test = get_rt_task();

	SET_PORT_CLOCK(ENABLE, PORTD);
	SET_PORT_PIN(PORTD, 2, PIN_OUTPUT_50MHZ);
	while (1) {
		if (usart_kbhit(USART1)) {
			PUT_PORT(PORTD, GET_PORT(PORTD) ^ 4);
			usart_getc(USART1);
			//printf("idle() %c = %d\n", usart_getc(USART1), get_shared());
	show_free_list(NULL);
			print_rq();
			set_task_state(rt_test, TASK_RUNNING);
			runqueue_add(rt_test);
			schedule();
		}
		//printf("idle()\n");
		//mdelay(500);
	}
}

REGISTER_TASK(idle, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);
