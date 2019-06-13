/*
MIT License

Copyright (c) 2018 James, https://github.com/zhuguangxiang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _KOALA_LIST_H_
#define _KOALA_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * double linked list, from linux kernel include/linux/list.h
 * Some functions ("__xxx") are used internally.
 */

struct list_head {
	struct list_head *next, *prev;
};

/* Define a variable with the head and tail of the list. */
#define LIST_HEAD(name) \
	struct list_head name = { &(name), &(name) }

/* Initialize a new list head. */
#define INIT_LIST_HEAD(ptr) \
	(ptr)->next = (ptr)->prev = (ptr)

#define LIST_HEAD_INIT(name) { &(name), &(name) }

/* Add new element at the head of the list. */
static inline void list_add(struct list_head *newp, struct list_head *head)
{
	head->next->prev = newp;
	newp->next = head->next;
	newp->prev = head;
	head->next = newp;
}

/* Add new element at the tail of the list. */
static inline void list_add_tail(struct list_head *newp, struct list_head *head)
{
	head->prev->next = newp;
	newp->next = head;
	newp->prev = head->prev;
	head->prev = newp;
}

/* Remove element from list. */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/* Remove element from list, initializing the element's list pointers. */
static inline void list_del(struct list_head *elem)
{
	__list_del(elem->prev, elem->next);
	INIT_LIST_HEAD(elem);
}

/* Get typed element from list at a given position. */
#define list_entry(ptr, type, member) \
	((type *) ((char *) (ptr) - offsetof(type, member)))

/* Get first entry from a list. */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/* Iterate forward over the elements of the list. */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/*
 * Iterate forward over the elements list. The list elements can be
 * removed from the list while doing this.
 */
#define list_for_each_safe(pos, p, head)  \
	for (pos = (head)->next, p = pos->next; \
       pos != (head);                     \
       pos = p, p = pos->next)

/* Iterate backward over the elements of the list. */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/*
 * Iterate backwards over the elements list. The list elements can be
 * removed from the list while doing this.
 */
#define list_for_each_prev_safe(pos, p, head) \
	for (pos = (head)->prev, p = pos->prev;     \
       pos != (head);                         \
       pos = p, p = pos->prev)

static inline int list_empty(struct list_head *head)
{
	return head == head->next;
}

#ifdef __cplusplus
}
#endif
#endif /* _KOALA_LIST_H_ */