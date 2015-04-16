#include <foundation.h>
#include <sched.h>

static void test_task1()
{
	//while (1) {
		printf("test1()\n");
		printf("psr : %x sp : %x int : %x control : %x lr : %x\n", GET_PSR(), GET_SP(), GET_INT(), GET_CON(), GET_LR());
		printf("PC = %x\n", GET_PC());
		mdelay(1000);

		runqueue_del(current);
	//}
}

#include <task.h>
REGISTER_TASK(test_task1, STACK_SIZE_DEFAULT, NORMAL_PRIORITY);
