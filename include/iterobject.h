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

#ifndef _KOALA_ITER_OBJECT_H_
#define _KOALA_ITER_OBJECT_H_

#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct iterobject {
  OBJECT_HEAD
  Object *ob;
  Object *args;
  Object *step;
} IterObject;

extern TypeObject iter_type;
#define iter_check(ob) (OB_TYPE(ob) == &iter_type)
void init_iter_type(void);
Object *iter_new(Object *ob, Object *args, Object *step);
#define iter_obj(self)  ((IterObject *)self)->ob
#define iter_args(self) ((IterObject *)self)->args
#define iter_step(self) ((IterObject *)self)->step

#ifdef __cplusplus
}
#endif

#endif /* _KOALA_ITER_OBJECT_H_ */
