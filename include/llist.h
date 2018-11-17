#ifndef __YAOS_LLIST_H__
#define __YAOS_LLIST_H__

/* doubly-linked list */
struct llist2 {
	struct llist2 *next, *prev;
};

#define INIT_LINKS_HEAD(name) 		{ &(name), &(name) }
#define DEFINE_LINKS_HEAD(name)		\
	struct llist2 name = INIT_LINKS_HEAD(name)

static inline void llist2_init(struct llist2 *node)
{
	node->next = node;
	node->prev = node;
}

static inline void llist2_add(struct llist2 *new, struct llist2 *ref)
{
	new->prev = ref;
	new->next = ref->next;
	ref->next->prev = new;
	ref->next = new;
}

static inline void llist2_del(struct llist2 *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

static inline bool llist2_empty(const struct llist2 *node)
{
	return (node->next == node) && (node->prev == node);
}

/* singly-linked list */
struct llist {
	struct llist *next;
};

#define DEFINE_LINK_HEAD(name)		struct llist name = { NULL }

static inline void llist_init(struct llist *node)
{
	node->next = NULL;
}

static inline void llist_add(struct llist *new, struct llist *ref)
{
	new->next = ref->next;
	ref->next = new;
}

static inline void llist_add_tail(struct llist *new, struct llist *head)
{
	struct llist **curr = &head;

	while ((*curr) && (*curr)->next)
		curr = &(*curr)->next;

	new->next = (*curr)->next;
	(*curr)->next = new;
}

static inline void llist_del(struct llist *node, struct llist *ref)
{
	struct llist **curr = &ref;

	while (*curr && *curr != node)
		curr = &(*curr)->next;

	*curr = node->next;
}

static inline bool llist_empty(const struct llist *node)
{
	return node->next == NULL;
}

#endif /* __YAOS_LLIST_H__ */
