#include "heap.h"
#include "types.h"
#include "kernel/lock.h"
#include "kernel/init.h"
#include "kernel/task.h"

#include <assert.h>
#include <string.h>

extern char _ram_end, _heap_start;

static DEFINE_LIST_HEAD(freelist);
static lock_t freelist_global_lock;

void *kmalloc(size_t size)
{
	void *p = NULL;

	if (!is_honored())
		return NULL;

	spin_lock_irqsave(&freelist_global_lock);
	p = firstfit_alloc(&freelist, size);
	spin_unlock_irqrestore(&freelist_global_lock);

	return p;
}

void kfree(void *ptr)
{
	if (!ptr || !is_honored())
		return;

	spin_lock_irqsave(&freelist_global_lock);
	firstfit_free(&freelist, ptr);
	spin_unlock_irqrestore(&freelist_global_lock);
}

void heap_init(void)
{
	uintptr_t *from;
	size_t size;

	from = (void *)&_heap_start;
	//size = BASE((uintptr_t)&_ram_end - (uintptr_t)from, WORD_SIZE);

	/* TODO: preserve initial kernel stack to be free later */
	size = BASE((uintptr_t)&_ram_end - STACK_SIZE_DEFAULT - (uintptr_t)from, WORD_SIZE);

	int res = firstfit_init(&freelist, from, size);
	assert(res == 0);
}

void *__wrap_malloc(size_t size)
{
	void *p = firstfit_alloc(&current->heap.freelist, size);

	if (p)
		memset(p, 0, size);

	return p;
}

void __wrap_free(void *ptr)
{
	if (!ptr)
		return;

	firstfit_free(&current->heap.freelist, ptr);
}
