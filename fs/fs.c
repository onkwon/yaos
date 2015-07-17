#include <fs/fs.h>

#include <kernel/page.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

static DEFINE_LIST_HEAD(fdtable);

unsigned int mkfile(struct file *file)
{
	struct file *new;

	if ((new = kmalloc(sizeof(struct file))) == NULL)
		return 0;

	memcpy(new, file, sizeof(struct file));
	list_add(&new->list, &fdtable);

	return (unsigned int)new;
}

void remove_file(struct file *file)
{
	list_del(&file->list);
	kfree(file->inode);
	kfree(file);
}

struct file *getfile(int fd)
{
	struct file *file;
	struct list *p;
	unsigned int *addr = (unsigned int *)fd;

	for (p = fdtable.next; p; p = p->next) {
		file = get_container_of(p, struct file, list);

		if ((unsigned int)file == (unsigned int)addr)
			return file;
	}

	return NULL;
}

static unsigned int fslist = 0;

int add_file_system(struct file_system_type *fs, const char *name)
{
	char *str;

	if ((str = kmalloc(strlen(name)+1)) == NULL)
		return -ERR_ALLOC;

	strncpy(str, name, strlen(name)+1);
	fs->name = str;
	fs->next = (struct file_system_type *)fslist;
	fslist = (unsigned int)fs;

	return 0;
}

struct file_system_type *get_file_system(const char *name)
{
	struct file_system_type *p = (struct file_system_type *)fslist;

	while (p) {
		if (!strcmp(p->name, name))
			break;

		p = p->next;
	}

	return p;
}

static DEFINE_LIST_HEAD(sblist);

int mount(const struct device *dev, const char *mnt_point, const char *fs_type)
{
	/* check if the same mount point exists */

	struct superblock *sb;
	struct file_system_type *fs;

	if ((fs = get_file_system(fs_type)) == NULL)
		return -ERR_UNDEF;

	if ((sb = kmalloc(sizeof(struct superblock))) == NULL)
		return -ERR_ALLOC;

	if (fs->read_super(sb, dev)) {
		kfree(sb);
		return -ERR_ATTR; /* wrong file system */
	}

	sb->pathname_len = strlen(mnt_point);
	/* skip the last `/` */
	while ((sb->pathname_len > 1) && (mnt_point[sb->pathname_len-1] == '/'))
		sb->pathname_len--;

	if ((sb->pathname = kmalloc(sb->pathname_len+1)) == NULL) {
		kfree(sb);
		return -ERR_ALLOC;
	}

	strncpy(sb->pathname, mnt_point, sb->pathname_len);
	sb->pathname[sb->pathname_len] = '\0';

	INIT_LOCK(sb->lock);
	sb->dev  = dev;
	sb->type = fs;

	list_add(&sb->list, &sblist);

	sb->op->mount(dev);

	return 0;
}

struct superblock *search_super(const char *pathname)
{
	struct list *i, *head;
	struct superblock *p, *bestfit;

	bestfit = NULL;
	head = &sblist;
	for (i = head->next; i != head; i = i->next) {
		p = get_container_of(i, struct superblock, list);

		if (!strncmp(p->pathname, pathname, p->pathname_len)) {
			if (!bestfit || bestfit->pathname_len < p->pathname_len)
				bestfit = p;
		}
	}

	return bestfit;
}

#include <kernel/init.h>
#include "ramfs.h"
#include "embedfs.h"

const struct device *devfs;

void __init fs_init()
{
	ramfs_register();
	embedfs_register();

	/* it is nessesary to mount devfs first to populate device nodes in */
	devfs = ramfs_build(1024, NULL);
	mount(devfs, DEVFS_ROOT, "ramfs");
}
