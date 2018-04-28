#include "shell.h"
#include <foundation.h>
#include <string.h>
#include <stdlib.h>

#define DUMPSIZE_DEFAULT	256
#define getbyte(addr)		(*(unsigned char *)(addr))

static void dump(unsigned int saddr, unsigned int len, unsigned int width)
{
	unsigned int curr;
	unsigned int i;

	for (curr = saddr; curr < (saddr + len); curr += width) {
		printk("\n%08x  ", curr); 
		for (i = 0; i < width; i++) {
			printk("%02x ", getbyte(curr + i));
			if(!((i + 1) % 4)) printk(" ");
		}
		printk(" ");
		for (i = 0; i < width; i++) {
			if (getbyte(curr + i) >= 0x20 && getbyte(curr + i) < 0x7f) 
				printk("%c", getbyte(curr + i));
			else 
				printk(".");
		}   
	}
	printk("\n");
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
