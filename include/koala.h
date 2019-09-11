/*
 * MIT License
 * Copyright (c) 2018 James, https://github.com/zhuguangxiang
 */

#ifndef _KOALA_API_H_
#define _KOALA_API_H_

#include "version.h"
#include "log.h"
#include "memory.h"
#include "atom.h"
#include "eval.h"
#include "fieldobject.h"
#include "methodobject.h"
#include "classobject.h"
#include "intobject.h"
#include "floatobject.h"
#include "stringobject.h"
#include "arrayobject.h"
#include "tupleobject.h"
#include "mapobject.h"
#include "moduleobject.h"
#include "codeobject.h"
#include "fmtmodule.h"
#include "iomodule.h"
#include "osmodule.h"
#include "langmodule.h"
#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

void koala_initialize(void);
void koala_finalize(void);
void Koala_ReadLine(void);
void Koala_Compile(char *path);
void Koala_Run(char *path);

#ifdef __cplusplus
}
#endif

#endif /* _KOALA_API_H_ */
