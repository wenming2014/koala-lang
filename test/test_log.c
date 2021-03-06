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

#include <inttypes.h>
#include "log.h"

int main(int argc, char *argv[])
{
  debug("hello, %s:%d", "world", 100);
  warn("this is a warnning: %s,%d", "not a dog", 100);
  error("this is an error: %s,%d", "not a cat", 100);
  debug("hello, koala");
  warn("hello, koala");
  error("hello, koala");
  printf("sizeof(int):%ld\n", sizeof(int));
  printf("sizeof(long):%ld\n", sizeof(long));
  printf("%lx\n", (uintptr_t)&argv);
  //panic(0, "hello, %s", "world");
  return 0;
}
