#include <io.h>
#include <kernel/syscall.h>

void (*putchar)(int c) = putc;

void fputc(int fd, int c)
{
	write(fd, &c, 1);

	if (c == '\n') fputc(fd, '\r');
}

void putc(int c)
{
	fputc(stdout, c);
}

void puts(const char *s)
{
	while (*s) putchar(*s++);
}
