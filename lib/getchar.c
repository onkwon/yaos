#include <io.h>
#include <kernel/syscall.h>

int getc()
{
	int c;

	if (!read(stdin, &c, 1))
		c = -1;

	return c;
}
