#include <kernel/init.h>

#include "system_nrf52.h"

REGISTER_INIT(SystemInit, 0);

#include <foundation.h>

int __init console_init()
{
	extern int sys_open_core(char *filename, int mode, void *opt);

	stdin = stdout = stderr =
		sys_open_core(DEVFS_ROOT CONSOLE, O_RDWR | O_NONBLOCK, NULL);

	return 0;
}
