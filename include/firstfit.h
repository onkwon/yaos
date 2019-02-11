#ifndef __YAOS_FIRSTFIT_H__
#define __YAOS_FIRSTFIT_H__

/** A simple first fit memory allocator
 *
 * firstfit_free_addr() frees the memory block to the freelist in address
 * order as address order is known better when it comes to fragmentation than
 * recently-used order.
 *
 * Otherwise firstfit_free_mru() does so in recently-used order since the same
 * size allocation would likely occur in most of the time. It also seems
 * better for cache efficiency.
 *
 * It splits a node in two if the requested size is smaller than the size
 * found and the remainder size after split is bigger than MIN_SIZE. There is
 * unlikely such a request of that small size anyway.
 *
 * Figure 1. split P into C1 and C2:
 *
 * +---------------------------------------+
 * | meta | data space | meta | data space |
 * +---------------------------------------+
 * |      |            |      |<- C2size ->|
 * |      |            |<------- C2 ------>|
 * |      |<- C1size ->|                   |
 * |<------- C1 ------>|                   |
 * |      |<------------- P_size --------->|
 * |<----------------- P ----------------->|
 *
 * Figure 2. free list
 *
 * +---------------------+    +---------------------+
 * | size | *next | //// | -> | size | *next | //// |
 * + --------------------+    +---------------------+
 *
 * Figure 3. alloced block
 *
 * +-------------+
 * | size | //// |
 * +-------------+
 *        ^
 *        return address
 *
 */

#include <stdlib.h>

/** Add a memory block to the freelist
 *
 * @param head A pointer to a freelist
 * @param addr A pointer to a memory block
 * @param size Size of memory block to add
 * @return 0 on success
 */
int firstfit_init(void *head, void *addr, size_t size);
void *firstfit_alloc(void *head, size_t size);
void firstfit_free(void *head, void *addr);
void firstfit_coalesce(void *head);
size_t firstfit_left(void *head, size_t *largest, int *nr);
int firstfit_fragmentation(void *head);

#endif /* __YAOS_FIRSTFIT_H__ */
