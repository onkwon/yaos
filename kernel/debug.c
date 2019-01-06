#include "kernel/debug.h"

void debug_putc(const int c)
{
	hw_debug_putc(c);
}

void debug_init(void)
{
	hw_debug_init(1, 2000000);
}
