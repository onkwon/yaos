#include <asm/power.h>
#include <asm/sysclk.h>
#include <io.h>

void __enter_sleep_mode()
{
	__wfe();
}

void __enter_stop_mode()
{
}

void __enter_standby_mode()
{
}

void __sleep_on_exit()
{
	SCB_SCR |= 2;
}

#ifdef CONFIG_DEBUG
void disp_sysinfo()
{
}
#endif

unsigned int __read_reset_source()
{
	return 4;
}
