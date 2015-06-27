#include <foundation.h>
#include <kernel/task.h>

static void mayfly()
{
	printf("onetime function\n");
}
REGISTER_TASK(mayfly, 0, DEFAULT_PRIORITY);
