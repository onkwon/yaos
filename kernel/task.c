#include "kernel/task.h"
#include "kernel/sched.h"
#include "syslog.h"

struct task *current, init;

#include "kernel/systick.h"
#include "drivers/gpio.h"
#include "drivers/uart.h"
#include <string.h>
#include "arch/mach/hw_clock.h"
static uart_t uart1;
static char mybuf[80];
extern unsigned long systick;

static void myputc(const int c)
{
	uart1.writeb(&uart1, c);
}

void cb(int vector)
{
	static int val = 0;
	val ^= 1;
	gpio_put(45, val);

	debug("VECTOR %d", vector);
}

static void idle(void)
{
	gpio_init(45, GPIO_MODE_OUTPUT, NULL);
	gpio_init(20, GPIO_MODE_INPUT | GPIO_CONF_PULLUP | GPIO_INT_FALLING, cb);

	do {
		gpio_put(45, 0);
		for (unsigned int i = 0; i < 0xfffff; i++) ;
		gpio_put(45, 1);
		for (unsigned int i = 0; i < 0xfffff; i++) ;
		printf("Hello, World!\r\n");
	//	test();
	} while (0);

	const char *str = "Hello, World!\r\n";
	uint32_t t;
	char byte;
	uart1 = uart_new(UART1);
	uart1.open_static(&uart1, &t, sizeof(t), NULL, 0);
	printc = myputc;

	uart1.write(&uart1, str, strlen(str));
	sprintf(mybuf, "clk %lu\r\n", hw_clock_get_hclk());
	uart1.write(&uart1, mybuf, strlen(mybuf));

	while (1) {
		syslog("syslog %lu", systick);
		syslog("stack used %d", (uintptr_t)current->stack.base + STACK_SIZE_MIN - (uintptr_t)current->stack.watermark);
		syslog("stack margin left %ld", current->stack.watermark - current->stack.base);
		syslog("kernel stack used %d", (uintptr_t)current->kstack.base + STACK_SIZE_MIN - (uintptr_t)current->kstack.watermark);
		syslog("kernel stack margin left %ld", current->kstack.watermark - current->kstack.base);
		uart1.write(&uart1, "ON\r\n", 4);
		while (uart1.readb(&uart1, &byte));
		uart1.write(&uart1, ">> ", 3);
		uart1.writeb(&uart1, byte);
		uart1.write(&uart1, "\r\n", 2);
		gpio_put(45, 0);
		uart1.write(&uart1, "OFF\r\n", 5);
		while (uart1.readb(&uart1, &byte));
		uart1.write(&uart1, ">> ", 3);
		uart1.writeb(&uart1, byte);
		uart1.write(&uart1, "\r\n", 2);
		gpio_put(45, 1);
	}
}

static void set_task_dressed(struct task *task, unsigned long flags, void *addr)
{
	set_task_flags(task, flags);
	set_task_state(task, TASK_STOPPED);
	set_task_pri(task, TP_DEFAULT);
	task->addr = addr;
	task->irqflag = INITIAL_IRQFLAG;

	task->parent = current;
	list_init(&task->children);
	/* FIXME: lock before adding to parent children list */
	list_add(&task->sibling, &current->children);
	//links_init(&task->rq);

	//INIT_SCHED_ENTITY(task->se);
	//task->se.vruntime = current->se.vruntime;

	lock_init(&task->lock);

	//link_init(&task->timer_head);
}

#if 0
static void __attribute__((noinline, used)) wrapper_info(void)
{
	debug("[%08x] New task %x started, type:%x state:%x pri:%x rank:%s",
	      systick, current->addr, get_task_type(current),
	      get_task_state(current), get_task_pri(current),
	      get_current_rank() == TF_USER? "Unprivileged" : "Privileged");
}

static void __attribute__((noinline, used)) wrapper_info_dtor(void)
{
	debug("[%08x] The task %x done", systick, current->addr);
}

void __attribute__((naked)) task_wrapper(void)
{
	__wrapper_save_regs();
#ifdef CONFIG_DEBUG_TASK
	__wrapper_jump(wrapper_info);
	__wrapper_restore_regs_and_exec(current->addr);
	__wrapper_jump(wrapper_info_dtor);
#else
	__wrapper_restore_regs_and_exec(current->addr);
#endif

	kill(current);
	freeze(); /* never reaches here */
}
#endif

#if defined(CONFIG_SCHEDULER)
#define NR_SP		(STACK_SIZE_MIN / sizeof(uintptr_t))
#define NR_KSP		(STACK_SIZE_DEFAULT / sizeof(uintptr_t))
#define NR_HEAP		(HEAP_SIZE_MIN / sizeof(uintptr_t))

static uintptr_t _init_sp[NR_SP],
		 _init_ksp[NR_KSP],
		 _init_heap[NR_HEAP];

void task_init(void)
{
#if defined(CONFIG_MEM_WATERMARK)
	for (unsigned int i = 0; i < NR_SP; i++)
		_init_sp[i] = STACK_WATERMARK;
	for (unsigned int i = 0; i < NR_KSP; i++)
		_init_ksp[i] = STACK_WATERMARK;
	for (unsigned int i = 0; i < NR_HEAP; i++)
		_init_heap[i] = STACK_WATERMARK;
#endif
	/* stack must be allocated first. and to build root relationship
	 * properly `current` must be set to `init`. */
	current = &init;

	init.stack.base = _init_sp;
	init.stack.p = (void *)
		BASE((uintptr_t)&_init_sp[NR_SP - 1], STACK_ALIGNMENT);
	init.kstack.base = _init_ksp;
	init.kstack.p = (void *)
		BASE((uintptr_t)&_init_ksp[NR_KSP - 1], STACK_ALIGNMENT);
	init.heap.base = _init_heap;
	init.heap.limit = (void *)
		BASE((uintptr_t)&_init_heap[NR_HEAP - 1], STACK_ALIGNMENT);

	set_task_dressed(&init, TASK_KERNEL | TASK_STATIC, idle);
	set_task_context_hard(&init, idle); // TODO: wrapper
	set_task_state(&init, TASK_RUNNING);

	init.name = "idle";

	/* make it the sole */
	list_init(&init.children);
	list_init(&init.sibling);
}
#endif
