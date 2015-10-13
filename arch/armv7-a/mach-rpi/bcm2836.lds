OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

MEMORY
{
	ram (rwx)	: ORIGIN = 0x00000000, LENGTH = 512M
}

PROVIDE(_ram_start   = ORIGIN(ram));
PROVIDE(_ram_size    = LENGTH(ram));
PROVIDE(_ram_end     = ORIGIN(ram) + LENGTH(ram));

systick = systick64;
SECTIONS
{
	. = 0x8000;
	.text . :
	{
		*(.vector)

		*(.text.init)

		. = ALIGN(4);
		_init_func_list = .;
		*(.text.init.0)
		*(.text.init.*)
		LONG(0); /* null sentinel */

		*(.text)
		*(.text.*)

		. = ALIGN(4);
		*(.rodata)
		*(.rodata.*)

		. = ALIGN(4);
		_device_list = .;
		*(.device_list)
		LONG(0);

		_shell_cmdlist = .;
		*(.shell_cmdlist)
		LONG(0);
		_etext = .;
	} > ram

	.data :
	{
		_data = .;
		*(.data)
		*(.iap)

		. = ALIGN(8);
		_user_task_list = .;
		*(.user_task_list)
		. = ALIGN(4);
		LONG(0);
		_edata = .;
	} > ram

	.bss :
	{
		_bss = .;
		*(.bss)
		*(COMMON)
		_ebss = .;
	} > ram
}
