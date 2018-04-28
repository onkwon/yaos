#include "shell.h"
#include <foundation.h>
#include <kernel/timer.h>
#include <string.h>
#include <stdlib.h>

#include <lib/xmodem.h>

static int get()
{
	int c, ret;

	ioctl(stdin, C_EVENT, &c);
	if (c)
		read(stdin, &ret, 1);
	else
		return -1;

	return ret;
}

static int xmodem_recv(int argc, char **argv)
{
	unsigned int addr, len;
	int downloaded = 0;

	if (argc != 3)
		return -EINVAL;

	/* TODO: Check address range and length
	 * The address range should not exceed free ram area */
	addr = (unsigned int)atoi(argv[1]);
	len  = (unsigned int)atoi(argv[2]);

	printf("Downloading %d bytes at 0x%x\n", len, addr);

	ioctl(stdin, C_FLUSH);
	downloaded = xmodem_receive((void *)addr, len,
			get, (void (*)(uint8_t))putchar);

	sleep(1);
	printf("%d bytes downloaded\n", downloaded);

	return 0;
}
REGISTER_CMD(dn, xmodem_recv, "dn addr len");
