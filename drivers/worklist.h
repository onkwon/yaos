/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

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
