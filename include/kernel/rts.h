#ifndef __RTS_H__
#define __RTS_H__

#include <kernel/sched.h>

void inline rts_rq_add(struct sched_t *q, struct task_t *new);
void inline rts_rq_del(struct sched_t *q, struct task_t *p);

struct task_t inline *rts_pick_next(struct sched_t *q);

#endif /* __CFS_H__ */
