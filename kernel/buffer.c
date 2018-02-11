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

#include <kernel/buffer.h>
#include <kernel/page.h>
#include <error.h>

#define BUFFER_UNDEF		(unsigned int)-1
#define BUFFER_INITIAL		(unsigned int)-2

static struct buffer_cache *getbuf(unsigned int nblock, buf_t *head)
{
	struct links *curr;
	struct buffer_cache *p;

	if ((head == NULL) || (head->next == head))
		return NULL;

	for (curr = head->next; curr != head; curr = curr->next) {
		p = get_container_of(curr, struct buffer_cache, list);

		if (p->nblock == nblock)
			return p;
	}

	return NULL;
}

static void update_lru(struct buffer_cache *p, buf_t *head)
{
	links_del(&p->list);
	links_add(&p->list, head);
}

static unsigned int getbuf_lru(buf_t *head, struct buffer_cache **buffer_cache)
{
	unsigned int nblock;
	struct buffer_cache *p;
	struct device *dev;

	if ((head == NULL) || (head->prev == head))
		return BUFFER_UNDEF;

	dev = get_container_of(head, struct device, buffer);
	p   = get_container_of(head->prev, struct buffer_cache, list);

	/* to prevent corruption taking the same buffer as lru again
	 * before previous process like copying data to user space gets done.
	 * It must be unlocked by calling `putblk_unlock()`. */
	mutex_lock(&p->mutex);

	nblock = p->nblock;
	p->nblock = BUFFER_INITIAL; 
	update_lru(p, dev->buffer);
	*buffer_cache = p;

	return nblock;
}

void *getblk_lock(unsigned int nblock_new, struct device *dev)
{
	struct buffer_cache *p;

	mutex_lock(&dev->mutex);

	if ((p = getbuf(nblock_new, dev->buffer)) == NULL) {
		unsigned int nblock_old;

		if ((nblock_old = getbuf_lru(dev->buffer, &p)) == BUFFER_UNDEF) {
			mutex_unlock(&dev->mutex);
			return NULL;
		}

		nblock_old += dev->base_addr;

		if (p->dirty) { /* write back first */
			dev->op->write((struct file *)&nblock_old,
					p->buf, p->size);
		}

		p->dirty = 0;
		nblock_old = nblock_new + dev->base_addr;

		dev->op->read((struct file *)&nblock_old,
				p->buf, p->size);
	}

	update_lru(p, dev->buffer);
	p->nblock = nblock_new;

	mutex_unlock(&dev->mutex);

	return (void *)p->buf;
}

void *getblk(unsigned int nblock, struct device *dev)
{
	void *buf = getblk_lock(nblock, dev);
	putblk_unlock(nblock, dev);
	return buf;
}

void putblk_unlock(unsigned int nblock, struct device *dev)
{
	struct buffer_cache *buffer_cache;

	mutex_lock(&dev->mutex);

	if ((buffer_cache = getbuf(nblock, dev->buffer))) {
		/* TODO:
		 * If previously buffer taken, gebuf_lru(), which is called
		 * from getblk_lock(), doesn't need to get a buffer again.  So
		 * I put here the condition not to manipulate lock in case of
		 * it.
		 *
		 * But this kind of mechanism seems quite ugly. I want to get
		 * back to improve this later */
		if (is_locked(buffer_cache->mutex.counter))
			mutex_unlock(&buffer_cache->mutex);
	}

	mutex_unlock(&dev->mutex);
}

void updateblk(unsigned int nblock, struct device *dev)
{
	struct buffer_cache *buffer_cache;

	mutex_lock(&dev->mutex);

	if ((buffer_cache = getbuf(nblock, dev->buffer)))
		buffer_cache->dirty = 1;

	mutex_unlock(&dev->mutex);
}

static struct buffer_cache *mkbuf(unsigned short int block_size)
{
	struct buffer_cache *buffer_cache;

	if ((buffer_cache = kmalloc(sizeof(struct buffer_cache)))) {
		buffer_cache->nblock = BUFFER_INITIAL;
		buffer_cache->dirty = 0;
		buffer_cache->size = block_size;
		links_init(&buffer_cache->list);
		mutex_init(&buffer_cache->mutex);

		if ((buffer_cache->buf = kmalloc(block_size)) == NULL) {
			kfree(buffer_cache);
			buffer_cache = NULL;
		}
	}

	return buffer_cache;
}

int __sync(struct device *dev)
{
	unsigned int nblock;
	struct buffer_cache *p;
	struct links *curr;
	buf_t *head;

	mutex_lock(&dev->mutex);

	if (((head = dev->buffer) == NULL) || (head->next == head)) {
		mutex_unlock(&dev->mutex);
		return BUFFER_UNDEF;
	}

	for (curr = head->next; curr != head; curr = curr->next) {
		p = get_container_of(curr, struct buffer_cache, list);

		mutex_lock(&p->mutex);
		if (p->dirty && (p->nblock != BUFFER_INITIAL)) {
			nblock = p->nblock + dev->base_addr;
			dev->op->write((struct file *)&nblock, p->buf, p->size);
			p->dirty = 0;
		}
		mutex_unlock(&p->mutex);
	}

	mutex_unlock(&dev->mutex);

	return 0;
}

buf_t *request_buffer(unsigned short int n, unsigned short int block_size)
{
	struct buffer_cache *p;
	buf_t *head;

	if ((head = kmalloc(sizeof(buf_t))) == NULL)
		return NULL;

	links_init(head);

	while (n--) {
		if ((p = mkbuf(block_size)) == NULL) {
			if (head->next == head) {
				kfree(head);
				head = NULL;
			}
			break;
		}

		links_add(&p->list, head);
	}

	return head;
}

void release_buffer(buf_t *head)
{
	struct links *curr;
	struct buffer_cache *p;

	if (head == NULL)
		return;

	for (curr = head->next; curr != head; curr = curr->next) {
		links_del(curr);

		p = get_container_of(curr, struct buffer_cache, list);

		/* FIXME:
		 * no need to unlock.
		 * but what if a task came in waitqueue in the meantime?
		 * the task would be hung forever. */
		mutex_lock(&p->mutex);
		kfree(p->buf);
		kfree(p);
	}

	kfree(head);
}

int expand_buffer(buf_t *head, unsigned short int block_size)
{
	struct buffer_cache *p;

	if (head == NULL)
		return -BUFFER_UNDEF;

	if ((p = mkbuf(block_size)) == NULL)
		return -ENOMEM;

	links_add(&p->list, head->prev);

	return 0;
}
