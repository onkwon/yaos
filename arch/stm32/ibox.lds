OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

MEMORY
{
	ram (rwx)	: ORIGIN = 0x20000000, LENGTH = 64K
	rom (rx)	: ORIGIN = 0x00000000, LENGTH = 512K
}

PROVIDE(_sram_start  = ORIGIN(ram));
PROVIDE(_sram_end    = ORIGIN(ram) + LENGTH(ram) - 1);
PROVIDE(_vector_size = 0x200); /* The minimum alignment is 128 words. */

systick = systick_64;
SECTIONS
{
	. = 0x0;
	.text :
	{
		*(.vector)
		. = (. + 3) & ~ 3;
		_init_func_list = .;
		*(.text.init_list.0)
		*(.text.init_list.*)
		. = (. + 3) & ~ 3;
		LONG(0);
		*(.text.init)
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
		. = (. + 3) & ~ 3;
		_cmd_list = .;
		*(.cmdlist)
		. = (. + 3) & ~ 3;
		LONG(0);
		_etext = .;
	} > rom

	. = _sram_start + _vector_size;
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
