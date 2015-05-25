#include <kernel/task.h>
#include <kernel/page.h>
#include <asm/context.h>
#include <lib/firstfit.h>
#include <error.h>

struct task_t init;
struct task_t *current = &init;

int alloc_mm(struct task_t *p, void *ref, int option)
{
	if ((p->mm.base = (unsigned int *)kmalloc(STACK_SIZE)) == NULL)
		return -ERR_ALLOC;

	/* make its stack pointer to point out the highest memory address */
	p->mm.sp = p->mm.base + (STACK_SIZE / sizeof(int) - 1);

	/* initialize heap for malloc() */
	p->mm.heap = (unsigned int *)ff_freelist_init(p->mm.base,
			&p->mm.base[HEAP_SIZE / sizeof(int)]);

	switch (option) {
	case STACK_SEPARATE:
		if ((p->mm.kernel = (unsigned int *)kmalloc(KERNEL_STACK_SIZE)))
			p->mm.kernel = &p->mm.kernel[KERNEL_STACK_SIZE /
				sizeof(int) - 1];
		break;
	case STACK_SHARE:
		p->mm.kernel = ref;
		break;
	}

	if (p->mm.kernel == NULL) {
		kfree(p->mm.base);
		return -ERR_ALLOC;
	}

	return 0;
}

void set_task_dressed(struct task_t *task, unsigned int flags, void *addr)
{
	task->state = 0;

	if (flags & TASK_KERNEL)
		set_task_type(task, TASK_KERNEL);

	set_task_state(task, TASK_STOPPED);
	set_task_priority(task, DEFAULT_PRIORITY);
	task->addr = addr;
	INIT_IRQFLAG(task->irqflag);

	task->parent = current;
	list_link_init(&task->children);
	list_add(&task->sibling, &current->children);
	list_link_init(&task->rq);
	INIT_SCHED_ENTITY(task->se);
}

struct task_t *make(unsigned int flags, void (*func)(), void *ref, int option)
{
	struct task_t *new;

	if ((new = (struct task_t *)kmalloc(sizeof(struct task_t)))) {
		if (alloc_mm(new, ref, option)) {
			kfree(new);
			new = NULL;
		} else {
			set_task_dressed(new, flags, func);
			set_task_context_hard(new);
		}
	}

	return new;
}

void kill(struct task_t *task)
{
}

struct task_t *find_task(unsigned int id, struct task_t *head)
{
	struct task_t *next, *p;

	if ((unsigned int)head->addr == id)
		return head;

	if (list_empty(&head->children))
		return NULL;

	next = get_container_of(head->children.next, struct task_t, sibling);

	if ((p = find_task(id, next)))
		return p;

	head = next;

	while (head->sibling.next != &head->parent->children) {
		next = get_container_of(
				head->sibling.next, struct task_t, sibling);
		if ((p = find_task(id, next)))
			return p;
		head = next;
	}

	return NULL;
}
