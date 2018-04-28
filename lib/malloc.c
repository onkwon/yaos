#include <stdlib.h>
#include <lib/firstfit.h>
#include <kernel/task.h>

void *malloc(size_t size)
{
	return ff_alloc(&current->mm.heaphead, size);

	//return (void *)syscall(SYSCALL_BRK, size);
}

void free(void *addr)
{
	ff_free(&current->mm.heaphead, addr);
}

void task_heap_free(void *addr, struct task *task)
{
	ff_free(&task->mm.heaphead, addr);
}
