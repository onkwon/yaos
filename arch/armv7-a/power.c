#include <power.h>
#include <foundation.h>

void __reboot()
{
	dsb();
	freeze();
}

void __enter_sleep_mode()
{
}

void __enter_stop_mode()
{
}

void __enter_standby_mode()
{
}

void __sleep_on_exit()
{
}

#ifdef CONFIG_DEBUG
static void disp_clkinfo()
{
}

void disp_sysinfo()
{
}
#endif

unsigned int __read_reset_source()
{
}
