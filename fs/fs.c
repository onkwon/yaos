#include <fs/fs.h>

#include <kernel/page.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

static DEFINE_LIST_HEAD(fdtable);
static DEFINE_SPINLOCK(fdtable_lock);

unsigned int mkfile(struct file *file)
{
	struct file *new;

	if ((new = kmalloc(sizeof(struct file))) == NULL)
		return 0;

	memcpy(new, file, sizeof(struct file));

	unsigned int irqflag;
	spin_lock_irqsave(&fdtable_lock, irqflag);
	list_add(&new->list, &fdtable);
	spin_unlock_irqrestore(&fdtable_lock, irqflag);

	return (unsigned int)new;
}

void rmfile(struct file *file)
{
	unsigned int irqflag;
	spin_lock_irqsave(&fdtable_lock, irqflag);
	list_del(&file->list);
	spin_unlock_irqrestore(&fdtable_lock, irqflag);

	mutex_lock_atomic(&file->inode->lock);
	if (--file->inode->count <= 0) {
		list_del(&file->inode->list);
		kfree(file->inode);
		/* no need to unlock as it's gone */
		goto out;
	}
	mutex_unlock_atomic(&file->inode->lock);

out:
	kfree(file->op);
	kfree(file);
}

struct file *getfile(int fd)
{
	struct list  *p;
	struct file  *file = NULL;
	unsigned int *addr = (unsigned int *)fd;

	unsigned int irqflag;
	spin_lock_irqsave(&fdtable_lock, irqflag);

	for (p = fdtable.next; p != &fdtable; p = p->next) {
		file = get_container_of(p, struct file, list);

		if ((unsigned int)file == (unsigned int)addr) break;
		else file = NULL;
	}

	spin_unlock_irqrestore(&fdtable_lock, irqflag);

	return file;
}

#include <hash.h>

#define HASH_SHIFT			4
#define TABLE_SIZE			(1 << HASH_SHIFT)

static struct list itab[TABLE_SIZE];
static DEFINE_RWLOCK(lock_itab);

struct inode *iget(struct superblock *sb, unsigned int id)
{
	struct inode *inode;
	struct list *head, *curr;

	head = &itab[hash(id, HASH_SHIFT)];
	curr = head;

	do {
		read_lock(&lock_itab);
		curr = curr->next;
		read_unlock(&lock_itab);

		if (curr == head) {
			inode = NULL;
			break;
		}

		inode = get_container_of(curr, struct inode, list);
	} while (inode->addr != id || inode->sb != sb);

	return inode;
}

void iput(struct inode *inode)
{
	struct list *head = &itab[hash(inode->addr, HASH_SHIFT)];

	write_lock(&lock_itab);
	list_add(&inode->list, head);
	write_unlock(&lock_itab);
}

static void itab_init()
{
	unsigned int i;
	for (i = 0; i < TABLE_SIZE; i++) {
		list_link_init(&itab[i]);
	}
}

static struct file_system_type *file_system_list = NULL;

int add_file_system(struct file_system_type *fs, const char *name)
{
	char *str;

	if ((str = kmalloc(strlen(name)+1)) == NULL)
		return -ERR_ALLOC;

	strncpy(str, name, strlen(name)+1);
	fs->name = str;
	fs->next = file_system_list;
	file_system_list = fs;

	return 0;
}

struct file_system_type *get_file_system(const char *name)
{
	struct file_system_type *p = file_system_list;

	while (p) {
		if (!strcmp(p->name, name))
			break;

		p = p->next;
	}

	return p;
}

static DEFINE_LIST_HEAD(sblist);

int mount(struct device *dev, const char *mnt_point, const char *fs_type)
{
	/* TODO:
	 * check if the same mount point exists and if the same device is
	 * already mounted */

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
	/* skip the last `/`s */
	while ((sb->pathname_len > 1) && (mnt_point[sb->pathname_len-1] == '/'))
		sb->pathname_len--;

	if ((sb->pathname = kmalloc(sb->pathname_len+1)) == NULL) {
		kfree(sb);
		return -ERR_ALLOC;
	}

	strncpy(sb->pathname, mnt_point, sb->pathname_len);
	sb->pathname[sb->pathname_len] = '\0';

	INIT_MUTEX(sb->lock);
	sb->dev  = dev;
	sb->type = fs;

	list_add(&sb->list, &sblist);

	sb->op->mount(dev);

	return 0;
}

struct superblock *search_super(const char *pathname)
{
	struct list *curr, *head;
	struct superblock *p, *bestfit;

	bestfit = NULL;
	head = &sblist;
	for (curr = head->next; curr != head; curr = curr->next) {
		p = get_container_of(curr, struct superblock, list);

		if (!bestfit && p->pathname_len == 1) /* root */
			bestfit = p;

		if (pathname[p->pathname_len] &&
				pathname[p->pathname_len] != '/')
			continue;

		if (!strncmp(pathname, p->pathname, p->pathname_len)) {
			if (!bestfit || bestfit->pathname_len < p->pathname_len)
				bestfit = p;
		}
	}

	return bestfit;
}

#include <kernel/init.h>
#include "ramfs.h"
#include "embedfs.h"

struct device *devfs;

void __init fs_init()
{
	itab_init();

	ramfs_register();
#ifdef CONFIG_FS
	embedfs_register();
#endif

	/* it is nessesary to mount devfs first to populate device nodes in */
	devfs = ramfs_build(2048, NULL);
	mount(devfs, DEVFS_ROOT, "ramfs");
}
