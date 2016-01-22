#ifndef __FS_H__
#define __FS_H__

#define NR_DATA_BLOCK			10
#define NR_DATA_BLOCK_DIRECT		(NR_DATA_BLOCK - 3)

#define GET_FILE_TYPE(mode)		(mode)

#define DEVFS_ROOT			"/dev/"

#include <types.h>
#include <kernel/lock.h>

struct superblock;
struct device;
struct file_system_type {
	const char *name;
	int (*read_super)(struct superblock *sb, const struct device *dev);
	struct file_system_type *next;
};

int add_file_system(struct file_system_type *fs, const char *name);
struct file_system_type *get_file_system(const char *name);

struct super_operations;
struct superblock {
	char *pathname; /* mount point */
	size_t pathname_len;
	unsigned int root_inode;

	lock_t lock;

	struct super_operations *op;

	const struct device *dev; /* associated block device */
	const struct file_system_type *type;

	struct list list; /* list of all superblocks */
};

struct inode;
struct super_operations {
	void (*read_inode)(struct inode *inode);
	int (*mount)(const struct device *dev);
};

struct file;
struct file_operations;

struct inode {
	unsigned int addr; /* or block number or ID */
	mode_t mode;
	union {
		size_t size;
		dev_t dev;
	};
	refcnt_t count;

	struct inode_operations *iop;
	struct file_operations *fop;

	struct superblock *sb;

	lock_t lock;

	struct list list;
};

struct inode_operations {
	int (*lookup)(struct inode *inode, const char *pathname);
	int (*create)(struct inode *inode, const char *pathname, mode_t mode);
	//mkdir
	//rmdir
	//rename
	//delete
};

enum file_type {
	FT_UNKNOWN	= 0x00,
	FT_FILE		= 0x01,
	FT_DIR		= 0x02,
	FT_DEV		= 0x04,
};

enum file_flag {
	O_RDONLY	= 0x01,
	O_WRONLY	= 0x02,
	O_RDWR		= (O_RDONLY | O_WRONLY),
	O_NONBLOCK	= 0x04,
	O_CREATE	= 0x08,
};

enum whence {
	SEEK_SET	= 0x00, /* beginning of file */
	SEEK_CUR	= 0x01, /* current position of the file pointer */
	SEEK_END	= 0x02, /* end of file */
};

struct file {
	unsigned int offset; /* keep this first */
	mode_t flags;
	struct inode *inode;

	struct file_operations *op;

	struct list list;

	void *option;
};

struct file_operations {
	int    (*open) (struct inode *inode, struct file *file);
	size_t (*read) (struct file *file, void *buf, size_t len);
	size_t (*write)(struct file *file, void *buf, size_t len);
	int    (*close)(struct file *file);
	int    (*seek) (struct file *file, unsigned int offset, int whence);
	int    (*ioctl)(struct file *file, unsigned int request, void *args);
};

unsigned int mkfile(struct file *file);
void rmfile(struct file *file);
struct file *getfile(int fd);

struct inode *iget(struct superblock *sb, unsigned int id);
void iput(struct inode *inode);

int mount(const struct device *dev, const char *mnt_point, const char *fs_type);
struct superblock *search_super(const char *pathname);

void fs_init();

int sys_mknod(const char *name, unsigned int mode, dev_t id); /* ramfs.c */

#endif /* __FS_H__ */
