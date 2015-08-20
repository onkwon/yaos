# YAOS

YAOS is Yet Another Operating System for Internet of Things(IoT) devices, specifically for a single-core processor without MMU virtualization. It is designed for energy efficiency and hardware independent development.

Refer to `/Documentation` directory for more information such as compiling, porting, APIs, etc.

Any feedback is welcome to kwon@toanyone.net. And let me know if any of you are interested in porting to new MCU so that I can give you a hand.

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

## Getting Started
