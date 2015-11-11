#include <kernel/task.h>
#include <kernel/page.h>
#include <asm/context.h>
#include <lib/firstfit.h>
#include <error.h>

struct task init;
struct task *current = &init;

int alloc_mm(struct task *new, void *ref, unsigned int flags)
{
	if ((new->mm.base = kmalloc(USER_SPACE_SIZE)) == NULL)
		return -ERR_ALLOC;

	/* make its stack pointer to point out the highest memory address.
	 * full descending stack */
	new->mm.sp = new->mm.base + (USER_SPACE_SIZE / WORD_SIZE);

	/* initialize heap for malloc() */
	new->mm.heap = (unsigned int *)ff_freelist_init(new->mm.base,
			&new->mm.base[HEAP_SIZE / WORD_SIZE]);

	new->mm.base[HEAP_SIZE / WORD_SIZE] = STACK_SENTINEL;

	if (flags & STACK_SHARED) {
		new->mm.kernel.sp = ((struct task *)ref)->mm.kernel.sp;
		new->mm.kernel.base = ((struct task *)ref)->mm.kernel.base;
	} else {
		if ((new->mm.kernel.sp = kmalloc(STACK_SIZE))) {
			new->mm.kernel.base = new->mm.kernel.sp;
			new->mm.kernel.base[0] = STACK_SENTINEL;
			new->mm.kernel.sp = &new->mm.kernel.base[STACK_SIZE /
				WORD_SIZE];
		}
	}

	if (new->mm.kernel.sp == NULL) {
		kfree(new->mm.base);
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
	list_link_init(&task->children);
	list_add(&task->sibling, &current->children);
	list_link_init(&task->rq);

	INIT_SCHED_ENTITY(task->se);
	task->se.vruntime = current->se.vruntime;
}

struct task *make(unsigned int flags, void *addr, void *ref)
{
	struct task *new;

	if ((new = kmalloc(sizeof(struct task)))) {
		if (alloc_mm(new, ref, flags)) {
			kfree(new);
			new = NULL;
		} else {
			set_task_dressed(new, flags, addr);
			set_task_context(new, wrapper); /* get dressed first */
		}
	}

	return new;
}

#include <kernel/interrupt.h>
#include <kernel/lock.h>

static unsigned int *zombie = NULL;
static DEFINE_SPINLOCK(zombie_lock);

static void unlink_task(struct task *task)
{
	if (task == &init)
		return;

	if ((get_task_state(task) == TASK_RUNNING) && (current != task))
		runqueue_del(task);

	unsigned int irqflag;
	irq_save(irqflag);
	local_irq_disable();

	/* Clean its relationship. Hand children to grand parents
	 * if it has its own children */
	list_del(&task->children);
	list_del(&task->sibling);

	/* add it to zombie list */
	spin_lock(zombie_lock);
	task->addr = zombie;
	zombie = (unsigned int *)task;
	spin_unlock(zombie_lock);

	set_task_state(task, TASK_ZOMBIE);

	irq_restore(irqflag);

	/* wake init() up to do the rest of job destroying a task */
	if (current == &init)
		return;

	if (get_task_state(&init) == TASK_RUNNING)
		runqueue_del(&init);

	set_task_pri(&init, get_task_pri(current));
	set_task_state(&init, TASK_RUNNING);
	runqueue_add(&init);

	resched();
}

unsigned int kill_zombie()
{
	struct task *task;
	unsigned int irqflag, cnt;

	for (cnt = 0; zombie; cnt++) {
		spin_lock_irqsave(zombie_lock, irqflag);
		task = (struct task *)zombie;
		zombie = task->addr;
		spin_unlock_irqrestore(zombie_lock, irqflag);
		destroy(task);
	}

	return cnt;
}

void destroy(struct task *task)
{
	if (task == &init)
		return;

	kfree(task->mm.base);
	if (!(get_task_flags(task) & STACK_SHARED))
		kfree(task->mm.kernel.base);

	if (!(get_task_flags(task) & TASK_STATIC))
		kfree(task);
}

struct task *find_task(unsigned int addr, struct task *head)
{
	struct task *next, *p;

	if ((unsigned int)head->addr == addr)
		return head;

	if (list_empty(&head->children))
		return NULL;

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

void wrapper()
{
	debug(MSG_DEBUG, "addr %x", current->addr);
	debug(MSG_DEBUG, "type %08x, state %08x, pri %08x",
			get_task_type(current),
			get_task_state(current),
			get_task_pri(current));
	debug(MSG_DEBUG, "control %08x, sp %08x, msp %08x, psp %08x",
			GET_CNTL(), GET_SP(), GET_KSP(), GET_USP());

	((void (*)())current->addr)();
	kill((unsigned int)current);

	/* never reaches here */
	while (1);
}

#include <stdlib.h>

int clone(unsigned int flags, void *ref)
{
	struct task *child;
	unsigned int regs[NR_CONTEXT], *p;

	p = regs;
	__save_curr_context(p); /* child starts to run from here unless
				   it is a user task. A user task starts
				   from the next instruction of `fork()`
				   in the user context */

	if (get_task_flags(current) & TASK_CLONED) {
		set_task_flags(current, get_task_flags(current) & ~TASK_CLONED);
		return get_task_tid(current);
	}

	if ((child = kmalloc(sizeof(struct task))) == NULL)
		return -ERR_ALLOC;

	if (alloc_mm(child, ref, flags)) {
		kfree(child);
		return -ERR_ALLOC;
	}

	/* copy stack */
	unsigned int base, top, size;
	unsigned int *src, *dst;
	unsigned int sp;

	sp   = __get_usp();
	base = (unsigned int)&current->mm.base[HEAP_SIZE / WORD_SIZE];
	top  = base + USER_STACK_SIZE;

	if (flags & TASK_HANDLER) { /* if handler mode */
		sp   = __get_sp();
		base = (unsigned int)current->mm.kernel.base;
		top  = base + STACK_SIZE;
	}

#ifdef ARMv7A
	if (!(flags & TASK_HANDLER))
		sp  -= NR_CONTEXT * WORD_SIZE;
#endif
	size = top - sp;
	src  = (unsigned int *)sp;
	dst  = (unsigned int *)((unsigned int)child->mm.sp - size);
	memcpy(dst, src, size);

	child->mm.sp = dst; /* update stack pointer */

	if (!(flags & TASK_SYSCALL)) { /* if not syscall */
#ifdef ARMv7A
		if (!(flags & TASK_HANDLER)) {
			child->mm.sp += NR_CONTEXT;
			size         -= NR_CONTEXT * WORD_SIZE;
		}
#endif
		p = regs;
		set_task_flags(child, flags);
		set_task_context_hard(child, NULL);
#ifdef ARMv7A
		regs[INDEX_PSR] = child->mm.sp[INDEX_PSR];
#endif
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
	unsigned int limit = (unsigned int)child->mm.base + USER_SPACE_SIZE;
	for (i = 0; (i < size / WORD_SIZE) &&
			((unsigned int)(child->mm.sp+i) < limit); i++) {
		if ((child->mm.sp[i] > base) && (child->mm.sp[i] < top))
			child->mm.sp[i] = limit - (top - child->mm.sp[i]);
	}

	if (!(flags & TASK_SYSCALL))
		flags |= TASK_CLONED;
	else
		__set_retval(child, (int)child);

	set_task_dressed(child, flags, NULL);
	set_task_pri(child, get_task_pri(current));
	set_task_state(child, TASK_RUNNING);
	runqueue_add(child);

	return 0;
}

int sys_fork()
{
	int tid = clone(TASK_SYSCALL | STACK_SHARED |
			(get_task_flags(current) & TASK_PRIVILEGED), &init);
	if (tid > 0) /* the newly forked task never gets back here */
		sys_kill((unsigned int)current);

	/* schedule to give chance for child to run first */
	resched();

	return tid;
}

int sys_kill(unsigned int tid)
{
	unlink_task((struct task *)tid);
	return 0;
}
