#include "heap.h"
#include "firstfit.h"
#include "types.h"
#include "kernel/lock.h"
#include "kernel/init.h"

#include <assert.h>

extern char _ram_end, _heap_start;

static DEFINE_LIST_HEAD(freelist);
static lock_t freelist_global_lock;

void *kmalloc(size_t size)
{
	uintptr_t irqflag;
	void *p = NULL;

	if (!is_honored())
		return NULL;

	spin_lock_irqsave(&freelist_global_lock, &irqflag);
	p = firstfit_alloc(&freelist, size);
	spin_unlock_irqrestore(&freelist_global_lock, irqflag);

	return p;
}

void kfree(void *ptr)
{
	uintptr_t irqflag;

	if (!ptr || !is_honored())
		return;

	spin_lock_irqsave(&freelist_global_lock, &irqflag);
	firstfit_free(&freelist, ptr);
	spin_unlock_irqrestore(&freelist_global_lock, irqflag);
}

void heap_init(void)
{
	uintptr_t *from;
	size_t size;

	from = (void *)&_heap_start;
	//size = BASE((uintptr_t)&_ram_end - (uintptr_t)from, WORD_SIZE);

	/* TODO: preserve initial kernel stack to be free later */
	size = BASE((uintptr_t)&_ram_end - 2048 - (uintptr_t)from, WORD_SIZE);

	assert(firstfit_init(&freelist, from, size) == 0);
}
