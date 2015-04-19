#ifndef __CFS_H__
#define __CFS_H__

#include <kernel/sched.h>

void inline cfs_rq_add(struct sched_t *cfs, struct task_t *new);
void inline cfs_rq_del(struct sched_t *cfs, struct task_t *p);

struct task_t inline *cfs_pick_next(struct sched_t *cfs);

#endif /* __CFS_H__ */
