# ibox

Two types of task are handled, normal priority and real time priority tasks. Completly fair scheduler for normal priority tasks while FIFO scheduler for real time priority tasks.

It generates a periodic interrupt rated by HZ that is the heart rate of system. Change it as you wish.

Put a user task under /tasks directory. Code what is needed in general way using provided API and any other library. And simply register the task by REGISTER_TASK(main_function, stack_size, priority).

To access system resource, use provided API after checking how critical regions, race conditions and wait queue are treated in the right sections below. Do not disable interrupt directly but use API.

shell environment provided.

only stm32f103 supported at the moment.

----

I wanted something like uploading a task through serial communication, showing some text guidance. Needed information about task priority and stack size would be dealt by protocol. But soon I realized that to do this dynamic loader or mmu necessary. mmu, that high functionality? then it would be better to go with the linux.

But still I want to make it for dynamic loader version providing shared library. not manupulating linker script and compiling all together statically at once that request you to draw the whole map of memory where to put code in, which is not that charming.

## API

### Preventing preemption (for time critical task)

`preempt_disable()`
`preempt_enable()`

For a task that should not be interrupted. In real time priority only.

### waitqueue

### Critical regions and race conditions

api for lock and semaphore

## Memory map

User stack gets allocated by malloc() call in alloc_user_stack(). Therefore the size of HEAP_SIZE(default 32KiB) determines the seating capacity of both of user stacks and user's heap allocation.

	--- .text --- 0x00000000(0x08000000)	--- .data --- 0x20000000
	|           |                           |           |
	|           |                           |           |
	|           |                           |           |
	|           |                           |           |
	------------- 0x0007ffff(0x0807ffff)    ------------- 0x2000ffff

	--------------------------------- 0x2000ffff
	| | system stack                |
	| v                             |
	|-------------------------------|
	| ^                             |
	| | heap (including user stack) |
	|-------------------------------|
	| .bss                          |
	|-------------------------------|
	| .data                         |
	|-------------------------------|
	| .vector                       |
	--------------------------------- 0x20000000

## New task

### register user task

	REGISTER_TASK() - collect tasks in .user_task_list section
	  flags      \
	  stack      |
	  stack_size |-- struct task_t
	  addr       |
	             /

Task priority in flags

### load tasks from reset (in system's view)

1. sanity check
2. stack allocation
3. initial task register set

	 __________ 
	| psr      |  |
	| pc       |  | stack
	| lr       |  |
	| r12      |  v
	| r3       |
	| r2       |
	| r1       |
	| r0       |
	 ----------
	| r4 - r11 |
	| lr       |
	 ----------

4. put into runqueue
5. the initial task takes place

### User stack

	alloc_user_stack()
	  malloc()

## Scheduler

CF scheduler for NORMAL_PRIORITY tasks while FIFO scheduler for REALTIME_PRIORITY tasks.

the initial task takes place when no task in runqueue

	schedule()

Be aware that interrupt enabled after calling schedule().

### NORMAL_PRIORITY

### REALTIME_PRIORITY

REALTIME_PRIORITY + n in REGISTER_TASK() in case there are real time tasks more than one. up to ? considering other flags field.

### Runqueue

It has only a queue for running task. When a task goes to sleep or wait state, it just removed from runqueue. Because the task must remain in a waitqueue in any situation except temination, task can get back into runqueue from the link that can be found from waitqueue.
