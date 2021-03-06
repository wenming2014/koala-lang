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

#include <stdio.h>

int main(int argc, char *argv[])
{
  short a, b;
  short r;

  a = 32767;
  b = 10;
  r = a + b;
  if (r < 0)
    printf("%d + %d = %d overflow\n", a, b, r);

  a = -32768;
  b = 10;
  r = a - b;
  if (r > 0)
    printf("%d - %d = %d overflow\n", a, b, r);
  r = b - a;
  if (r < 0)
    printf("%d - %d = %d overflow\n", b, a, r);


  a = 32767;
  b = 32766;
  r = (unsigned short)a + b;
  if (r < 0)
    printf("%d + %d = %d overflow\n", a, b, r);
  return 0;
}
