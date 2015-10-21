#ifndef __RT_H__
#define __RT_H__

#include <kernel/sched.h>

void rts_rq_add(struct scheduler *q, struct task *new);
void rts_rq_del(struct scheduler *q, struct task *p);

struct task *rts_pick_next(struct scheduler *q);

void rts_init(struct scheduler *rts);

#endif /* __RT_H__ */
