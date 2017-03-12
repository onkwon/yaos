#include <kernel/task.h>
#include <kernel/page.h>
#include <asm/context.h>
#include <lib/firstfit.h>
#include <error.h>

struct task init;
struct task *current = &init;

int alloc_mm(struct task *new, size_t size, unsigned int flags, void *ref)
{
	if ((new->mm.base = kmalloc(size)) == NULL)
		return -ERR_ALLOC;

	if ((new->mm.heap = kmalloc(HEAP_SIZE_DEFAULT)) == NULL) {
		kfree(new->mm.base);
		return -ERR_ALLOC;
	}

	/* make its stack pointer to point out the highest memory address.
	 * full descending stack */
	new->mm.sp = new->mm.base + (size / WORD_SIZE);
	new->mm.base[0] = STACK_SENTINEL;

	/* initialize heap for malloc() */
	heap_init(&new->mm.heaphead, new->mm.heap,
			&new->mm.heap[HEAP_SIZE_DEFAULT / WORD_SIZE]);

	if (flags & STACK_SHARED) {
		new->mm.kernel.sp = ((struct task *)ref)->mm.kernel.sp;
		new->mm.kernel.base = ((struct task *)ref)->mm.kernel.base;
	} else {
		if ((new->mm.kernel.sp = kmalloc(size))) {
			new->mm.kernel.base = new->mm.kernel.sp;
			new->mm.kernel.base[0] = STACK_SENTINEL;
			new->mm.kernel.sp = &new->mm.kernel.base[size /
				WORD_SIZE];
		}
	}

	if (new->mm.kernel.sp == NULL) {
		kfree(new->mm.base);
		kfree(new->mm.heap);

		return -ERR_ALLOC;
	}

	return 0;
}

void set_task_dressed(struct task *task, unsigned int flags, void *addr)
{
	set_task_flags(task, flags);
	set_task_state(task, TASK_STOPPED);
	set_task_pri(task, DEFAULT_PRIORITY);
	task->addr = addr;
	INIT_IRQFLAG(task->irqflag);

	task->parent = current;
	links_init(&task->children);
	/* FIXME: lock before adding to parent children list */
	links_add(&task->sibling, &current->children);
	links_init(&task->rq);

	INIT_SCHED_ENTITY(task->se);
	task->se.vruntime = current->se.vruntime;

	lock_init(&task->lock);
}

struct task *make(unsigned int flags, size_t size, void *addr, void *ref)
{
	struct task *new;
	void *entry = wrapper;

	if (flags & TF_ATOMIC) {
		entry = addr;
		flags &= ~TF_ATOMIC;
	}

	if ((new = kmalloc(sizeof(struct task)))) {
		if (alloc_mm(new, size, flags, ref)) {
			kfree(new);
			new = NULL;
		} else {
			set_task_dressed(new, flags, addr);
			set_task_context(new, entry); /* get dressed first */
			new->name = NULL;
		}
	}

	return new;
}

#include <kernel/interrupt.h>
#include <kernel/lock.h>

static volatile void *zombie;
static DEFINE_MUTEX(zombie_mutex);

/* NOTE: A task can kill only itself. Otherwise a task would be destroyed while
 * being in a waitqueue or somewhere else. Others only can send a signal to
 * request to be killed by itself voluntarily. */
void sys_kill_core(struct task *target, struct task *killer)
{
	unsigned int irqflag;

	if (target == &init)
		return;

	if (killer->mm.base[0] != STACK_SENTINEL)
		error("stack overflow %x(%x)" , killer, killer->addr);

	irq_save(irqflag);
	local_irq_disable();

	assert(!is_locked(target->lock));
	set_task_state(target, TASK_ZOMBIE);
	/* safe to unlink again even if it's already removed from the runqueue
	 * since its links become empty on once unlinked */
	runqueue_del_core(target);

	/* Clean its relationship. */
	/* FIXME: hand its own children to the grand parents if it has any */
	/* TODO: is this kind of relationship amongst tasks really needed? */
	if (!links_empty(&target->children)) {
		links_del(&target->children);
	}
	links_del(&target->sibling);

	/* wake init() up to do the rest of job of destroying a task */
	if (killer != &init) {
		/* boost the init task priority up to the task's so that the
		 * memory used by it can be free as soon as possible */
		assert(!is_locked(init.lock));

		runqueue_del_core(&init);
		set_task_pri(&init, get_task_pri(killer));
		set_task_state(&init, TASK_RUNNING);
		runqueue_add_core(&init);
	}

	irq_restore(irqflag);

	/* add it to zombie list */
	mutex_lock(&zombie_mutex);
	target->addr = (void *)zombie;
	zombie = target;
	mutex_unlock(&zombie_mutex);

	if (killer == current) /* called directly or called by sys_kill() */
		resched();
}

static void destroy(struct task *task)
{
	if (!task || task == &init)
		return;

	kfree(task->mm.base);
	kfree(task->mm.heap);

	if (!(get_task_flags(task) & STACK_SHARED))
		kfree(task->mm.kernel.base);

	/* TODO: free the static tasks as well
	 * not possible to free the static tasks at the moment as its memory
	 * region is not managed by buddy. */
	if (!(get_task_flags(task) & TASK_STATIC))
		kfree(task);
}

unsigned int kill_zombie()
{
	struct task *task;
	unsigned int cnt;

	for (cnt = 0; zombie; cnt++) {
		mutex_lock(&zombie_mutex);
		task = (void *)zombie;
		zombie = task->addr;
		mutex_unlock(&zombie_mutex);

		destroy(task);
	}

	return cnt;
}

struct task *find_task(unsigned int addr, struct task *head)
{
	struct task *next, *p;

	if ((unsigned int)head->addr == addr)
		return head;

	if (links_empty(&head->children))
		return NULL;

	/* FIXME: lock. link can be broken while searching */
	next = get_container_of(head->children.next, struct task, sibling);

	if ((p = find_task(addr, next)))
		return p;

	head = next;

	while (head->sibling.next != &head->parent->children) {
		next = get_container_of(
				head->sibling.next, struct task, sibling);
		if ((p = find_task(addr, next)))
			return p;
		head = next;
	}

	return NULL;
}

#include <kernel/systick.h>

void wrapper()
{
	debug("[%08x] New task %x started, type:%x state:%x pri:%x",
	      systick, current->addr, get_task_type(current),
	      get_task_state(current), get_task_pri(current));

	((void (*)())current->addr)();

	debug("[%08x] The task %x done", systick, current->addr);

	kill(current);
	freeze(); /* never reaches here */
}

void syscall_delegate_return(struct task *task, int ret)
{
	assert(!is_locked(task->lock));

	__set_retval(task, ret);
	/* FIXME: it messes up calling sum_curr_stat() when make() */
	//sum_curr_stat(task);
	go_run(task);

	/* time to die as the delegated job is done. */
	sys_kill_core(current, current);

	freeze(); /* never reaches here */
}

#include <stdlib.h>

/* It must be kept as noinline function because of inline assembler placed in
 * the wrapper function, clone(). Otherwise `__save_curr_context()` macro will
 * not make the right register set and the current context will be ruined. */
static int __attribute__((noinline)) clone_core(unsigned int flags, void *ref,
		struct regs *regs, unsigned int sp)
{
	struct task *child;

	if ((child = kmalloc(sizeof(struct task))) == NULL)
		return -ERR_ALLOC;

	if (alloc_mm(child, STACK_SIZE_DEFAULT, flags, ref)) {
		kfree(child);
		return -ERR_ALLOC;
	}

	/* copy stack */
	unsigned int base, top, size;
	unsigned int *src, *dst;

	/* A user task starts from the next instruction of `fork()` in the user
	 * context, not from `clone()` in the handler mode. */
	if (flags & TF_SYSCALL)
		sp = __get_usp();
	else
		flags |= TF_CLONED;

	base = (unsigned int)current->mm.base;
	top  = base + STACK_SIZE_DEFAULT;

	if (flags & TF_HANDLER) { /* if handler mode */
		base = (unsigned int)current->mm.kernel.base;
		top  = base + STACK_SIZE_DEFAULT;
	}

#ifdef ARMv7A
	if (!(flags & TF_HANDLER))
		sp  -= NR_CONTEXT * WORD_SIZE;
#endif
	size = top - sp;
	src  = (unsigned int *)sp;
	dst  = (unsigned int *)((unsigned int)child->mm.sp - size);
	memcpy(dst, src, size);

	child->mm.sp = dst; /* update stack pointer */

	if (!(flags & TF_SYSCALL)) { /* if not syscall */
#ifdef ARMv7A
		if (!(flags & TF_HANDLER)) {
			child->mm.sp += NR_CONTEXT;
			size         -= NR_CONTEXT * WORD_SIZE;
		}
#endif
		unsigned int *p = (unsigned int *)regs;

		set_task_flags(child, flags);
		set_task_context_hard(child, NULL);

		/* set status register to default */
		p[INDEX_PSR] = child->mm.sp[INDEX_PSR - NR_CONTEXT_SOFT];

		memcpy(child->mm.sp, p + NR_CONTEXT_SOFT,
				NR_CONTEXT_HARD * WORD_SIZE);
		size += NR_CONTEXT_HARD * WORD_SIZE;
	}

	set_task_context_soft(child);
	memcpy(child->mm.sp, regs, NR_CONTEXT_SOFT * WORD_SIZE);
	size += NR_CONTEXT_SOFT * WORD_SIZE;

	/* without MMU virtualization needs to manipulate stack to rearrange
	 * the addresses to point out the right ones, not addressing
	 * the original(its parent) address space. this way doesn't seem
	 * charming and probably leads problems or bugs in the future. */
	unsigned int i;
	unsigned int limit = (unsigned int)child->mm.base + STACK_SIZE_DEFAULT;
	for (i = 0; (i < size / WORD_SIZE) &&
			((unsigned int)(child->mm.sp+i) < limit); i++) {
		if ((child->mm.sp[i] > base) && (child->mm.sp[i] < top))
			child->mm.sp[i] = limit - (top - child->mm.sp[i]);
	}

	__set_retval(child, (int)child);

	set_task_dressed(child, flags, NULL);
	set_task_pri(child, get_task_pri(current));
	set_task_state(child, TASK_RUNNING);
	runqueue_add(child);

	return 0;
}

#ifdef CONFIG_DEBUG_CLONE
int clone_overhead;
#endif

int clone(unsigned int flags, void *ref)
{
	struct regs regs;
	int result;

	__save_curr_context((unsigned int *)&regs);

	if (get_task_flags(current) & TF_CLONED) {
		set_task_flags(current, get_task_flags(current) & ~TF_CLONED);
		return get_task_tid(current);
	}

#ifdef CONFIG_DEBUG_CLONE
	clone_overhead = get_sysclk();
#endif
	result = clone_core(flags, ref, &regs, __get_sp());
#ifdef CONFIG_DEBUG_CLONE
	clone_overhead -= get_sysclk();
#endif

	return result;
}

#ifdef ARMv7A
int sys_fork()
{
	int tid = clone(TASK_SYSCALL | STACK_SHARED |
			(get_task_flags(current) & TASK_PRIVILEGED), &init);
	if (tid > 0) /* the newly forked task never gets back here */
		sys_kill_core(current, current);

	/* schedule to give chance for child to run first */
	resched();

	return tid;
}
#else
/* TODO: remove architecture dependent code under `/arch` directory */
void __attribute__((naked)) sys_fork_finish()
{
	__asm__ __volatile__(
			"push	{r0}		\n\t" /* save return value */
			"tst	r0, 0		\n\t"
			"ittt	gt		\n\t"
			"movgt	r0, %0		\n\t"
			"movgt  r1, %0		\n\t"
			"blgt	sys_kill_core	\n\t"
			"bl	resched		\n\t"
			:
			: "r"(current)
			: "r0",
			"r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
			"memory");

	__asm__ __volatile__("pop {r0, pc}" ::: "memory");
}

int __attribute__((naked)) sys_fork()
{
	__asm__ __volatile__("push {lr}" ::: "memory");

	__asm__ __volatile__(
			"and	r0, %0, %1	\n\t"
			"orr	r0, r0, %2	\n\t"
			"orr	r0, r0, %3	\n\t"
			"mov	r1, %4		\n\t"
			"bl	clone		\n\t"
			"b	sys_fork_finish	\n\t"
			:
			: "r"(get_task_flags(current)), "I"(TASK_PRIVILEGED),
			"I"(TASK_SYSCALL), "I"(STACK_SHARED), "r"(&init)
			: "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
			"memory");
}
#endif

static void do_sys_kill(struct task *task)
{
	sys_kill_core(task, current->parent);
	sys_kill_core(current, current);
	freeze(); /* never reaches here */
}

int sys_kill(void *task)
{
	struct task *thread;

	if ((struct task *)task != current) {
		warn("no permission");
		return -ERR_PERM;
	}

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					do_sys_kill, current)) == NULL)
		return -ERR_ALLOC;

	syscall_put_arguments(thread, task, NULL, NULL, NULL);
	syscall_delegate(current, thread);

	return 0;
}
