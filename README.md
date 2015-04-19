# ibox

Two types of task are handled, normal priority and real time priority tasks. Completly fair scheduler for normal priority tasks while FIFO scheduler for real time priority tasks.

It generates a periodic interrupt rated by HZ fot the heart rate of system. Change HZ as you wish, `include/foundation.h`.

Put a user task under /tasks directory. Code what you want in general way using provided API and any other libraries. And simply register the task by `REGISTER_TASK(your_task, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY)`.

To access system resource, use provided API after checking how synchronization and wait queue are handled in the right sections below. Do not disable interrupt directly but use API.

shell environment provided.

tested on stm32f103.

## API

### Synchronization

[공유 메모리 동기화](https://viewy.org/bbs/board.php?bo_table=note&wr_id=19)

`schedule()` never takes place in an interrupt context. If there are any interrupts active or pending, `schedule()` gets its chance to run after all that interrupts handled first.

1. If data or region is accessed by more than a task, `use mutex_lock()`
  - It guarantees you access the data exclusively. You go sleep until you get the key.
2. If the one you are accessing to is the resource that the system also manipulates in an interrupt, `use spinlock_irqsave()`.
  - spinlock never goes to sleep.
3. If you code an interrupt handler, already preemption disabled, `spin_lock()` enough to use since `spinlock_irqsave()` only do `preempt_disable()` before `spin_lock()`.

Don't use `cli()` direct but `preempt_disable()`.

#### atomic data type

~~`atomic_t` guarantees manipulating on the data is not interruptible.~~

Let's just go with `int` type and `str`/`ldr` instructions.

#### semaphore(mutex)

long term waiting.
sleeping lock.

`mutex_lock()`, `mutex_unlock()`

`semaphore_down()`, `semaphore_up()`

#### spin lock

~~short term waiting~~

`spin_lock()` - in context of interrupt. Actually it doesn't seem useful in a single processor because being in context of interrupt proves no one has the keys to where using that spin lock. That means whenever you use spin lock in a task you must disable interrupts before you get the key using `spinlock_irqsave()`.

`spinlock_irqsave()` - for both of interrupt handlers and user tasks.

#### Preventing preemption

	preempt_disable()
	... here can't be interrupted ...
	preempt_enable()

`preempt_disable()` increases count by 1 while `preempt_enable()` decreases count. When the count reaches 0, interrupts get enabled.

### waitqueue

### timer

Variable `systick` can be accessed directly. `systick` is ~~not~~ counted every HZ, ~~but hardware system clock, `get_stkclk()`.~~ To calculate elapsed time in second, ~~`systick` / `get_stkclk()`~~ `systick` / `HZ`.

`get_systick_64()` - to get whole 64-bit counter.
	        
## Memory map

User stack gets allocated by `malloc()` call in `alloc_user_stack()`. Therefore the size of `HEAP_SIZE`(default 32KiB) determines the seating capacity of both of user stacks and user's heap allocation.

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
	  state      \
	  primask    |
	  stack      |
	  sp         |-- struct task_t
	  stack_size |
	  addr       |
	  runqueue   /

Task priority in state

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

CF scheduler for normal priority tasks while FIFO scheduler for real time priority tasks.

When a real time task takes place, scheduler can go off, `schedule_off()`, to remove scheduling overhead. In that case, don't forget `schedule_on()` afterward finishing its job.

The initial task takes place when no task in runqueue

실행이 종료되거나 대기상태에 들어가지 않는 한 리얼타임 태스크는 cpu 자원을 놓지 않도록 리얼타임 스케쥴러를 구현할 생각이었다. 그런데 그런 정도의 time critical한 태스크라면 인터럽트 금지 시키는 편이 나을 것이다. 태스크 내에서 스케쥴링만 금지 `schedule_off()` 시키는 방법도 있다. 스케쥴러 단위에서 그런 행동은 효용이 없어 보인다. 리얼타임 런큐 내 동일 우선순위 태스크에게 정확한 러닝타임을 확보해주는 것이 그 취지에 보다 알맞은 듯 보인다.

리얼타임 스케쥴러를 위한 새로운 시나리오:

1. 높은 우선순위의 태스크가 종료되기 전까지 그보다 낮은 우선순위의 태스크는 실행 기회를 얻지 못한다.
2. 동일 우선순위의 태스크는 RR방식으로 고정된(확정적인) 러닝타임으로 순환한다.

한가지 떠오르는 문제는(사용목적에 따라 다르겠지만), 인터럽트 금지등으로 할당된 러닝타임을 초과하여 실행된 태스크는 그 다음 턴에 핸디캡을 가져야 할지, 아니면 무시해야할지.

### context switch

[Cortex-M 문맥전환](https://viewy.org/bbs/board.php?bo_table=note&wr_id=17)

### Task priority

	 REALTIME |  NORMAL
	----------|-----------
	  0 - 99  | 100 - 120

	DEFAULT_PRIORITY = 110

The lower number, the higher priority.

`GET_PRIORITY()`

`IS_TASK_REALTIME()`

### Runqueue

It has only a queue for running task. When a task goes to sleep or wait state, it is removed from runqueue. Because the task must remain in an any waitqueue in any situation except temination, task can get back into runqueue from the link that can be found from waitqueue.

## Generalization

	/entry()
	|-- main()
	|   |-- sys_init()
	|   |-- console_open()
	|   |-- systick_init()
	|   |-- scheduler_init()
	|   |-- load_user_task()
	|   `-- init_task()
	|       |-- cleanup()
	|       `-- schedule_on()

* `entry()` - is very first hardware setup code to boot, usally in assembly.
* `sys_init()` - calls functions registered by `REGISTER_INIT_FUNC()`, architecture specific initialization.
