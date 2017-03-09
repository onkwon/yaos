#ifndef __WORKLIST_H__
#define __WORKLIST_H__

struct link;
struct task;
struct file;

struct worklist {
	struct link list;
	struct task *task;
	struct file *file;
	void *data;
	size_t size;
};

#define DEFINE_WORKLIST_HEAD(name)	DEFINE_LINK_HEAD(name)
#define worklist_empty(p)		link_empty(p)

static inline void worklist_init(struct worklist *work, struct task *task,
		struct file *file, void *data, size_t size)
{
	work->task = task;
	work->file = file;
	work->data = data;
	work->size = size;
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
