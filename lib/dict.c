//TODO: Implement lock-free
#include "dict.h"
#include "hash.h"

#include <stdbool.h>
#include <stddef.h>

#define OPEN_ADDRESSING
#define hash_key(key, slot)			(hash(key) % (slot))

static inline struct dict_item *get_item_next(const struct dict_item * const item)
{
	return (struct dict_item *)((uintptr_t)item->next & ~1UL);
}

static inline bool is_item_empty(const struct dict_item * const item)
{
	return !(uintptr_t)(item->next);
}

static inline void mark_item(struct dict_item * const item)
{
	item->next = (void *)1UL;
}

static inline void clear_item(struct dict_item * const item)
{
	item->next = (void *)0UL;
}

#if defined(OPEN_ADDRESSING)
int idict_get(const dict_t * const dict, const uintptr_t key, void *value)
{
	const struct dict_item *item;
	uintptr_t *p = value;
	uint16_t i;

	if (!dict || !p)
		return -EINVAL;

	if (!dict->table || !dict->n)
		return -ENOENT;

	item = &dict->table[hash_key(key, dict->slot)];

	for (i = 0; i < dict->slot; i++) {
#if defined(NO_DELETION)
		if (is_item_empty(item)) {
			break;
		} else
#endif
		if ((key == (uintptr_t)item->key)
				&& !is_item_empty(item)) {
			*p = (uintptr_t)item->value;
			return 0;
		}

		item = &dict->table[(hash_key(key, dict->slot) + i) % dict->slot];
	}

	return -ENODATA;
}

int idict_add(dict_t * const dict, const uintptr_t key, const uintptr_t value)
{
	struct dict_item *item;
	uint16_t i;

	if (!dict || !dict->table)
		return -EINVAL;

	item = &dict->table[hash_key(key, dict->slot)];

	for (i = 0; i < dict->slot; i++) {
		if (is_item_empty(item)) {
			item->key = (void *)key;
			item->value = (void *)value;
			mark_item(item);
			dict->n++;
			return 0;
		} else if (key == (uintptr_t)item->key) {
			return -EEXIST;
		}

		item = &dict->table[(hash_key(key, dict->slot) + i) % dict->slot];
	}

	return -ENOSPC;
}

int idict_del(dict_t * const dict, const uintptr_t key)
{
	struct dict_item *item;
	uint16_t i;

	if (!dict || !dict->table)
		return -EINVAL;

	item = &dict->table[hash_key(key, dict->slot)];

	for (i = 0; i < dict->slot; i++) {
		if ((key == (uintptr_t)item->key)
				&& !is_item_empty(item)) {
			item->key = NULL;
			item->value = NULL;
			clear_item(item);
			dict->n--;
			return 0;
		}

		item = &dict->table[(hash_key(key, dict->slot) + i) % dict->slot];
	}

	return -ENODATA;
}

#else // Chaining
int idict_get(const dict_t * const dict, const uintptr_t key, void *value)
{
	const struct dict_item *item;
	uintptr_t *p = value;

	if (!dict || !p)
		return -EINVAL;

	if (!dict->table || !dict->n)
		return -ENOENT;

	for (item = &dict->table[hash_key(key, dict->slot)];
			item; item = get_item_next(item)) {
		if (!is_item_empty(item)
				&& (key == (uintptr_t)item->key)) {
			*p = (uintptr_t)item->value;
			return 0;
		}
	}

	return -ENODATA;
}

int idict_add(dict_t * const dict, const uintptr_t key, const uintptr_t value)
{
	struct dict_item *head, *item;

	if (!dict || !dict->table)
		return -EINVAL;

	head = &dict->table[hash_key(key, dict->slot)];

	for (item = head; item; item = get_item_next(item)) {
		if (is_item_empty(item)) {
			item->key = (void *)key;
			item->value = (void *)value;
			mark_item(item);
			break;
		}

		if (key == (uintptr_t)item->key)
			return -EEXIST;
	}

	if (!item) {
#if 0
		item = malloc(sizeof(*item));
		item->key = (void *)key;
		item->value = (void *)value;
		item->next = head->next;
		head->next = item;
		mark_item(item);
#else
		syslog("malloc\n");
#endif
	}

	dict->n++;

	return 0;
}
#endif
