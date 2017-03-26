#ifndef __FS_H__
#define __FS_H__

#define NR_DATA_BLOCK			10
#define NR_DATA_BLOCK_DIRECT		(NR_DATA_BLOCK - 3)

#define GET_FILE_TYPE(mode)		(mode)

#define DEVFS_ROOT			"/dev/"

#include <types.h>
#include <kernel/lock.h>

#include <kernel/page.h>
#ifdef CONFIG_PAGE
#define FILENAME_MAX			PAGESIZE
#else
#define FILENAME_MAX			64
#endif

struct superblock;
struct device;
struct file_system_type {
	const char *name;
	int (*read_super)(struct superblock *sb, struct device *dev);
	struct file_system_type *next;
};

int add_file_system(struct file_system_type *fs, const char *name);
struct file_system_type *get_file_system(const char *name);

struct super_operations;
struct superblock {
	char *pathname; /* mount point */
	size_t pathname_len;
	unsigned int root_inode;

	mutex_t lock;

	struct super_operations *op;

	struct device *dev; /* associated block device */
	const struct file_system_type *type;

	struct links list; /* list of all superblocks */
};

struct inode;
struct super_operations {
	void (*read_inode)(struct inode *inode);
	int (*mount)(struct device *dev);
};

struct file;
struct file_operations;

struct inode {
	unsigned int addr; /* or block number or ID */
	int mode;
	union {
		size_t size;
		dev_t dev;
	};
	int refcount;

	struct inode_operations *iop;
	struct file_operations *fop;

	struct superblock *sb;

	mutex_t lock;

	struct links list;
};

struct inode_operations {
	int (*lookup)(struct inode *inode, const char *pathname);
	int (*create)(struct inode *inode, const char *pathname, int mode);
	int (*delete)(struct inode *inode, const char *pathname);
	//mkdir
	//rmdir
	//rename
};

enum file_type {
	FT_UNKNOWN	= 0x00,
	FT_FILE		= 0x01,
	FT_DIR		= 0x02,
	FT_DEV		= 0x04,
	FT_DELETED	= 0x08,
};

enum file_oflag {
	O_RDONLY	= 0x01,
	O_WRONLY	= 0x02,
	O_RDWR		= (O_RDONLY | O_WRONLY),
	O_NONBLOCK	= 0x04,
	O_CREATE	= 0x08,
	O_MANUAL	= 0x10,
};

enum file_cflag {
	C_FLUSH		= 0x01,
	C_SET		= 0x02,
	C_GET		= 0x03,
	C_EVENT		= 0x04,
	C_BUFSIZE	= 0x05, /* getter when the next argument value is 0 */
	C_FREQ		= 0x06, /* getter when the next argument value is 0 */
	C_RUN		= 0x07,
};

enum whence {
	SEEK_SET	= 0x00, /* beginning of file */
	SEEK_CUR	= 0x01, /* current position of the file pointer */
	SEEK_END	= 0x02, /* end of file */
};

struct file {
	unsigned int offset; /* keep this first */
	int flags;
	struct inode *inode;

	struct file_operations *op;

	struct links list;

	void *option;
};

struct file_operations {
	int    (*open) (struct inode *inode, struct file *file);
	size_t (*read) (struct file *file, void *buf, size_t len);
	size_t (*write)(struct file *file, void *buf, size_t len);
	int    (*close)(struct file *file);
	int    (*seek) (struct file *file, unsigned int offset, int whence);
	int    (*ioctl)(struct file *file, int request, void *data);
};

unsigned int mkfile(struct file *file);
void rmfile(struct file *file);
struct file *getfile(int fd);

struct inode *iget(struct superblock *sb, unsigned int id);
void ilink(struct inode *inode);
void iunlink(struct inode *inode);

int mount(struct device *dev, const char *mnt_point, const char *fs_type);
struct superblock *search_super(const char *pathname);

int sys_mknod(const char *name, unsigned int mode, dev_t id); /* ramfs.c */

#endif /* __FS_H__ */
