/*
 * Copyright (c) 2018 James, https://github.com/zhuguangxiang
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _KOALA_LIST_H_
#define _KOALA_LIST_H_

#include "common.h"

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

#define LIST_HEAD_INIT(name) {&(name), &(name)}
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

static inline
void init_list_head(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

#define list_unlinked(node) \
	((node)->next == (node) && (node)->prev == (node))

/* Insert a new entry between two known consecutive entries. */
static inline
void __list_add(struct list_head *entry,
								struct list_head *prev, struct list_head *next)
{
	entry->next = next;
	entry->prev = prev;
	prev->next = entry;
	next->prev = entry;
}

/* Add a new entry at head */
static inline
void list_add(struct list_head *entry, struct list_head *head)
{
	__list_add(entry, head, head->next);
}


/* Add a new entry to tail */
static inline
void list_add_tail(struct list_head *entry, struct list_head *head)
{
	__list_add(entry, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline
void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/* Deletes entry from list and reinitializes it. */
static inline
void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	init_list_head(entry);
}

/* Tests whether a list is empty */
#define list_empty(head) ((head)->next == head)

/* Gets the first element from a list */
#define list_first(head) (list_empty(head) ? NULL : (head)->next)

/* Gets the last element from a list */
#define list_last(head) (list_empty(head) ? NULL : (head)->prev)

/* iterate over a list */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/* iterate over a list backwards */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/* iterate over a list safe against removal of list entry */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; \
			 pos != (head); \
			 pos = n, n = pos->next)

/* iterate over a list backwards safe against removal of list entry */
#define list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
			 pos != (head); \
			 pos = n, n = pos->prev)

/*
 * Double linked lists with a single pointer of list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */
struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define HLIST_HEAD_INIT {.first = NULL}
#define HLIST_HEAD(name) struct hlist_head name = {.first = NULL}
#define init_hlist_head(ptr) ((ptr)->first = NULL)
static inline
void init_hlist_node(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline
int hlist_unhashed(const struct hlist_node *node)
{
	return !node->pprev;
}

static inline
int hlist_empty(const struct hlist_head *head)
{
	return !head->first;
}

static inline
void hlist_del(struct hlist_node *n)
{
	if (!hlist_unhashed(n)) {
		struct hlist_node *next = n->next;
		struct hlist_node **pprev = n->pprev;
		*pprev = next;
		if (next)
			next->pprev = pprev;
		init_hlist_node(n);
	}
}

static inline
void hlist_add(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({n = pos->next; 1;}); pos = n)

#ifdef __cplusplus
}
#endif
#endif /* _KOALA_LIST_H_ */
