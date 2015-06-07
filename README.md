# ~~ibox~~ yaos

ibox was initially the name of a device that I was making. In the course of it this OS came out, holding affection from me. And now I think it's time to give its own name, YAOS. Well, not final. I will pick one amongst others of coming up in mind.

Two types of task are handled, normal priority and real time priority tasks. Completly fair scheduler for normal priority tasks while FIFO scheduler for real time priority tasks.

It generates a periodic interrupt rated by HZ fot the heart rate of system. Change HZ as you wish, `include/foundation.h`.

Put a user task under /tasks directory. Code what you want in general way using provided API and any other libraries. And simply register the task by `REGISTER_TASK(function, type, priority)`.

To access system resource, use provided API after checking how synchronization and wait queue are handled in the right sections below. Do not disable interrupt directly but use API.

인터럽트는 우선순위에 따라 중첩될 수 있으나 동일한 인터럽트가 실행중인 현재 인터럽트를 선점할 수는 없다. 스케쥴러는 인터럽트 컨텍스트 내에서 실행될 수 없다. 스케줄러를 포함한 시스템 콜은 최하위 우선순위를 가진다. 시스템 콜은 다른 시스템 콜 및 스케줄러를 선점하지 못하고 그 반대도 마찬가지다. 하지만 우선순위가 높은 인터럽트에 의해 선점될 수 있다. 다만, 스케줄러는 스케줄링 단계에서 로컬 인터럽트를 비활성화 시키므로 선점되지 않는다.

태스크다마 커널 스택과 태스크 스택 하나씩을 갖는다. 태스크 문맥 내에서 태스크 스택(psp)을 사용하고, 핸들러(인터럽트) 문맥에서 커널스택(msp)을 사용한다. 시스템 콜과 문맥전환은 최하위 우선순위로 일반모드에서만 발생할 수 있다. 따라서 유저 태스크의 인터럽트 진입점인 시스템 콜과 문맥전환에서는 태스크 스택psp만 고려하면 된다. 인터럽트 진입 후 스택은 커널스택msp가 사용되고 스택 포인터는 인터럽트 중첩이 아니라면 항상 top을 가리킨다.

Each task has one kernel stack and one task stack. Each task's kernel stack pointer address the same one memory region at the moment actually because allocating kernel stack per each task seemed quite waste of memory. But it can easily alloc one for each.
	        
Both of kernel task and user task use user stack, let me call it task stack rather than user stack, while interrupt handler uses kernel stack. At the point of entering an interrput kernel stack always points its top unless it is not nested interrupt.

shell environment provided.

tested on stm32f103.

## CONFIGURE

### `CONFIG_DEBUG`

디버깅용 코드와 구체적인 로그 출력

### `CONFIG_REALTIME`

리얼타임 스케줄러 추가

### `CONFIG_PAGING`

페이징 메모리 관리자, 버디 할당자 추가

### `CONFIG_SYSCALL`

시스템 콜 추가

### `CONFIG_FS`

가상 파일 시스템 추가

### include/foundation.h

#### HZ

시스템 클럭 주파수

#### CONSOLE

콘솔 디바이스 설정. 디폴트 usart1

## API

### Synchronization

`lock_t` - 락 기본형. 0 이하 값은 잠김, 1 이상의 값은 풀림이라는 것이 구현된 모든 락의 기본 전제.

[공유 메모리 동기화](https://note.toanyone.net/bbs/board.php?bo_table=note&wr_id=19)

`schedule()` never takes place in an interrupt context. If there are any interrupts active or pending, `schedule()` gets its chance to run after all that interrupts handled first.

1. If data or region is accessed by more than a task, `use mutex_lock()`
  - It guarantees you access the data exclusively. You go sleep until you get the key.
2. If the one you are accessing to is the resource that the system also manipulates in an interrupt, `use spin_lock_irqsave()`.
  - spinlock never goes to sleep.
3. If you code an interrupt handler, already preemption disabled, `spin_lock()` enough to use since `spin_lock_irqsave()` only do `preempt_disable()` before `spin_lock()`.

Don't use `cli()` direct but `preempt_disable()`.

데드락은 아무래도 골치거리라서 체크할 방법이 필요한데, 전체락을 리스트로 일괄관리하면 어떨까? 디버깅시에 인터럽트 우선순위를 조정해서 벽돌이 된 지점에 걸린 락을 찾을 수 있도록. 아니면 디버깅용 인터럽트 하나를 최상위 우선순위로 지정하고 스택킹된 pc의 값을 출력.

(1)디폴트 우선순위를 0보다 크게 설정. (2)디버깅용 핀을 하나 지정, 우선순위를 0으로 가장 높게 설정. (3) 핀에 신호가 걸리면 스택킹된 pc(즉 실행중이던 위치)를 출력.

#### atomic data type

~~`atomic_t` guarantees manipulating on the data is not interruptible.~~

Let's just go with `int` type and `str`/`ldr` instructions.

기본 자료형 int을 atomic 형 으로 사용할 경우 단순 대입연산(e.g. `a = value`)과 다르게 `a |= value` 와 같은 연산은 atomic으로 수행되지 않음을 주지할 필요가 있다.

#### semaphore(mutex)

long term waiting.
sleeping lock.

`mutex_lock()`, `mutex_unlock()`

`semaphore_down()`, `semaphore_up()`

락을 얻지 못하면 대기큐에 들어가 `TASK_WAITING` 상태가 되는데, 락이 풀릴 때 들어간 순서대로 하나씩 깨어남. 일괄적으로 한번에 깨우기엔 오버헤드만 크고, 그렇게 해야만 하는 상황에 대해서는 아직 떠오르지 않음.

#### spin lock

싱글코어가 타겟이지만 락을 쓸 때는 어쩐지 멀티코어까지 가정하게 되어서, 싱글코어로 최적화 한다면 `spin_lock()`은 공백으로, `spin_lock_irqsave()`는 `local_irq_disable()`로 변경.

멀티코어에 대해 아는바가 별로 없어서 동기화 관련 코드를 작성할 때마다 고민이 많다. 멀티코어는 잊어버리자 싶다가도, 왠지 얼렁뚱땅 허술한 인간이 되는 거 같아서..

~~short term waiting~~

`spin_lock()` - in context of interrupt. Actually it doesn't seem useful in a single processor because being in context of interrupt proves no one has the keys to where using that spin lock. That means whenever you use spin lock in a task you must disable interrupts before you get the key using `spin_lock_irqsave()`.

`spin_lock_irqsave()` - for both of interrupt handlers and user tasks.

rw lock 필요. 콘솔이나 irc 값 읽는데 스핀락은 비용이 큰 듯. 자료구조와 락 형태 개선.

#### rw lock

`read_lock()`
`write_lock()`

`read_lock()`으로 임계영역에 진입하더라도 그 와중에 `write_lock()`이 발생하고 `read_unlock()`전에 write 연산이 종료된다면 read 쪽에서는 이를 감지하지 못함. read 연산이 write 연산보다 짧다는 것이 보장되어야 함.

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

주의할 점은 해당 시스템 콜을 추가하고 `SYSCALL_NR` 값 역시 증가시켜주어야 한다. `SYSCALL_NR`은 등록되지 않은 시스템 콜 호출을 방지하는 데 사용된다. 유효하지 않은 시스템 콜을 호출할 경우 0번 시스템 콜 `SYSCALL_RESERVED` 가 실행된다.

그리고 추가한 번호 순서에 알맞은 테이블 위치에(`kernel/syscall.c`) 해당 함수를 등록한다.

	void *syscall_table[] = {
		sys_reserved,		/* 0 */
	        sys_schedule,
	        sys_test,

시스템 콜은 보통 관련소스(파일 시스템이라면 fs.c) 하단에 정의되어 있다.

### Device Driver

`devtab` 해시 테이블에 모든 디바이스가 등록된다.

`getdev()` - 디바이스 자료구조 구하기

`dev_get_newid()` - 새로운 디바이스 아이디 얻기

`register_dev(id, ops, name)` - 디바이스 등록

`/dev/` 아래 `name` 노드 생성. `name`이 NULL일 경우 노드가 이미 생성된 것으로 가정하고 디바이스만 등록. minor 번호가 `name` 접미사로 붙음. e.g. name1

`MODULE_INIT(func)` - 디바이스 초기화 함수 등록

e.g.

	static int your_open(struct file *file)
	{
		...
	}

	static size_t your_read(struct file *file, void *buf, size_t size)
	{
		...
	}

	static struct file_operations your_ops = {
		.open  = your_open,
		.read  = your_read,
		.write = NULL,
		.close = NULL,
	};

	static your_init()
	{
		dev_t id = mkdev(0, 0);

		register_dev(id, &your_ops, "your_dev");
		...
	}

	MODULE_INIT(console_init);

`mkdev()` 리턴값이  0이면 오류. 디바이스를 등록할 가용공간이 없음.

첫번째 인자 major 값이 0이면 자동할당. 이미 할당된 major 번호를 공유하는 공통 드라이버를 구현할 경우 이미 생성된 노드에서 major 번호를 구할 것. major 번호는 순차적으로 증가한 값을 갖는다. 즉, 지정한 major 번호가 등록되지 않은 번호라면 요청한 번호는 적절한 값으로 변경될 수 있다. 반면 지정한 minor 번호가 이미 존재할 경우에는 자동으로 비어있는 번호를 찾아 등록된다. 대부분의 경우, `mkdev(0, 0)`으로 충분.

major 16 비트, minor 16 비트. 각 65535개씩 등록 가능(0번째 항목은 에러체크로 제외).

id는 사실 전달할 필요가 없다. 인자를 하나씩 제거할 수 있지만, 디바이스 상위 시스템 추상화를 어떻게 할지 감을 못잡았으므로 일단 그대로 두는 걸로.

`MODULE_INIT`은 부트 타임 등록만 된다. 런타임 등록도 고려해볼 것.

통합 버퍼 관리자 고려해볼 것. 디바이스 드라이버에서 사용하는 버퍼 같은 경우 직접 kmalloc 사용하기보단 상위 관리자를 이용.

### GPIO

`gpio_init()` - 인터럽트 벡터 번호 리턴. 비인터럽트일 경우 마이너스 값 리턴. flags 설명(gpio.h)

각 포트와 핀은 0~n으로 지시. 포트당 8핀일 경우 PORTA와 PORTB의 각 마지막 핀 번호는 7과 13으로 지시.

`ret_from_gpio_int()` - 인터럽트 서비스 루틴 구현시 이 함수로 리턴해야 함

### Softirq

`request_softirq()`
`raise_softirq()`

할당받은 softirq 번호가 `SOFTIRQ_MAX` 보다 크거나 같다면 오류. softirq pool이 이미 꽉 찬 상태. 등록시 반드시 체크필요.

`softirqd` 커널 태스크는 softirq가 raise된 경우에만 실행된다. `RT_LEAST_PRIORITY` 우선순위. 우선순위별 softirq 운행할 필요가 있어보임. 일반 태스크를 모두 선점해버리는 softirq라면 인터럽트와 다를바가 없고, 반면 리얼타임 태스크에서 동작을 하지 않는 softirq라면 리얼타임 태스크가 필요한 자원을 얻지 못해서 데드락에 걸리는 문제.

리얼타임 태스크 : softirqd 의 우선순위가 `RT_LEAST_PRIORITY` 이기 때문에 보다 우선순위가 높은 리얼타임 태스크가 softirq 자원이 필요한 경우에는 태스크의 우선순위를 일시적으로 낮추어 자원을 확보해야한다. 또는, softirq 핸들러 우선순위를 일시적으로 높이는 방법?

일반 태스크 : HIGH, NORMAL, LOW 세가지를 고려중.

	LOW = LEAST_PRIORITY
	NORMAL = DEFAULT_PRIORITY
	HIGH = 리얼타임 스케줄러 모듈을 빼고 컴파일 했다면 RT_LEAST_PRIORITY + 1,
	       아니면 RT_LEAST_PRIORITY

	       * RT_LEAST_PRIORITY 라면, softirq 수행이 종료될 때까지 일반 태스크는 실행되지 않는다. 동일(최하위) 우선순위의 리얼타임 태스크의 경우 함께 스케줄링되는 반면 상위 리얼타임 태스크가 실행중일 때는 상위 리얼타임 태스크가 종료할 때까지 softirq는 지연된다.

	       * RT_LEAST_PRIORITY+1 인 경우 일반 태스크와 함께 스케쥴링 된다(다만 가장 높은 일반 태스크 우선순위를 갖고 있으므로 프로세서 점유시간이 낮은 우선순위 태스크보다 길다).

----

~~일단, 기본 softirqd 우선순위는 `DEFAULT_PRIORITY`. 우선순위 변경이 필요한 경우 콜백에서 처리하는 걸로 하고 다음 진행.~~ time critical 한 부분은 이미 인터럽트 핸들러에서 처리되었으니 softirq는 current 태스크와 동일한 우선순위로 스케줄링하면 적절하지 않을까? 우선순위를 current에 맞추면 리얼타임 태스크/일반 태스크 구분할 필요가 사라진다. 그리고 softirq의 의도 역시 독점보다 자원을 효율적으로 배분하는 데 있으니까 이 방법이 적당해 보인다. 32비트 시스템에서 32개, 8비트 시스템에서 8개의 softirq 밖에 운용하지 못한다는 게 현재 자료구조의 한계랄수도 있지만 적절한 제한이 효율적일지도 모를일이다. 어쩌면 내 머리의 한계일지도 모른다. 그렇게 많은 softirq를 운용해야할 필요성이 딱히 떠오르지 않는다. 차후 필요성이 생기면 보완하기로.

softirq가 32개라면 그사이에서의 우선순위도 필요해 보인다. 등록시 배열 인덱스를 지정해서 구현하면 간단하겠으나 순차적으로 32개의 루틴을 모두 돈다면 우선순위 효과를 의심해볼만 하다. 떠오르는 대안은 두가지인데 (1) 최상위 우선순위 pending 만 실행하고 탈출한다. 그럼 스케줄될 때마다 다음 pending이 순차적으로 실행될 것이다. 물론 그 사이 더 높은 pending이 발생했다면 높은 pending을 먼저 처리한다. 한번의 스케줄에 가장 높은 하나의 pending 만을 처리한다는 게 요점인데, 우선순위가 낮은 pending이 무한정 지연될 가능성이 있다. (2) 스레드 처리한다. 몽땅 스레드 처리하기는 비용이 커보이고, 높은 우선순위 몇개만 스레드처리하고 나머지는 순차처리.

### Memory allocation

`kmalloc()`
`kfree()`
`malloc()`
`free()`

디폴트 스택 사이즈가 1KB이므로 버퍼 용도의 사이즈가 큰 로컬 변수 사용은 피해야 한다. 가급적 동적할당 malloc 또는 kmalloc을 사용해야 한다.

#### Buddy allocator

지수법칙을 활용한 메모리 할당자. 2의 누승에서 `n^2 = n + n` 이 된다. 쪼개진 이 두 개의 n을 버디라고 한다. 두개가 한 쌍인 버디는 하나의 상태비트를 갖는다. 상태비트는 버디가 할당되거나 해제될 때마다 토글된다. 즉, 비트 값이 0일 경우 한쌍인 두개의 버디는 모두 할당되었거나 해제된 상태이고, 비트 값이 1일 경우 둘 중 하나의 버디만 할당된 상태를 뜻한다.

할당시 요청한 크기의 버디가 없다면 상위 버디 하나를 쪼개어 할당하고 남는 한 쪽의 버디는 하위 버디 리스트에 추가한다.

해제시 해당 크기의 버디 리스트에 추가하는데 이때 상태비트를 확인하여 비트 값이 0인 경우(두개의 버디 모두가 해제된 경우) 두 버디를 합쳐 상위 버디 리스트에 추가한다. 상위 버디 리스트로 합칠 수 있는만큼 반복한다.

버디 할당자는 페이징 메모리 관리자 하에서 동작한다.

#### First fit allocator

연속된 주소 공간을 하나의 구간chunk으로 가용한 구간들을 서로 연결 관리한다. 메모리 할당 요청이 일어나면 요청 크기를 만족하는 구간을 찾을 때까지 처음부터 검색한다. 찾은 구간이 동일한 크기일 경우 변경사항 없이 해당 구간을 할당하고, 그보다 큰 구간일 경우 쪼갠 나머지를 다시 `free_area` 영역으로 연결한다.

할당된 메모리가 다시 `free_area` 영역으로 연결하면서 해제를 수행한다. 단편화를 고려하여 해제된 구간은 다음 구간과 연속된 메모리 어드레스를 갖는지 확인한 후 연속된 구간이라면 두 구간을 합친다. 연속된 구간이 나타나지 않을 때까지 반복한다.

	 ----------------------     ----------------------
	| meta | ///////////// | - | meta | ///////////// | - ...
	 ----------------------     ----------------------
	^HEAD

할당은 낮은주소-높은주소 순으로 이루어지기 때문에 할당/해제 두 경우 모두 head 포인터를 적절히 변경해주어야 한다.

페이징 대용뿐만 아니라 유저용 `malloc()`으로 사용할까? `kmalloc()`으로 커널로부터 확보된 메모리의 2차 할당자로. 고비용의 인터럽트 금지 부분을 제거할 수 있으니까. 해제함수에서 내부 단편화를 고려한 루프문도 제거하고.

#### miscellaneous

`itoa()` 함수원형: `char *itoa(int v, char *buf, unsigned int base, size_t maxlen)`. 오버플로어를 방지하기 위해 마지막 maxlen 인자가 추가됨. 변환된 문자열의 시작주소가 반환됨. 인자로 넘긴 buf변수를 변환된 문자열로 사용하면 안됨. 반드시 리턴값을 문자열로 사용해야 함.

## Memory map

	* memory map at boot time
	+-------------------------+ _ram_end(e.g. 0x2000ffff)
	| | initial kernel stack  |
	| |                       |
	| v                       |
	|-------------------------|
	| .bss                    |
	|-------------------------|
	| .data                   |
	|-------------------------|
	| .vector                 |
	+-------------------------+ _ram_start(e.g. 0x20000000)

	* task memory space
	+-------------+ stack top      -
	|   |         |                ^
	|   |         |                |
	|   |         |                |
	|   | stack   |     STACK_SIZE |
	|   |         |                |
	|   |         |                |
	|   |         |                |
	|   v         |                |
	|   -         |              - |
	|   ^         |              ^ |
	|   | heap    |    HEAP_SIZE | |
	|   |         |              v v
	+-------------+ mm.base      - -

	+-------------+ mm.kernel      -
	|   |         |                ^
	|   | kernel  |                |
	|   | stack   | KERNEL_STACK_SIZE
	|   |         |                |
	|   v         |                v
	+-------------+                -

초기 커널 스택은 `_ram_end`을 시작으로 `STACK_SIZE`만큼 할당되어야 함. 초기 커널 스택에 해당하는 버디 `free_area`의 마지막 페이지들이 사용중으로 마크됨.

malloc()은 kmalloc()의 랩퍼일 뿐, 차후에 단편화를 고려한 slab과 같은 캐시를 구현하는 것도 고려해볼만..

~~시스템 부팅부터 init 태스크로 제어가 넘어가기 전까지 커널 스택이 보호되지 않고 버디 `free_area`에 들어가 있음. 초기화 막바지에 커널 스택은 init 태스크 스택으로 교체되기 때문에 문제는 없을 것으로 보이지만, 주의하고 있을 필요가 있음. 버디 초기화 시에 확실히 마크하고 init으로 제어가 넘어갈 때 free해야함!~~미뤄두었다가 피봤음.

AVR에 포팅할 것도 고려하고 있는데 페이징은 좀 무리인 듯. page 구조체 사이즈가 못해도 7바이트는 되어야 하는데 atmega128 sram이 4kb 밖에 안되니까.

그에 알맞은 메모리 관리자를 생각해보고 구현하자. 그리고 페이징 메모리 관리자와 새로운 메모리 관리자 모두 CONFIGURE 할 수 있도록. 페이징과 버디 할당자를 각 파일로 떼어낼 것.

### Paging

	_ram_start(e.g. 0x20000000)                                _ram_end
	^----------------------------------------------------------^
	| kernel data | mem_map | bitmap |//////////////////////////|
	 -----------------------------------------------------------
	              ^mem_map

	page flags:
	31 30 29 ...... 26 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
	 -----------------------------------------------------------------------
	| reserved              |   ORDER   |           PAGE_FLAGS*             |
	 -----------------------------------------------------------------------

	  *PAGE_FLAGS
	   -------------------------------------------------------
	  | NO | ELEMENT        | DESCRIPTION                     |
	   -------------------------------------------------------
	  |  1 | PAGE_BUDDY     | page(s) allocated by buddy      |
	  |  2 |                |                                 |
	  |  3 |                |                                 |
	  |  4 |                |                                 |
	  |      ...                                              |
	  | 12 |                |                                 |
	   -------------------------------------------------------

	   PAGE_BUDDY flag set by `alloc_pages()` and cleared by `free()`,
	   checking if it is really allocated by buddy allocator.

`mem_map`은 시스템 전체 메모리를 관리하기 위한 메타정보를 저장한 배열이고, 전체 메모리에서 zone 단위로 나눠 사용할 수 있다.

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

* 이미 런큐에 등록/삭제된 태스크가 중복 등록/삭제되어서는 안됨. 런큐 링크가 깨짐.
* 리얼타임 태스크의 경우 중복 등록/삭제는 `nr_running` 값을 망침.

`TASK_RUNNING` - 스케줄링 연산량을 줄이기 위해 상태 체크를 `0 아니면 그외 모든 값`식으로 하기 때문에 값이 반드시 0 이어야 함.
`TASK_STOPPED`
`TASK_WAITING`
`TASK_SLEEPING`

`*_task_*` 패밀리 매크로에서 이루어지는 포인터 캐스팅 연산량도 체크후, 인자를 포인터가 아니라 일반 변수로 받도록 수정

`schedule()` - 스케줄링. 태스크 상태는 그대로 유지
`yield()` - 스케줄링. 태스크 상태는 `TASK_SLEEPING`로 전환

`update_curr()` - 태스크 정보는 스케줄링시 업데이트. 인터럽트 핸들러의 점유시간 역시 각 태스크의 사용시간에 포함된다. 섬세하게 구분하기에는 오버헤드가 큰데다, 시스템 콜 같은 경우에도 사용자 태스크니까. 업데이트를 원래 타임 인터럽트에서 처리했는데, `yield()`할 경우 한 사이클을 까먹게되어서 수정.

### context switch

[문맥전환](https://note.toanyone.net/bbs/board.php?bo_table=note&wr_id=17)

### Task priority

	 REALTIME |  NORMAL
	----------|-----------
	  0 - 10  | 11 - 255

	DEFAULT_PRIORITY = 132

The lower number, the higher priority.

`get_task_pri()`

`is_task_realtime()`

### Runqueue

It has only a queue for running task. When a task goes to sleep or wait state, it is removed from runqueue. Because the task must remain in an any waitqueue in any situation except temination, task can get back into runqueue from the link that can be found from waitqueue.

## Generalization

	/entry() #interrupt disabled
	|-- main()
	|   |-- sys_init()
	|   |-- mm_init()
	|   |-- fs_init()
	|   |-- device_init()
	|   |-- systick_init()
	|   |-- scheduler_init()
	|   |-- make_init_task()
	|   |-- load_user_task()
	|   |-- softirq_init()
	|   |-- console_init()
	|   |-- sei() #interrupt enabled
	|   `-- idle()
	|       |-- cleanup()
	|       `-- sleep

* `entry()` - is very first hardware setup code to boot, usally in assembly.
* `sys_init()` - calls functions registered by `REGISTER_INIT()`, architecture specific initialization.

`softirq_init()` 내부에서 커널스레드를(init 자식인) 생성하기 때문에 init 태스크가 형성된 후에 호출되어야 함.

## Porting

Change "machine dependant" part in `Makefile`.

Uncomment or comment out lines in `CONFIGURE` file to enable or disable its functionalities.

Change `HZ` in `include/foundation.h`

You can not use system call before init task runs, where interrupt gets enabled. So there is restriction using such functions like printf(), triggering system call.

~~데이터 타입 사이즈 정의 및 통일. 기본 시스템 데이터 타입을 long으로 썼는데 int로 수정해야 할까 싶다. 간혹 64비트 시스템에 int형이 32비트인 경우가 있다지만 64비트는 고려하지 않은데다, avr의 경우 long이 32비트 int가 16비트인지라 공통으로 사용하기엔 long보다 int가 더 적절할 지 모르겠다. 사용자 지정 데이터 타입은 코드 가독성을 망쳐서 피하고 싶고, word 타입을 표준화 해주면 좋을텐데.~~

sizeof(int)를 기본 word 타입으로 결정했다. 위키피디아의 다음 문장이 결정적이었다:

> The type int should be the integer type that the target processor is most efficient working with.

[http://en.wikipedia.org/wiki/C_data_types](http://en.wikipedia.org/wiki/C_data_types)
[https://gcc.gnu.org/wiki/avr-gcc](https://gcc.gnu.org/wiki/avr-gcc)

가독성을 위해 사용자 지정 데이터 형 사용을 최대한 삼가했다. 기본 데이터 형 이외에 일관(호환)성을 유지하기 위해 지정한 데이터 타입은 다음과 같다:

	bool         false or true         - enum
	refcnt_t     reference count       - unsigned short int
	mode_t       mode(flags)           - unsigned short int
	lock_t       lock                  - volatile int
	size_t       size(len)             - unsigned int
	offset_t     offset                - unsigned int
	dev_t        device                - unsigned int
	uint64_t     64 bit unsigned       - unsigned long long

	struct list  doubly linked list
	struct fifo  first in first out

	buf_t        buffer cache          - struct list
	mutex_t      mutual exclusive lock - struct semaphore

`list_add`, `slist_add` - new 노드는 ref의 다음 노드가 된다. 즉 `list_add_next`. 그 이외의 함수는 지원하지 않는다. ref 조작으로 추가적인 함수(`list_add_prev`)를 실현할 수 있기 때문이다.

`slist_del_next` - 단일 링크드 리스트 자료구조는 인자로 넘어 온 노드의 다음 노드를 삭제하는 함수만 지원한다. 자료구조의 특성상 리스트 순회중에 삽입/삭제가 이루어지기 때문이다.

	용량 확보를 위해 단일 링크드 리스트로 변경 고려:

	struct device
	struct active_superblock_list

시스템 콜은 OS에 필수적인 요소라 모듈화하지 않고 빌트인 하는 게 좋을 듯 한데, 여전히 AVR 포팅을 고려중이라 고민이다. AVR에서도 소프트웨어 인터럽트 트릭을 사용할 수 있을 것 같지만, 8비트 시스템의 낮은 클럭 주파수에서 시스템 콜을 호출하는 비용이 만만찮을 것이다. 그렇다고 시스템 콜을 때에 따라 빼버릴 수 있도록 그대로 모듈로 두기엔 구조적 취약성이 거슬리고.

prefix `__` 는 machine dependant 함수나 변수에 사용. prefix `_` 는 링커 스크립트 변수에 사용. postfix `_core` 는 bare 함수에 사용.

대문자명 함수는 매크로. 인자를 넘길 때 포인터 대신 변수로 넘겨야 함. 더불어 `func_init()` 규칙과 반대로 `INIT_FUNC()`처럼 init을 먼저 쓴다.

`INIT_IRQFLAG()` - MCU 마다 다른 레지스터 초기값을 설정하기 위한 매크로

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
	 ----------

`atomic_set()` in `asm/lock.h`

### System clock

`get_systick()`
`get_systick_max()`

### Memory Manager

메모리 관리자를 초기화하기 위해서 아키텍처 단에서 `_ebss`, `_ram_start`, `_ram_end` 제공해주어야 함(`ibox.lds`). `PAGE_SHIFT` 디폴트 값이 4KiB이므로 이것도 아키텍처 단에서 지정해줄 것(`include/asm/mm.h`).

### Task Manager

`set_task_context(struct task *p)`
`set_task_context_soft(struct task *p)`
`set_task_context_hard(struct task *p)`

`STACK_SEPARATE` 서로 다른 메모리 공간 사용
`STACK_SHARE`    커널스택만 공유

`set_task_type()` 태스크 타입을 변경하면 태스크 상태는 리셋된다

## Interrupt

자원배분에 대한 공정성은 스케줄러의 역량에 달려있지만 인터럽트 처리의 경우, 바쁜 인터럽트 때문에 자원이 계속 인터럽트 처리에 매달릴 수 있다. 해당 인터럽트에 핸디캡 주는 방법을 찾아볼 것.

## File system

프로토타입인데다 공간낭비를 막기 위해 블럭 쓰기/읽기를 바이트 단위로 처리하다보니 최적화 여지가 많다. 아직 전체그림을 그리지 못해서 구조적으로도 허술한데 FAT이나 ext2 드라이버 구현하면서 대폭 개선될 듯.

직접블럭 7개 1/2/3차 간접 블럭 각 하나씩 총 10개의 데이터 블럭. 블럭 사이즈에 따라 파일의 최대 크기는 달라질 수 있지만, `WORD_SIZE`의 주소범위를 넘어서지는 못한다. inode의 사이즈 변수가 `int`형이기 때문이다. 즉, 32비트 시스템에서 파일의 사이즈는 4GB가 최대이다.

	블럭 사이즈(bytes) || 64      | 128       | 256        | 512
	-------------------||---------|-----------|------------|--------------
	파일 최대 크기     || 280,000 | 4,330,368 | 68,175,616 | 1,082,199,552

데이터 블럭 갯수를 늘리고 블럭 사이즈를 작게 유지하는 게 메모리 절약에 유리해보인다. 하지만 데이터 블럭의 갯수를 늘리거나 4차 5차까지 확장하는 건 코딩에 부담이 될 뿐더러 오버헤드가 커진다. 1k 미만 파일의 비중이 압도적인 것과 멀티미디어 파일까지 수용할 것을 고려한다면 직접블럭으로 1k까지 커버, 간접블럭 포함하여 수MB 단위를 지원하는 게 적당해보인다. 루트 파일시스템의 경우 `/dev` 용도정도 뿐, 멀티미디어 파일 같은 경우 ext나 fat등의 여타 파일시스템을 마운트 해서 사용하게 될 것이므로 디폴트는 페이지 사이즈로.

`FT_DEV` 타입일 경우 디바이스의 주번호와 부번호는 `data[0]`에 저장됨.

할당받은 고유한 물리 메모리 주소를 그대로 inode로 사용하는 것은 보안상 문제가 발생할 수 있음.

파일 시스템 구현을 앞두고 보름 넘게 손을 놓았다. 잘 몰라서 그런지 한없이 게을러지는. 이번주에는 마무리 합시다!!

----

램 상에 인터페이스만 구현해두고 디바이스 노드만 생성해서 사용한다는 대충의 생각으로 코딩을 시작했는데, 결과가 썩 만족스럽지 않다. 그래서 어설픈 위의 초기 구현을 대체할 파일 시스템을 구축하기로. 버퍼와 같은 최적화는 차후에 고려하도록 하고, 커널외의 플래시 공간 또는 EEPROM을 적절한 블럭크기의 파일 시스템을 구축한다.

플래시 메모리의 비영구적 쓰기 특성을 고려하지 않은 소형 시스템을 운영하기 위한 기본 파일 시스템으로 설계되었기 때문에 쓰기는 가급적 제한하도록 한다. 소형 시스템의 내부 플래시 메모리는 파일 시스템을 운용할만큼 용량이 크지도 않다. 그리고 내장 플래시 메모리에 쓰는 동안 플래시 메모리로 접근이 stall된다. 즉 그동안 플래시 메모리에 저장된 코드를 실행할 수 없으므로(중지되므로) 시스템 성능이 저하된다.

파일 시스템을 구축하기로 한 이상, 모든 디바이스 노드는 파일을 통해, 즉 inode 형으로 일반화한다.

	내부 플래시 메모리 맵:

	                         [0]      [1]      [2]                ......               [N-1]
	 --------------------------------------------------- ~ --------- ~ ------------ ~ ----------
	| .text | .data | .bss   | super  | inode  | data       | inode     | data   |  ~  | data   |
	|                        | block  | bitmap | bitmap     | table     | block  |  ~  | block  |
	|------------------------|-------------------------- ~ --------- ~ ------------ ~ ----------|
	|<---- kernel space ---->|<------------------------- file system -------------------------->|
	v
	0x00000000

	kernel space 와 super block 사이에는 블럭크기로 정렬하기 위한 padding이 삽입된다.

	block[0] = superblock
	block[1] = inode bitmap
	block[2] = data bitmap

inode는 디스크 전체 사이즈의 1% 를 취하도록 한다.

	The total number of inodes = disk_size * 1/100 / inode_size

inode 크기를 최소 64바이트라 가정할 때 최소 10개의 inode를 확보하기 위해서는 적어도 64KiB 이상의 디스크 용량이 필요하다. 최소한의 정보만 유지하더라도 inode 사이즈는 60바이트 전후가 최소다. 커널 자체 공간도 고려한다면 64KiB 이하는 무리다(하지만, SD 카드등 외부 디스크를 활용할 수 있을 것이다). 최소한의 디바이스 노드만 생성할 요량이라면 inode 비율을 늘려 32KiB 까지 어찌저찌 커버할 수 있을지도 모르겠다. 커널 크기가 있으므로 그 이하는 욕심내지 않기로.

메타 데이터 백업본을 유지하는 게 좋겠지만 차후 추가하는 걸로.

디렉토리 항목에서 레코드 길이를 유지함으로써 파일 삭제시 해당 공간을 재할당 가능하도록 한다(램 상에 그냥 인터페이스만 구현할 때는 왜 레코드 길이를 유지해야 하나 싶었는데).

구현에 앞서 포팅을 염두해 둔 시스템들의 롬 블럭 사이즈와 in application programming 의 가능여부를 확인해 둘 것.

super block 과 inode bitmap 크기는 항상 1블럭. 할당 가능한 inode와 block 갯수만 유지하고, 할당시 순차검색.

할당 가능한 최대 inode 갯수는 65536(2 bytes).

블럭 사이즈가 커질 경우, 버퍼로 사용되는 로컬 변수들 때문에 스택 오버플로우 발생 여지

### mount

파일 시스템 드라이버는 해당 파일 시스템 타입을 등록한다(`add_file_system()`). 등록된 파일 시스템 타입은 링크드 리스트로 모두 연결되어 있으며 name으로 구분된다.

마운트 시 해당 파일 시스템 타입을 찾아 수퍼블럭을 읽어들인다. `read_super()` 함수는 특정 파일시스템의 고유 수퍼블럭을 VFS에서 사용할 수 있도록 공통 수퍼블럭(struct superblock)형으로 변환해 제공한다.

마운트 포인트가 해당 파일 시스템의 진입점이 된다. 마운트된 모든 수퍼블럭은 링크드 리스트로 연결된다.

### `fs_init()`

디바이스 노드를 등록하기 위해서 devfs가 마운트 되어 있어야 한다. devfs는 ramfs으로 구현된다. `device_init()`에서 디바이스 노드를 등록할 수 있도록 `fs_init()`에서 devfs를 마운트한다.

### super block

	inode_size
	block_size
	free_inodes_count
	free_blocks_count
	inode_table
	data_block
	root_inode
	magic

### inode bitmap

항상 1블럭만 차지하므로 할당할 수 있는 최대 inode 갯수는 `block_size * 8`. inodes 전체 갯수 이상의 비트맵 영역은 사용중(1)으로 체크.

### data bitmap

	disk_size / block_size / 8

`block_size` 단위로 정렬. 남은 비트맵 영역은 사용중(1)으로 체크.

## embedfs

내장 플래시 파일 시스템(`/dev/efm`).

커널이 위치한 내장 플래시 메모리 공간은 /dev/efm. embedfs 사용할 공간은 /dev/efm1.

## VFS

### mount

file system 자료구조 필요.

	mount(pathname, fs_type);

	1. 블럭 디바이스
	2. 해당 블럭 디바이스 특정 파일시스템으로 마운트
	3. 수퍼블럭의 fop는 파일시스템의 op로 등록
	4. 파일 시스템의 디바이스 접근은 수퍼블럭의 dev 변수로
	open/read/write 류 함수는 file 타입을 매개변수로 받기 때문에 특정디바이스 연산은 file->dev->op로 접근가능

	fs_list_head

	1. 해당 수퍼블럭 찾음
	2. 수퍼블럭 fop ---> file->op

	e.g.
	1. ls dev
	2. find superblock
	3. sb->fop->read(pathname)
	4. find inode of pathname starting from sb->root_inode
	5. return data of inode

	1. open(pathname)
	2. find superblock
	3. sb->fop->read(pathname)
	4. find inode of pathname starting from sb->root_inode
	5. return data of inode

	1. read(fd)
	2. retrieve file object of fd from filetab

struct file

`filelist`에 열린 전체 파일을 관리. open 시 새로운 항목을 할당, index 리턴. close 시 삭제(참조하는 객체가 없는 경우).

`file_link()` 새로운 파일을 등록

`file_get()` 등록된 파일 얻기

`file_unlink()`

### super block

	dev
	pathname
	pathname_len
	count
	fop
	iop
	list
	magic
	lock

root inode 는 항상 0으로 가정.

파일 접근시 연결리스트에 등록된 수퍼블럭의 마운트 포인트(pathname)와 비교해 올바른 파일 시스템을 찾는다. 매칭 문자열 길이가 가장 긴 쪽이 해당 파일 시스템. 다음 단계부터는 파일 시스템 특정 루틴(iop) 사용.

### inode

	mode
	size
	count
	data
	parent
	sb
	lock

parent는 꼭 필요한 요소는 아니지만 연산 오버헤드를 줄일 수 있지 않을까 싶은 생각에..

	|<---- block 1 ---->|<---- block 2 ---->|
	 -------------------|-------------------
	|            | fore | rest |            |
	 -------------------|-------------------
	             |<-- entry -->|
	             v
	           offset

	fore_size = entry_size
	if offset + entry_size > block_size
	  fore_size = block_size - offset
	  rest_size = entry_size - fore_size

dir

	(bytes) |    2    |    2    |  1  |  1  |        4        |
	         -------------------------------------------------
	        |  inode  | rec_len | ty- | name| name string     |
		|         |         | pe  | len |                 |
	         -------------------------------------------------

	neme_len이 1바이트이므로 파일명은 255자가 최대.

## buffer cache

버퍼, 캐시와 같은 최적화 기술과 거듭 추상화 단계를 거친 자료구조가 코드에 대한 진입장벽을 높이기 때문에 최대한 그 수준을 낮게 유지하는 것이 하나의 목표였다. 하지만, 블럭 디바이스를 다루면서 최소한의 최적화는 불가피해보인다. 그렇지 않고서는 연산 오버헤드가 클 뿐더러 코드의 일관성을 유지하기 힘들다.

블럭 디바이스 같은 경우 커널로부터 버퍼를 할당받아 사용.

커널에서 버퍼를 관리. 드라이버에서 버퍼를 요청하면(`request_buffer()`) 인자로 넘어온 사이즈의 버퍼를 할당해서 `buf_t` 리턴.

최근에 사용된 버퍼는 head 바로 뒤에, 가장 오래된 버퍼는 끝에 위치하게 된다. 따라서 사용자가 요청한 블럭이 이미 버퍼로 읽혀졌는지 리스트를 순회할 때, 마지막 버퍼에 이르기까지 해당 블럭이 존재하지 않는다면 마지막 버퍼에 자료를 읽어들이면 된다. 마지막 버퍼가 사용된지 가장 오래된 버퍼이기 때문이다.

`buf_t *request_buffer(nr, BLOCK_SIZE)` - nr: 요청 버퍼 갯수. 리턴값이 NULL 이면 오류. 가용공간이 부족한 경우, 요청된 버퍼 갯수보다 적은 버퍼가 할당될 수 있음.

`void release_buffer(buf_t)`

`void *getblk(unsigned int n, struct device *dev)` - 블럭 디바이스 dev의 n번째 블럭을 읽어들임. 리턴되는 버퍼캐시는 유저 스페이스로 복사된 것이 아닌 공통 버퍼. ~~고려해야할 문제가 하나 있는 데 여러 태스크가 하나의 버퍼캐시에 접근하는 경우 버퍼의 내용을 사용자 공간으로 복사하는 와중에 해당 버퍼가 lru로 채택되어 다른 값이 써질 수 있다. 파일시스템 쪽에서 락을 걸면 간단히 해결되긴 하는데 버퍼캐시 내에서 어떻게 해결할 수 없나...~~ `getblk_lock()`사용 할 것.

`void *getblk_lock(unsigned int n, struct device *dev)` - `getbuf_lru()`에서 개별 버퍼락을 걸고 해당 버퍼 락을 해제하는 `putblk_unlock()` 함수를 대칭적으로 사용해야 함.

`void putblk_unlock(unsigned int nblock, struct device *dev)` - 해당 블럭 버퍼캐시 락 해제

!! sleep 가능한 뮤텍스를 사용하므로 시스템 콜을 선점할 방법을 강구해야 함. 혹은 커널 스레드로 업무를 넘기던가.

`void updateblk(unsigned int nblock, struct device *dev)` - 버퍼를 수정한 경우 사용. 더티 플래그를 셋함.


아래는 사용자에게 보이지 않는 함수들(deprecated).

`struct buffer_cache *getbuf(block_no, buf_t)` - 버퍼 리스트에서(`buf_t`) 해당 블럭이(`block_no`) 존재하는 버퍼를 리턴. ~~리턴 받은 버퍼와 블럭번호가 일치하지 않다면, 버퍼는 사용된지 가장 오래된 버퍼임. 고로 리턴된 버퍼에 새로운 블럭을 읽어오면 됨.~~ NULL 이면 오류.

`unsigned int getbuf_lru(buf_t, struct buffer_cache *)` - 인자로 넘어간 버퍼 변수에 사용한지 가장 오래된 버퍼를 얻음. 얻어진 버퍼는 더이상 유효하지 않음. 다른 곳에서 사용되지 않는다는 것이 보장되어야 함. 리턴값은 버퍼의 블럭넘버. 리턴값이 -1(0xffffffff) 이면 오류. (자료구조를 더블리 링크드 리스트로 변경하면 검색시간을 O(1)로 대폭 줄일 수 있지만, 검색연산 대비 자료구조의 크기 증가 및 사용되는 버퍼 갯수를 고려할 때 자료구조의 경량화가 소형 시스템에 더 이득을 주리라 생각됨). write back 할 시점으로 적당함. dirty를 추가할지 무조건 라이트할지..

주기적으로 sync해주지 않으면 자료유실 가능성.

`int putbuf(struct buffer_cache *buffer, block_no, buf_t)` - 버퍼에 할당된 블럭 넘버를 기입. lru 업데이트. 리턴값이 0이 아니면 오류.

`getblk()`

`putblk()`

### 블럭 디바이스 드라이버, 블럭 버퍼 사용하기

	blkbuf_t *buffer;

	if ((buffer = request_blkbuf(nr, block_size)) == NULL)
		error;

	char *data = blkbuf_get_lru(buffer);
	read(fd, data, block_size);
	blkbuf_put(buffer, data, block_no);

## fs superblock list

동일 파일 시스템은 하나의 드라이버를 공유하지만 개별적인 자료구조를 유지해야 한다. 활성화된 파일시스템의 각 수퍼블럭은 `fs_superblocks_list` 라는 커널 자료구조로 관리된다. 파일시스템마다 수퍼블럭이 다르므로 void 형 포인터를 사용.

`int register_superblock(void *sb, dev_t id)` - 새로운 수퍼블럭을 등록. 리턴값이 0이 아니면 오류

`void *get_superblock(dev_t id)` - 해당 디바이스로 등록된 파일시스템의 수퍼블럭을 구함. 리턴값이 NULL 이면 오류
