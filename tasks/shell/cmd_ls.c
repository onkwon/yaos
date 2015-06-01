#include <foundation.h>
#include <shell.h>

#include <kernel/fs.h>
#include <string.h>
#include <stdlib.h>

static int ls(int argc, char **argv)
{
	struct inode_t *inode;
	struct dir_t dir;
	unsigned int offset;

	if (argc == 1)
		argv[1] = "/";

	inode = get_inode(argv+1, NULL);

	if (argv[1]) {
		printf("%s: no such file or directory\n", argv[1]);
		return 0;
	}

	if (!(inode->mode & FT_DIR)) {
		printf("\t%02x %d %08x\n", inode->mode, inode->size, inode->data[0]);
		return 0;
	}

	for (offset = 0; offset < inode->size; offset += sizeof(struct dir_t)) {
		readblk(inode, offset, &dir, sizeof(struct dir_t));

		printf("%s %02x (%08x)\n", dir.name, dir.type, dir.inode);
	}

	return 0;
}
REGISTER_CMD(ls, ls, "list directory contents");
