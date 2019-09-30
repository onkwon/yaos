#include "unity.h"
#include "firstfit.h"
#include "list.h"
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#define HEAPSIZE		(256UL * 1024)
#define MIN_SIZE		16UL
#define ALIGNMENT		sizeof(uintptr_t)
#define TMPGAP			2

static char _heap[HEAPSIZE];

void setUp(void)
{
}

void tearDown(void)
{
}

void test_firstfit_init(void)
{
	DEFINE_LIST_HEAD(freelist);

	TEST_ASSERT_EQUAL(-EINVAL, firstfit_init(&freelist, NULL, HEAPSIZE));
	TEST_ASSERT_EQUAL(-EINVAL, firstfit_init(&freelist, NULL, 0));
	TEST_ASSERT_EQUAL(-EINVAL, firstfit_init(&freelist, _heap, 0));
	TEST_ASSERT_EQUAL(-EINVAL, firstfit_init(NULL, _heap, 0));
	TEST_ASSERT_EQUAL(-EINVAL, firstfit_init(NULL, NULL, HEAPSIZE));
	TEST_ASSERT_EQUAL(-EINVAL, firstfit_init(NULL, _heap, HEAPSIZE));

	for (size_t i = 1; i < MIN_SIZE; i++)
		TEST_ASSERT_EQUAL(-ENOSPC, firstfit_init(&freelist, _heap, i));

	TEST_ASSERT_EQUAL(0, firstfit_init(&freelist, _heap, MIN_SIZE+16));
	TEST_ASSERT_EQUAL(&_heap[ALIGNMENT], freelist.next);
	TEST_ASSERT_EQUAL(NULL, freelist.next->next);

	// must sizeof(struct freelist) <= ALIGNMENT * TMPGAP
	TEST_ASSERT_EQUAL(0, firstfit_init(&freelist, &_heap[ALIGNMENT*TMPGAP-1], HEAPSIZE-ALIGNMENT*TMPGAP-1));
	TEST_ASSERT_EQUAL(&_heap[ALIGNMENT*TMPGAP+ALIGNMENT], freelist.next);
	TEST_ASSERT_EQUAL(NULL, freelist.next->next);
}

void test_firstfit_alloc(void)
{
	DEFINE_LIST_HEAD(freelist);
	char *p;

	TEST_ASSERT_EQUAL(0, firstfit_init(&freelist, _heap, HEAPSIZE));
	TEST_ASSERT_EQUAL(&_heap[ALIGNMENT], freelist.next);
	TEST_ASSERT_EQUAL(NULL, freelist.next->next);

	TEST_ASSERT_EQUAL(NULL, firstfit_alloc(NULL, MIN_SIZE-1));

	for (int i = 0; i <= MIN_SIZE; i++) {
		TEST_ASSERT(p = firstfit_alloc(&freelist, i));
		TEST_ASSERT_EQUAL(MIN_SIZE, *(size_t *)(p - ALIGNMENT));
	}

	for (int i = 1; i; i++) {
		if (!(p = firstfit_alloc(&freelist, MIN_SIZE * i))) {
			// corner case of no block is in the freelist
			size_t largest;
			firstfit_left(&freelist, &largest, 0);
			if (largest)
				p = firstfit_alloc(&freelist, largest);
			TEST_ASSERT_EQUAL(NULL, firstfit_alloc(&freelist, MIN_SIZE));
			if (p)
				firstfit_free(&freelist, p);
			break;
		}
		TEST_ASSERT((MIN_SIZE * i) == *(size_t *)(p - ALIGNMENT));
	}
}

void test_firstfit_free(void)
{
	DEFINE_LIST_HEAD(freelist);
	char *p;

	TEST_ASSERT_EQUAL(0, firstfit_init(&freelist, _heap, HEAPSIZE));
	TEST_ASSERT_EQUAL(&_heap[ALIGNMENT], freelist.next);
	TEST_ASSERT_EQUAL(NULL, freelist.next->next);

	size_t largest;
	printf("left %zu, largest free block %zu, fragmentation %d\n",
			firstfit_left(&freelist, &largest, 0), largest,
			firstfit_fragmentation(&freelist));

	TEST_ASSERT(p = firstfit_alloc(&freelist, MIN_SIZE));
	TEST_ASSERT_EQUAL(MIN_SIZE, *(size_t *)(p - ALIGNMENT));
	TEST_ASSERT_EQUAL(&_heap[ALIGNMENT], freelist.next);
	firstfit_free(&freelist, p);
	TEST_ASSERT(p = firstfit_alloc(&freelist, MIN_SIZE));
	TEST_ASSERT_EQUAL(MIN_SIZE, *(size_t *)(p - ALIGNMENT));
	TEST_ASSERT_EQUAL(&_heap[ALIGNMENT], freelist.next);
	firstfit_free(&freelist, p);
	TEST_ASSERT(p = firstfit_alloc(&freelist, MIN_SIZE-1));
	TEST_ASSERT_EQUAL(MIN_SIZE, *(size_t *)(p - ALIGNMENT));
	TEST_ASSERT_EQUAL(&_heap[ALIGNMENT], freelist.next);
	firstfit_free(&freelist, p);

	printf("left %zu, largest free block %zu, fragmentation %d\n",
			firstfit_left(&freelist, &largest, 0), largest,
			firstfit_fragmentation(&freelist));
}

void test_firstfit_alloc_free(void)
{
	DEFINE_LIST_HEAD(freelist);
	char *p15, *p16, *p30, *p35, *p64, *p200, *p1024, *p2000, *p4096, *p128000;
	size_t largest;
	int n;

	TEST_ASSERT_EQUAL(0, firstfit_init(&freelist, _heap, HEAPSIZE));
	TEST_ASSERT_EQUAL(&_heap[ALIGNMENT], freelist.next);
	TEST_ASSERT_EQUAL(NULL, freelist.next->next);

	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(1 == n);

	TEST_ASSERT(p16 = firstfit_alloc(&freelist, 16));
	TEST_ASSERT(p128000 = firstfit_alloc(&freelist, 128000));
	TEST_ASSERT(p30 = firstfit_alloc(&freelist, 30));
	TEST_ASSERT(p64 = firstfit_alloc(&freelist, 64));
	TEST_ASSERT(p200 = firstfit_alloc(&freelist, 200));
	TEST_ASSERT(p1024 = firstfit_alloc(&freelist, 1024));
	TEST_ASSERT(p2000 = firstfit_alloc(&freelist, 2000));
	TEST_ASSERT(p4096 = firstfit_alloc(&freelist, 4096));
	TEST_ASSERT(p15 = firstfit_alloc(&freelist, 15));
	TEST_ASSERT(p35 = firstfit_alloc(&freelist, 35));

	printf("left %zu, largest free block %zu, fragmentation %d\n",
			firstfit_left(&freelist, &largest, NULL), largest,
			firstfit_fragmentation(&freelist));

	firstfit_free(&freelist, p16);
	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(2 == n);
	firstfit_free(&freelist, p30);
	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(3 == n);
	firstfit_free(&freelist, p200);
	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(4 == n);
	firstfit_free(&freelist, p2000);
	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(5 == n);
	firstfit_free(&freelist, p15);
	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(6 == n);
	firstfit_free(&freelist, p128000);
	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(5 == n);
	printf("left %zu, largest free block %zu, fragmentation %d\n",
			firstfit_left(&freelist, &largest, &n), largest,
			firstfit_fragmentation(&freelist));
	firstfit_free(&freelist, p35);
	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(4 == n);
	firstfit_free(&freelist, p4096);
	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(3 == n);
	firstfit_free(&freelist, p1024);
	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(2 == n);
	firstfit_free(&freelist, p64);
	firstfit_left(&freelist, &largest, &n);
	TEST_ASSERT(1 == n);

	printf("left %zu, largest free block %zu, fragmentation %d\n",
			firstfit_left(&freelist, &largest, &n), largest,
			firstfit_fragmentation(&freelist));
}

void test_firstfit_alloc_overlap(void)
{
#define N		1000
#define MAXSIZE		10000
	struct {
		void *addr;
		size_t size;
	} p[N];

	size_t largest;
	int i, n;
	DEFINE_LIST_HEAD(freelist);

	TEST_ASSERT_EQUAL(0, firstfit_init(&freelist, _heap, HEAPSIZE));
	TEST_ASSERT_EQUAL(&_heap[ALIGNMENT], freelist.next);
	TEST_ASSERT_EQUAL(NULL, freelist.next->next);

	srand(time(NULL));

	for (i = 0; i < N; i++) {
		size_t size = rand() % MAXSIZE;
		if (!(p[i].addr = firstfit_alloc(&freelist, size)))
			break;
		p[i].size = size;
	}

	// make sure no overlap one another
	for (int j = 0; j < i; j++) {
		uintptr_t s1, e1;
		s1 = (uintptr_t)p[j].addr;
		e1 = s1 + p[j].size;
		for (int k = j + 1; k < i; k++) {
			uintptr_t s2, e2;
			s2 = (uintptr_t)p[k].addr;
			e2 = s2 + p[k].size;

			if (s1 < s2) {
				TEST_ASSERT(e1 < s2 && s2 < e2);
			} else {
				TEST_ASSERT(e2 < s1 && s1 < e1);
			}
		}
	}

	printf("left %zu, largest free block %zu, nr %d, fragmentation %d\n",
			firstfit_left(&freelist, &largest, &n), largest, n,
			firstfit_fragmentation(&freelist));

	for (int j = 0; j < N; j++) {
		int idx = rand() % i;
		size_t size = rand() % (MAXSIZE / 10);
		if (size < MIN_SIZE) size = MIN_SIZE;
		firstfit_free(&freelist, p[idx].addr);
		TEST_ASSERT(p[idx].addr = firstfit_alloc(&freelist, size));
		p[idx].size = size;
	}

	for (int j = 0; j < i; j++) {
		uintptr_t s1, e1;
		s1 = (uintptr_t)p[j].addr;
		e1 = s1 + p[j].size;
		for (int k = j + 1; k < i; k++) {
			uintptr_t s2, e2;
			s2 = (uintptr_t)p[k].addr;
			e2 = s2 + p[k].size;

			if (s1 < s2) {
				if (e1 < s2 && s2 < e2);
				else printf("s1 %lx - %lx\ns2 %lx - %lx\n", s1, e1, s2, e2);
				TEST_ASSERT(e1 < s2 && s2 < e2);
			} else {
				if (e2 < s1 && s1 < e1);
				else printf("s1 %lx - %lx\ns2 %lx - %lx\n", s1, e1, s2, e2);
				TEST_ASSERT(e2 < s1 && s1 < e1);
			}
		}
	}

	printf("left %zu, largest free block %zu, nr %d, fragmentation %d\n",
			firstfit_left(&freelist, &largest, &n), largest, n,
			firstfit_fragmentation(&freelist));
}

void test_firstfit_coalesce(void)
{
	TEST_IGNORE();
}
