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

#include <string.h>
#include "hashmap.h"

#define FNV32_BASE  ((unsigned int) 0x811c9dc5)
#define FNV32_PRIME ((unsigned int) 0x01000193)

unsigned int strhash(const char *str)
{
	unsigned int c, hash = FNV32_BASE;
	while ((c = (unsigned char) *str++))
		hash = (hash * FNV32_PRIME) ^ c;
	return hash;
}

unsigned int memhash(const void *buf, size_t len)
{
	unsigned int hash = FNV32_BASE;
	unsigned char *ucbuf = (unsigned char *) buf;
	while (len--) {
		unsigned int c = *ucbuf++;
		hash = (hash * FNV32_PRIME) ^ c;
	}
	return hash;
}

#define HASHMAP_INITIAL_SIZE 32
#define HASHMAP_LOAD_FACTOR  80

static void alloc_entries(struct hashmap *self, int size)
{
  self->size = size;
  self->entries = kmalloc(size * sizeof(struct hashmap_entry *));
  /* calculate new thresholds */
  self->grow_at = size * HASHMAP_LOAD_FACTOR / 100;
  if (size <= HASHMAP_INITIAL_SIZE)
    self->shrink_at = 0;
  else
    self->shrink_at = self->grow_at / 5;
}

void hashmap_init(struct hashmap *self, hashmap_cmp_fn cmp_fn)
{
  memset(self, 0, sizeof(*self));
  self->cmpfn = cmp_fn;
  int size = HASHMAP_INITIAL_SIZE;
  alloc_entries(self, size);
}

void hashmap_free(struct hashmap *self, hashmap_free_fn free_fn, void *data)
{
  if (!self || !self->entries)
    return;

  if (free_fn) {
    HASHMAP_ITERATOR(iter, self);
    struct hashmap_entry *e;
    iter_for_each(&iter, e)
      free_fn(e, data);
  }

  kfree(self->entries);
  memset(self, 0, sizeof(*self));
}

static inline int bucket(struct hashmap *self, struct hashmap_entry *e)
{
  return e->hash & (self->size - 1);
}

static inline
int entry_equals(struct hashmap *self,
                 struct hashmap_entry *e1, struct hashmap_entry *e2)
{
  return (e1 == e2) || (e1->hash == e2->hash && !self->cmpfn(e1, e2));
}

static inline
struct hashmap_entry **find_entry(struct hashmap *self,
                                  struct hashmap_entry *key)
{
  assert(key->hash);
	struct hashmap_entry **e = &self->entries[bucket(self, key)];
	while (*e && !entry_equals(self, *e, key))
		e = &(*e)->next;
	return e;
}

void *hashmap_get(struct hashmap *self, void *key)
{
  return *find_entry(self, key);
}

static void rehash(struct hashmap *self, int newsize)
{
  int oldsize = self->size;
  struct hashmap_entry **oldentries = self->entries;

  alloc_entries(self, newsize);

  struct hashmap_entry *e;
  struct hashmap_entry *n;
  int b;
  for (int i = 0; i < oldsize; i++) {
    e = oldentries[i];
    while (e) {
      n = e->next;
      b = bucket(self, e);
      e->next = self->entries[b];
      self->entries[b] = e;
      e = n;
    }
  }

  kfree(oldentries);
}

int hashmap_add(struct hashmap *self, void *entry)
{
  if (*find_entry(self, entry))
    return -1;

  int b = bucket(self, entry);
  ((struct hashmap_entry *)entry)->next = self->entries[b];
  self->entries[b] = entry;
  self->count++;
  if (self->count > self->grow_at)
    rehash(self, self->size << 1);
  return 0;
}

void *hashmap_put(struct hashmap *self, void *entry)
{
  struct hashmap_entry *old = hashmap_remove(self, entry);
  hashmap_add(self, entry);
  return old;
}

void *hashmap_remove(struct hashmap *self, void *key)
{
  struct hashmap_entry **e = find_entry(self, key);
  if (!*e)
    return NULL;

  struct hashmap_entry *old;
  old = *e;
  *e = old->next;
  old->next = NULL;

  self->count--;
  if (self->count < self->shrink_at)
    rehash(self, self->size >> 1);

  return old;
}

void *hashmap_iter_next(struct iterator *iter)
{
  struct hashmap *map = iter->iterable;
  struct hashmap_entry *current = iter->item;
  for (;;) {
    if (current) {
      iter->item = current->next;
      return current;
    }

    if (iter->index >= map->size)
      return NULL;

    current = map->entries[iter->index++];
  }
}