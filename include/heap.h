#ifndef __HEAP_H__
#define __HEAP_H__

#include "kernel/init.h"
#include <stdlib.h>

void *kmalloc(size_t size);
void kfree(void *ptr);
void __init heap_init(void);

#endif /* __HEAP_H__ */
