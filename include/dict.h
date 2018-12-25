#ifndef __YAOS_DICT_H__
#define __YAOS_DICT_H__

#include <stdint.h>
#include <errno.h>

struct dict_item {
	struct dict_item *next;
	void *key;
	void *value;
};

/** Dictionary data type */
typedef struct dict {
	uint16_t slot; /** Size of the pointer table */
	uint16_t n; /** Number of items stored */
	struct dict_item *table; /** Table */
} dict_t;

#define DEFINE_DICTIONARY_TABLE(name, slot)			\
	struct dict_item _dict_##name[slot] = { NULL, }
#define DEFINE_DICTIONARY(name, slot)				\
	struct dict name = { slot, 0, _dict_##name }

/**
 * Search the most recently inserted value associated with a key
 *
 * @param dict A pointer to dictionary
 * @param key key
 * @param value The value associated with the key
 * @return 0 on success or negative error code
 */
int idict_get(const dict_t * const dict, const uintptr_t key, void *value);
/**
 * Add a key-value pair to a dictionary
 *
 * @param dict A pointer to dictionary
 * @param key key
 * @param value The value associated with the key
 * @return 0 on success or negative error code
 */
int idict_add(dict_t * const dict, const uintptr_t key, const uintptr_t value);
/**
 * Delete a key-value pair in a dictionary
 *
 * @param dict A pointer to dictionary
 * @param key key to delete
 * @return 0 on success or negative error code
 */
int idict_del(dict_t * const dict, const uintptr_t key);
/**
 * Clear a dictionary
dict_clear();
 */
/**
 * Return a list of key-value pairs in a dictionary
dict_items();
 */
/**
 * Return a list of keys in a dictionary
dict_keys();
 */
/**
 * Return a list of values in a dictionary
dict_values();
 */

#endif /* __YAOS_DICT_H__ */
