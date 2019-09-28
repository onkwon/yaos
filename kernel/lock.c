#include "kernel/lock.h"
#include "kernel/syscall.h"
#include "kernel/task.h"

#include <assert.h>
#include <errno.h>

void lock_init(struct lock * const lock)
{
	listq_init(&lock->waitq_head);
	lock->ticket = lock->turn = 0;
}

/* NOTE: kernel should handle a kind of sem_id to check its validity
 * otherwise memory corruption would be followed since user may hand over
 * invalid memory address to kernel */
int sem_init(sem_t * const sem, int shared, uint16_t value)
{
	lock_init(sem);
	sem->counter = value;

	return 0;
	(void)shared;
}

int sem_wait(sem_t * const sem)
{
	int rc = -EFAULT;

	atomic_faah(&sem->counter, -1);

	if (sem->counter < 0) {
		rc = syscall(SYSCALL_WAIT, &sem->waitq_head, current);
	}

	return rc;
}

int sem_post(sem_t * const sem)
{
	int rc = -EFAULT;

	atomic_faah(&sem->counter, 1);

	if (/*sem->counter > 0 && */!listq_empty(&sem->waitq_head)) {
		rc = syscall(SYSCALL_WAKE, &sem->waitq_head);
	}

	return rc;
}

void mutex_init(struct lock * const lock)
{
	assert(sem_init(lock, 0, 1) == 0);
}

void mutex_lock(struct lock * const lock)
{
	sem_wait(lock);
}

void mutex_unlock(struct lock * const lock)
{
	sem_post(lock);
}

// TODO: implement mutex_lock_timeout() or sem_wait_timeout()
// int sem_wait_timeout(sem_t * const sem, unsigned long msec)
// void mutex_lock_timeout(lock_t * const lock, unsigned long msec)
