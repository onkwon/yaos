#ifndef __RTS_H__
#define __RTS_H__

#include <kernel/sched.h>

inline void rts_rq_add(struct sched_t *q, struct task_t *new);
inline void rts_rq_del(struct sched_t *q, struct task_t *p);

inline struct task_t *rts_pick_next(struct sched_t *q);

#endif /* __CFS_H__ */
