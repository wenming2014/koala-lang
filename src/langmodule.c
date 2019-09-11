/*
 * MIT License
 * Copyright (c) 2018 James, https://github.com/zhuguangxiang
 */

#include "koala.h"

static Object *load_module(Object *self, Object *args)
{

}

static Object *new_instance(Object *self, Object *args)
{

}

static Object *is_module(Object *self, Object *args)
{

}

static MethodDef lang_methods[] = {
  {"loadmodule", "s",    "Llang.Module;", load_module },
  {"new",        "s...", "A",             new_instance},
  {"ismodule",   "A",    "z",             is_module   },
  {"isclass",    "A",    "z",             is_module   },
  {"ismethod",   "A",    "z",             is_module   },
  {"isfield",    "A",    "z",             is_module   },
  {NULL}
};

void init_lang_module(void)
{
  Object *m = Module_New("lang");
  Module_Add_Type(m, &any_type);
  Module_Add_Type(m, &byte_type);
  Module_Add_Type(m, &integer_type);
  Module_Add_Type(m, &bool_type);
  Module_Add_Type(m, &string_type);
  Module_Add_Type(m, &char_type);
  Module_Add_Type(m, &float_type);
  Module_Add_Type(m, &array_type);
  Module_Add_Type(m, &tuple_type);
  Module_Add_Type(m, &map_type);
  Module_Add_Type(m, &Field_Type);
  Module_Add_Type(m, &method_type);
  Module_Add_Type(m, &proto_type);
  Module_Add_Type(m, &class_type);
  Module_Add_Type(m, &module_type);
  Module_Add_Type(m, &code_type);
  //Module_Add_FuncDefs(m, lang_methods);
  Module_Install("lang", m);
  OB_DECREF(m);
}

void fini_lang_module(void)
{
  Module_Uninstall("lang");
}