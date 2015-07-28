#ifndef __RT_H__
#define __RT_H__

#include <kernel/sched.h>

inline void rts_rq_add(struct scheduler *q, struct task *new);
inline void rts_rq_del(struct scheduler *q, struct task *p);

inline struct task *rts_pick_next(struct scheduler *q);

void rts_init(struct scheduler *rts);

#endif /* __RT_H__ */
