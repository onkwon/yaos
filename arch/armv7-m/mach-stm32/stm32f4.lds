OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

MEMORY
{
	ram  (rwx)	: ORIGIN = 0x10000000, LENGTH = 64K
	ram2 (rwx)	: ORIGIN = 0x20000000, LENGTH = 128K
	rom  (rx)	: ORIGIN = 0x00000000, LENGTH = 512K
}

PROVIDE(_rom_start   = ORIGIN(rom));
PROVIDE(_rom_size    = LENGTH(rom));
PROVIDE(_ram_start   = ORIGIN(ram));
PROVIDE(_ram_size    = LENGTH(ram));
PROVIDE(_ram_end     = ORIGIN(ram) + LENGTH(ram));
PROVIDE(_vector_size = 0x200); /* The minimum alignment is 128 words. */

resched = sys_schedule;
systick = systick64;
SECTIONS
{
	.text :
	{
		*(.vector)

		*(.text.init)

		. = (. + 3) & ~ 3;
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
	} > rom

	. = _ram_start + _vector_size;
	.data . :
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
	} > ram AT > rom

	.bss :
	{
		_bss = .;
		*(.bss)
		*(COMMON)
		_ebss = .;
	} > ram AT > rom
}
