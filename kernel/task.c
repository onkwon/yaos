#include "kernel/task.h"
#include "kernel/sched.h"
#include "kernel/systick.h"
#include "kernel/syscall.h"
#include "kernel/lock.h"
#include "syslog.h"
#include "heap.h"

#include <string.h>
#include <errno.h>
#include <assert.h>

static DEFINE_LISTQ_HEAD(zombie_list);

static void set_task_dressed(struct task *task, unsigned long flags,
		const void *addr)
{
	set_task_flags(task, flags);
	set_task_state(task, TASK_STOPPED);
	set_task_pri(task, TASK_PRIORITY_DEFAULT);
	task->addr = addr;
	task->irqflag = INITIAL_IRQFLAG;

	lock_init(&task->lock);

	task->parent = current;
	list_init(&task->children);

	spin_lock_irqsave(&current->lock);
	list_add(&task->sibling, &current->children);
	spin_unlock_irqrestore(&current->lock);

#if defined(CONFIG_TASK_EXECUTION_TIME)
	task->sum_exec_runtime = 0;
#endif
	list_init(&task->rq);

	//INIT_SCHED_ENTITY(task->se);
	//task->se.vruntime = current->se.vruntime;

	//link_init(&task->timer_head);
}

static void __attribute__((noinline)) task_ctor(void)
{
	debug("[%08lx] %s task %s@%p newly started, f:%lx s:%lx p:%x",
			get_systick(),
			is_privileged()?  "Privileged" : "Unprivileged",
			current->name, current->addr, get_task_flags(current),
			get_task_state(current), get_task_pri(current));
}

static void __attribute__((noinline)) task_dtor(void)
{
	debug("[%08lx] %s task %s@%p done",
			get_systick(),
			is_privileged()?  "Privileged" : "Unprivileged",
			current->name, current->addr);
}

static void __attribute__((naked)) task_decorator(void)
{
	task_decorator_prepare();

	task_decorator_run_helper(task_ctor);
	task_decorator_exec(current->addr);
	task_decorator_run_helper(task_dtor);

	task_kill(current);
	yield();
}

static inline int task_alloc(struct task *task, unsigned int flags, void *ref)
{
	uintptr_t *stack, *heap;
	uintptr_t *kstack = NULL;
	size_t heap_size = HEAP_SIZE_DEFAULT;

	if (!task)
		return -EFAULT;

	if ((stack = kmalloc(task->size)) == NULL)
		return -ENOMEM;

	if ((heap = kmalloc(heap_size)) == NULL) {
		kfree(stack);
		return -ENOMEM;
	}

	/* NOTE: full descending stack */
	task->stack.base = stack;
	task->stack.p = (void *)BASE((uintptr_t)
			&stack[task->size / sizeof(*stack)], STACK_ALIGNMENT);
	task->heap.base = heap;
	task->heap.limit = (void *)BASE((uintptr_t)
			&heap[heap_size / sizeof(*heap)], STACK_ALIGNMENT);

	if (ref && (flags & TF_SHARED)) {
		task->kstack.base = ((struct task *)ref)->kstack.base;
		task->kstack.p = ((struct task *)ref)->kstack.p;
	} else {
		size_t kernel_stack_size = STACK_SIZE_MIN;

		if ((kstack = kmalloc(kernel_stack_size)) == NULL) {
			kfree(stack);
			kfree(heap);
			return -ENOMEM;
		}

		task->kstack.base = kstack;
		task->kstack.p = (void *)BASE((uintptr_t)
				&kstack[kernel_stack_size / sizeof(*kstack)],
				STACK_ALIGNMENT);
	}

	int res = firstfit_init(&task->heap.freelist, task->heap.base, heap_size);
	assert(res == 0);

#if defined(CONFIG_MEM_WATERMARK)
	while ((uintptr_t)stack < (uintptr_t)task->stack.p)
		*stack++ = STACK_WATERMARK;
	if (kstack) {
		while ((uintptr_t)kstack < (uintptr_t)task->kstack.p)
			*kstack++ = STACK_WATERMARK;
	}
#endif

	return 0;
}

static void task_destroy(struct task *task)
{
	if (!task || task == &init_task)
		return;

	// TODO: implement
	// 1. change task state first to TASK_STOPPED to make sure nothing shared with others
	// 2. remove from wait queue and run queue
	// 3. clean its relationship, child and siblings
	// 4. free stack, heap, and kstack if not shared one

	uintptr_t stack = (uintptr_t)task->stack.base;
	uintptr_t heap = (uintptr_t)task->heap.base;
	uintptr_t kstack = (uintptr_t)task->kstack.base;

	kfree((void *)stack);
	kfree((void *)heap);

	if (!(get_task_flags(task) & TF_SHARED))
		kfree((void *)kstack);
}

void task_kill(struct task *task)
{
	if (!task || task == &init_task)
		return;

	set_task_state(task, TASK_ZOMBIE);
	// FIXME: it gets stuck if called from an interrupt not a thread
	sysq_push(&task->rq, &zombie_list);
}

void free_zombie(void)
{
	struct list *p;

	while ((p = sysq_pop(&zombie_list))) {
		struct task *task = container_of(p, struct task, rq);
		task_destroy(task);
	}
}

/* assume that the task itself is to be locked and the wait queue is also
 * to be locked here, meaning nothing is holdind the locks so that the locks
 * can be obtained immediately here */
int task_wait(void *waitqueue, struct task *task)
{
	assert(waitqueue && task && get_task_state(task) == TASK_RUNNING);

	set_task_state(task, TASK_WAITING);
	listq_push(&task->rq, waitqueue);

	resched();

	return 0;
}

int task_wake(void *waitqueue)
{
	assert(waitqueue);

	struct listq_head *q = waitqueue;

	if (listq_empty(q))
		return -ENOENT;

	struct task *task;
	struct list *node;

	node = listq_pop(q);
	assert(node && node != (struct list *)&q->next);

	task = container_of(node, struct task, rq);
	assert(get_task_state(task) == TASK_WAITING);
	set_task_state(task, TASK_RUNNING);
	runqueue_add_core(task);

	return 0;
}

extern struct task _user_task_list;

static void load_user_tasks(void)
{
	struct task *task;
	int pri;
	size_t nr_tasks;

	nr_tasks = 0;

	for (task = (struct task *)&_user_task_list; *(uintptr_t *)task; task++) {
		if (task->addr == NULL)
			continue;

		if (task_alloc(task, TF_SHARED, &init_task)) {
			error("failed loading task %s@%p",
					task->name, task->addr);
			continue;
		}

		pri = get_task_pri(task);
		set_task_dressed(task, task->flags | TF_SHARED, task->addr);
		set_task_pri(task, pri);
		set_task_context(task, task_decorator);

		if (!(get_task_flags(task) & TF_MANUAL)) {
			/* make it runnable, and add into runqueue */
			set_task_state(task, TASK_RUNNING);
			if (runqueue_add_core(task)) {
				task_destroy(task);
			}
		}

		set_task_flags(task, get_task_flags(task) & ~TF_MANUAL);
		nr_tasks++;
	}

	debug("number of tasks created: %u", nr_tasks);
}

void task_init(void)
{
#define NR_SP_ITEMS		(STACK_SIZE_MIN / sizeof(uintptr_t))
#define NR_KSP_ITEMS		(STACK_SIZE_DEFAULT / sizeof(uintptr_t))
#define NR_HEAP_ITEMS		(HEAP_SIZE_MIN / sizeof(uintptr_t))

	static uintptr_t init_task_sp[NR_SP_ITEMS],
			 init_task_ksp[NR_KSP_ITEMS],
			 init_task_heap[NR_HEAP_ITEMS];

#if defined(CONFIG_MEM_WATERMARK)
	for (unsigned int i = 0; i < NR_SP_ITEMS; i++)
		init_task_sp[i] = STACK_WATERMARK;
	for (unsigned int i = 0; i < NR_KSP_ITEMS; i++)
		init_task_ksp[i] = STACK_WATERMARK;
#endif
	/* stack must be allocated first. and to build root relationship
	 * properly `current` must be set to `init`. */
	current = &init_task;

	init_task.stack.base = init_task_sp;
	init_task.stack.p = (void *)
		BASE((uintptr_t)&init_task_sp[NR_SP_ITEMS], STACK_ALIGNMENT);
	init_task.kstack.base = init_task_ksp;
	init_task.kstack.p = (void *)
		BASE((uintptr_t)&init_task_ksp[NR_KSP_ITEMS], STACK_ALIGNMENT);
	init_task.heap.base = init_task_heap;
	init_task.heap.limit = (void *)
		BASE((uintptr_t)&init_task_heap[NR_HEAP_ITEMS], STACK_ALIGNMENT);

	int res = firstfit_init(&init_task.heap.freelist, init_task.heap.base, HEAP_SIZE_MIN);
	assert(res == 0);

	set_task_dressed(&init_task, TASK_KERNEL | TASK_STATIC, idle_task);
	set_task_context_hard(&init_task, task_decorator);
	set_task_state(&init_task, TASK_RUNNING);

	init_task.name = "idle";
	init_task.sched = NULL;

	/* make it the sole */
	list_init(&init_task.children);
	list_init(&init_task.sibling);

	/* done setting the init task
	 * now load user tasks registered statically */
	load_user_tasks();

	debug("kmem left %u", kmem_left());
}
