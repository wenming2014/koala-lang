/*
 * MIT License
 * Copyright (c) 2018 James, https://github.com/zhuguangxiang
 */

#include <inttypes.h>
#include <stddef.h>
#include "common.h"
#include "opcode.h"

#define OPCODE(op, size) { op, #op, size }

static struct opcode {
  int op;
  char *opstr;
  uint8_t argsize;
} opcodes [] = {
  OPCODE(OP_HALT, 0),
  OPCODE(OP_POP_TOP,  0),
  OPCODE(OP_DUP, 0),
  OPCODE(OP_CONST_BYTE,  1),
  OPCODE(OP_CONST_SHORT, 2),
  OPCODE(OP_LOAD_CONST,  2),
  OPCODE(OP_LOAD_MODULE, 2),

  OPCODE(OP_LOAD,    1),
  OPCODE(OP_LOAD_0,  0),
  OPCODE(OP_LOAD_1,  0),
  OPCODE(OP_LOAD_2,  0),
  OPCODE(OP_LOAD_3,  0),
  OPCODE(OP_STORE,   1),
  OPCODE(OP_STORE_0, 0),
  OPCODE(OP_STORE_1, 0),
  OPCODE(OP_STORE_2, 0),
  OPCODE(OP_STORE_3, 0),

  OPCODE(OP_GET_OBJECT, 2),
  OPCODE(OP_GET_VALUE, 2),
  OPCODE(OP_SET_VALUE, 2),
  OPCODE(OP_RETURN_VALUE, 0),
  OPCODE(OP_RETURN, 0),
  OPCODE(OP_CALL, 3),
  OPCODE(OP_PRINT, 0),

  OPCODE(OP_ADD, 0),
  OPCODE(OP_SUB, 0),
  OPCODE(OP_MUL, 0),
  OPCODE(OP_DIV, 0),
  OPCODE(OP_MOD, 0),
  OPCODE(OP_NEG, 0),

  OPCODE(OP_GT,  0),
  OPCODE(OP_GE,  0),
  OPCODE(OP_LT,  0),
  OPCODE(OP_LE,  0),
  OPCODE(OP_EQ,  0),
  OPCODE(OP_NEQ, 0),

  OPCODE(OP_BIT_AND, 0),
  OPCODE(OP_BIT_OR,  0),
  OPCODE(OP_BIT_XOR, 0),
  OPCODE(OP_BIT_NOT, 0),

  OPCODE(OP_AND, 0),
  OPCODE(OP_OR,  0),
  OPCODE(OP_NOT, 0),

  OPCODE(OP_INPLACE_ADD, 0),
  OPCODE(OP_INPLACE_SUB, 0),
  OPCODE(OP_INPLACE_MUL, 0),
  OPCODE(OP_INPLACE_DIV, 0),
  OPCODE(OP_INPLACE_POW, 0),
  OPCODE(OP_INPLACE_MOD, 0),
  OPCODE(OP_INPLACE_AND, 0),
  OPCODE(OP_INPLACE_OR,  0),
  OPCODE(OP_INPLACE_XOR, 0),

  OPCODE(OP_SUBSCR_LOAD,  0),
  OPCODE(OP_SUBSCR_STORE, 0),
  OPCODE(OP_SLICE_LOAD,  0),
  OPCODE(OP_SLICE_STORE, 0),

  OPCODE(OP_JMP,        0),
  OPCODE(OP_JMP_TRUE,   0),
  OPCODE(OP_JMP_FALSE,  0),
  OPCODE(OP_JMP_CMPEQ,  0),
  OPCODE(OP_JMP_CMPNEQ, 0),
  OPCODE(OP_JMP_CMPLT,  0),
  OPCODE(OP_JMP_CMPGT,  0),
  OPCODE(OP_JMP_CMPLE,  0),
  OPCODE(OP_JMP_CMPGE,  0),
  OPCODE(OP_JMP_NIL,    0),
  OPCODE(OP_JMP_NOTNIL, 0),

  OPCODE(OP_NEW_OBJECT, 2),
  OPCODE(OP_NEW_TUPLE,  2),
  OPCODE(OP_NEW_ARRAY,  2),
  OPCODE(OP_NEW_SET,    2),
  OPCODE(OP_NEW_MAP,    0),
  OPCODE(OP_NEW_ITER,   0),
  OPCODE(OP_FOR_EACH,   0),
  OPCODE(OP_NEW_EVAL,   3),
  OPCODE(OP_NEW_CLOSURE, 0),
};

#define OP_MAP(op, map) { op, #map }

struct opmap {
  uint8_t op;
  char *map;
} opmaps [] = {
  OP_MAP(OP_ADD, __add__),
  OP_MAP(OP_SUB, __sub__),
  OP_MAP(OP_MUL, __mul__),
  OP_MAP(OP_DIV, __div__),
  OP_MAP(OP_MOD, __mod__),
  OP_MAP(OP_POW, __pow__),
  OP_MAP(OP_NEG, __neg__),

  OP_MAP(OP_GT,  __gt__),
  OP_MAP(OP_GE,  __ge__),
  OP_MAP(OP_LT,  __lt__),
  OP_MAP(OP_LE,  __le__),
  OP_MAP(OP_EQ,  __eq__),
  OP_MAP(OP_NEQ, __neq__),

  OP_MAP(OP_BIT_AND, __and__),
  OP_MAP(OP_BIT_OR,  __or__),
  OP_MAP(OP_BIT_XOR, __xor__),
  OP_MAP(OP_BIT_NOT, __not__),

  OP_MAP(OP_INPLACE_ADD, __inadd__),
  OP_MAP(OP_INPLACE_SUB, __insub__),
  OP_MAP(OP_INPLACE_MUL, __inmul__),
  OP_MAP(OP_INPLACE_DIV, __indiv__),
  OP_MAP(OP_INPLACE_POW, __inpow__),
  OP_MAP(OP_INPLACE_MOD, __inmod__),
  OP_MAP(OP_INPLACE_AND, __inand__),
  OP_MAP(OP_INPLACE_OR,  __inor__),
  OP_MAP(OP_INPLACE_XOR, __inxor__),

  OP_MAP(OP_SUBSCR_LOAD,  __getitem__),
  OP_MAP(OP_SUBSCR_STORE, __setitem__),
};

int opcode_argc(int op)
{
  struct opcode *opcode;
  for (int i = 0; i < COUNT_OF(opcodes); i++) {
    opcode = &opcodes[i];
    if (opcode->op == op)
      return opcode->argsize;
  }
  return -1;
}

char *opcode_str(int op)
{
  struct opcode *opcode;
  for (int i = 0; i < COUNT_OF(opcodes); i++) {
    opcode = &opcodes[i];
    if (opcode->op == op)
      return opcode->opstr;
  }
  return NULL;
}

char *opcode_map(int op)
{
  struct opmap *map;
  for (int i = 0; i < COUNT_OF(opmaps); i++) {
    map = &opmaps[i];
    if (map->op == op)
      return map->map;
  }
  return NULL;
}