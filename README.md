# ibox

Two types of task are handled, normal priority and real time priority tasks. Completly fair scheduler for normal priority tasks while FIFO scheduler for real time priority tasks.

It generates a periodic interrupt rated by HZ fot the heart rate of system. Change HZ as you wish.

Put a user task under /tasks directory. Code what is needed in general way using provided API and any other libraries. And simply register the task by REGISTER_TASK(main_function, stack_size, priority).

To access system resource, use provided API after checking how synchronization and wait queue are handled in the right sections below. Do not disable interrupt directly but use API.

shell environment provided.

tested on stm32f103.

----

I wanted something like uploading a task through serial communication, showing some text guidance. Needed information about task priority and stack size would be dealt by protocol. But soon I realized that to do this dynamic loader or mmu necessary. mmu, that high functionality? then it would be better to go with the linux.

But still I want to make it for dynamic loader version providing shared library. not manupulating linker script and compiling all together statically at once that request you to draw the whole map of memory where to put code in, which is not that charming.

## API

### Synchronization

schedule() never takes place in an interrupt context. When there is any interrupts active or pending, schedule() gets its chance to run after all the interrupts handled first.

Considering a single processor, `cli()` and semaphore(mutex) are enough to guarantee synchronization, I guess.

1. If data or region is accessed by more than a task, use mutex_lock()
  - It guarantees you access the data exclusively. You go sleep until you get the key.
2. If the one you are accessing to is the resource that the system also manipulates in an interrupt, use spinlock_irqsave().
  - spinlock never goes to sleep.
3. Or using mutex_lock(), do preempt_disable() as soon as you get the key.
  - Ensure that irq is not disabled before getting the key so that the task can go sleep. If you disable irq first before getting the key you never get back, infinite loop.
  - Watch out when you use cli() direct.

mutex and semaphore can go sleep.

`preempt_xxx()` and `spinlock()_xxx` never get preempted.

scheduler can be preempted but disable all local interrupts, `cli()`.

#### atomic data type

~~`atomic_t` guarantees manipulating on the data is not interruptible.~~

Let's just go with `int` type and `str`/`ldr` instructions.

#### semaphore(mutex)

long term waiting.
sleeping lock.

`mutex_lock()`, `semaphore_down()`

`semaphore_down_atomic()`

#### spin lock

~~short term waiting~~

`spin_lock()` - in context of interrupt. Actually it doesn't seem useful in a single processor because being in context of interrupt proves no one has the keys to where using spin lock. That means whenever you use spin lock in a task you must disable interrupts as you get the key using `spinlock_irqsave()`.

`spinlock_irqsave()` - for both of interrupt handlers and user tasks.

#### Preventing preemption

	preempt_disable()
	... here can't be interrupted ...
	preempt_enable()

preempt_disable() increases count by 1 while preempt_enable() decreases count. When the count reaches 0, interrupts get enabled.

### waitqueue

### timer

`get_ticks_64()` - `ticks` is not counted every HZ, but hardware system clock, get_stkclk(). To calculate elapsed time in second, ticks / get_stkclk().
	        
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
	  primask    |
	  stack      |
	  sp         |-- struct task_t
	  stack_size |
	  addr       |
	  runqueue   /

Task priority in flags

### load tasks from reset (in system's view)

1. sanity check
2. stack allocation
3. initial task register set
4. put into runqueue
5. the initial task takes place

Initial task register set:

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

### User stack

	alloc_user_stack()
	  malloc()

## Scheduler

CF scheduler for NORMAL_PRIORITY tasks while FIFO scheduler for REALTIME_PRIORITY tasks.

the initial task takes place when no task in runqueue

	schedule()

Be aware that interrupt enabled after calling schedule().

### context switch

explained here: [https://viewy.org/bbs/board.php?bo_table=note&wr_id=17](https://viewy.org/bbs/board.php?bo_table=note&wr_id=17)

### NORMAL_PRIORITY

### REALTIME_PRIORITY

REALTIME_PRIORITY + n in REGISTER_TASK() in case there are real time tasks more than one. up to ? considering other flags field.

### Runqueue

It has only a queue for running task. When a task goes to sleep or wait state, it just removed from runqueue. Because the task must remain in a waitqueue in any situation except temination, task can get back into runqueue from the link that can be found from waitqueue.

## Generalization

	/
	|-- sys_init()
	|   |-- clock_init()
	|   `-- mem_init()
	|-- main()
	|   |-- console_open()
	|   |-- systick_init()
	|   `-- init_task()
	|       |-- load_user_task()
	|       `-- schedule_on()
