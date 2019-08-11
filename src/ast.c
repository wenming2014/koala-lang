/*
 * MIT License
 * Copyright (c) 2018 James, https://github.com/zhuguangxiang
 */

#include <inttypes.h>
#include "parser.h"
#include "memory.h"
#include "strbuf.h"

Expr *Expr_From_Nil(void)
{
  Expr *exp = kmalloc(sizeof(Expr));
  exp->kind = NIL_KIND;
  return exp;
}

Expr *Expr_From_Self(void)
{
  Expr *exp = kmalloc(sizeof(Expr));
  exp->kind = SELF_KIND;
  return exp;
}

Expr *Expr_From_Super(void)
{
  Expr *exp = kmalloc(sizeof(Expr));
  exp->kind = SUPER_KIND;
  return exp;
}

Expr *expr_from_integer(int64_t val)
{
  ConstExpr *constExp = kmalloc(sizeof(ConstExpr));
  constExp->kind = LITERAL_KIND;
  constExp->desc = typedesc_getbase(BASE_INT);
  TYPE_INCREF(constExp->desc);
  constExp->value.kind = BASE_INT;
  constExp->value.ival = val;
  return (Expr *)constExp;
}

Expr *expr_from_float(double val)
{
  ConstExpr *constExp = kmalloc(sizeof(ConstExpr));
  constExp->kind = LITERAL_KIND;
  constExp->desc = typedesc_getbase(BASE_FLOAT);
  TYPE_INCREF(constExp->desc);
  constExp->value.kind = BASE_FLOAT;
  constExp->value.fval = val;
  return (Expr *)constExp;
}

Expr *expr_from_bool(int val)
{
  ConstExpr *constExp = kmalloc(sizeof(ConstExpr));
  constExp->kind = LITERAL_KIND;
  constExp->desc = typedesc_getbase(BASE_BOOL);
  TYPE_INCREF(constExp->desc);
  constExp->value.kind = BASE_BOOL;
  constExp->value.bval = val;
  return (Expr *)constExp;
}

Expr *expr_from_string(char *val)
{
  ConstExpr *constExp = kmalloc(sizeof(ConstExpr));
  constExp->kind = LITERAL_KIND;
  constExp->desc = typedesc_getbase(BASE_STR);
  TYPE_INCREF(constExp->desc);
  constExp->value.kind = BASE_STR;
  constExp->value.str = val;
  return (Expr *)constExp;
}

Expr *expr_from_char(wchar val)
{
  ConstExpr *constExp = kmalloc(sizeof(ConstExpr));
  constExp->kind = LITERAL_KIND;
  constExp->desc = typedesc_getbase(BASE_CHAR);
  TYPE_INCREF(constExp->desc);
  constExp->value.kind = BASE_CHAR;
  constExp->value.cval = val;
  return (Expr *)constExp;
}

Expr *expr_from_ident(char *val)
{
  IdentExpr *idExp = kmalloc(sizeof(IdentExpr));
  idExp->kind = ID_KIND;
  idExp->name = val;
  return (Expr *)idExp;
}

/* FIXME: unchanged variable, see parse_operator.c */
int Expr_Is_Const(Expr *exp)
{
  if (exp->kind == LITERAL_KIND)
    return 1;

  if (exp->kind == ID_KIND) {
    Symbol *sym = exp->sym;
    if (sym != NULL && sym->kind == SYM_CONST)
      return 1;
  }

  if (exp->kind == UNARY_KIND) {
    UnaryExpr *unExpr = (UnaryExpr *)exp;
    if (unExpr->val.kind != 0)
      return 1;
  }

  if (exp->kind == BINARY_KIND) {
    BinaryExpr *biExpr = (BinaryExpr *)exp;
    if (biExpr->val.kind != 0)
      return 1;
  }

  return 0;
}

Expr *Expr_From_Unary(UnaryOpKind op, Expr *exp)
{
  UnaryExpr *unaryExp = kmalloc(sizeof(UnaryExpr));
  unaryExp->kind = UNARY_KIND;
  /* it does not matter that exp->desc is null */
  unaryExp->desc = exp->desc;
  TYPE_INCREF(unaryExp->desc);
  unaryExp->op = op;
  unaryExp->exp = exp;
  return (Expr *)unaryExp;
}

Expr *Expr_From_Binary(BinaryOpKind op, Expr *left, Expr *right)
{
  BinaryExpr *binaryExp = kmalloc(sizeof(BinaryExpr));
  binaryExp->kind = BINARY_KIND;
  /* it does not matter that exp->desc is null */
  binaryExp->desc = left->desc;
  TYPE_INCREF(binaryExp->desc);
  binaryExp->op = op;
  binaryExp->lexp = left;
  binaryExp->rexp = right;
  return (Expr *)binaryExp;
}

Expr *Expr_From_Attribute(Ident id, Expr *left)
{
  AttributeExpr *attrExp = kmalloc(sizeof(AttributeExpr));
  attrExp->kind = ATTRIBUTE_KIND;
  attrExp->id = id;
  attrExp->lexp = left;
  left->right = (Expr *)attrExp;
  return (Expr *)attrExp;
}

Expr *Expr_From_SubScript(Expr *index, Expr *left)
{
  SubScriptExpr *subExp = kmalloc(sizeof(SubScriptExpr));
  subExp->kind = SUBSCRIPT_KIND;
  subExp->index = index;
  subExp->lexp = left;
  left->right = (Expr *)subExp;
  return (Expr *)subExp;
}

Expr *Expr_From_Call(Vector *args, Expr *left)
{
  CallExpr *callExp = kmalloc(sizeof(CallExpr));
  callExp->kind = CALL_KIND;
  callExp->args = args;
  callExp->lexp = left;
  left->right = (Expr *)callExp;
  return (Expr *)callExp;
}

Expr *Expr_From_Slice(Expr *start, Expr *end, Expr *left)
{
  SliceExpr *sliceExp = kmalloc(sizeof(SliceExpr));
  sliceExp->kind = SLICE_KIND;
  sliceExp->start = start;
  sliceExp->end = end;
  sliceExp->lexp = left;
  left->right = (Expr *)sliceExp;
  return (Expr *)sliceExp;
}

#if 0

static int arraylist_get_nesting(Vector *vec)
{
  int max = 0;
  ListExpr *listExp;
  Expr *e;
  Vector_ForEach(e, vec) {
    if (e->kind == ARRAY_LIST_KIND) {
      listExp = (ListExpr *)e;
      if (max < listExp->nesting)
        max = listExp->nesting;
    }
  }
  return max;
}

Expr *Expr_From_ArrayListExpr(Vector *vec)
{
  int nesting = arraylist_get_nesting(vec) + 1;
  ListExpr *listExp = kmalloc(sizeof(ListExpr));
  listExp->kind = ARRAY_LIST_KIND;
  listExp->nesting = nesting;
  listExp->vec = vec;
  return (Expr *)listExp;
}

Expr *Expr_From_Array(Vector *dims, PosType base, Expr *listExp)
{
  ArrayExpr *arrayExp = kmalloc(sizeof(ArrayExpr));
  arrayExp->kind = ARRAY_KIND;
  arrayExp->dims = dims;
  arrayExp->base = base;
  assert(listExp != NULL ? listExp->kind == ARRAY_LIST_KIND : 1);
  arrayExp->listExp = (ListExpr *)listExp;
  return (Expr *)arrayExp;
}

Expr *Parser_New_Array(Vector *vec, int dims, PosType type, Expr *listExp)
{
  Vector *dimsVec;
  TypeDesc *base;
  if (vec != NULL) {
    assert(dims == 0);
    dimsVec = vec;
    if (type.desc->kind == TYPE_ARRAY) {
      ArrayDesc *arrayDesc = (ArrayDesc *)type.desc;
      base = arrayDesc->base;
      /* append null to occupy a position */
      for (int i = 0; i < arrayDesc->dims; i++)
        Vector_Append(dimsVec, NULL);
      /* free array desc */
      TYPE_INCREF(base);
      TYPE_DECREF(type.desc);
    } else {
      base = type.desc;
      TYPE_INCREF(base);
    }
  } else {
    assert(dims != 0);
    dimsVec = Vector_Capacity(dims);
    /* append null to occupy a position */
    for (int i = 0; i < dims; i++)
      Vector_Append(dimsVec, NULL);
    assert(type.desc->kind != TYPE_ARRAY);
    base = type.desc;
    TYPE_INCREF(base);
  }
  PosType basetype = {base, type.pos};
  return Expr_From_Array(dimsVec, basetype, listExp);
}

static int maplist_get_nesting(Vector *vec)
{
  int max = 0;
  ListExpr *listExp;
  Expr *v;
  MapEntryExpr *e;
  Vector_ForEach(e, vec) {
    assert(e->kind == MAP_ENTRY_KIND);
    v = e->val;
    assert(v != NULL);
    if (v->kind == MAP_LIST_KIND) {
      listExp = (ListExpr *)v;
      if (max < listExp->nesting)
        max = listExp->nesting;
    }
  }
  return max;
}

Expr *Expr_From_MapListExpr(Vector *vec)
{
  int nesting = maplist_get_nesting(vec) + 1;
  ListExpr *listExp = kmalloc(sizeof(ListExpr));
  listExp->kind = MAP_LIST_KIND;
  listExp->nesting = nesting;
  listExp->vec = vec;
  return (Expr *)listExp;
}

Expr *Expr_From_MapEntry(Expr *k, Expr *v)
{
  MapEntryExpr *entExp = kmalloc(sizeof(MapEntryExpr));
  entExp->kind = MAP_ENTRY_KIND;
  entExp->key = k;
  entExp->val = v;
  return (Expr *)entExp;
}

Expr *Expr_From_Map(PosType type, Expr *listExp)
{
  MapExpr *mapExp = kmalloc(sizeof(MapExpr));
  mapExp->kind = MAP_KIND;
  TYPE_INCREF(type.desc);
  mapExp->type = type;
  assert(listExp != NULL ? listExp->kind == MAP_LIST_KIND : 1);
  mapExp->listExp = (ListExpr *)listExp;
  return (Expr *)mapExp;
}

Expr *Expr_From_Anony(Vector *args, TypeDesc *ret, Vector *body)
{
  AnonyExpr *anonyExp = kmalloc(sizeof(AnonyExpr));
  anonyExp->kind = ANONY_FUNC_KIND;
  anonyExp->args = args;
  TYPE_INCREF(ret);
  anonyExp->ret = ret;
  anonyExp->body = body;
  return (Expr *)anonyExp;
}

#endif

Expr *Expr_From_ConstDecl(Ident id, PosType type, Expr *exp)
{
  VarDeclExpr *varExpr = kmalloc(sizeof(VarDeclExpr));
  varExpr->kind = CONST_KIND;
  varExpr->id = id;
  varExpr->type = type;
  varExpr->exp = exp;
  return (Expr *)varExpr;
}

Expr *Expr_From_VarDecl(Ident id, PosType type, Expr *exp)
{
  VarDeclExpr *varExpr = kmalloc(sizeof(VarDeclExpr));
  varExpr->kind = VAR_KIND;
  varExpr->id = id;
  varExpr->type = type;
  varExpr->exp = exp;
  return (Expr *)varExpr;
}

Expr *Expr_From_Assign(AssignOpKind op, Expr *left, Expr *right)
{
  AssignExpr *assignExpr = kmalloc(sizeof(AssignExpr));
  assignExpr->kind = ASSIGN_KIND;
  assignExpr->op = op;
  assignExpr->lexp = left;
  assignExpr->rexp = right;
  return (Expr *)assignExpr;
}

Expr *Expr_From_FuncDecl(Ident id, Vector *typeparams, Vector *args,
                         PosType ret, Vector *stmts)
{
  FuncDeclExpr *funcExpr = kmalloc(sizeof(FuncDeclExpr));
  funcExpr->kind = FUNC_KIND;
  funcExpr->id = id;
  funcExpr->args = args;
  funcExpr->ret = ret;
  funcExpr->body = stmts;
  return (Expr *)funcExpr;
}

Expr *expr_from_return(Expr *exp)
{
  ReturnExpr *retExpr = kmalloc(sizeof(ReturnExpr));
  retExpr->kind = RETURN_KIND;
  retExpr->exp = exp;
  return (Expr *)retExpr;
}

static void free_unary_expr(Expr *exp)
{
  UnaryExpr *unExp = (UnaryExpr *)exp;
  free_expr(unExp->exp);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_binary_expr(Expr *exp)
{
  BinaryExpr *binExp = (BinaryExpr *)exp;
  free_expr(binExp->lexp);
  free_expr(binExp->rexp);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_attribute_expr(Expr *exp)
{
  AttributeExpr *attrExp = (AttributeExpr *)exp;
  free_expr(attrExp->lexp);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_subscript_expr(Expr *exp)
{
  SubScriptExpr *subExp = (SubScriptExpr *)exp;
  free_expr(subExp->index);
  free_expr(subExp->lexp);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_call_expr(Expr *exp)
{
  CallExpr *callExp = (CallExpr *)exp;
  //free_exprlist(callExp->args);
  free_expr(callExp->lexp);
  free_expr(exp);
}

static void free_slice_expr(Expr *exp)
{
  SliceExpr *sliceExp = (SliceExpr *)exp;
  free_expr(sliceExp->start);
  free_expr(sliceExp->end);
  free_expr(sliceExp->lexp);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_list_expr(Expr *exp)
{
  if (exp == NULL)
    return;
  ListExpr *listExp = (ListExpr *)exp;
  //free_exprlist(listExp->vec);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_mapentry_expr(Expr *exp)
{
  MapEntryExpr *entExp = (MapEntryExpr *)exp;
  free_expr(entExp->key);
  free_expr(entExp->val);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_array_expr(Expr *exp)
{
  ArrayExpr *arrayExp = (ArrayExpr *)exp;
  //free_exprlist(arrayExp->dims);
  TYPE_DECREF(arrayExp->base.desc);
  //free_list_expr((Expr *)arrayExp->listExp);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_map_expr(Expr *exp)
{
  MapExpr *mapExp = (MapExpr *)exp;
  TYPE_DECREF(mapExp->type.desc);
  //free_list_expr((Expr *)mapExp->listExp);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_anony_expr(Expr *exp)
{
  AnonyExpr *anonyExp = (AnonyExpr *)exp;
  //Vector_Free_Self(anonyExp->args);
  TYPE_DECREF(anonyExp->ret);
  //vector_free(anonyExp->body, Free_Stmt_Func, NULL);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_vardecl_expr(Expr *exp)
{
  VarDeclExpr *varExpr = (VarDeclExpr *)exp;
  TYPE_DECREF(varExpr->type.desc);
  free_expr(varExpr->exp);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_assign_expr(Expr *exp)
{
  AssignExpr *assignExpr = (AssignExpr *)exp;
  free_expr(assignExpr->lexp);
  free_expr(assignExpr->rexp);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_funcdecl_epxr(Expr *exp)
{
  FuncDeclExpr *funcExpr = (FuncDeclExpr *)exp;
  //Vector_Free_Self(funcExpr->args);
  TYPE_DECREF(funcExpr->ret.desc);
  //Vector_Free(funcExpr->body, Free_Stmt_Func, NULL);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void free_return_epxr(Expr *exp)
{
  ReturnExpr *retExpr = (ReturnExpr *)exp;
  free_expr(retExpr->exp);
  TYPE_DECREF(exp->desc);
  kfree(exp);
}

static void (*free_expr_funcs[])(Expr *) = {
  NULL,                                  /* INVALID          */
  free_expr,                             /* NIL_KIND         */
  free_expr,                             /* SELF_KIND        */
  free_expr,                             /* SUPER_KIND       */
  free_expr,                             /* LITERAL_KIND     */
  free_expr,                             /* ID_KIND          */
  free_unary_expr,                       /* UNARY_KIND       */
  free_binary_expr,                      /* BINARY_KIND      */
  free_attribute_expr,                   /* ATTRIBUTE_KIND   */
  free_subscript_expr,                   /* SUBSCRIPT_KIND   */
  free_call_expr,                        /* CALL_KIND        */
  free_slice_expr,                       /* SLICE_KIND       */
  free_list_expr,                        /* ARRAY_LIST_KIND  */
  free_list_expr,                        /* MAP_LIST_KIND    */
  free_mapentry_expr,                    /* MAP_ENTRY_KIND   */
  free_array_expr,                       /* ARRAY_KIND       */
  free_map_expr,                         /* MAP_KIND         */
  free_anony_expr,                       /* ANONY_FUNC_KIND  */
  NULL,                                  /* IMPORT_KIND      */
  free_vardecl_expr,                     /* CONST_KIND       */
  free_vardecl_expr,                     /* VAR_KIND         */
  free_assign_expr,                      /* ASSIGN_KIND      */
  free_funcdecl_epxr,                    /* FUNC_KIND        */
  free_return_epxr,                      /* RETURN_KIND      */
};

void free_expr(Expr *exp)
{
  if (exp == NULL)
    return;
  panic(exp->kind < 1 || exp->kind >= COUNT_OF(free_expr_funcs),
        "invalid expression kind %d", exp->kind);
  void (*fn)(Expr *) = free_expr_funcs[exp->kind];
  panic(!fn, "null pointer");
  fn(exp);
}

int expr_maybe_stored(Expr *exp)
{
  if (exp->kind == ID_KIND ||
      exp->kind == ATTRIBUTE_KIND ||
      exp->kind == SUBSCRIPT_KIND)
    return 1;
  else
    return 0;
}

#if 0

static void free_expr_func(void *item, void *arg)
{
  Free_Expr(item);
}

static inline void free_exprlist(Vector *vec)
{
  vector_free(vec, free_expr_func, NULL);
}

static void free_expr_stmt(Stmt *stmt)
{
  ExprStmt *expStmt = (ExprStmt *)stmt;
  Free_Expr(expStmt->exp);
  kfree(stmt);
}

static void free_list_stmt(Stmt *stmt)
{
  ListStmt *listStmt = (ListStmt *)stmt;
  Vector_Free(listStmt->vec, Free_Stmt_Func, NULL);
  kfree(stmt);
}

static void free_typedesc_func(void *item, void *arg)
{
  TYPE_DECREF(item);
}

static void free_klass_stmt(Stmt *stmt)
{
  KlassStmt *klsStmt = (KlassStmt *)stmt;
  Vector_Free(klsStmt->super, free_typedesc_func, NULL);
  Vector_Free_Self(klsStmt->body);
  kfree(stmt);
}

static void free_enum_stmt(Stmt *stmt)
{
  EnumStmt *eStmt = (EnumStmt *)stmt;
  // no free, eStmt->paratypes is assigned to paratypes in EnumSymbol
  Vector_Free(eStmt->body, Free_Stmt_Func, NULL);
  kfree(stmt);
}

static void free_eval_stmt(Stmt *stmt)
{
  EnumValStmt *evStmt = (EnumValStmt *)stmt;
  //Vector_Free(evStmt->types, free_typedesc_func, NULL);
  Free_Expr(evStmt->exp);
  kfree(evStmt);
}


static void (*__free_stmt_funcs[])(Stmt *) = {
  NULL,                     /* INVALID         */
  free_vardecl_stmt,        /* CONST_KIND      */
  free_vardecl_stmt,        /* VAR_KIND        */
  NULL,                     /* TUPLE_KIND      */
  free_funcdecl_stmt,       /* FUNC_KIND       */
  free_funcdecl_stmt,       /* PROTO_KIND      */
  free_klass_stmt,          /* CLASS_KIND      */
  free_klass_stmt,          /* TRAIT_KIND      */
  free_enum_stmt,           /* ENUM_KIND       */
  free_eval_stmt,           /* ENUM_VALUE_KIND */
  free_expr_stmt,           /* EXPR_KIND       */
  free_assign_stmt,         /* ASSIGN_KIND     */
  free_return_stmt,         /* RETURN_KIND     */
  NULL,                     /* BREAK_KIND      */
  NULL,                     /* CONTINUE_KIND   */
  free_list_stmt,           /* LIST_KIND       */
};

void Free_Stmt_Func(void *item, void *arg)
{
  Stmt *stmt = item;
  assert(stmt->kind >= 1 && stmt->kind < nr_elts(__free_stmt_funcs));
  void (*__free_stmt_func)(Stmt *) = __free_stmt_funcs[stmt->kind];
  __free_stmt_func(stmt);
}

Stmt *Stmt_From_ProtoDecl(Ident id, Vector *args, PosType ret)
{
  FuncDeclStmt *protoStmt = kmalloc(sizeof(FuncDeclStmt));
  protoStmt->kind = PROTO_KIND;
  protoStmt->id = id;
  protoStmt->args = args;
  protoStmt->ret = ret;
  return (Stmt *)protoStmt;
}

Stmt *Stmt_From_Expr(Expr *exp)
{
  ExprStmt *expStmt = kmalloc(sizeof(ExprStmt));
  expStmt->kind = EXPR_KIND;
  expStmt->exp = exp;
  return (Stmt *)expStmt;
}

Stmt *Stmt_From_List(Vector *vec)
{
  ListStmt *listStmt = kmalloc(sizeof(ListStmt));
  listStmt->kind = LIST_KIND;
  listStmt->vec = vec;
  return (Stmt *)listStmt;
}

Stmt *Stmt_From_Class(Ident id, Vector *types, Vector *super, Vector *body)
{
  KlassStmt *clsStmt = kmalloc(sizeof(KlassStmt));
  clsStmt->kind = CLASS_KIND;
  clsStmt->id = id;
  clsStmt->typeparas = types;
  clsStmt->super = super;
  clsStmt->body = body;
  return (Stmt *)clsStmt;
}

Stmt *Stmt_From_Trait(Ident id, Vector *types, Vector *super, Vector *body)
{
  KlassStmt *traitStmt = kmalloc(sizeof(KlassStmt));
  traitStmt->kind = TRAIT_KIND;
  traitStmt->id = id;
  traitStmt->typeparas = types;
  traitStmt->super = super;
  traitStmt->body = body;
  return (Stmt *)traitStmt;
}

Stmt *New_EnumValue(Ident id, Vector *types, Expr *exp)
{
  EnumValStmt *evalStmt = kmalloc(sizeof(EnumValStmt));
  evalStmt->kind = ENUM_VALUE_KIND;
  evalStmt->id = id;
  evalStmt->types = types;
  evalStmt->exp = exp;
  return (Stmt *)evalStmt;
}

Stmt *Stmt_From_Enum(Ident id, Vector *types, Vector *body)
{
  EnumStmt *enumStmt = kmalloc(sizeof(EnumStmt));
  enumStmt->kind = ENUM_KIND;
  enumStmt->id = id;
  enumStmt->typeparas = types;
  enumStmt->body = body;
  return (Stmt *)enumStmt;
}


static int file_exist(char *path)
{
  struct stat attr;
  if (stat(path, &attr) == - 1)
    return 0;

  if (!S_ISREG(attr.st_mode))
    return 0;

  return 1;
}

static int dir_exist(char *path)
{
  struct stat attr;
  if (stat(path, &attr) == - 1)
    return 0;

  if (!S_ISDIR(attr.st_mode))
    return 0;

  return 1;
}

static int dir_later_file(char *dirpath, char *filepath)
{
  char *path;
  time_t mtime = {0};
  struct stat attr;
  DIR *dir = opendir(dirpath);
  struct dirent *dent;
  while ((dent = readdir(dir))) {
    if (!strcmp(dent->d_name, ".") ||
        !strcmp(dent->d_name, ".."))
      continue;
      path = AtomString_Format("#/#", dirpath, dent->d_name);
    if (stat(path, &attr) == -1)
      continue;
    Log_Printf("%s:  %s", path, ctime(&attr.st_mtime));
    if (difftime(mtime, attr.st_mtime) < 0)
      mtime = attr.st_mtime;
  }
  closedir(dir);

  if (stat(filepath, &attr) != -1) {
    Log_Printf("%s:  %s", filepath, ctime(&attr.st_mtime));
    return difftime(mtime, attr.st_mtime);
  }

  return 1;
}

static inline Symbol *__in_extstbl(ParserState *ps, char *name)
{
  if (ps->extstbl == NULL)
    return NULL;
  return STable_Get(ps->extstbl, name);
}

static inline RefSymbol *__is_inextdots(ParserState *ps, char *name)
{
  if (ps->extdots == NULL)
    return NULL;
  return (RefSymbol *)STable_Get(ps->extdots, name);
}

struct load_dotsym_param {
  ParserState *ps;
  char *path;
  Position pos;
};

static void load_dotsym_func(Symbol *sym, void *arg)
{
  struct load_dotsym_param *param = arg;
  ParserState *ps = param->ps;

  if (ps->errnum > MAX_ERRORS)
    return;

  Symbol *pkgSym = __in_extstbl(ps, sym->name);
  if (pkgSym != NULL) {
    Syntax_Error(&param->pos, "'%s' redeclared during import '%s',\n"
                 "\tprevious declaration at %s:%d:%d", sym->name, param->path,
                 pkgSym->filename, pkgSym->pos.row, pkgSym->pos.col);
    return;
  }

  RefSymbol *dot = STable_Add_Reference(ps->extdots, sym->name);
  if (dot == NULL) {
    dot = (RefSymbol *)STable_Get(ps->extdots, sym->name);
    Syntax_Error(&param->pos, "'%s' redeclared during import '%s',\n"
                 "\tprevious declaration during import '%s' at %s:%d:%d",
                 sym->name, param->path, dot->path, dot->filename,
                 dot->pos.row, dot->pos.col);
  } else {
    dot->sym = sym;
    dot->path = param->path;
    dot->filename = ps->filename;
    dot->pos = param->pos;
  }
}

static Import *new_import(Ident *id, Ident *path)
{
  Import *import = kmalloc(sizeof(Import));
  if (id != NULL) {
    import->id = id->name;
    import->idpos = id->pos;
  }
  import->path = path->name;
  import->pathpos = path->pos;
  return import;
}

static inline void free_import(Import *import)
{
  kfree(import);
}

void Parse_Imports(ParserState *ps)
{
  char *name;
  Symbol *sym;
  Import *import;
  Vector_ForEach(import, &ps->imports) {
    Package *pkg = Find_Package(import->path);
    assert(pkg != NULL);

    if (import->id == NULL) {
      /* use package's name as name */
      name = pkg->pkgname;
    } else {
      /* use id as name */
      name = import->id;
    }

    if (name[0] != '.') {
      RefSymbol *dot = __is_inextdots(ps, name);
      if (dot != NULL) {
        Syntax_Error(&import->pathpos,
                     "'%s' redeclared as imported package name,\n"
                     "\tprevious declaration during import '%s' at %s:%d:%d",
                     name, dot->path,
                     dot->filename, dot->pos.row, dot->pos.col);
      } else {
        PkgSymbol *sym = STable_Add_Package(ps->extstbl, name);
        if (sym != NULL) {
          Log_Debug("add package '%s <- %s' successfully", name, import->path);
          sym->pkg = pkg;
          sym->filename = ps->filename;
          sym->pos = (import->id == NULL) ? import->pathpos : import->idpos;
        } else {
          Symbol *sym = STable_Get(ps->extstbl, name);
          Syntax_Error(&import->pathpos,
                       "'%s' redeclared as imported package name,\n"
                       "\tprevious declaration at %s:%d:%d", name,
                       sym->filename, sym->pos.row, sym->pos.col);
        }
      }
    } else {
      /* load all symbols to current module */
      struct load_dotsym_param param = {ps, import->path, import->pathpos};
      STable_Visit(pkg->stbl, load_dotsym_func, &param);
    }

    free_import(import);
  }
}

void Parser_New_Import(ParserState *ps, Ident *id, Ident *path)
{
  if (!strcmp(path->name, ps->grp->pkg->path)) {
    Syntax_Error(&path->pos, "imported self-package");
    return;
  }

  Import *import;
  Vector_ForEach(import, &ps->imports) {
    if (!strcmp(import->path, path->name)) {
      Syntax_Error(&path->pos, "'%s' imported duplicately,\n"
                   "\tprevious import at %s:%d:%d",
                   path->name, ps->filename,
                   import->pathpos.row, import->pathpos.col);
      return;
    }

    if ((id != NULL && id->name[0] != '.') && (import->id != NULL)) {
      if (!strcmp(import->id, id->name)) {
        Syntax_Error(&id->pos, "'%s' redeclared as imported package name,\n"
                     "\tprevious declaration during import at %s:%d:%d",
                     id->name, ps->filename,
                     import->idpos.row, import->idpos.col);
        return;
      }
    }
  }

  Package *pkg = Find_Package(path->name);
  if (pkg == NULL) {
    /* find source path */
    Options *opts = &options;
    char *dir = path->name;
    if (opts->srcpath != NULL)
      dir = AtomString_Format("#/#", opts->srcpath, dir);

    if (dir_exist(dir)) {
      /* find .klc in source path */
      char *klc = AtomString_Format("#.klc", dir);

      if (!file_exist(klc)) {
        Log_Debug("compile package '%s' for it's not exist", dir);
        Add_ParserGroup(path->name);
      } else {
        if (dir_later_file(dir, klc) > 0) {
          Log_Debug("compile package '%s' for it's later", dir);
          Add_ParserGroup(path->name);
        } else {
          /* load image */
          Log_Debug("load package '%s'", dir);
          pkg = New_Package(path->name);
          STable_From_Image(klc, &pkg->pkgname, &pkg->stbl);
        }
      }
    } else {
      /* find .klc in search pathes */
      int found = 0;
      char *klc;
      char *pre;
      Vector_ForEach(pre, &opts->pathes) {
        klc = AtomString_Format("#/#.klc", pre, path->name);
        if (file_exist(klc)) {
          Log_Debug("load package '%s/%s'", pre, path->name);
          pkg = New_Package(path->name);
          STable_From_Image(klc, &pkg->pkgname, &pkg->stbl);
          found = 1;
          break;
        }
      }
      if (!found) {
        Syntax_Error(&path->pos, "cannot find '%s' package", path->name);
        return;
      }
    }
  }

  Vector_Append(&ps->imports, new_import(id, path));
}

void CheckConflictWithExternal(ParserState *ps)
{
  Symbol *sym;
  Symbol *item;
  Vector_ForEach(item, &ps->symbols) {
    sym = __in_extstbl(ps, item->name);
    if (sym != NULL) {
      Syntax_Error(&item->pos, "'%s' redeclared,\n"
                   "\tprevious declaration at %s:%d:%d", item->name,
                   sym->filename, sym->pos.row, sym->pos.col);
    }
  }
}

#endif

#if 0
static inline void __add_stmt(ParserState *ps, Stmt *stmt)
{
  Vector_Append(&ps->stmts, stmt);
}

static inline void __add_varstmt(ParserState *ps, Stmt *stmt)
{
  Vector_Append(&ps->varstmts, stmt);
}

static void __new_var(ParserState *ps, Ident *id, TypeDesc *desc)
{
  ParserUnit *u = ps->u;
  VarSymbol *sym = STable_Add_Var(u->stbl, id->name, desc);

  if (sym != NULL) {
    Log_Debug("add var '%s' successfully", id->name);
    sym->filename = ps->filename;
    sym->pos = id->pos;
    Vector_Append(&ps->symbols, sym);
  } else {
    Symbol *sym = STable_Get(u->stbl, id->name);
    Syntax_Error(&id->pos, "'%s' redeclared,\n"
                 "\tprevious declaration at %s:%d:%d", id->name,
                 sym->filename, sym->pos.row, sym->pos.col);
  }
}

void Parser_New_Var(ParserState *ps, Stmt *stmt)
{
  if (stmt == NULL)
    return;
  VarDeclStmt *varStmt = (VarDeclStmt *)stmt;
  __add_varstmt(ps, stmt);
  __new_var(ps, &varStmt->id, varStmt->type.desc);
}

static void __new_const(ParserState *ps, Ident *id, TypeDesc *desc)
{
  ParserUnit *u = ps->u;
  VarSymbol *sym = STable_Add_Const(u->stbl, id->name, desc);
  if (sym != NULL) {
    Log_Debug("add const '%s' successfully", id->name);
    sym->filename = ps->filename;
    sym->pos = id->pos;
    Vector_Append(&ps->symbols, sym);
  } else {
    Symbol *sym = STable_Get(u->stbl, id->name);
    Syntax_Error(&id->pos, "'%s' redeclared,\n"
                 "\tprevious declaration at %s:%d:%d", id->name,
                 sym->filename, sym->pos.row, sym->pos.col);
  }
}

void parser_new_const(ParserState *ps, Expr *exp)
{
  if (exp == NULL)
    return;
  VarDeclExpr *varExpr = (VarDeclExpr *)exp;
  __add_varstmt(ps, stmt);
  __new_const(ps, &varStmt->id, varStmt->type.desc);
}

#endif

#if 0

static TypeDesc *__get_proto(Vector *idtypes, TypeDesc *ret)
{
  Vector *para = Vector_Capacity(Vector_Size(idtypes));
  IdType *idType;
  Vector_ForEach(idType, idtypes) {
    TYPE_INCREF(idType->type.desc);
    Vector_Append(para, idType->type.desc);
  }
  return TypeDesc_New_Proto(para, ret);
}

TypeDesc *getparatype(Vector *paratypes, TypeDesc *type)
{
  if (type->kind == TYPE_KLASS) {
    KlassDesc *klazz = (KlassDesc *)type;
    if (klazz->typeparas == NULL) {
      if (klazz->path.str == NULL) {
        Vector *bases = NULL;
        TypePara *para;
        Vector_ForEach(para, paratypes) {
          if (!strcmp(para->id.name, klazz->type.str)) {
            Log_Debug("type '%s' is a type parameter", para->id.name);
            bases = para->types;
            break;
          }
        }
        return TypeDesc_New_ParaRef(klazz->type.str, bases);
      }
    } else {
      //FIXME
    }
  }
  /* not a parameter type */
  return NULL;
}

static void update_proto(ParserState *ps, ClassSymbol *clsSym, Stmt *stmt)
{
  FuncDeclStmt *funcStmt = (FuncDeclStmt *)stmt;
  TypeDesc *type;

  IdType *idtype;
  Vector_ForEach(idtype, funcStmt->args) {
    type = getparatype(clsSym->typeparas, idtype->type.desc);
    if (type != NULL) {
      TYPE_DECREF(idtype->type.desc);
      idtype->type.desc = type;
    }
  }

  PosType *postype = &funcStmt->ret;
  if (postype->desc != NULL) {
    type = getparatype(clsSym->typeparas, postype->desc);
    if (type != NULL) {
      TYPE_DECREF(postype->desc);
      postype->desc = type;
    }
  }
}

static void __parse_funcdecl(ParserState *ps, Stmt *stmt)
{
  ParserUnit *u = ps->u;
  FuncDeclStmt *funcStmt = (FuncDeclStmt *)stmt;
  assert(funcStmt->kind == FUNC_KIND);
  char *name = funcStmt->id.name;
  if (!strcmp(name, "__init__")) {
    if (funcStmt->args != NULL || funcStmt->ret.desc != NULL) {
      Syntax_Error(&funcStmt->id.pos, "__init__ needs no args and no return");
      return;
    }
  }

  Symbol *sym;
  TypeDesc *proto = __get_proto(funcStmt->args, funcStmt->ret.desc);
  sym = (Symbol *)STable_Add_Func(u->stbl, name, proto);
  if (sym != NULL) {
    Log_Debug("add func '%s' successfully", name);
    sym->filename = ps->filename;
    sym->pos = funcStmt->id.pos;
    Vector_Append(&ps->symbols, sym);
  } else {
    sym = STable_Get(u->stbl, name);
    Syntax_Error(&funcStmt->id.pos, "'%s' redeclared,\n"
                  "\tprevious declaration at %s:%d:%d", name,
                  sym->filename, sym->pos.row, sym->pos.col);
  }

  TYPE_DECREF(proto);
}

static void __parse_methdecl(ParserState *ps, ClassSymbol *clsSym, Stmt *stmt)
{
  ParserUnit *u = ps->u;
  FuncDeclStmt *funcStmt = (FuncDeclStmt *)stmt;
  char *name = funcStmt->id.name;
  if (!strcmp(name, "__init__")) {
    if (funcStmt->ret.desc != NULL) {
      /* __init__ of class must not have return value */
      Syntax_Error(&funcStmt->id.pos, "__init__ needs no return");
      return;
    }
  }

  /* update type parameters */
  update_proto(ps, clsSym, stmt);

  Symbol *sym;
  TypeDesc *proto = __get_proto(funcStmt->args, funcStmt->ret.desc);
  sym = (Symbol *)STable_Add_Func(u->stbl, name, proto);
  if (sym != NULL) {
    Log_Debug("add func '%s' successfully", name);
    sym->filename = ps->filename;
    sym->pos = funcStmt->id.pos;
    Vector_Append(&ps->symbols, sym);
  } else {
    sym = STable_Get(u->stbl, name);
    Syntax_Error(&funcStmt->id.pos, "'%s' redeclared,\n"
                  "\tprevious declaration at %s:%d:%d", name,
                  sym->filename, sym->pos.row, sym->pos.col);
  }

  TYPE_DECREF(proto);
}

void parser_new_func(ParserState *ps, Expr *expr)
{
  if (expr == NULL)
    return;
  __add_stmt(ps, stmt);
  __parse_funcdecl(ps, stmt);
}

static void __parse_protodecl(ParserState *ps, Stmt *stmt)
{
  ParserUnit *u = ps->u;
  FuncDeclStmt *funcStmt = (FuncDeclStmt *)stmt;
  assert(funcStmt->kind == PROTO_KIND);
  char *name = funcStmt->id.name;
  Symbol *sym;
  TypeDesc *proto = __get_proto(funcStmt->args, funcStmt->ret.desc);

  if (funcStmt->native) {
    sym = STable_Add_NFunc(u->stbl, name, proto);
    if (sym != NULL) {
      Log_Debug("add native func '%s' successfully", name);
      sym->filename = ps->filename;
      sym->pos = funcStmt->id.pos;
      Vector_Append(&ps->symbols, sym);
    } else {
      sym = STable_Get(u->stbl, name);
      Syntax_Error(&funcStmt->id.pos, "'%s' redeclared,\n"
                    "\tprevious declaration at %s:%d:%d", name,
                    sym->filename, sym->pos.row, sym->pos.col);
    }
  } else {
    sym = STable_Add_IFunc(u->stbl, name, proto);
    if (sym != NULL) {
      Log_Debug("add proto '%s' successfully", name);
      sym->filename = ps->filename;
      sym->pos = funcStmt->id.pos;
      Vector_Append(&ps->symbols, sym);
    } else {
      sym = STable_Get(u->stbl, name);
      Syntax_Error(&funcStmt->id.pos, "'%s' redeclared,\n"
                    "\tprevious declaration at %s:%d:%d", name,
                    sym->filename, sym->pos.row, sym->pos.col);
    }
  }

  TYPE_DECREF(proto);
}

void Parser_New_Proto(ParserState *ps, Stmt *stmt)
{
  if (stmt == NULL)
    return;
  __add_stmt(ps, stmt);
  __parse_protodecl(ps, stmt);
}

int __check_paratypes_duplicated(Vector *paratypes)
{
  int j, k;
  TypePara *inner;
  TypePara *para;
  Vector_ForEach(para, paratypes) {
    j = i;
    Vector_ForEach(inner, paratypes) {
      k = i;
      if (k > j && !strcmp(inner->id.name, para->id.name))
        return -1;
    }
  }
  return 0;
}

static Vector *getparatypes(Vector *paratypes, Vector *types)
{
  if (types == NULL)
    return NULL;

  Vector *vec = Vector_Capacity(Vector_Size(types));
  TypeDesc *desc;
  PosType *type;
  Vector_ForEach(type, types) {
    desc = getparatype(paratypes, type->desc);
    if (desc == NULL) {
      desc = type->desc;
      TYPE_INCREF(desc);
    }
    Vector_Append(vec, desc);
  }
  return vec;
}

static void __new_field(ParserState *ps, ClassSymbol *sym, VarDeclStmt *stmt)
{
  Ident *id = &stmt->id;
  TypeDesc *desc = stmt->type.desc;

  Log_Debug("add field:%s", id->name);

  TypeDesc *type = getparatype(sym->typeparas, desc);
  if (type == NULL) {
    //not parameter type, use itself
    type = desc;
  } else {
    //update statement's typedesc
    TYPE_DECREF(desc);
    TYPE_INCREF(type);
    stmt->type.desc = type;
  }
  __new_var(ps, id, type);
}

void Parser_New_Class(ParserState *ps, Stmt *stmt)
{
  if (stmt == NULL)
    return;
  __add_stmt(ps, stmt);

  ParserUnit *u = ps->u;
  KlassStmt *klsStmt = (KlassStmt *)stmt;
  if (__check_paratypes_duplicated(klsStmt->typeparas)) {
    Syntax_Error(&klsStmt->id.pos,
                 "parameter type declaration duplicated", klsStmt->id.name);
    return;
  }
  char *name = klsStmt->id.name;
  ClassSymbol *sym;
  assert(stmt->kind == CLASS_KIND);

  sym = STable_Add_Class(u->stbl, name);
  if (sym != NULL) {
    Log_Debug("add class '%s' successfully", sym->name);
    sym->filename = ps->filename;
    sym->pos = klsStmt->id.pos;
    sym->typeparas = klsStmt->typeparas;
    Vector_Append(&ps->symbols, sym);
  } else {
    Symbol *sym = STable_Get(u->stbl, name);
    Syntax_Error(&klsStmt->id.pos, "'%s' redeclared,\n"
                  "\tprevious declaration at %s:%d:%d", name,
                  sym->filename, sym->pos.row, sym->pos.col);
  }

  Parser_Enter_Scope(ps, SCOPE_CLASS);
  /* ClassSymbol */
  ps->u->sym = (Symbol *)sym;
  ps->u->stbl = sym->stbl;

  Stmt *s;
  Vector_ForEach(s, klsStmt->body) {
    if (s->kind == VAR_KIND) {
      VarDeclStmt *varStmt = (VarDeclStmt *)s;
      __new_field(ps, sym, varStmt);
    } else if (s->kind == FUNC_KIND) {
      __parse_methdecl(ps, sym, s);
    } else if (s->kind == PROTO_KIND) {
      FuncDeclStmt *funcStmt = (FuncDeclStmt *)s;
      assert(funcStmt->native == 1);
      __parse_protodecl(ps, s);
    } else {
      assert(0);
    }
  }

  Parser_Exit_Scope(ps);
}

void Parser_New_Trait(ParserState *ps, Stmt *stmt)
{
  if (stmt == NULL)
    return;
  __add_stmt(ps, stmt);

  ParserUnit *u = ps->u;
  KlassStmt *klsStmt = (KlassStmt *)stmt;
  char *name = klsStmt->id.name;
  ClassSymbol *sym;
  assert(stmt->kind == TRAIT_KIND);

  sym = STable_Add_Trait(u->stbl, name);
  if (sym != NULL) {
    Log_Debug("add trait '%s' successfully", sym->name);
    sym->filename = ps->filename;
    sym->pos = klsStmt->id.pos;
    Vector_Append(&ps->symbols, sym);
  } else {
    Symbol *sym = STable_Get(u->stbl, name);
    Syntax_Error(&klsStmt->id.pos, "'%s' redeclared,\n"
                  "\tprevious declaration at %s:%d:%d", name,
                  sym->filename, sym->pos.row, sym->pos.col);
  }

  Parser_Enter_Scope(ps, SCOPE_CLASS);
  /* ClassSymbol */
  ps->u->sym = (Symbol *)sym;
  ps->u->stbl = sym->stbl;

  Stmt *s;
  Vector_ForEach(s, klsStmt->body) {
    if (s->kind == VAR_KIND) {
      VarDeclStmt *varStmt = (VarDeclStmt *)s;
      __new_var(ps, &varStmt->id, varStmt->type.desc);
    } else if (s->kind == FUNC_KIND) {
      __parse_funcdecl(ps, s);
    } else if (s->kind == PROTO_KIND) {
      __parse_protodecl(ps, s);
    } else {
      assert(0);
    }
  }

  Parser_Exit_Scope(ps);
}

static void add_eval_topkg(ParserState *ps, STable *stbl,
                           Stmt *s, EnumSymbol *e)
{
  ParserUnit *u = ps->u;
  EnumValStmt *evStmt = (EnumValStmt *)s;
  Ident *id = &evStmt->id;
  Vector *types = getparatypes(e->typeparas, evStmt->types);
  EnumValSymbol *evSym = STable_Add_EnumValue(stbl, id->name);
  if (evSym != NULL) {
    Log_Debug("add enum value '%s' in pkg successfully", id->name);
    evSym->filename = ps->filename;
    evSym->pos = id->pos;
    evSym->esym = e;
    evSym->types = types;
    evSym->desc = e->desc;
    TYPE_INCREF(evSym->desc);
    Vector_Append(&ps->symbols, evSym);
  } else {
    Symbol *sym = STable_Get(stbl, id->name);
    Syntax_Error(&id->pos, "'%s' redeclared,\n"
                 "\tprevious declaration at %s:%d:%d", id->name,
                 sym->filename, sym->pos.row, sym->pos.col);
  }
}

static void __new_eval(ParserState *ps, Stmt *s, EnumSymbol *e)
{
  ParserUnit *u = ps->u;
  EnumValStmt *evStmt = (EnumValStmt *)s;
  Ident *id = &evStmt->id;
  Vector *types = getparatypes(e->typeparas, evStmt->types);
  EnumValSymbol *evSym = STable_Add_EnumValue(u->stbl, id->name);
  if (evSym != NULL) {
    Log_Debug("add enum value '%s' successfully", id->name);
    evSym->filename = ps->filename;
    evSym->pos = id->pos;
    evSym->esym = e;
    evSym->types = types;
    evSym->desc = e->desc;
    TYPE_INCREF(evSym->desc);
  } else {
    Symbol *sym = STable_Get(u->stbl, id->name);
    Syntax_Error(&id->pos, "'%s' redeclared,\n"
                 "\tprevious declaration at %s:%d:%d", id->name,
                 sym->filename, sym->pos.row, sym->pos.col);
  }
}

static int __check_enum_decl(ParserState *ps, Stmt *stmt)
{
  assert(stmt->kind == ENUM_KIND);
  EnumStmt *eStmt = (EnumStmt *)stmt;
  Stmt *s;
  // check enum value
  EnumValStmt *evStmt;
  int hasExps = 0;
  int hasTypes = 0;
  Vector_ForEach(s, eStmt->body) {
    if (s->kind == ENUM_VALUE_KIND) {
      evStmt = (EnumValStmt *)s;
      if (evStmt->types != NULL)
        hasTypes++;
      if (evStmt->exp != NULL)
        hasExps++;
      if (evStmt->types != NULL && hasExps) {
        Syntax_Error(&eStmt->id.pos,
                    "mix declaration(associated types and int values)"
                    " by enum '%s'", eStmt->id.name);
        return -1;
      }
      if (evStmt->exp != NULL && hasTypes) {
        Syntax_Error(&eStmt->id.pos,
                    "mix declaration(associated types and int values)"
                    " by enum '%s'", eStmt->id.name);
        return -1;
      }
    } else {
      assert(s->kind == FUNC_KIND);
    }
  }
  // check enum para types
  if (__check_paratypes_duplicated(eStmt->typeparas)) {
    Syntax_Error(&eStmt->id.pos,
                 "parameter type declaration duplicated", eStmt->id.name);
    return -1;
  }
  // no error
  return 0;
}

void Parser_New_Enum(ParserState *ps, Stmt *stmt)
{
  if (stmt == NULL)
    return;

  if (__check_enum_decl(ps, stmt)) {
    Free_Stmt_Func(stmt, NULL);
    return;
  }
  __add_stmt(ps, stmt);

  ParserUnit *u = ps->u;
  EnumStmt *eStmt = (EnumStmt *)stmt;
  char *name = eStmt->id.name;
  assert(stmt->kind == ENUM_KIND);
  EnumSymbol *sym = STable_Add_Enum(u->stbl, name);
  if (sym != NULL) {
    Log_Debug("add enum '%s' successfully", sym->name);
    sym->filename = ps->filename;
    sym->pos = eStmt->id.pos;
    sym->typeparas = eStmt->typeparas;
    Vector_Append(&ps->symbols, sym);
  } else {
    Symbol *sym = STable_Get(u->stbl, name);
    Syntax_Error(&eStmt->id.pos, "'%s' redeclared,\n"
                  "\tprevious declaration at %s:%d:%d", name,
                  sym->filename, sym->pos.row, sym->pos.col);
  }

  Parser_Enter_Scope(ps, SCOPE_CLASS);
  ps->u->sym = (Symbol *)sym;
  ps->u->stbl = sym->stbl;

  Stmt *s;
  Vector_ForEach(s, eStmt->body) {
    if (s->kind == ENUM_VALUE_KIND) {
      __new_eval(ps, s, sym);
      add_eval_topkg(ps, u->stbl, s, sym);
    } else if (s->kind == FUNC_KIND) {
      __parse_funcdecl(ps, s);
    } else {
      assert(0);
    }
  }

  Parser_Exit_Scope(ps);
}

void __Syntax_Error(ParserState *ps, Position *pos, char *fmt, ...)
{
  /* if errors are more than MAX_ERRORS, discard left errors shown */
  if (++ps->errnum > MAX_ERRORS) {
    fprintf(stderr, "%s:%d:%d: \x1b[31merror:\x1b[0m too many errors\n",
            ps->filename, pos->row, pos->col);
    return;
  }

  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "%s:%d:%d: \x1b[31merror:\x1b[0m ",
          ps->filename, pos->row, pos->col);
  vfprintf(stderr, fmt, ap);
  puts(""); /* newline */
  va_end(ap);
}

int Lexer_DoYYInput(ParserState *ps, char *buf, int size, FILE *in)
{
  LineBuffer *linebuf = &ps->line;

  if (linebuf->lineleft <= 0) {
    if (!fgets(linebuf->buf, LINE_MAX_LEN, in)) {
      if (ferror(in))
        clearerr(in);
      return 0;
    }

    linebuf->linelen = strlen(linebuf->buf);
    linebuf->lineleft = linebuf->linelen;
    linebuf->lastpos = linebuf->pos;
    linebuf->lastpos.col += linebuf->len;
    linebuf->len = 0;
    linebuf->pos.row++;
    linebuf->pos.col = 0;
  }

  int sz = min(linebuf->lineleft, size);
  memcpy(buf, linebuf->buf, sz);
  linebuf->lineleft -= sz;
  return sz;
}

void Lexer_DoUserAction(ParserState *ps, char *text)
{
  LineBuffer *linebuf = &ps->line;
  linebuf->pos.col += linebuf->len;
  strncpy(linebuf->token, text, TOKEN_MAX_LEN);
  linebuf->len = strlen(text);
}
#endif
