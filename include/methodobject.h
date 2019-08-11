/*
 * MIT License
 * Copyright (c) 2018 James, https://github.com/zhuguangxiang
 */

#ifndef _KOALA_METHOD_OBJECT_H_
#define _KOALA_METHOD_OBJECT_H_

#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct methodobject {
  OBJECT_HEAD
  /* method name */
  char *name;
  /* method owner */
  Object *owner;
  /* method type descriptor */
  TypeDesc *desc;
  /* call this method */
  callfunc call;
  /* cfunc or kfunc pointer */
  void *ptr;
} MethodObject;

typedef struct protoobject {
  OBJECT_HEAD
  /* method name */
  char *name;
  /* method owner */
  TypeObject *owner;
  /* method type descriptor */
  TypeDesc *desc;
} ProtoObject;

extern TypeObject Method_Type;
extern TypeObject Proto_Type;

#define Method_Check(ob) (OB_TYPE(ob) == &Method_Type)
#define Proto_Check(ob) (OB_TYPE(ob) == &Proto_Type)
Object *CMethod_New(MethodDef *m);
static inline Object *Method_Call(Object *self, Object *ob, Object *args)
{
  if (!Method_Check(self)) {
    error("object of '%.64s' is not a Method", OB_TYPE_NAME(self));
    return NULL;
  }
  MethodObject *meth = (MethodObject *)self;
  return meth->call(self, ob, args);
}

#ifdef __cplusplus
}
#endif

#endif /* _KOALA_FIELD_OBJECT_H_ */
