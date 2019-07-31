/*
 * MIT License
 * Copyright (c) 2018 James, https://github.com/zhuguangxiang
 */

#include <stdarg.h>
#include "tupleobject.h"
#include "log.h"

#define MSIZE(size) \
  (sizeof(TupleObject) + sizeof(void *) * (size))

Object *Tuple_New(int size)
{
  TupleObject *tuple = kmalloc(MSIZE(size));
  Init_Object_Head(tuple, &Tuple_Type);
  tuple->size = size;
  return (Object *)tuple;
}

Object *Tuple_Pack(int size, ...)
{
  Object *ob = Tuple_New(size);
  va_list ap;

  va_start(ap, size);
  Object *item;
  TupleObject *tuple = (TupleObject *)ob;
  for (int i = 0; i < size; ++i) {
    item = va_arg(ap, Object *);
    tuple->items[i] = item;
  }
  va_end(ap);

  return ob;
}

int Tuple_Size(Object *self)
{
  if (self == NULL) {
    warn("tuple pointer is null.");
    return 0;
  }

  if (!Tuple_Check(self)) {
    error("object of '%.64s' is not a tuple.", OB_TYPE(self)->name);
    return -1;
  }
  return ((TupleObject *)self)->size;
}

Object *Tuple_Get(Object *self, int index)
{
  if (!Tuple_Check(self)) {
    error("object of '%.64s' is not a tuple.", OB_TYPE(self)->name);
    return NULL;
  }

  TupleObject *tuple = (TupleObject *)self;
  if (index < 0 || index > tuple->size) {
    error("tuple index out of range.");
    return NULL;
  }

  return tuple->items[index];
}

int Tuple_Set(Object *self, int index, Object *val)
{
  if (!Tuple_Check(self)) {
    error("object of '%.64s' is not a tuple.", OB_TYPE(self)->name);
    return -1;
  }

  TupleObject *tuple = (TupleObject *)self;
  if (index < 0 || index > tuple->size) {
    error("tuple index out of range.");
    return -1;
  }

  tuple->items[index] = OB_INCREF(val);
  return 0;
}

static Object *_tuple_getitem_cb_(Object *self, Object *args)
{
  return NULL;
}

static Object *_tuple_setitem_cb_(Object *self, Object *args)
{
  return 0;
}

static MappingMethods tuple_mapping = {
  .getitem = _tuple_getitem_cb_,
  .setitem = _tuple_setitem_cb_,
};

static Object *_tuple_length_cb_(Object *self, Object *args)
{
  return NULL;
}

static FieldDef tuple_fields[] = {
  {"len", "i", _tuple_length_cb_, NULL},
  {NULL}
};

static Object *_tuple_get_cb_(Object *self, Object *args)
{
  return NULL;
}

static Object *_tuple_set_cb_(Object *ob, Object *args)
{
  return NULL;
}

static MethodDef tuple_methods[] = {
  {"get", "i", "A", _tuple_get_cb_},
  {"set", "iA", "z", _tuple_set_cb_},
  {NULL}
};

TypeObject Tuple_Type = {
  OBJECT_HEAD_INIT(&Type_Type)
  .name = "Tuple",
  .mapping = &tuple_mapping,
  .fields = tuple_fields,
  .methods = tuple_methods,
};

void *tuple_iter_next(struct iterator *iter)
{
  TupleObject *tuple = iter->iterable;
  if (tuple->size <= 0)
    return NULL;

  if (iter->index < tuple->size) {
    iter->item = Tuple_Get((Object *)tuple, iter->index);
    ++iter->index;
  } else {
    iter->item = NULL;
  }
  return iter->item;
}
