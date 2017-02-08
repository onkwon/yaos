#include "shell.h"
#include <foundation.h>
#include <string.h>

static int usart(int argc, char **argv)
{
	char *dev;
	int fd, req;
	unsigned int brr;

	if (argc < 3) goto out;

	if        (!strcmp(argv[1], "setbrr")) { req = C_BAUDRATE;
	} else if (!strcmp(argv[1], "getbrr")) { req = C_GET_BAUDRATE;
	} else if (!strcmp(argv[1], "flush"))  { req = C_FLUSH;
	} else if (!strcmp(argv[1], "kbhit"))  { req = C_KBHIT;
	} else {
		printf("wrong request %s\n", argv[1]);
		goto out;
	}

	if        (!strcmp(argv[2], "com1")) { dev = "/dev/usart1";
	} else if (!strcmp(argv[2], "com2")) { dev = "/dev/usart2";
	} else if (!strcmp(argv[2], "com3")) { dev = "/dev/usart3";
	} else if (!strcmp(argv[2], "com4")) { dev = "/dev/usart4";
	} else if (!strcmp(argv[2], "com5")) { dev = "/dev/usart5";
	} else {
		printf("unknown port %s\n", argv[2]);
		goto out;
	}

	if (argc == 4) brr = atoi(argv[3]);
	else brr = 0;

	if ((fd = open(dev, O_RDWR)) <= 0) {
		printf("open error: %s\n", dev);
		goto out;
	}

	printf("%s\n", (ioctl(fd, req, brr) < 0)? "Error" : "OK");

	close(fd);

out:
	return 0;
}
REGISTER_CMD(usart, usart, "usart <setbrr|getbrr|flush|kbhit> <comN> <baudrate>");
