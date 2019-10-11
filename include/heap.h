#ifndef __HEAP_H__
#define __HEAP_H__

#include "kernel/init.h"
#include "firstfit.h"
#include <stdlib.h>

void *kmalloc(size_t size);
void kfree(void *ptr);
size_t kmem_left(void);
void __init heap_init(void);
void __init bootmem_free(void);

void *__wrap_malloc(size_t size);
void __wrap_free(void *ptr);
void free_to(void *ptr, void *task);

#endif /* __HEAP_H__ */
