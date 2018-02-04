OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

resched = sys_schedule;
systick = systick64;

PROVIDE(_rom_start   = ORIGIN(rom));
PROVIDE(_rom_size    = LENGTH(rom));
PROVIDE(_ram_start   = ORIGIN(ram));
PROVIDE(_ram_size    = LENGTH(ram));
PROVIDE(_ram_end     = ORIGIN(ram) + LENGTH(ram));
PROVIDE(_vector_size = 0x200); /* The minimum alignment is 128 words. */

SECTIONS
{
	.text :
	{
		*(.vector)
		*(.vector_irq)
		/* TODO: fill irq_handler in here */
		FILL(0xDEAFC0DE);

		. = _vector_size;
		LONG(0); /* null sentinel */

		*(.text.init)

		. = ALIGN(4);
		_init_func_list = .;
		*(.text.init.0)
		*(.text.init.1)
		*(.text.init.2)
		*(.text.init.*)
		LONG(0);

		*(.text)
		*(.text.*)

		. = ALIGN(4);
		*(.rodata)
		*(.rodata.*)

		. = ALIGN(4);
		_driver_list = .;
		*(.driver_list)
		LONG(0);

		. = ALIGN(4);
		_device_list = .;
		*(.device_list)
		LONG(0);

		_shell_cmdlist = .;
		*(.shell_cmdlist)
		LONG(0);

		. = ALIGN(4);
		_etext = .;
	} > rom

#ifdef CONFIG_COMMON_IRQ_FRAMEWORK
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
		*(.user_task_list)
		LONG(0);

		. = ALIGN(4);
		_edata = .;
	} > ram AT > rom

	.bss :
	{
		. = ALIGN(4);
		_bss = .;

		*(.bss .bss.*)
		*(COMMON)

		. = ALIGN(4);
		_ebss = .;
	} > ram AT > rom
}
