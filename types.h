/*
 * types.h - define basic types and useful micros.
 */
#ifndef _KOALA_TYPES_H_
#define _KOALA_TYPES_H_

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* stddef.h - standard type definitions */
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Basic types */
typedef signed char  sint8;
typedef signed short sint16;
typedef signed int   sint32;
typedef signed long long sint64;

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
#define align_down(val, size) ((val) & (~((size)-1)))
#define align_up(val, size)   (((val)+(size)-1) & ~((size)-1))

/* Count the number of elements in an array. */
#define nr_elts(arr)  ((int)(sizeof(arr) / sizeof((arr)[0])))

/* Get the struct address from its member's address */
#define container_of(ptr, type, member) \
  ((type *)((char *)ptr - offsetof(type, member)))

/* For -Wunused-parameter */
#define UNUSED_PARAMETER(var) ((var) = (var))

#ifdef __cplusplus
}
#endif
#endif /* _KOALA_TYPES_H_ */
