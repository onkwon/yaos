#ifndef __YAOS_LLIST_H__
#define __YAOS_LLIST_H__

/* doubly-linked list */
struct llist {
	struct llist *next, *prev;
};

#define INIT_LINKS_HEAD(name) 		{ &(name), &(name) }
#define DEFINE_LINKS_HEAD(name)		\
	struct llist name = INIT_LINKS_HEAD(name)

static inline void llist_init(struct llist *node)
{
	node->next = node;
	node->prev = node;
}

static inline void llist_add(struct llist *new, struct llist *ref)
{
	new->prev = ref;
	new->next = ref->next;
	ref->next->prev = new;
	ref->next = new;
}

static inline void llist_del(struct llist *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

static inline bool llist_empty(const struct llist *node)
{
	return (node->next == node) && (node->prev == node);
}

/* singly-linked list */
struct list {
	struct list *next;
};

#define DEFINE_LINK_HEAD(name)		struct list name = { NULL }

static inline void list_init(struct list *node)
{
	node->next = NULL;
}

static inline void list_add(struct list *new, struct list *ref)
{
	new->next = ref->next;
	ref->next = new;
}

static inline void list_add_tail(struct list *new, struct list *head)
{
	struct list **curr = &head;

	while ((*curr) && (*curr)->next)
		curr = &(*curr)->next;

	new->next = (*curr)->next;
	(*curr)->next = new;
}

static inline void list_del(struct list *node, struct list *ref)
{
	struct list **curr = &ref;

	while (*curr && *curr != node)
		curr = &(*curr)->next;

	*curr = node->next;
}

static inline bool list_empty(const struct list *node)
{
	return node->next == NULL;
}

#endif /* __YAOS_LLIST_H__ */
