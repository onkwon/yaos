#include <foundation.h>
#include "shell.h"

#define WIDTH		16

static int cat(int argc, char **argv)
{
	int fd, len, i;
	char buf[16];

	if (argc != 2) {
		printf("few argument\n");
		return 0;
	}

	printf("%s:\n", argv[1]);

	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		printf("can not open\n");
		return 0;
	}

	while ((len = read(fd, buf, WIDTH))) {
		for (i = 0; i < WIDTH; i++) {
			if (i < len)
				printf("%02x ", buf[i]);
			else
				puts("   ");

			if (!((i + 1) % 4)) putchar(' ');
		}
		putchar(' ');
		for (i = 0; i < len; i++) {
			if (buf[i] >= 0x20 && buf[i] < 0x7f)
				putchar(buf[i]);
			else
				putchar('.');
		}
		putchar('\n');
	}

	close(fd);

	return 0;
}
REGISTER_CMD(cat, cat, "cat filename");
