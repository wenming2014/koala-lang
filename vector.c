
#include "vector.h"
#include "common.h"
#include "log.h"

#define MIN_ALLOCATED 8

Vector *Vector_New(void)
{
  Vector *vec = malloc(sizeof(Vector));
  Vector_Init(vec);
  return vec;
}

void Vector_Free(Vector *vec, itemfunc fn, void *arg)
{
  if (!vec) return;
  Vector_Fini(vec, fn, arg);
  free(vec);
}

int Vector_Init(Vector *vec)
{
  vec->allocated = 0;
  vec->size = 0;
  vec->items = NULL;
  return 0;
}

void Vector_Fini(Vector *vec, itemfunc fn, void *arg)
{
  if (!vec->items) return;

  if (fn) {
    info("finalizing objs...");
    void *item;
    Vector_ForEach(item, vec)
      fn(item, arg);
  }

  free(vec->items);

  vec->allocated = 0;
  vec->size = 0;
  vec->items = NULL;
}

static int vector_expand(Vector *vec, int newsize)
{
  int allocated = vec->allocated;
  if (newsize <= allocated) {
    vec->size = newsize;
    return 0;
  }

  int new_allocated = (allocated == 0) ? MIN_ALLOCATED : allocated << 1;
  info("vec allocated:%d", new_allocated);
  void *items = calloc(new_allocated, sizeof(void *));
  if (!items) return -1;

  if (vec->items) {
    memcpy(items, vec->items, Vector_Size(vec) * sizeof(void *));
    free(vec->items);
  }
  vec->allocated = new_allocated;
  vec->size = newsize;
  vec->items = items;
  return 0;
}

int Vector_Insert(Vector *vec, int where, void *item)
{
  int n = Vector_Size(vec);
  if (vector_expand(vec, n + 1) < 0) return -1;
  if (where < 0) where = 0;
  if (where > n) where = n;
  void **items = vec->items;
  for (int i = n; i > where; i--)
    items[i] = items[i-1];
  items[where] = item;
  return 0;
}

void *Vector_Set(Vector *vec, int index, void *item)
{
  if (!vec->items) {
    error("vec is empty");
    return NULL;
  }

  if (index < 0 || index >= Vector_Size(vec)) {
    error("index(%d) is out of range(0-%d)", index, Vector_Size(vec));
    return NULL;
  }

  void *old = vec->items[index];
  vec->items[index] = item;
  return old;
}

void *Vector_Get(Vector *vec, int index)
{
  if (!vec || !vec->items) {
    error("vec is null or empty");
    return NULL;
  }

  if (index < 0 || index >= Vector_Size(vec)) {
    error("index(%d) is out of range(0-%d)", index, Vector_Size(vec));
    return NULL;
  }

  return vec->items[index];
}

int Vector_Append(Vector *vec, void *item)
{
  int n = Vector_Size(vec);
  if (vector_expand(vec, n + 1) < 0) return -1;
  vec->items[n] = item;
  return 0;
}

int Vector_Concat(Vector *dest, Vector *src)
{
  void *item;
  Vector_ForEach(item, src)
    Vector_Append(dest, item);
  return 0;
}

int Vector_ToArray(Vector *vec, int bsz, copyfunc fn, void **arr)
{
  if (!vec) {
    *arr = NULL;
    return 0;
  }

  void *data = malloc(bsz * vec->size);
  void *item;
  Vector_ForEach(item, vec) {
    memcpy((char *)data + i * bsz, item, bsz);
    if (fn) fn((char *)data + i * bsz, item);
  }
  *arr = data;
  return vec->size;
}
