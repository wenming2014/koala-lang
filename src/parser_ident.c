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

#include "parser.h"
#include "log.h"

LOGGER(0)

/* symbol is found in current scope */
static void parse_ident_current_scope(ParserUnit *u, Expr *exp)
{
  Symbol *sym = exp->sym;

  switch (u->scope) {
  case SCOPE_MODULE:
  case SCOPE_CLASS:
    /*
     * expr, in module/class scope,
     * is variable/field declaration's right expr
     */
    assert(exp->ctx == EXPR_LOAD);
    if (sym->kind == SYM_CONST || sym->kind == SYM_VAR) {
      Log_Debug("symbol '%s' is %s", sym->name,
                sym->kind == SYM_CONST ? "constant" : "variable");
      if (exp->desc == NULL) {
        exp->desc = ((VarSymbol *)sym)->desc;
        TYPE_INCREF(exp->desc);
      }

      Expr *right = exp->right;
      if (exp->desc->kind == TYPE_PROTO &&
          right != NULL && right->kind == CALL_KIND) {
        /* call function(variable) */
        /* FIXME: anonymous */
        CallExpr *callExp = (CallExpr *)right;
        Log_Debug("variable '%s' is function", sym->name);
        Log_Debug("call '%s' function", sym->name);
        Inst_Append_NoArg(u->block, OP_LOAD0);
        ConstValue val = {.kind = BASE_STRING, .str = sym->name};
        Inst *i = Inst_Append(u->block, OP_CALL, &val);
        i->argc = Vector_Size(callExp->args);
      } else {
        /* load variable */
        Log_Debug("load '%s' variable", sym->name);
        Inst_Append_NoArg(u->block, OP_LOAD0);
        ConstValue val = {.kind = BASE_STRING, .str = sym->name};
        Inst_Append(u->block, OP_GETFIELD, &val);
      }
    } else {
      assert(sym->kind == SYM_FUNC);
      Log_Debug("symbol '%s' is function", sym->name);
      if (exp->desc == NULL) {
        exp->desc = ((FuncSymbol *)sym)->desc;
        TYPE_INCREF(exp->desc);
      }

      Expr *right = exp->right;
      if (right != NULL && right->kind == CALL_KIND) {
        /* call function */
        CallExpr *callExp = (CallExpr *)right;
        Log_Debug("call '%s' function", sym->name);
        Inst_Append_NoArg(u->block, OP_LOAD0);
        ConstValue val = {.kind = BASE_STRING, .str = sym->name};
        Inst *i = Inst_Append(u->block, OP_CALL, &val);
        i->argc = Vector_Size(callExp->args);
      } else {
        /* load function */
        Log_Debug("load '%s' function", sym->name);
        Inst_Append_NoArg(u->block, OP_LOAD0);
        ConstValue val = {.kind = BASE_STRING, .str = sym->name};
        Inst_Append(u->block, OP_GETFIELD, &val);
      }
    }
    break;
  case SCOPE_FUNCTION:
  case SCOPE_BLOCK:
  case SCOPE_CLOSURE:
    /*
     * expr, in function/block/closure scope, is local variable or parameter
     * it's context is load or store
     */
    assert(sym->kind == SYM_VAR);
    VarSymbol *varSym = (VarSymbol *)sym;
    Log_Debug("symbol '%s' is local variable", varSym->name);
    if (exp->desc == NULL) {
      exp->desc = varSym->desc;
      TYPE_INCREF(exp->desc);
    }

    Expr *right = exp->right;
    if (exp->desc->kind == TYPE_PROTO &&
        right != NULL && right->kind == CALL_KIND) {
      /* call function(variable) */
      /* FIXME: anonymous */
      CallExpr *callExp = (CallExpr *)right;
      Log_Debug("variable '%s' is function", sym->name);
      Log_Debug("call '%s' function", sym->name);
      assert(exp->ctx == EXPR_LOAD);
      Inst_Append_NoArg(u->block, OP_LOAD0);
      ConstValue val = {.kind = BASE_STRING, .str = varSym->name};
      Inst *i = Inst_Append(u->block, OP_CALL, &val);
      i->argc = Vector_Size(callExp->args);
    } else {
      /* load variable */
      assert(exp->ctx == EXPR_LOAD || exp->ctx == EXPR_STORE);
      if (exp->ctx == EXPR_LOAD)
        Log_Debug("load '%s' variable", varSym->name);
      else
        Log_Debug("store '%s' variable", varSym->name);
      int opcode = (exp->ctx == EXPR_LOAD) ? OP_LOAD : OP_STORE;
      ConstValue val = {.kind = BASE_STRING, .ival = varSym->index};
      Inst_Append(u->block, opcode, &val);
    }
    break;
  default:
    assert(0);
    break;
  }
}

/*
  a.b.c.attribute
  a.b.c()
  a.b.c[10]
  a.b.c[1:10]
  leftmost identifier is variable or imported external package name
 */
void Parse_Ident_Expr(ParserState *ps, Expr *exp)
{
  assert(exp->kind == ID_KIND);
  ParserUnit *u = ps->u;
  IdentExpr *idExp = (IdentExpr *)exp;
  char *name = idExp->name;
  Symbol *sym;

  /* find ident from current scope */
  sym = STable_Get(u->stbl, name);
  if (sym != NULL) {
    Log_Debug("find symbol '%s' in local scope-%d(%s)",
              name, ps->depth, scope_strings[u->scope]);
    exp->sym = sym;
    parse_ident_current_scope(u, exp);
    return;
  }

  Syntax_Error(ps, &exp->pos, "cannot find symbol '%s'", name);
}

void Code_Ident_Expr(ParserState *ps, Expr *exp)
{
  assert(exp->desc != NULL && exp->sym != NULL);
  ParserUnit *u = ps->u;
  Symbol *sym = exp->sym;
}