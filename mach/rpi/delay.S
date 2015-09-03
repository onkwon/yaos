.section .text

.global udelay
.func udelay
udelay:
	nop
	sub r0, r0, #1
	cmp r0, #0
	bne udelay
	bx lr
.endfunc
