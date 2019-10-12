#include "shell.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define DEFAULT_DUMP_SIZE		256UL
#define MAX_DUMP_SIZE			8096U
#define getbyte(addr)			(*(unsigned char *)(addr))

static void (*putch)(int c);
static int (*getch)(void);

static inline void putstr(const char * const str)
{
	for (size_t i = 0; i < strnlen(str, 256); i++)
		putch(str[i]);
}

static void dump(uintptr_t saddr, size_t len, int width)
{
	uintptr_t curr, end;
	char buf[16];

	for (curr = saddr, end = curr + len;
			curr < end; curr += width) {
		snprintf(buf, 16, "\r\n%08x  ", curr);
		putstr(buf);

		for (int i = 0; i < width; i++) {
			if ((curr+i) >= end) {
				putstr("   ");
			} else {
				snprintf(buf, 8, "%02x ", getbyte(curr + i));
				putstr(buf);
			}

			if(!((i + 1) % 4))
				putch(' ');
		}

		putch(' ');

		for (int i = 0; i < width && (curr+i) < end; i++) {
			char c = getbyte(curr + i);
			if (c >= 0x20 && c < 0x7f)
				putch(c);
			else
				putch('.');
		}
	}

	putch('\r');
	putch('\n');
}

STATIC int memdump(int argc, char **argv)
{
	static size_t len = DEFAULT_DUMP_SIZE;
	static uintptr_t addr;

	if (argc >= 3)
		len = min((unsigned int)atoi(argv[2]), MAX_DUMP_SIZE);
	if (argc >= 2)
		addr = (uintptr_t)strtol(argv[1], NULL, 16);

	getch = (void *)argv[argc];
	putch = (void *)argv[argc+1];

	dump(addr, len, len < 16 ? len : 16);

	addr += len;

	return 0;
}
REGISTER_CMD(md, memdump, "md 0xADDR LEN");
