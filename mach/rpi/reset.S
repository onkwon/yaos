
.section ".vector"
.arm
vector:
	ldr	pc, _reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _reserved
	ldr	pc, _irq
	ldr	pc, _fiq

	_reset:			.word	reset
	_undefined_instruction:	.word	undefined_instruction
	_software_interrupt:	.word	svc_handler
	_prefetch_abort:	.word	prefetch_abort
	_data_abort:		.word	data_abort
	_reserved:		.word	reset
	_irq:			.word	irq_handler
	_fiq:			.word	fiq

reset:
	/* set sp and cpsr for each modes */
	/* irq mode: irq,fiq disabled */
	mov	r0, #0xd2
	msr	cpsr_c, r0
	mov	sp, #0x8000

	/* user/system mode: irq,fiq enabled */
	mov	r0, #0x1f
	msr	cpsr_c, r0
	mov	sp, #0x8000

	/* svc mode: irq,fiq disabled */
	mov	r0, #0xd3
	msr	cpsr_c, r0
	ldr	sp, =_ram_end

	bl	__cli
	b	main

undefined_instruction:
prefetch_abort:
data_abort:
fiq:
	b	fiq

.global __sei
__sei:
	mrs	r0, cpsr
	bic	r0, r0, #0x80
	msr	cpsr_c, r0
	mov	pc, lr

.global __cli
__cli:
	mrs	r0, cpsr
	orr	r0, r0, #0x80
	msr	cpsr_c, r0
	mov	pc, lr
