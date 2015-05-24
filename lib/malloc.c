#include <stdlib.h>
#include <lib/firstfit.h>
#include <kernel/task.h>

void *malloc(size_t size)
{
	return ff_alloc(&current->mm.heap, size);

	//return (void *)syscall(SYSCALL_BRK, size);
}

void free(void *addr)
{
	ff_free(&current->mm.heap, addr);
}
