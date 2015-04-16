#include <foundation.h>
#include <asm/arch.h>

void __attribute__((used)) sys_init()
{
	cli();

	clock_init();
	mem_init();

	sei();

	extern int main();
	main();
}
