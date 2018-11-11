
#include "object.h"
#include "codeobject.h"
#include "hash.h"
#include "log.h"
#include "parser.h"

Symbol *Symbol_New(int kind)
{
  Symbol *sym = calloc(1, sizeof(Symbol));
  Init_HashNode(&sym->hnode, sym);
  Vector_Init(&sym->locvec);
  sym->kind = kind;
  Vector_Init(&sym->traits);
  return sym;
}

void Symbol_Free(Symbol *sym)
{
  if (sym->kind == SYM_VAR) {
    //FIXME:share with struct var
    //TypeDesc_Free(sym->desc);
  } else if (sym->kind == SYM_PROTO) {
    //FIXME: TypeDesc_Free
    //if (sym->desc) TypeDesc_Free(sym->desc);
    if (sym->ob) CodeObject_Free(sym->ob);
    if (sym->ptr) codeblock_free(sym->ptr);
  } else if (sym->kind == SYM_CLASS || sym->kind == SYM_TRAIT) {
    Klass *klazz = sym->ob;
    //FIXME
    //if (klazz->dynamic) {
      //Klass_Free(klazz);
    //} else {
      Fini_Klass(klazz);
    //}
  } else if (sym->kind == SYM_STABLE) {
    STable_Free(sym->ptr);
    free(sym->path);
  } else if (sym->kind == SYM_IPROTO) {
  } else {
    assert(0);
  }

  free(sym->name);
  free(sym);
}

static uint32 symbol_hash(void *k)
{
  Symbol *s = k;
  return hash_uint32(s->nameidx, 0);
}

static int symbol_equal(void *k1, void *k2)
{
  Symbol *s1 = k1;
  Symbol *s2 = k2;
  return s1->nameidx == s2->nameidx;
}

/*-------------------------------------------------------------------------*/

static HashTable *__get_hashtable(STable *stbl)
{
  if (!stbl->htbl) {
    HashInfo hashinfo;
    Init_HashInfo(&hashinfo, symbol_hash, symbol_equal);
    stbl->htbl = HashTable_New(&hashinfo);
  }
  return stbl->htbl;
}

int STable_Init(STable *stbl, AtomTable *atbl)
{
  stbl->htbl = NULL;
  if (!atbl) {
    HashInfo hashinfo;
    Init_HashInfo(&hashinfo, item_hash, item_equal);
    stbl->atbl = AtomTable_New(&hashinfo, ITEM_MAX);
    stbl->flag = 1;
  } else {
    stbl->atbl = atbl;
    stbl->flag = 0;
  }
  //FIXME: start from 1st position, because position-0 is for object
  stbl->varcnt = 1;
  return 0;
}

static void __symbol_free_fn(HashNode *hnode, void *arg)
{
  UNUSED_PARAMETER(arg);
  Symbol *sym = container_of(hnode, Symbol, hnode);
  Symbol_Free(sym);
}

void STable_Fini(STable *stbl)
{
  HashTable_Free(stbl->htbl, __symbol_free_fn, NULL);
  stbl->htbl = NULL;
  if (stbl->flag) AtomTable_Free(stbl->atbl, item_free, NULL);
  stbl->atbl = NULL;
  stbl->varcnt = 1;
}

STable *STable_New(AtomTable *atbl)
{
  STable *stbl = malloc(sizeof(STable));
  STable_Init(stbl, atbl);
  return stbl;
}

void STable_Free(STable *stbl)
{
  if (!stbl) return;
  STable_Fini(stbl);
  free(stbl);
}

/*-------------------------------------------------------------------------*/

Symbol *STable_Add_Var(STable *stbl, char *name, TypeDesc *desc, int bconst)
{
  Symbol *sym = STable_Add_Symbol(stbl, name, SYM_VAR, bconst);
  if (!sym) return NULL;

  int32 idx = -1;
  if (desc) {
    idx = TypeItem_Set(stbl->atbl, desc);
    assert(idx >= 0);
  }
  sym->descidx = idx;
  sym->desc = desc;
  sym->index = stbl->varcnt++;
  return sym;
}

int STable_Update_Symbol(STable *stbl, Symbol *sym, TypeDesc *desc)
{
  int32 idx = TypeItem_Set(stbl->atbl, desc);
  assert(idx >= 0);
  sym->descidx = idx;
  sym->desc = desc;
  return 0;
}

Symbol *STable_Add_Proto(STable *stbl, char *name, TypeDesc *proto)
{
  Symbol *sym = STable_Add_Symbol(stbl, name, SYM_PROTO, 0);
  if (!sym) return NULL;

  int32 idx = ProtoItem_Set(stbl->atbl, proto);
  assert(idx >= 0);
  sym->descidx = idx;
  sym->desc = proto;
  return sym;
}

Symbol *STable_Add_TypeAlias(STable *stbl, char *name, TypeDesc *desc)
{
  Symbol *sym = STable_Add_Symbol(stbl, name, SYM_TYPEALIAS, 0);
  if (!sym) return NULL;

  int32 idx = -1;
  if (desc)
  {
    if (desc->kind == TYPE_PROTO) {
      idx = ProtoItem_Set(stbl->atbl, desc);
      assert(idx >= 0);
    } else {
      idx = TypeItem_Set(stbl->atbl, desc);
      assert(idx >= 0);
    }
  }

  sym->descidx = idx;
  sym->desc = desc;
  return sym;
}

Symbol *STable_Add_Symbol(STable *stbl, char *name, int kind, int bconst)
{
  Symbol *sym = Symbol_New(kind);
  int32 idx = StringItem_Set(stbl->atbl, name);
  assert(idx >= 0);
  sym->nameidx = idx;
  sym->access = SYMBOL_ACCESS(name, bconst);
  if (HashTable_Insert(__get_hashtable(stbl), &sym->hnode) < 0) {
    Symbol_Free(sym);
    return NULL;
  }
  sym->name = strdup(name);
  return sym;
}

Symbol *STable_Get(STable *stbl, char *name)
{
  if (!stbl->htbl) {
    warn("STable is empty");
    return NULL;
  }
  int32 index = StringItem_Get(stbl->atbl, name);
  if (index < 0) return NULL;
  Symbol key = {.nameidx = index};
  return HashTable_Find(__get_hashtable(stbl), &key);
}

/*-------------------------------------------------------------------------*/

struct visit_entry {
  symbolfunc fn;
  void *arg;
};

static void __symbol_visit_fn(HashNode *hnode, void *arg)
{
  struct visit_entry *data = arg;
  Symbol *sym = container_of(hnode, Symbol, hnode);
  data->fn(sym, data->arg);
}

void STable_Traverse(STable *stbl, symbolfunc fn, void *arg)
{
  struct visit_entry data = {fn, arg};
  HashTable_Traverse(stbl->htbl, __symbol_visit_fn, &data);
}

/*-------------------------------------------------------------------------*/

static void __symbol_show_fn(HashNode *hnode, void *arg)
{
  UNUSED_PARAMETER(arg);
  Symbol *sym = container_of(hnode, Symbol, hnode);
  switch (sym->kind) {
    case SYM_VAR: {
      char buf[64];
      /* show's format: "type name desc;" */
      printf("%s %s ", sym->access & ACCESS_CONST ? "const":"var", sym->name);
      Type_ToString(sym->desc, buf);
      printf("%s;\n", buf); /* with newline */
      break;
    }
    case SYM_PROTO: {
      /* show's format: "func name args rets;" */
      printf("func %s", sym->name);
      //FIXME
      //proto_show(sym->proto);
      puts(""); /* with newline */
      break;
    }
    case SYM_CLASS: {
      printf("class %s;\n", sym->name);
      break;
    }
    case SYM_TRAIT: {
      printf("trait %s;\n", sym->name);
      break;
    }
    case SYM_IPROTO: {
      printf("func %s;\n", sym->name);
      break;
    }
    default: {
      assert(0);
      break;
    }
  }
}

void STable_Show(STable *stbl, int detail)
{
  HashTable_Traverse(stbl->htbl, __symbol_show_fn, stbl);
  if (detail) AtomTable_Show(stbl->atbl);
}
