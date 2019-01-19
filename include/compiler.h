#ifndef __YAOS_COMPILER_H__
#define __YAOS_COMPILER_H__

/** Compiler barrier */
#define barrier()			__asm__ __volatile__("" ::: "memory")
#define ACCESS_ONCE(val)		(*(volatile typeof(val) *)&(val))

#endif /* __YAOS_COMPILER_H__ */
