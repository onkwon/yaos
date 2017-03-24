#include <fs/fs.h>

#include <kernel/page.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

static DEFINE_LINKS_HEAD(fdtable);

unsigned int mkfile(struct file *file)
{
	struct file *new;

	if ((new = kmalloc(sizeof(struct file))) == NULL)
		return 0;

	memcpy(new, file, sizeof(struct file));

	unsigned int irqflag;
	spin_lock_irqsave(nospin, irqflag);
	links_add(&new->list, &fdtable);
	spin_unlock_irqrestore(nospin, irqflag);

	return (unsigned int)new;
}

void rmfile(struct file *file)
{
	unsigned int irqflag;
	spin_lock_irqsave(nospin, irqflag);
	links_del(&file->list);
	spin_unlock_irqrestore(nospin, irqflag);

	mutex_lock_atomic(&file->inode->lock);
	if (--file->inode->refcount <= 0) {
		iunlink(file->inode);
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
	struct links  *p;
	struct file  *file = NULL;
	unsigned int *addr = (unsigned int *)fd;

	unsigned int irqflag;
	spin_lock_irqsave(nospin, irqflag);

	/* TODO: make O(1) or run with not interrupt disabled */
	for (p = fdtable.next; p != &fdtable; p = p->next) {
		file = get_container_of(p, struct file, list);

		if ((unsigned int)file == (unsigned int)addr) break;
		else file = NULL;
	}

	spin_unlock_irqrestore(nospin, irqflag);

	return file;
}

#include <hash.h>

#define HASH_SHIFT			4
#define TABLE_SIZE			(1 << HASH_SHIFT)

static struct links itab[TABLE_SIZE];
static DEFINE_RWLOCK(lock_itab);

struct inode *iget(struct superblock *sb, unsigned int id)
{
	struct inode *inode;
	struct links *head, *curr;

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

/* TODO: remove old inodes if there are more than enough in the list */
void ilink(struct inode *inode)
{
	struct links *head = &itab[hash(inode->addr, HASH_SHIFT)];

	write_lock(&lock_itab);
	links_add(&inode->list, head);
	write_unlock(&lock_itab);
}

void iunlink(struct inode *inode)
{
	write_lock(&lock_itab);
	links_del(&inode->list);
	write_unlock(&lock_itab);
}

static void itab_init()
{
	unsigned int i;
	for (i = 0; i < TABLE_SIZE; i++) {
		links_init(&itab[i]);
	}
}

static struct file_system_type *file_system_list = NULL;

int add_file_system(struct file_system_type *fs, const char *name)
{
	char *str;

	if (!name)
		return ERANGE;

	if ((str = kmalloc(strnlen(name, FILENAME_MAX)+1)) == NULL)
		return ENOMEM;

	strncpy(str, name, strnlen(name, FILENAME_MAX)+1);
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

static DEFINE_LINKS_HEAD(sblist);

int mount(struct device *dev, const char *mnt_point, const char *fs_type)
{
	/* TODO: Check if the same mount point exists or/and if the same device
	 * is already mounted */

	struct superblock *sb;
	struct file_system_type *fs;

	if (!mnt_point || !(fs = get_file_system(fs_type)))
		return EFAULT;

	if ((sb = kmalloc(sizeof(struct superblock))) == NULL)
		return ENOMEM;

	if (fs->read_super(sb, dev)) {
		kfree(sb);
		return ENOENT; /* wrong file system */
	}

	sb->pathname_len = strnlen(mnt_point, FILENAME_MAX);
	/* skip the last `/`s */
	while ((sb->pathname_len > 1) && (mnt_point[sb->pathname_len-1] == '/'))
		sb->pathname_len--;

	if ((sb->pathname = kmalloc(sb->pathname_len+1)) == NULL) {
		kfree(sb);
		return ENOMEM;
	}

	strncpy(sb->pathname, mnt_point, sb->pathname_len);
	sb->pathname[sb->pathname_len] = '\0';

	mutex_init(&sb->lock);
	sb->dev  = dev;
	sb->type = fs;

	links_add(&sb->list, &sblist);

	sb->op->mount(dev);

	return 0;
}

struct superblock *search_super(const char *pathname)
{
	struct links *curr, *head;
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
