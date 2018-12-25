#include "unity.h"
#include "dict.h"
#include <stdint.h>
#include <errno.h>

void setUp(void)
{
}

void tearDown(void)
{
}

#define SLOT		100

static DEFINE_DICTIONARY_TABLE(tbl, SLOT);
static DEFINE_DICTIONARY(tbl, SLOT);

void test_dict(void)
{
	uintptr_t val = 0, t;

	TEST_ASSERT_EQUAL(SLOT, tbl.slot);
	TEST_ASSERT_EQUAL(-ENOENT, idict_get(&tbl, 0, &val));

	for (int i = 0; i < SLOT; i++) {
		TEST_ASSERT_EQUAL(0, idict_add(&tbl, i, i * i));
		TEST_ASSERT_EQUAL(0, idict_get(&tbl, i, &t));
		TEST_ASSERT_EQUAL(i * i, t);
		TEST_ASSERT_EQUAL(i+1, tbl.n);
	}

	for (int i = 0; i < SLOT; i++) {
		TEST_ASSERT_EQUAL(-EEXIST, idict_add(&tbl, i, i + i));
		TEST_ASSERT_EQUAL(0, idict_get(&tbl, i, &t));
		TEST_ASSERT_EQUAL(i * i, t);
		TEST_ASSERT_EQUAL(SLOT, tbl.n);
	}

	TEST_ASSERT_EQUAL(-ENOSPC, idict_add(&tbl, SLOT, SLOT*SLOT));
	TEST_ASSERT_EQUAL(-ENODATA, idict_get(&tbl, SLOT, &t));

	for (int i = 0; i < SLOT; i++) {
		TEST_ASSERT_EQUAL(0, idict_get(&tbl, i, &t));
		TEST_ASSERT_EQUAL(i * i, t);
	}

	TEST_ASSERT_EQUAL(0, idict_del(&tbl, 4));
	TEST_ASSERT_EQUAL(-ENODATA, idict_del(&tbl, 4));
	TEST_ASSERT_EQUAL(-ENODATA, idict_get(&tbl, 4, &t));
	TEST_ASSERT_EQUAL(SLOT - 1, tbl.n);
	TEST_ASSERT_EQUAL(0, idict_add(&tbl, 4, 16));
	TEST_ASSERT_EQUAL(0, idict_get(&tbl, 4, &t));
	TEST_ASSERT_EQUAL(16, t);
	TEST_ASSERT_EQUAL(SLOT, tbl.n);

	for (int i = 0; i < SLOT; i++) {
		TEST_ASSERT_EQUAL(0, idict_get(&tbl, i, &t));
		TEST_ASSERT_EQUAL(i * i, t);
		TEST_ASSERT_EQUAL(0, idict_del(&tbl, i));
		TEST_ASSERT_EQUAL(SLOT - i - 1, tbl.n);
	}
}