#include <foundation.h>
#include <kernel/task.h>

static void fork_test()
{
	int tid = fork();

	if (tid == 0) { /* parent */
		printf("fork_test: parent\n");
	} else if (tid > 0) { /* child */
		printf("fork_test: child\n");
	} else { /* error */
		printf("fork_test: error\n");
	}
}
REGISTER_TASK(fork_test, 0, DEFAULT_PRIORITY);
