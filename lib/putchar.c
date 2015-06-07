#include <io.h>
#include <kernel/syscall.h>

void putc(int c)
{
	write(stdout, &c, 1);

	if (c == '\n')
		putc('\r');
}

void (*putchar)(int c) = putc;

void puts(const char *s)
{
	while (*s) putchar(*s++);
}
