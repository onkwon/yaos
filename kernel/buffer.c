#include <kernel/buffer.h>
#include <kernel/page.h>
#include <error.h>

#define BUFFER_UNDEF		(unsigned int)-1
#define BUFFER_INITIAL		(unsigned int)-2

static struct buffer_cache *getbuf(unsigned int nblock, buf_t *head)
{
	struct list *curr;
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
	list_del(&p->list);
	list_add(&p->list, head);
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
	mutex_lock(p->lock);

	nblock = p->nblock;
	p->nblock = BUFFER_INITIAL; 
	update_lru(p, dev->buffer);
	*buffer_cache = p;

	return nblock;
}

void *getblk_lock(unsigned int nblock_new, const struct device *dev)
{
	struct buffer_cache *p;

	if ((p = getbuf(nblock_new, dev->buffer)) == NULL) {
		unsigned int nblock_old;

		if ((nblock_old = getbuf_lru(dev->buffer, &p)) == BUFFER_UNDEF)
			return NULL;

		nblock_old += dev->base_addr;

		if (p->dirty) { /* write back first */
			mutex_lock(dev->lock);
			dev->op->write((struct file *)&nblock_old,
					p->buf, dev->block_size);
			mutex_unlock(dev->lock);
		}

		p->dirty = 0;
		nblock_old = nblock_new + dev->base_addr;

		mutex_lock(dev->lock);
		dev->op->read((struct file *)&nblock_old,
				p->buf, dev->block_size);
		mutex_unlock(dev->lock);
	}

	update_lru(p, dev->buffer);
	p->nblock = nblock_new;

	return (void *)p->buf;
}

void *getblk(unsigned int nblock, const struct device *dev)
{
	void *buf = getblk_lock(nblock, dev);
	putblk_unlock(nblock, dev);
	return buf;
}

void putblk_unlock(unsigned int nblock, const struct device *dev)
{
	struct buffer_cache *buffer_cache;
	if ((buffer_cache = getbuf(nblock, dev->buffer)))
		mutex_unlock(buffer_cache->lock);
}

void updateblk(unsigned int nblock, const struct device *dev)
{
	struct buffer_cache *buffer_cache;
	if ((buffer_cache = getbuf(nblock, dev->buffer)))
		buffer_cache->dirty = 1;
}

static struct buffer_cache *mkbuf(unsigned short int block_size)
{
	struct buffer_cache *buffer_cache;

	if ((buffer_cache = kmalloc(sizeof(struct buffer_cache)))) {
		buffer_cache->nblock = BUFFER_INITIAL;
		buffer_cache->dirty = 0;
		list_link_init(&buffer_cache->list);
		INIT_MUTEX(buffer_cache->lock);

		if ((buffer_cache->buf = kmalloc(block_size)) == NULL) {
			kfree(buffer_cache);
			buffer_cache = NULL;
		}
	}

	return buffer_cache;
}

buf_t *request_buffer(unsigned short int n, unsigned short int block_size)
{
	struct buffer_cache *p;
	buf_t *head;

	if ((head = kmalloc(sizeof(buf_t))) == NULL)
		return NULL;

	list_link_init(head);

	while (n--) {
		if ((p = mkbuf(block_size)) == NULL) {
			if (head->next == head) {
				kfree(head);
				head = NULL;
			}
			break;
		}

		list_add(&p->list, head);
	}

	return head;
}

void release_buffer(buf_t *head)
{
	struct list *curr;
	struct buffer_cache *p;

	if (head == NULL)
		return;

	for (curr = head->next; curr != head; curr = curr->next) {
		list_del(curr);

		p = get_container_of(curr, struct buffer_cache, list);

		/* no need to unlock.
		 * but what if a task came in waitqueue in the meantime?
		 * the task would be hung forever. */
		mutex_lock(p->lock);
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
		return -ERR_ALLOC;

	list_add(&p->list, head->prev);

	return 0;
}
