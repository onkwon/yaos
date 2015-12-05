#include <io.h>
#include <error.h>

int sys_reboot()
{
	if (!(get_task_type(current) & TASK_PRIVILEGED))
		return -ERR_PERM;

	dsb();
	freeze();

	return 0;
}
