#ifndef __FAIR_H__
#define __FAIR_H__

#include <kernel/sched.h>

extern void cfs_rq_add(struct scheduler *cfs, struct task *new);
extern void cfs_rq_del(struct scheduler *cfs, struct task *task);

extern struct task *cfs_pick_next(struct scheduler *cfs);

void cfs_init(struct scheduler *cfs);

#endif /* __FAIR_H__ */
