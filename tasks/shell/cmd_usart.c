#include "shell.h"
#include <foundation.h>
#include <string.h>
#include <stdlib.h>

static int uart(int argc, char **argv)
{
	char *dev;
	int fd, req;
	unsigned int brr;

	if (argc < 3) goto err;

	if        (!strcmp(argv[1], "setbrr")) { req = C_FREQ;
	} else if (!strcmp(argv[1], "getbrr")) { req = C_FREQ;
	} else if (!strcmp(argv[1], "flush"))  { req = C_FLUSH;
	} else if (!strcmp(argv[1], "kbhit"))  { req = C_EVENT;
	} else {
		printf("wrong request %s\n", argv[1]);
		goto err;
	}

	if        (!strcmp(argv[2], "com1")) { dev = "/dev/uart1";
	} else if (!strcmp(argv[2], "com2")) { dev = "/dev/uart2";
	} else if (!strcmp(argv[2], "com3")) { dev = "/dev/uart3";
	} else if (!strcmp(argv[2], "com4")) { dev = "/dev/uart4";
	} else if (!strcmp(argv[2], "com5")) { dev = "/dev/uart5";
	} else {
		printf("unknown port %s\n", argv[2]);
		goto err;
	}

	if (argc == 4) brr = atoi(argv[3]);
	else brr = 0;

	if ((fd = open(dev, O_RDWR)) <= 0) {
		printf("open error: %s\n", dev);
		goto err;
	}

	printf("%s\n", (ioctl(fd, req, &brr) < 0)? "Error" : "OK");
	if (req == C_FREQ)
		printf("baudrate: %d\n", brr);

	close(fd);

	return 0;
err:
	return -1;
}
REGISTER_CMD(uart, uart, "uart <setbrr|getbrr|flush|kbhit> <comN> <baudrate>");
