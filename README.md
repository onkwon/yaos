# YAOS

YAOS is Yet Another Operating System for Internet of Things(IoT) devices, specifically for a single-core processor without MMU virtualization. It is designed for energy efficiency and hardware independent development.

Refer to `/Documentation` directory for more information such as compiling, porting, APIs, etc.

Any feedback is welcome to *kwon@toanyone.net*. And let me know if any of you are interested in porting to new MCU so that I can give you a hand.

## Getting Started

### 1. Download YAOS

`git clone git://github.com/onkwon/yaos`

### 2. Get a Toolchain

Get one from [here](https://launchpad.net/gcc-arm-embedded) if you don't have one installed yet. Or you can compile it from source code putting more effort, which is not recommended but still worth trying.

### 3. Build

#### STM32

	make clean
	make stm32f1 *(or stm32f4)*
	make
	make burn

> Tested on STM32F103 and STM32F407

#### Raspberry Pi(2)

	make clean
	make rpi *(or rpi2)*
	make

That's it. Copy `yaos.bin` file into SD card as name of `kernel.img`, where `bootcode.bin` and `start.elf` files exist. Insert SD card in your RPI, turn it on, and enjoy!

> You can get the GPU firmware and bootloaders [here](https://github.com/raspberrypi/firmware).

You will see shell prompt `>` after some system log if uart rs232 cable connected.

Character LCD is also opened by default. You can change default pin assignment in `/mach/rpi/include/pinmap.h`. Pinout:

	          -----
	+3V3 ----|1   2|--\
	         |3   4|---- +5V0
	         |5   6|---- GND
	 DB7 ----|7   8|---- TXD0
	         |9  10|---- RXD0
	 DB6 ----|11 12|
	 DB5 ----|13 14|
	 DB4 ----|15 16|
	         |17 18|
	  E  ----|19 20|
	  RW ----|21 22|
	  RS ----|23 24|
	         |25 26| ---- IR
	          -----

## Features

### Task management and scheduling

Two types of task are handled: normal and real time tasks. Simplified fair scheduler for normal tasks while FIFO scheduler for real time tasks. Each task is given a priority which can be dynamically changed with `set_task_pri()`. For real time tasks a higher priority task always preempts lower priority tasks while the same priority tasks take place in turn under round-robin scheduling. Scheduler can be stopped to reduce even the scheduling overhead in case of a time critical task. On the other hand nomal tasks get chance to run by simplified fair scheduler, that picks the minimum value of vruntime up for the next task to run.

Tasks are always in one of five states: running, stopped, waiting, sleeping, or zombie. And a task can be created both statically and dynamically at run-time.

### System call interface

System resource is accessed by the system call interface entering privileged mode as a user task runs in user(unpriviliged) mode.

### Virtual file system

The concept of virtual file system(VFS) is implemented. The embedded flash rom in SoC can be mounted as the root file system(embedfs) while a ramfs is mounted as a devfs for a device node.

### Memory management

Page is unit of memory management but alternative memory manager can be used in such a system of memory shortage.

Buddy allocator and first-fit allocator are implemented.

### Deferred interrupt servicing (softirq)

Softirqs will preempt any work except the response to a real interrupt as they run at a high priority. In fact softirq is just a kernel task running with interrupts enabled and can sleep but has the highest priority amongst running tasks.

### Synchronization

Synchronization primitives such as semaphore, spinlock, etc.

### Blocking & non-blocking I/O operations

### Device driver
