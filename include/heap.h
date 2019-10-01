#ifndef __HEAP_H__
#define __HEAP_H__

#include "kernel/init.h"
#include "firstfit.h"
#include <stdlib.h>

void *kmalloc(size_t size);
void kfree(void *ptr);
void __init heap_init(void);

void *__wrap_malloc(size_t size);
void __wrap_free(void *ptr);

#endif /* __HEAP_H__ */