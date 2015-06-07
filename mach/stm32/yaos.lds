OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

MEMORY
{
	ram (rwx)	: ORIGIN = 0x20000000, LENGTH = 64K
	rom (rx)	: ORIGIN = 0x00000000, LENGTH = 512K
}

PROVIDE(_rom_start   = ORIGIN(rom));
PROVIDE(_rom_size    = LENGTH(rom));
PROVIDE(_ram_start   = ORIGIN(ram));
PROVIDE(_ram_end     = ORIGIN(ram) + LENGTH(ram) - 1);
PROVIDE(_vector_size = 0x200); /* The minimum alignment is 128 words. */

jiffies = jiffies_64;
SECTIONS
{
	. = 0x0;
	.text :
	{
		*(.vector)
		*(.text.init)
		. = (. + 3) & ~ 3;
		_init_func_list = .;
		*(.text.init.0)
		*(.text.init.*)
		. = (. + 3) & ~ 3;
		LONG(0); /* null sentinel */
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
		. = (. + 3) & ~ 3;
		_device_list = .;
		*(.device_list)
		. = (. + 3) & ~ 3;
		LONG(0);
		_shell_cmdlist = .;
		*(.shell_cmdlist)
		. = (. + 3) & ~ 3;
		LONG(0);
		_etext = .;
	} > rom

	. = _ram_start + _vector_size;
	.data . :
	{
		_data = .;
		*(.data)
		*(.iap)
		. = (. + 3) & ~ 3;
		_user_task_list = .;
		*(.user_task_list)
		. = (. + 3) & ~ 3;
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
