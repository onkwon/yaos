#include "sysclk.h"
#include <foundation.h>

int sysclk_init()
{
	reset_sysclk();
	set_sysclk(get_sysclk_freq() / HZ - 1);

	return IRQ_SYSTICK;
}
