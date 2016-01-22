#ifndef __FAIR_H__
#define __FAIR_H__

#include <kernel/sched.h>

void cfs_rq_add(struct scheduler *cfs, struct task *new);
void cfs_rq_del(struct scheduler *cfs, struct task *task);

struct task *cfs_pick_next(struct scheduler *cfs);

void cfs_init(struct scheduler *cfs);

#endif /* __FAIR_H__ */
