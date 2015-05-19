#include <kernel/task.h>
#include <stdlib.h>

struct task_t init;
struct task_t *current = &init;

unsigned long *alloc_stack(struct task_t *p)
{
	if ( (p->stack.size <= 0) || !(p->stack.heap =
				(unsigned long *)kmalloc(p->stack.size)) )
		return NULL;

	/* make its stack pointer to point out the highest memory address */
	p->stack.sp = p->stack.heap + (p->stack.size / sizeof(long) - 1);

	/* heap size takes one fourth out of stack size */
	p->stack.brk = p->stack.heap + (p->stack.size >> 2) / sizeof(long);

	/* initialize heap for malloc() */
	p->stack.heap = (unsigned long *)
		ff_freelist_init(p->stack.heap, p->stack.brk);

	return p->stack.sp;
}

struct task_t *find_task(unsigned long id, struct task_t *head)
{
	struct task_t *next, *p;

	if ((unsigned long)head->addr == id)
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
