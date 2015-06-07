#include <kernel/task.h>
#include <kernel/page.h>
#include <asm/context.h>
#include <lib/firstfit.h>
#include <error.h>

struct task init;
struct task *current = &init;

int alloc_mm(struct task *new, void *ref, int option)
{
	if ((new->mm.base = kmalloc(STACK_SIZE)) == NULL)
		return -ERR_ALLOC;

	/* make its stack pointer to point out the highest memory address */
	new->mm.sp = new->mm.base + (STACK_SIZE / WORD_SIZE - 1);

	/* initialize heap for malloc() */
	new->mm.heap = (unsigned int *)ff_freelist_init(new->mm.base,
			&new->mm.base[HEAP_SIZE / WORD_SIZE]);

	new->mm.base[HEAP_SIZE / WORD_SIZE] = STACK_SENTINEL;

	switch (option) {
	case STACK_SEPARATE:
		if ((new->mm.kernel = kmalloc(KERNEL_STACK_SIZE)))
			new->mm.kernel = &new->mm.kernel[KERNEL_STACK_SIZE /
				WORD_SIZE - 1];
		break;
	case STACK_SHARE:
		new->mm.kernel = ref;
		break;
	}

	if (new->mm.kernel == NULL) {
		kfree(new->mm.base);
		return -ERR_ALLOC;
	}

	return 0;
}

void set_task_dressed(struct task *task, unsigned int flags, void *addr)
{
	if (flags & TASK_KERNEL)
		set_task_type(task, TASK_KERNEL);
	else
		set_task_type(task, TASK_USER);

	set_task_state(task, TASK_STOPPED);
	set_task_pri(task, DEFAULT_PRIORITY);
	task->addr = addr;
	INIT_IRQFLAG(task->irqflag);

	task->parent = current;
	list_link_init(&task->children);
	list_add(&task->sibling, &current->children);
	list_link_init(&task->rq);
	INIT_SCHED_ENTITY(task->se);
}

struct task *make(unsigned int flags, void (*func)(), void *ref, int option)
{
	struct task *new;

	if ((new = kmalloc(sizeof(struct task)))) {
		if (alloc_mm(new, ref, option)) {
			kfree(new);
			new = NULL;
		} else {
			set_task_dressed(new, flags, func);
			set_task_context(new); /* get dressed first */
		}
	}

	return new;
}

void kill(struct task *task)
{
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
