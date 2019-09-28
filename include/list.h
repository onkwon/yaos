#ifndef __YAOS_LLIST_H__
#define __YAOS_LLIST_H__

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>

/** doubly-linked list */
struct llist {
	struct llist *next, *prev;
};

#define INIT_LLIST_HEAD(name) 		{ &(name), &(name) }
/** Define a doubly-linked list initializing the links to itself */
#define DEFINE_LLIST_HEAD(name)		\
	struct llist name = INIT_LLIST_HEAD(name)

/** Initialize doubly-linked list pointing to itself */
static inline void llist_init(struct llist *node)
{
	node->next = node;
	node->prev = node;
}

/** Insert a node next to :c:data:`ref`
 *
 * @param new new entry to be added
 * @param ref a reference to add it after
 */
static inline void llist_add(struct llist *new, struct llist *ref)
{
	new->prev = ref;
	new->next = ref->next;
	ref->next->prev = new;
	ref->next = new;
}

/** Delete entry from list
 *
 * @param node the element to delete from the list
 */
static inline void llist_del(struct llist *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

/** tests whether a list is empty
 *
 * @param node the list to test
 * @return `true` if empty or `false`
 */
static inline bool llist_empty(const struct llist *node)
{
	return (node->next == node) && (node->prev == node);
}

/** singly-linked list */
struct list {
	struct list *next;
};

/** Define a singly-linked list initializing the link to NULL */
#define DEFINE_LIST_HEAD(name)		struct list name = { NULL }

/** Initialize singly-linked list pointing to NULL */
static inline void list_init(struct list *node)
{
	node->next = NULL;
}

/** Insert a node next to :c:data:`ref`
 *
 * @param new new entry to be added
 * @param ref a reference to add it after
 */
static inline void list_add(struct list *new, struct list *ref)
{
	new->next = ref->next;
	ref->next = new;
}

/** Insert an element at the end of the list
 *
 * @param new new entery to be added
 * @param head list head
 */
static inline void list_add_tail(struct list *new, struct list *head)
{
	struct list **curr = &head;

	while ((*curr) && (*curr)->next)
		curr = &(*curr)->next;

	new->next = (*curr)->next;
	(*curr)->next = new;
}

/** Delete entry from list
 *
 * @param node the element to delete from the list
 */
static inline int list_del(struct list *node, struct list *ref)
{
	struct list **curr = &ref;

	while (*curr && *curr != node)
		curr = &(*curr)->next;

	if (!*curr)
		return -ENOENT;

	*curr = node->next;

	return 0;
}

/** tests whether a list is empty
 *
 * @param node the list to test
 * @return `true` if empty or `false`
 */
static inline bool list_empty(const struct list *node)
{
	return node->next == NULL;
}

/** list queue head
 * queue implemented by singly linked list */
struct listq_head {
	struct list *next;
	struct list **last;
};

#define INIT_LISTQ_HEAD(name)		{ NULL, &((name).next)}
#define DEFINE_LISTQ_HEAD(name)		struct listq_head name = INIT_LISTQ_HEAD(name)

/* initialize list queue head */
static inline void listq_init(struct listq_head *head)
{
	head->next = NULL;
	head->last = &head->next;
}

/** push an entry into a list queue
 *
 * @param new entry to be added
 * @param head list queue head */
static inline void listq_push(struct list *new, struct listq_head *head)
{
	new->next = *head->last;
	assert(new->next == NULL);
	*head->last = new;
	head->last = &new;
}

/** pop an entry from a list queue
 *
 * @return an item or NULL if empty */
static inline struct list *listq_pop(struct listq_head *head)
{
	struct list *p = head->next;

	if (p) {
		head->next = p->next;

		if (p->next == NULL)
			head->last = &head->next;
	}

	return p;
}

static inline bool listq_empty(const struct listq_head *head)
{
	return head->next == NULL;
}

#endif /* __YAOS_LLIST_H__ */
