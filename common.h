/*
 * common.h - define basic types and useful micros.
 */
#ifndef _KOALA_COMMON_H_
#define _KOALA_COMMON_H_

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h> /* stddef.h - standard type definitions */
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Basic types */
typedef signed char  int8;
typedef signed short int16;
typedef signed int   int32;
typedef signed long long int64;

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long long uint64;

typedef float   float32;
typedef double  float64;

/* Get the min(max) one of the two numbers */
#define min(a, b) ((a) > (b) ? (b) : (a))
#define max(a, b) ((a) > (b) ? (a) : (b))

/* Get the aligned value */
#define ALIGN_DOWN(val, size) ((val) & (~((size)-1)))
#define ALIGN_UP(val, size) (((val)+(size)-1) & ~((size)-1))

/* Count the number of elements in an array. */
#define nr_elts(arr)  ((int)(sizeof(arr) / sizeof((arr)[0])))

/* Get the struct address from its member's address */
#define container_of(ptr, type, member) \
  ((type *)((char *)ptr - offsetof(type, member)))

/* For -Wunused-parameter */
#define UNUSED_PARAMETER(var) ((var) = (var))

/* Assert macros for koala */
#define kassert(val, fmt, ...) do { \
  if (!(val)) { \
    printf("[%s:%d]" fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    assert(val); \
  } \
} while (0)

#if __x86_64__
#define ptr2int(ptr, inttype) (inttype)((uint64)(ptr))
#else
#define ptr2int(ptr, inttype) (inttype)((void *)(ptr))
#endif

#ifdef __cplusplus
}
#endif
#endif /* _KOALA_COMMON_H_ */
