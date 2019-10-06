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

#include <math.h>
#include "intobject.h"
#include "stringobject.h"
#include "floatobject.h"
#include "fmtmodule.h"

static void int_free(Object *ob)
{
  if (!integer_check(ob)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(ob));
    return;
  }
  debug("[Freed] Integer %ld", integer_asint(ob));
  gcfree(ob);
}

static Object *int_equal(Object *self, Object *ob)
{
  if (!integer_check(self)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(self));
    return bool_false();
  }

  if (!integer_check(ob)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(ob));
    return bool_false();
  }

  IntegerObject *i1 = (IntegerObject *)self;
  IntegerObject *i2 = (IntegerObject *)ob;
  return (i1->value == i2->value) ? bool_true() : bool_false();
}

static Object *int_str(Object *self, Object *ob)
{
  if (!integer_check(self)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(self));
    return NULL;
  }

  IntegerObject *i = (IntegerObject *)self;
  char buf[256];
  sprintf(buf, "%ld", i->value);
  return string_new(buf);
}

static Object *integer_fmt(Object *self, Object *ob)
{
  if (!integer_check(self)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(self));
    return NULL;
  }

  Fmtter_WriteInteger(ob, self);
  return NULL;
}

static int64_t int_add(Object *x, Object *y)
{
  int64_t a = integer_asint(x);
  int64_t r;
  if (integer_check(y)) {
    int64_t b = integer_asint(y);
    r = (int64_t)((uint64_t)a + b);
    if ((r ^ a) < 0 && (r ^ b) < 0)
      panic("overflow:%ld + %ld = %ld", a, b, r);
  } else if (byte_check(y)) {
    int b = byte_asint(y);
    r = (int64_t)((uint64_t)a + b);
    if ((r ^ a) < 0 && (r ^ b) < 0)
      panic("overflow:%ld + %d = %ld", a, b, r);
  } else if (Float_Check(y)) {
    double b = Float_AsFlt(y);
    r = (int64_t)((double)a + b);
    if ((a > 0 && b > 0 && r < 0) ||
        (a < 0 && b < 0 && r > 0))
      panic("overflow:%ld + %lf = %ld", a, b, r);
  } else {
    error("Unsupported operand type(s) for +: 'Integer' and '%s'",
          OB_TYPE_NAME(y));
  }
  return r;
}

static int64_t int_sub(Object *x, Object *y)
{
  int64_t a = integer_asint(x);
  int64_t r;
  if (integer_check(y)) {
    int64_t b = integer_asint(y);
    r = (int64_t)((uint64_t)a - b);
    if ((r ^ a) < 0 && (r ^ ~b) < 0)
      panic("overflow:%ld + %ld = %ld", a, b, r);
  } else if (byte_check(y)) {
    int b = byte_asint(y);
    r = (int64_t)((uint64_t)a - b);
    if ((r ^ a) < 0 && (r ^ ~b) < 0)
      panic("overflow:%ld + %d = %ld", a, b, r);
  } else if (Float_Check(y)) {
    double b = Float_AsFlt(y);
    r = (int64_t)((double)a - b);
    if ((a > 0 && b < 0 && r < 0) ||
        (a < 0 && b > 0 && r > 0))
      panic("overflow:%ld + %lf = %ld", a, b, r);
  } else {
    error("Unsupported operand type(s) for -: 'Integer' and '%s'",
          OB_TYPE_NAME(y));
  }
  return r;
}

static int64_t int_mul(Object *x, Object *y)
{
  int64_t a = integer_asint(x);
  int64_t r;
  if (integer_check(y)) {
    int64_t b = integer_asint(y);
    r = (int64_t)(a * b);
  } else if (byte_check(y)) {
    int b = byte_asint(y);
    r = (int64_t)(a * b);
  } else if (Float_Check(y)) {
    double b = Float_AsFlt(y);
    r = (int64_t)(a * b);
  } else {
    error("Unsupported operand type(s) for *: 'Integer' and '%s'",
          OB_TYPE_NAME(y));
  }
  return r;
}

static int64_t int_div(Object *x, Object *y)
{
  int64_t a = integer_asint(x);
  int64_t r;
  if (integer_check(y)) {
    int64_t b = integer_asint(y);
    r = (int64_t)(a / b);
  } else if (byte_check(y)) {
    int b = byte_asint(y);
    r = (int64_t)(a / b);
  } else if (Float_Check(y)) {
    double b = Float_AsFlt(y);
    r = (int64_t)(a / b);
  } else {
    error("Unsupported operand type(s) for /: 'Integer' and '%s'",
          OB_TYPE_NAME(y));
  }
  return r;
}

static int64_t int_mod(Object *x, Object *y)
{
  int64_t a = integer_asint(x);
  int64_t r;
  if (integer_check(y)) {
    int64_t b = integer_asint(y);
    r = (int64_t)fmod(a, b);
  } else if (byte_check(y)) {
    int b = byte_asint(y);
    r = (int64_t)fmod(a, b);
  } else if (Float_Check(y)) {
    double b = Float_AsFlt(y);
    r = (int64_t)fmod(a, b);
  } else {
    error("Unsupported operand type(s) for %%: 'Integer' and '%s'",
          OB_TYPE_NAME(y));
  }
  return r;
}

static int64_t int_pow(Object *x, Object *y)
{
  int64_t a = integer_asint(x);
  int64_t r;
  if (integer_check(y)) {
    int64_t b = integer_asint(y);
    r = (int64_t)pow(a, b);
  } else if (byte_check(y)) {
    int b = byte_asint(y);
    r = (int64_t)pow(a, b);
  } else if (Float_Check(y)) {
    double b = Float_AsFlt(y);
    r = (int64_t)pow(a, b);
  } else {
    error("Unsupported operand type(s) for **: 'Integer' and '%s'",
          OB_TYPE_NAME(y));
  }
  return r;
}

static Object *int_num_add(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (y == NULL) {
    error("\"__add__\" must be two operands");
    return NULL;
  }

  return integer_new(int_add(x, y));
}

static Object *int_num_sub(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (y == NULL) {
    error("\"__sub__\" must be two operands");
    return NULL;
  }

  return integer_new(int_sub(x, y));
}

static Object *int_num_mul(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (y == NULL) {
    error("\"__mul__\" must be two operands");
    return NULL;
  }

  return integer_new(int_mul(x, y));
}

static Object *int_num_div(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (y == NULL) {
    error("\"__div__\" must be two operands");
    return NULL;
  }

  return integer_new(int_div(x, y));
}

static Object *int_num_mod(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (y == NULL) {
    error("\"__mod__\" must be two operands");
    return NULL;
  }

  return integer_new(int_mod(x, y));
}

static Object *int_num_pow(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (y == NULL) {
    error("\"__pow__\" must be two operands");
    return NULL;
  }

  return integer_new(int_pow(x, y));
}

static Object *int_num_neg(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (y != NULL) {
    error("'-' must be only one operand");
    return NULL;
  }

  Object *z;
  int64_t a = integer_asint(x);
  z = integer_new((int64_t)((uint64_t)0 - a));
  return z;
}

static int64_t int_num_cmp(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    //FIXME: retval?
    return 0;
  }

  int64_t a = integer_asint(x);
  int64_t r;
  if (integer_check(y)) {
    int64_t b = integer_asint(y);
    r = (int64_t)((uint64_t)a - b);
    if ((r ^ a) < 0 && (r ^ ~b) < 0)
      panic("overflow:%ld + %ld = %ld", a, b, r);
  } else if (byte_check(y)) {
    int b = byte_asint(y);
    r = (int64_t)((uint64_t)a - b);
    if ((r ^ a) < 0 && (r ^ ~b) < 0)
      panic("overflow:%ld + %d = %ld", a, b, r);
  } else if (Float_Check(y)) {
    double b = Float_AsFlt(y);
    r = (int64_t)((double)a - b);
    if ((a > 0 && b < 0 && r < 0) ||
        (a < 0 && b > 0 && r > 0))
      panic("overflow:%ld + %lf = %ld", a, b, r);
  } else {
    panic("Not Implemented");
  }
  return r;
}

static Object *int_num_gt(Object *x, Object *y)
{
  int64_t r = int_num_cmp(x, y);
  return (r > 0) ? bool_true() : bool_false();
}

static Object *int_num_ge(Object *x, Object *y)
{
  int64_t r = int_num_cmp(x, y);
  return (r >= 0) ? bool_true() : bool_false();
}

static Object *int_num_lt(Object *x, Object *y)
{
  int64_t r = int_num_cmp(x, y);
  return (r < 0) ? bool_true() : bool_false();
}

static Object *int_num_le(Object *x, Object *y)
{
  int64_t r = int_num_cmp(x, y);
  return (r <= 0) ? bool_true() : bool_false();
}

static Object *int_num_eq(Object *x, Object *y)
{
  int64_t r = int_num_cmp(x, y);
  return (r == 0) ? bool_true() : bool_false();
}

static Object *int_num_neq(Object *x, Object *y)
{
  int64_t r = int_num_cmp(x, y);
  return (r != 0) ? bool_true() : bool_false();
}

static Object *int_num_and(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (!integer_check(y)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(y));
    return NULL;
  }

  Object *z;
  int64_t a = integer_asint(x);
  int64_t b = integer_asint(y);
  z = integer_new(a & b);
  return z;
}

static Object *int_num_or(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (!integer_check(y)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(y));
    return NULL;
  }

  Object *z;
  int64_t a = integer_asint(x);
  int64_t b = integer_asint(y);
  z = integer_new(a | b);
  return z;
}

static Object *int_num_xor(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (!integer_check(y)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(y));
    return NULL;
  }

  Object *z;
  int64_t a = integer_asint(x);
  int64_t b = integer_asint(y);
  z = integer_new(a ^ b);
  return z;
}

static Object *int_num_not(Object *x, Object *y)
{
  if (!integer_check(x)) {
    error("object of '%.64s' is not an Integer", OB_TYPE_NAME(x));
    return NULL;
  }

  if (y != NULL) {
    error("'-' must be only one operand");
    return NULL;
  }

  Object *z;
  int64_t a = integer_asint(x);
  z = integer_new(~a);
  return z;
}

static NumberMethods int_numbers = {
  .add = int_num_add,
  .sub = int_num_sub,
  .mul = int_num_mul,
  .div = int_num_div,
  .mod = int_num_mod,
  .pow = int_num_pow,
  .neg = int_num_neg,

  .gt  = int_num_gt,
  .ge  = int_num_ge,
  .lt  = int_num_lt,
  .le  = int_num_le,
  .eq  = int_num_eq,
  .neq = int_num_neq,

  .and = int_num_and,
  .or  = int_num_or,
  .xor = int_num_xor,
  .not = int_num_not,
};

static MethodDef int_methods[]= {
  {"__fmt__", "Llang.Formatter;", NULL, integer_fmt},
  {"__add__", "A", "i", int_num_add},
  {NULL}
};

TypeObject integer_type = {
  OBJECT_HEAD_INIT(&type_type)
  .name    = "Integer",
  .free    = int_free,
  .equal   = int_equal,
  .str     = int_str,
  .number  = &int_numbers,
  .methods = int_methods,
};

void init_integer_type(void)
{
  integer_type.desc = desc_from_int;
  if (type_ready(&integer_type) < 0)
    panic("Cannot initalize 'Integer' type.");
}

Object *integer_new(int64_t val)
{
  IntegerObject *integer = gcnew(sizeof(IntegerObject));
  init_object_head(integer, &integer_type);
  integer->value = val;
  return (Object *)integer;
}

static void byte_free(Object *ob)
{
  if (!byte_check(ob)) {
    error("object of '%.64s' is not a Byte", OB_TYPE_NAME(ob));
    return;
  }
  ByteObject *b = (ByteObject *)ob;
  debug("[Freed] Byte %d", b->value);
  kfree(ob);
}

static Object *byte_str(Object *self, Object *ob)
{
  if (!byte_check(self)) {
    error("object of '%.64s' is not a Byte", OB_TYPE_NAME(self));
    return NULL;
  }

  ByteObject *b = (ByteObject *)ob;
  char buf[8];
  sprintf(buf, "%d", b->value);
  return string_new(buf);
}

TypeObject byte_type = {
  OBJECT_HEAD_INIT(&type_type)
  .name = "Byte",
  .free = byte_free,
  .str  = byte_str,
};

void init_byte_type(void)
{
  byte_type.desc = desc_from_byte;
  if (type_ready(&byte_type) < 0)
    panic("Cannot initalize 'Byte' type.");
}

Object *byte_new(int val)
{
  ByteObject *b = kmalloc(sizeof(ByteObject));
  init_object_head(b, &byte_type);
  b->value = val;
  return (Object *)b;
}

static Object *bool_str(Object *self, Object *ob)
{
  if (!bool_check(self)) {
    error("object of '%.64s' is not a Bool", OB_TYPE_NAME(self));
    return NULL;
  }

  BoolObject *b = (BoolObject *)self;
  char buf[8];
  sprintf(buf, "%s", b->value ? "true" : "false");
  return string_new(buf);
}

TypeObject bool_type = {
  OBJECT_HEAD_INIT(&type_type)
  .name = "Bool",
  .str  = bool_str,
};

void init_bool_type(void)
{
  bool_type.desc = desc_from_bool;
  if (type_ready(&bool_type) < 0)
    panic("Cannot initalize 'Bool' type.");
}

void fini_bool_type(void)
{
  int refcnt = OB_True.ob_refcnt;
  expect(refcnt == 1);
  refcnt = OB_False.ob_refcnt;
  expect(refcnt == 1);
}

BoolObject OB_True = {
  OBJECT_HEAD_INIT(&bool_type)
  .value = 1,
};

BoolObject OB_False = {
  OBJECT_HEAD_INIT(&bool_type)
  .value = 0,
};
