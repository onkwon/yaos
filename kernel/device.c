#include <kernel/device.h>

#define TABLE_SIZE	13
#define CONSTANT	49157

static struct list_t devtab[TABLE_SIZE];

static inline unsigned hash(int key)
{
	return (unsigned)(key * CONSTANT) % TABLE_SIZE;
}

struct device_t *getdev(int id)
{
	struct device_t *dev;
	struct list_t *head, *next;

	head = &devtab[hash(id)];
	next = head->next;

	while (next != head) {
		dev = get_container_of(next, struct device_t, link);
		if (dev->id == id)
			return dev;

		next = next->next;
	}

	return NULL;
}

void link_device(int id, struct device_t *new)
{
	struct list_t *head;

	head = &devtab[hash(id)];
	list_add(&new->link, head);
}

void __devman_init()
{
	int i;

	for (i = 0; i < TABLE_SIZE; i++) {
		LIST_LINK_INIT(&devtab[i]);
	}
}

#ifdef CONFIG_DEBUG
#include <foundation.h>

void display_devtab()
{
	struct device_t *dev;
	struct list_t *head, *next;
	int i;

	for (i = 0; i < TABLE_SIZE; i++) {
		head = &devtab[i];
		next = head->next;

		printf("[%02d] ", i);
		while (next != head) {
			dev = get_container_of(next, struct device_t, link);
			printf("--> %d ", dev->id);

			next = next->next;
		}
		printf("\n");
	}
}
#endif
