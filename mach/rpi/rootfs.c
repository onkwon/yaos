#include <kernel/module.h>
#include "../../fs/ramfs.h"

static void rootfs_init()
{
	mount(ramfs_build(1024, "/"), "/", "ramfs"); /* root file system */
}
MODULE_INIT(rootfs_init);
