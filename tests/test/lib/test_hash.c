#include "unity.h"
#include "hash.h"
#include <stdint.h>
#include <math.h>

void setUp(void)
{
}

void tearDown(void)
{
}

// 10			= 3
// 100			= 4
// 1,000		= 6
// 10,000		= 7
// 100,000 		= 8
// 1,000,000 		= 9
// 10,000,000 		= 10
// 100,000,000		= 10
// 1,000,000,000	= 5
#define TABLE_SIZE				10000000

static uint32_t tab[TABLE_SIZE];

void test_template(void)
{
	TEST_ASSERT(0 != hash(0));

	for (uint32_t i = 0; i < TABLE_SIZE; i++)
		tab[i] = 0;

	for (uint32_t i = 0; i < TABLE_SIZE; i++)
		tab[hash(i) % TABLE_SIZE]++;

	uint32_t worst = 0;
	uint32_t sum = 0;

	for (uint32_t i = 0; i < TABLE_SIZE; i++) {
		if (tab[i] > worst)
			worst = tab[i];
		sum += tab[i];

		//printf("%u : %u\n", i, tab[i]);
	}

	TEST_ASSERT(TABLE_SIZE == sum);
	TEST_ASSERT(((int)log10(TABLE_SIZE) + 3) >= worst);

	printf("worst %u sum %u\n", worst, sum);
}
