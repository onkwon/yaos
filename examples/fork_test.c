#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>

static void fork_test()
{
	int tid;

	while (1) {
		tid = fork();

		if (tid == 0) { /* parent */
			write(stdout, "fork_test: parent\r\n", 19);
		} else if (tid > 0) { /* child */
			write(stdout, "fork_test: child\r\n", 18);
			return;
		} else { /* error */
			printf("fork_test: error\n");
		}

		msleep(100);
	}
}
//REGISTER_TASK(fork_test, 0, DEFAULT_PRIORITY);
