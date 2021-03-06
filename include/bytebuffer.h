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

#ifndef _KOALA_BYTEBUFFER_H_
#define _KOALA_BYTEBUFFER_H_

#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/* a dynamic byte buffer */
typedef struct bytebuffer {
  /* block size */
  int bsize;
  /* total bytes in the buffer */
  int total;
  /* byteblock's vector */
  Vector vec;
} ByteBuffer;

/* Initialize the byte buffer. */
void bytebuffer_init(ByteBuffer *self, int bsize);

/* Free the byte buffer. */
void bytebuffer_fini(ByteBuffer *self);

/* Write n bytes data into byte buffer. */
int bytebuffer_write(ByteBuffer *self, char *data, int size);

/* Get byte buffer size. */
#define bytebuffer_size(self) (self)->total

/* Write one byte data into byte buffer. */
#define bytebuffer_write_byte(self, data) \
  bytebuffer_write(self, (char *)&data, 1)

/* Write 2 bytes data into byte buffer. */
#define bytebuffer_write_2bytes(self, data) \
  bytebuffer_write(self, (char *)&data, 2)

/* Write 4 bytes data into byte buffer. */
#define bytebuffer_write_4bytes(self, data) \
  bytebuffer_write(self, (char *)&data, 4)

/* Convert byte buffer to an array. */
int bytebuffer_toarr(ByteBuffer *self, char **arr);

#ifdef __cplusplus
}
#endif

#endif /* _KOALA_BYTEBUFFER_H_ */
