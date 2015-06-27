#include <foundation.h>
#include <kernel/page.h>

#include <io.h>

#define NEC_DATA_BITS		32
#define NEC_HEAD_LEN
#define NEC_HIGH_THRESHOLD	20000
#define NEC_LOW_THRESHOLD	3000

unsigned int ir_get_nec(const unsigned int *buf)
{
	unsigned int val = 0;
	int i;

	/* check header */
	buf = buf + 2;

	for (i = 0; i < (NEC_DATA_BITS*2); i += 2) {
		val <<= 1;

		if ((buf[i] + buf[i+1]) > NEC_HIGH_THRESHOLD)
			val |= 1;
	}

	return val;
}

#include <kernel/task.h>

static void test_ir()
{
	/* alloc kernel stack for test */
	set_task_type(current, TASK_KERNEL);
	set_task_state(current, TASK_RUNNING);
	schedule();

	unsigned int *kstack = (unsigned int *)kmalloc(STACK_SIZE);
	current->mm.kernel.base = kstack;
	current->mm.kernel.sp = &kstack[STACK_SIZE / sizeof(int) - 1];
	printf("kernel stack 0x%08x - 0x%08x\n",
			kstack, (unsigned int)&current->mm.kernel.sp[1] - 1);

	set_task_state(current, TASK_USER);
	schedule();

	/* Make a way of killing a task, releasing all the resource used */

	unsigned int buf[80], i = 0;
	unsigned int timeout = set_timeout(1000);
	while (1) {
		//buf[i] = fifo_get(&ir_buf, sizeof(int));
		if (buf[i] != -1) {
			/* NEC_DATA_BITS * 2 = 1 cycle
			 * + 2 = header
			 * + 1 = dummy */
			//if ((i++ >= (NEC_DATA_BITS*2 + 2 + 1)) && siglevel) {
			if (i++ >= (NEC_DATA_BITS*2 + 2 + 1)) {
				/* buf+1 = except dummy */
				printf("result : %08x\n", ir_get_nec(buf+1));
				i = 0;
			}

			timeout = set_timeout(1000);
		}

		if (i && is_timeout(timeout)) /* if timeout, flush */
			i = 0;
	}
}
//REGISTER_TASK(test_ir, 0, DEFAULT_PRIORITY);
