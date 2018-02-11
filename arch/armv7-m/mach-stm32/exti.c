/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include <kernel/task.h>
#include <asm/lock.h>

void exti_enable(int pin, bool enable)
{
	reg_t *reg;
	unsigned int port, bit, val;

	port = pin2port(pin);
	pin = pin2portpin(pin);
	bit = pin % 4 * 4;
	pin = pin / 4 * 4;
	reg = (reg_t *)((SYSCFG_BASE+8) + pin);

	do {
		val = __ldrex(reg);
		val &= ~(0xf << bit);
		val |= port << bit;
	} while (__strex(val, reg));

	if (!(__read_apb2_clock() & (1 << RCC_SYSCFGEN_BIT)))
		__turn_apb2_clock(RCC_SYSCFGEN_BIT, ON);

	(void)enable;
}
