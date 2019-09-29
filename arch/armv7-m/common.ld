OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

systick = systick64;

PROVIDE(_rom_start   = LOADADDR);
PROVIDE(_rom_size    = LENGTH(rom) - (LOADADDR - ORIGIN(rom)));
PROVIDE(_ram_start   = ORIGIN(ram));
PROVIDE(_ram_size    = LENGTH(ram));
PROVIDE(_ram_end     = ORIGIN(ram) + LENGTH(ram));
PROVIDE(_vector_size = 0x200); /* The minimum alignment is 128 words. */

ENTRY(ISR_reset)

SECTIONS
{
	.text _rom_start :
	{
		KEEP(*(.vector))
		KEEP(*(.vector_irq))
		/* TODO: fill irq_handler in here */
		FILL(0xDEADC0DE);

		. = _vector_size;
		LONG(0); /* null sentinel */

		KEEP(*(.text.init))

		. = ALIGN(4);
		_init_func_list = .;
		KEEP(*(.text.init.0))
		KEEP(*(.text.init.1))
		KEEP(*(.text.init.2))
		KEEP(*(.text.init.*))
		LONG(0);

		*(.text)
		*(.text.*)

		. = ALIGN(4);
		*(.rodata)
		*(.rodata.*)

		. = ALIGN(4);
		_driver_list = .;
		KEEP(*(.driver_list))
		LONG(0);

		_shell_cmdlist = .;
		KEEP(*(.shell_cmdlist))
		LONG(0);

		. = ALIGN(4);
		_etext = .;
	} > rom

#ifdef CONFIG_COMMON_IRQ_HANDLER
	.data :
#else
	.data _ram_start + _vector_size :
#endif
	{
		. = ALIGN(4);
		_data = .;

		*(.data .data.*)
		*(.iap)

		. = ALIGN(4);
		_user_task_list = .;
		KEEP(*(.user_task_list))
		LONG(0);

		. = ALIGN(4);
		_edata = .;
	} > ram AT > rom

	.bss (NOLOAD) :
	{
		. = ALIGN(4);
		_bss = .;

		*(.bss .bss.*)
		*(COMMON)

		. = ALIGN(4);
		_ebss = .;
	} > ram AT > rom

	.heap (NOLOAD) :
	{
		. = ALIGN(8);
		end = .;
		_heap_start = .;
		_heap_end = .;
	} > ram

	/DISCARD/ :
	{
		libc.a (*)
		libm.a (*)
		libgcc.a (*)
	}
}
