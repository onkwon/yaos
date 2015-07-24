#include <foundation.h>
#include "shell.h"

#include <fs/fs.h>
#include <string.h>
#include <stdlib.h>

struct ramfs_dir {
	void *inode;
	unsigned char type;
	char *name;
} __attribute__((packed));

struct embed_dir {
	unsigned short int inode;
	unsigned short int rec_len;
	unsigned char type;
	unsigned char name_len;
	char *name;
} __attribute__((packed));

#define FS_TYPE_RAMFS		1
#define FS_TYPE_EMBED		2

static int ls(int argc, char **argv)
{
	struct file *file;
	int fd;

	if (argc == 1)
		argv[1] = "/";

	if ((fd = open(argv[1], O_RDONLY)) <= 0) {
		printf("%s: no such file or directory\n", argv[1]);
		return 0;
	}

	if ((file = getfile(fd)) == NULL)
		goto out;

	if (!(file->inode->mode & FT_DIR)) {
		printf("%02x %d %d\n",
				file->inode->mode, file->inode->size,
				file->inode->count);
		goto out;
	}

	union {
		struct ramfs_dir ramfs;
		struct embed_dir embed;
	} fs;

	unsigned char fs_type;
	size_t size;

	if (!strcmp(file->inode->sb->type->name, "ramfs")) {
		fs_type = FS_TYPE_RAMFS;
		size = sizeof(struct ramfs_dir);
	} else if (!strcmp(file->inode->sb->type->name, "embedfs")) {
		fs_type = FS_TYPE_EMBED;
		size = sizeof(struct embed_dir) - sizeof(int);
	} else {
		goto out;
	}

	unsigned int inode;
	unsigned char type;
	char *name;

	while (read(fd, &fs, size)) {
		switch (fs_type) {
		case FS_TYPE_RAMFS:
			inode = (unsigned int)fs.ramfs.inode;
			type  = fs.ramfs.type;
			name  = fs.ramfs.name;
			break;
		case FS_TYPE_EMBED:
			if ((name = malloc(fs.embed.name_len)) == NULL)
				goto out;
			read(fd, name, fs.embed.name_len);
			inode = fs.embed.inode;
			type  = fs.embed.type;
			break;
		}

		printf("0x%08x 0x%02x %s\n", inode, type, name);

		if (fs_type == FS_TYPE_EMBED)
			free(name);
	}

out:
	close(fd);
	return 0;
}
REGISTER_CMD(ls, ls, "list directory contents");
