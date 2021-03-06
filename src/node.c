/*
 MIT License

 Copyright (c) 2018 James, https://github.com/zhuguangxiang

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#include "hashmap.h"
#include "vector.h"
#include "atom.h"
#include "log.h"

struct node {
  HashMapEntry entry;
  /* node name */
  char *name;
  /* parent node */
  struct node *parent;
  /* leaf node */
  int leaf;
  /* leaf data or directory */
  void *data;
};

static HashMap nodetbl;
static struct node root;
static VECTOR(dir);

static int _node_cmp_cb_(void *k1, void *k2)
{
  struct node *n1 = k1;
  struct node *n2 = k2;
  if (n1->parent != n2->parent)
    return -1;
  if (n1->name == n2->name)
    return 0;
  return strcmp(n1->name, n2->name);
}

static void _node_free_cb_(void *entry, void *data)
{
  kfree(entry);
}

void node_init(void)
{
  hashmap_init(&nodetbl, _node_cmp_cb_);
  root.name = "/";
  root.parent = &root;
  root.data = &dir;
  hashmap_entry_init(&root, strhash(root.name));
}

void node_fini(void)
{
  hashmap_fini(&nodetbl, _node_free_cb_, NULL);
  vector_fini(&dir);
}

static inline struct node *find_node(char *name, struct node *parent)
{
  struct node key = {.name = name, .parent = parent};
  hashmap_entry_init(&key, strhash(name));
  return hashmap_get(&nodetbl, &key);
}

static struct node *__add_dirnode(char *name, struct node *parent)
{
  Vector *vec;
  struct node *node = kmalloc(sizeof(*node) + sizeof(*vec));
  hashmap_entry_init(node, strhash(name));
  node->name = name;
  node->parent = parent;
  vec = (Vector *)(node + 1);
  vector_init(vec);
  node->data = vec;
  hashmap_add(&nodetbl, node);
  return node;
}

static struct node *__add_leafnode(char *name, struct node *parent)
{
  struct node *node = kmalloc(sizeof(*node));
  hashmap_entry_init(node, strhash(name));
  node->name = name;
  node->parent = parent;
  node->leaf = 1;
  hashmap_add(&nodetbl, node);
  return node;
}

int add_leaf(char *pathes[], void *data)
{
  struct node *parent = &root;
  struct node *node;
  char **s;

  for (s = pathes; *(s + 1); s++) {
    node = find_node(*s, parent);
    if (!node) {
      /* new directory node */
      debug("new node '%s'.", *s);
      node = __add_dirnode(*s, parent);
    }
    parent = node;
  }

  /* new leaf data node */
  node = find_node(*s, parent);
  if (node) {
    debug("leaf '%s' exist.", *s);
    expect(node->leaf != NULL);
    return -1;
  } else {
    debug("new leaf '%s'.", *s);
    node = __add_leafnode(*s, parent);
    node->data = data;
    return 0;
  }
}

void *get_leaf(char *pathes[])
{
  struct node *parent = &root;
  struct node *node;
  char **s;

  for (s = pathes; *s; s++) {
    node = find_node(*s, parent);
    if (!node) {
      error("cannot find node '%s'.", *s);
      return NULL;
    }
    parent = node;
  }

  expect(node->leaf != NULL);
  return node->data;
}

char **path_toarr(char *path, int size)
{
  VECTOR(strs);
  char *end = path + size;
  char *p;
  char *s;
  int len;
  char **arr;

  while (*path && path < end) {
    p = path;
    while (*path && *path != '/' && path < end)
      path++;

    len = path - p;
    if (len > 0) {
      s = atom_nstring(p, len);
      vector_push_back(&strs, s);
    }

    /* remove trailing slashes */
    while (*path && *path == '/' && path < end)
      path++;
  }

  arr = vector_toarr(&strs);
  vector_fini(&strs);
  return arr;
}
