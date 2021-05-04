/*********************************************************************************************************
 *
 *  Module Name:	list.h
 *
 *  Abstract:		Doubly linked list definition and implementation.
 *
 *  Author:			XZ
 *
 *  Revision History:	05.08.2003	started
 *
 *  Classes, methods and structures:
 *
 *  TODO:
 *
 *********************************************************************************************************/

#ifndef __LIST_H__
#define __LIST_H__

struct list_head { struct list_head *next, *prev; };

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

__inline__ void __list_add(struct list_head * new, struct list_head * prev, struct list_head * next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

__inline__ void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

__inline__ void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

__inline__ void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

__inline__ void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

__inline__ void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry); 
}

__inline__ int list_empty(struct list_head *head)
{
	return head->next == head;
}

__inline__ void list_splice(struct list_head *list, struct list_head *head)
{
	struct list_head *first = list->next;

	if (first != list) {
		struct list_head *last = list->prev;
		struct list_head *at = head->next;

		first->prev = head;
		head->next = first;

		last->next = at;
		at->prev = last;
	}
}

#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(size_t)(&((type *)0)->member)))

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)
        	
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#endif /* __LIST_H__ */
