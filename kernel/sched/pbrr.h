#ifndef __PBRR_H__
#define __PBRR_H__

#include "kernel/sched.h"

int pbrr_enqueue(struct scheduler *sched, void *new);
void *pbrr_dequeue(struct scheduler *sched);
void pbrr_init(struct scheduler *rt);

#endif /* __PBRR_H__ */
