#include "kernel/debug.h"

void debug_init(void)
{
	hw_debug_init(1, 2000000);
}
