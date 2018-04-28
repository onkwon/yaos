#include "shell.h"
#include <foundation.h>
#include <string.h>
#include <stdlib.h>

extern size_t flash_program(void * const addr, const void * const buf,
		size_t len, bool overwrite);

/* NOTE: Make sure you have right permission and enough stack memory to call
 * flash_program(). Interrupts get disabled in the function. */
static int flash(int argc, char **argv)
{
	unsigned int dst, src, len;
	int written;

	if (argc != 4)
		return -EINVAL;

	/* TODO: Check address range and length */
	dst = (unsigned int)strtol(argv[1], NULL, 16);
	src = (unsigned int)strtol(argv[2], NULL, 16);
	len = (unsigned int)atoi(argv[3]);

	printf("Flash %d bytes from 0x%x to 0x%x\n", len, src, dst);

	written = flash_program((void *)dst, (void *)src, len, false);

	printf("%d bytes written\n", written);

	return 0;
}
REGISTER_CMD(flash, flash, "flash 0xdst 0xsrc len");
