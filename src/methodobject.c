/*
 * MIT License
 * Copyright (c) 2018 James, https://github.com/zhuguangxiang
 */

#include "methodobject.h"
#include "tupleobject.h"

static Object *cfunc_call(Object *self, Object *ob, Object *args)
{
  MethodObject *meth = (MethodObject *)self;
  cfunc fn = (cfunc)meth->ptr;
  return fn(ob, args);
}

Object *CMethod_New(MethodDef *def)
{
  MethodObject *method = kmalloc(sizeof(*method));
  Init_Object_Head(method, &Method_Type);
  method->name = def->name;
  method->desc = TypeStr_ToProto(def->ptype, def->rtype);
  method->call = cfunc_call,
  method->ptr = def->func;
  return (Object *)method;
}

static Object *method_call(Object *self, Object *args)
{
  if (!Method_Check(self)) {
    error("object of '%.64s' is not a Method", OB_TYPE_NAME(self));
    return NULL;
  }

  if (args == NULL) {
    error("'__call__' has no arguments");
    return NULL;
  }

  Object *ob;
  Object *para;
  if (!Tuple_Check(args)) {
    ob = OB_INCREF(args);
    para = NULL;
  } else {
    ob = Tuple_Get(args, 0);
    para = Tuple_Slice(args, 1, -1);
  }

  Object *res = Method_Call(self, ob, para);
  OB_DECREF(ob);
  OB_DECREF(para);
  return res;
}

static MethodDef meth_methods[] = {
  {"call", "...", "A", method_call},
  {NULL}
};

TypeObject Method_Type = {
  OBJECT_HEAD_INIT(&Type_Type)
  .name    = "Method",
  .lookup  = Object_Lookup,
  .methods = meth_methods,
};

TypeObject Proto_Type = {
  OBJECT_HEAD_INIT(&Type_Type)
  .name = "Proto",
};

Object *Method_Call(Object *self, Object *ob, Object *args)
{
  if (!Method_Check(self)) {
    error("object of '%.64s' is not a Method", OB_TYPE_NAME(self));
    return NULL;
  }
  MethodObject *meth = (MethodObject *)self;
  return meth->call(self, ob, args);
}
