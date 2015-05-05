# ibox

Two types of task are handled, normal priority and real time priority tasks. Completly fair scheduler for normal priority tasks while FIFO scheduler for real time priority tasks.

It generates a periodic interrupt rated by HZ fot the heart rate of system. Change HZ as you wish, `include/foundation.h`.

Put a user task under /tasks directory. Code what you want in general way using provided API and any other libraries. And simply register the task by `REGISTER_TASK(your_task, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY)`.

To access system resource, use provided API after checking how synchronization and wait queue are handled in the right sections below. Do not disable interrupt directly but use API.

인터럽트는 우선순위에 따라 중첩될 수 있으나 동일한 인터럽트가 실행중인 현재 인터럽트를 선점할 수는 없다. 스케쥴러는 인터럽트 컨텍스트 내에서 실행될 수 없다. 스케줄러를 포함한 시스템 콜은 최하위 우선순위를 가진다. 시스템 콜은 다른 시스템 콜 및 스케줄러를 선점하지 못하고 그 반대도 마찬가지다. 하지만 우선순위가 높은 인터럽트에 의해 선점될 수 있다. 다만, 스케줄러는 스케줄링 단계에서 로컬 인터럽트를 비활성화 시키므로 선점되지 않는다.

shell environment provided.

tested on stm32f103.

## API

### Synchronization

[공유 메모리 동기화](https://note.toanyone.net/bbs/board.php?bo_table=note&wr_id=19)

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

락을 얻지 못하면 대기큐에 들어가 `TASK_WAITING` 상태가 되는데, 락이 풀릴 때 들어간 순서대로 하나씩 깨어남. 일괄적으로 한번에 깨우기엔 오버헤드만 크고, 그렇게 해야만 하는 상황에 대해서는 아직 떠오르지 않음.

#### spin lock

싱글 코어에 스핀락까지 구현할 필요는 없었지만, 구상할 때는 어쩐지 멀티코어까지 고려하게 되니까..

~~short term waiting~~

`spin_lock()` - in context of interrupt. Actually it doesn't seem useful in a single processor because being in context of interrupt proves no one has the keys to where using that spin lock. That means whenever you use spin lock in a task you must disable interrupts before you get the key using `spinlock_irqsave()`.

`spinlock_irqsave()` - for both of interrupt handlers and user tasks.

#### Preventing preemption

	preempt_disable()
	... here can't be interrupted ...
	preempt_enable()

`preempt_disable()` increases count by 1 while `preempt_enable()` decreases count. When the count reaches 0, interrupts get enabled.

### waitqueue

	/* Wait for condition */
	DEFINE_WAIT_HEAD(q)
	DEFINE_WAIT(wait_task)

	while (!condition) {
		wait_in(&q, &wait_task);
	}

	/* Wake up */
	wait_up(&q, option);

	*option:
	WQ_EXCLUSIVE - 하나의 태스크만 깨움
	WQ_ALL - 대기큐의 모든 태스크 깨움
	`숫자` - 숫자만큼의 태스크 깨움

대기큐에 도착하는 순서대로 들어가서 그 순서 그대로 깨어남(FIFO). 동기화 관련부분만 테스트하고 대기큐 별도로는 테스트하지 않음.

### timer

Variable `jiffies` can be accessed directly. `jiffies` is ~~not~~ counted every HZ, ~~but hardware system clock, `get_stkclk()`.~~ To calculate elapsed time in second, ~~`jiffies` / `get_stkclk()`~~ `jiffies` / `HZ`.

`get_jiffies_64()` - to get whole 64-bit counter.

### System call

`syscall(number, arg1, ...)` 형식으로 시스템 콜을 호출할 수 있다. `int`형을 반환하고, 시스템 콜 번호를 포함해 최대 4개까지 매개변수를 전달할 수 있다.

	int sys_test(int a, int b)
	{
		return a+b;
	}

위와 같은 시스템 콜을 정의했다면, `syscall.h` 파일에 해당 시스템 콜을 추가한다.

	#define SYSCALL_RESERVED 0
	#define SYSCALL_SCHEDULE 1
	#define SYSCALL_TEST     2
	#define SYSCALL_NR       3

주의할 점은 해당 시스템 콜을 추가하고 `SYSCALL_NR` 값을 그만큼 증가시켜주어야 한다. `SYSCALL_NR`은 등록되지 않은 시스템 콜 호출을 방지하는 데 사용된다. 유효하지 않은 시스템 콜을 호출할 경우 0번 시스템 콜 `SYSCALL_RESERVED` 가 실행된다.

그리고 추가한 번호 순서에 알맞은 테이블 위치에(`kernel/syscall.c`) 해당 함수를 등록한다.

	void *syscall_table[] = {
		sys_reserved,		/* 0 */
	        sys_schedule,
	        sys_test,

### Device Driver

`devtab` 해시 테이블에 모든 디바이스가 등록된다.

`getdev()` - 디바이스 자료구조 구하기

`REGISTER_DEVICE()` - 디바이스 등록, 등록하면 부팅시 자동으로 테이블에 연결됨. 동적 등록시 다른 매커니즘 필요.

e.g.

	int your_open(int id)
	{
		...
	}

	size_t your_read(int id, void *buf, size_t size)
	{
		...
	}

	static struct driver_operations your_ops = {
		.open  = your_open,
		.read  = your_read,
		.write = NULL,
		.close = NULL,
	};
	REGISTER_DEVICE(your_device, &your_ops);

	그리고 `include/kernel/device.h`에 고유번호를 등록하면 끝
	#define your_device	고유번호

id는 사실 전달할 필요가 없다. 인자를 하나씩 제거할 수 있지만, 디바이스 상위 시스템 추상화를 어떻게 할지 감을 못잡았으므로 일단 그대로 두는 걸로.

### Memory allocation

`kmalloc()`
`free()`

## Memory map

User stack gets allocated by `malloc()` call in `alloc_user_stack()`. Therefore the size of `HEAP_SIZE`(default 32KiB) determines the seating capacity of both of user stacks and user's heap allocation.

	--- .text --- 0x00000000(0x08000000)	--- .data --- 0x20000000
	|           |                           |           |
	|           |                           |           |
	|           |                           |           |
	|           |                           |           |
	------------- 0x0007ffff(0x0807ffff)    ------------- 0x2000ffff

	--------------------------------- __mem_end(e.g. 0x2000ffff)
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
	--------------------------------- __mem_start(e.g. 0x20000000)

초기 커널 스택은 `__mem_end`을 시작으로 `DEFAULT_STACK_SIZE`만큼 할당되어야 함. 초기 커널 스택에 해당하는 버디 `free_area`의 마지막 페이지들이 사용중으로 마크됨.

malloc()은 kmalloc()의 랩퍼일 뿐, 차후에 단편화를 고려한 slab과 같은 캐시를 구현하는 것도 고려해볼만..

~~시스템 부팅부터 init 태스크로 제어가 넘어가기 전까지 커널 스택이 보호되지 않고 버디 `free_area`에 들어가 있음. 초기화 막바지에 커널 스택은 init 태스크 스택으로 교체되기 때문에 문제는 없을 것으로 보이지만, 주의하고 있을 필요가 있음. 버디 초기화 시에 확실히 마크하고 init으로 제어가 넘어갈 때 free해야함!~~미뤄두었다가 피봤음.

AVR에 포팅할 것도 고려하고 있는데 페이징은 좀 무리인 듯. page 구조체 사이즈가 못해도 7바이트는 되어야 하는데 atmega128 sram이 4kb 밖에 안되니까.

그에 알맞은 메모리 관리자를 생각해보고 구현하자. 그리고 페이징 메모리 관리자와 새로운 메모리 관리자 모두 CONFIG 할 수 있도록. 페이징과 버디 할당자를 각 파일로 떼어낼 것.

### Paging

	__mem_start(e.g. 0x20000000)                               __mem_end
	^----------------------------------------------------------^
	| kernel data | mem_map | bitmap |//////////////////////////|
	 -----------------------------------------------------------
	              ^mem_map

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

다음 할 일:

1. 런큐 자료구조 변경
2. 대기큐에 오래 잡혀있던 태스크의 vruntime 차이 때문에 wake-up 후 자원 독점 가능성. 모든 태스크에 최소 실행 시간이 보장되어야 함.
3. CFS 우선순위에 따른 vruntime 계산

리얼타임 우선순위가 너무 많음. 현재 0~100까지 있는데 메모리 아끼기 위해 대폭 줄일 필요성. 그리고 리얼타임 태스크 우선순위가 그렇게 많이 필요하지 않잖아?

* 이미 런큐에 등록된 태스크가 중복 등록되어서는 안됨.
* 이미 런큐에서 삭제된 태스크를 거듭 삭제하면 `nr_running`이 잘못된 값을 가짐.

### context switch

[Cortex-M 문맥전환](https://note.toanyone.net/bbs/board.php?bo_table=note&wr_id=17)

### Task priority

	 REALTIME |  NORMAL
	----------|-----------
	  0 - 100 | 101 - 120

	DEFAULT_PRIORITY = 110

The lower number, the higher priority.

`GET_PRIORITY()`

`IS_TASK_REALTIME()`

### Runqueue

It has only a queue for running task. When a task goes to sleep or wait state, it is removed from runqueue. Because the task must remain in an any waitqueue in any situation except temination, task can get back into runqueue from the link that can be found from waitqueue.

## Generalization

	/entry() #interrupt disabled
	|-- main()
	|   |-- sys_init()
	|   |-- mm_init()
	|   |-- devman_init()
	|   |-- sei() #interrupt enabled
	|   |-- console_open()
	|   |-- systick_init()
	|   |-- scheduler_init()
	|   |-- load_user_task()
	|   `-- init_task()
	|       |-- cleanup()
	|       `-- schedule_on()

* `entry()` - is very first hardware setup code to boot, usally in assembly.
* `sys_init()` - calls functions registered by `REGISTER_INIT_FUNC()`, architecture specific initialization.

## Porting

Change "machine dependant" part in `Makefile`.

Uncomment or comment out lines in `CONFIG` file to enable or disable its functionalities.

Change `HZ` in `include/foundation.h`

데이터 타입 사이즈 정의 및 통일. 기본 시스템 데이터 타입을 long으로 썼는데 int로 수정해야 할까 싶다. 간혹 64비트 시스템에 int형이 32비트인 경우가 있다지만 64비트는 고려하지 않은데다, avr의 경우 long이 32비트 int가 16비트인지라 공통으로 사용하기엔 long보다 int가 더 적절할 지 모르겠다.

### Memory Manager

메모리 관리자를 초기화하기 위해서 아키텍처 단에서 `_ebss`, `__mem_start`, `__mem_end` 제공해주어야 함(`ibox.lds`). `PAGE_SHIFT` 디폴트 값이 4KiB이므로 이것도 아키텍처 단에서 지정해줄 것(`include/asm/mm.h`).

### Task Manager

`init_task_context(struct task_t *p)`
