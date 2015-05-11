#include <foundation.h>
#include <kernel/task.h>
#include <stdlib.h>

static void test_task1()
{
	void *p, *p2;

	while (1) {
		printf("test1()\n");
		printf("control %08x, sp %08x, msp %08x, psp %08x\n", GET_CON(), GET_SP(), GET_KSP(), GET_USP());

		printf("#ALLOC\n");
		p = kmalloc(10);
		show_free_list(NULL);
		printf("#FREE\n");
		free(p);
		show_free_list(NULL);
		printf("#ALLOC\n");
		p = kmalloc(10);
		show_free_list(NULL);
		printf("#ALLOC\n");
		p2 = kmalloc(10);
		show_free_list(NULL);
		printf("#FREE\n");
		free(p);
		show_free_list(NULL);
		printf("#FREE\n");
		free(p2);
		show_free_list(NULL);

		sleep(1);
	}
}
REGISTER_TASK(test_task1, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);
