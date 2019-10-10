#include "kernel/power.h"
#include "kernel/systick.h"
#include "kernel/interrupt.h"
#include "kernel/task.h"
#include "kernel/syscall.h"
#include "io.h"
#include "syslog.h"

static void reboot_core(size_t msec)
{
	alert("rebooting in %u", msec);

	mdelay(msec);

	// sync
	dsb();
	hw_reboot();

	freeze();
}

void sys_reboot(size_t msec)
{
	if (!(get_task_flags(current) & TF_PRIVILEGED)) {
		debug("no permission");
		return;
	}

	syscall_delegate(reboot_core, &current->stack.p, &current->flags);
	(void)msec;
}

void enter_sleep_mode(sleep_t sleeptype)
{
	switch (sleeptype) {
	case SLEEP_DEEP:
		//__enter_stop_mode();
		debug("sleep_deep");
		break;
	case SLEEP_BLACKOUT:
		//__enter_standby_mode();
		break;
	case SLEEP_NAP:
		hw_enter_sleep_nap();
		break;
	default:
		break;
	}
}
