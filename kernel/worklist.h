#ifndef __WORKLIST_H__
#define __WORKLIST_H__

struct link;

struct worklist {
	struct link list;
	void *data;
};

#define DEFINE_WORKLIST_HEAD(name)	DEFINE_LINK_HEAD(name)
#define worklist_empty(p)		link_empty(p)

static inline void worklist_init(struct worklist *work, void *data)
{
	work->data = data;
}

static inline void worklist_add(struct worklist *work, void *head)
{
	link_add_tail(&work->list, head);
}

static inline struct worklist *getwork(void *ref)
{
	struct worklist *work;
	struct link *head;

	head = (struct link *)ref;
	work = (struct worklist *)head->next;
	head->next = work->list.next;

	return work;
}

#endif /* __WORKLIST_H__ */
