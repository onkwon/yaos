#include "shell.h"
#include <foundation.h>
#include <string.h>

static int usart(int argc, char **argv)
{
	char *dev;
	int request, fd;
	unsigned int value;

	if (argc < 3) goto out;

	switch (argv[2][0]) {
	case '1':
		dev = "/dev/usart1";
		break;
	case '2':
		dev = "/dev/usart2";
		break;
	case '3':
		dev = "/dev/usart3";
		break;
	case '4':
		dev = "/dev/usart4";
		break;
	case '5':
		dev = "/dev/usart5";
		break;
	default:
		goto out;
	}

	request = atoi(argv[1]);
	if (argc == 4) value = atoi(argv[3]);
	else value = 0;

	char *desc[4] = { "FLUSH", "KBHIT", "GET_BAUDRATE", "SET_BAUDRATE" };
	printf("Request %s to value of %d on %s\n", desc[request-1], value, dev);

	if ((fd = open(dev, O_RDWR)) <= 0) {
		debug(MSG_USER, "open error");
		goto out;
	}

	int ret = ioctl(fd, request, value);
	printf("%s %d\n", (ret < 0)? "ERR" : "Done", ret);

	close(fd);

out:
	return 0;
}
REGISTER_CMD(usart, usart, "usart nREQ channel value");
