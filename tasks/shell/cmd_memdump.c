#include <foundation.h>
#include "shell.h"
#include <string.h>

#define DUMPSIZE_DEFAULT	256
#define getbyte(addr)		(*(unsigned char *)(addr))

static void dump(unsigned int saddr, unsigned int len, unsigned int width)
{
	unsigned int curr;
	unsigned int i;

	for (curr = saddr; curr < (saddr + len); curr += width) {
		printf("\n%08x  ", curr); 
		for (i = 0; i < width; i++) {
			printf("%02x ", getbyte(curr + i));
			if(!((i + 1) % 4)) printf(" ");
		}   
		printf(" ");
		for (i = 0; i < width; i++) {
			if (getbyte(curr + i) >= 0x20 && getbyte(curr + i) < 0x7f) 
				printf("%c", getbyte(curr + i));
			else 
				printf(".");
		}   
	}
	printf("\n");
}

static int memdump(int argc, char **argv)
{
	static unsigned int len = DUMPSIZE_DEFAULT;
	static unsigned int addr;

	if (argc >= 3) len  = (unsigned int)atoi(argv[2]);
	if (argc >= 2) addr = (unsigned int)atoi(argv[1]);

	dump(addr, len, len < 16 ? len : 16);

	addr += len;

	return 0;
}
REGISTER_CMD(md, memdump, "md ADDR LEN");
