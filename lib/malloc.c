#include <stdlib.h>
#include <syscall.h>

void *malloc(size_t size)
{
	return ff_alloc(&current->stack.heap, size);

	//return (void *)syscall(SYSCALL_BRK, size);
}

void free(void *addr)
{
	ff_free(&current->stack.heap, addr);
}
