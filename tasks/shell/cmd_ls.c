#include <foundation.h>
#include "shell.h"

#include <fs/fs.h>
#include <string.h>
#include <stdlib.h>

static int ls(int argc, char **argv)
{
	int fd;
	char buf[16];
	size_t i, len;

	if (argc == 1)
		argv[1] = "/";

	fd = open(argv[1], 0);

	while ((len = read(fd, buf, 16))) {
		for (i = 0; i < len; i++)
			printf("%02x ", buf[i]);
		printf("\n");
	}
	/*
	struct ramfs_inode *inode;
	struct ramfs_dir dir;
	unsigned int offset;

	if (argc == 1)
		argv[1] = "/";

	inode = get_inode(argv[1]);

	if (inode == NULL) {
		printf("%s: no such file or directory\n", argv[1]);
		return 0;
	}

	if (!(inode->mode & FT_DIR)) {
		printf("\t%02x %d %08x\n", inode->mode, inode->size, inode->data[0]);
		return 0;
	}

	for (offset = 0; offset < inode->size; offset += sizeof(struct ramfs_dir)) {
		readblk(inode, offset, &dir, sizeof(struct ramfs_dir));

		printf("%s %02x (%08x)\n", dir.name, dir.type, dir.inode);
	}
	*/

	return 0;
}
REGISTER_CMD(ls, ls, "list directory contents");
