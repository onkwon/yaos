#ifndef __CFS_H__
#define __CFS_H__

#include <kernel/sched.h>

extern inline void cfs_rq_add(struct sched_t *cfs, struct task_t *new);
extern inline void cfs_rq_del(struct sched_t *cfs, struct task_t *p);

extern inline struct task_t *cfs_pick_next(struct sched_t *cfs);

#endif /* __CFS_H__ */
