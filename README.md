# yaos

yaos is an embedded operating system for Internet of Things(IoT) devices, specifically for a single-core processor without MMU virtualization. It is designed for energy efficiency and hardware independent development.

Refer to `/Documentation` directory for more information such as compiling, porting, APIs, etc.

Any feedback is welcome to *kwon@toanyone.net*. And let me know if any of you are interested in porting to new MCU so that I can give you a hand.

## Getting Started

### 1. Download yaos

`git clone git://github.com/onkwon/yaos`

### 2. Get a Toolchain

Get one from [here](https://launchpad.net/gcc-arm-embedded) if you don't have one installed yet. Or you can compile it from source code putting more effort, which is not recommended but still worth trying.

### 3. Build

#### STM32

	make clean
	make stm32f1 (or specify your board. e.g. mango-z1)
	make
	make burn

> Supported boards at the moment are :
>  * [stm32-lcd](https://www.olimex.com/Products/ARM/ST/STM32-LCD/)
>  * [mango-z1](http://www.mangoboard.com/main/?cate1=9&cate2=26&cate3=36)
>  * [mycortex-stm32f4](http://www.withrobot.com/mycortex-stm32f4/)
>  * [ust-mpb-stm32f103](https://www.devicemart.co.kr/1089642)

In case of getting error messages something like `undefined reference to __aeabi_uidiv`, specify library path when you `make` in the way below:

	make LD_LIBRARY_PATH=/usr/local/arm/lib/gcc/arm-none-eabi/4.9.2

> The path is dependent on your development environment.

## User task example

Let me put an example of blinking a LED for you to take a taste of how the code look like.

User tasks would be placed under /tasks(e.g. tasks/my-first-task.c):

	void main()
	{
		int fd, led = 0;

		if ((fd = open("/dev/gpio20", O_WRONLY)) <= 0) {
			printf("can not open, %x\n", fd);
			return;
		}

		while (1) {
			write(fd, &led, 1);
			led ^= 1;
			sleep(1);
		}

		close(fd);
	}
	REGISTER_TASK(main, 0, DEFAULT_PRIORITY, STACK_SIZE_DEFAULT);

## Features

### Task management and scheduling

Two types of task are handled: normal and real time tasks. Round-robin scheduler for normal tasks while priority scheduler for real time tasks. Each task is given a priority which can be dynamically changed with `set_task_pri()`. For real time tasks a higher priority task always preempts lower priority tasks while the same priority tasks take place in turn under round-robin scheduling. Scheduler can be stopped to reduce even the scheduling overhead in case of a time critical task. On the other hand normal tasks get chance to run by simplified fair scheduler, that picks the minimum value of runtime up for the next task to run.

Tasks are always in one of five states: running, stopped, waiting, sleeping, or zombie. And a task can be created both statically and dynamically at run-time.

### System call interface

System resource is accessed by the system call interface entering privileged mode as a user task runs in user(unprivileged) mode.

### Virtual file system

The concept of virtual file system(VFS) is implemented. The embedded flash rom in SoC can be mounted as the root file system(embedfs) while a ramfs is mounted as a devfs for a device node.

Empty flash memory is registerd as embedfs so that user can use it just like normal file system.

### Memory management

Page is unit of memory management but alternative memory manager can be used in such a system of memory shortage.

Buddy allocator and first-fit allocator are implemented.

### Deferred interrupt servicing (softirq)

Softirqs will preempt any work except the response to a real interrupt as they run at a high priority. In fact softirq is just a kernel task running with interrupts enabled and can sleep but has the highest priority amongst running tasks.

### Synchronization

Synchronization primitives such as semaphore, spinlock, etc.

### Blocking & non-blocking I/O operations

### Device driver
