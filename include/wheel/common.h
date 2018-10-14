/* common.h - usefull macro definations */

/*
 * Copyright (c) 2012 Fangming Chai
 */

/*
modification history
--------------------
01a,22jul12,cfm  writen
*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>             /* for size_t */

#include "compiler.h"

/*
 * offset_of - caculate the offset for a member in a struct
 * @t: the type of the container struct this is embedded in
 * @m: the name of the member within the struct
 */

#define offset_of(t, m)         ((size_t) &((t *) 0)->m)

/*
 * container_of - cast a member of a structure out to the containing structure
 * @p: the pointer to the member
 * @t: the type of the container struct this is embedded in
 * @m: the name of the member within the struct
 */

#define container_of(p, t, m)   ((t *) (((char *) (p)) - (offset_of (t, m))))

/*
 * round_up - make a value to be aligned forward
 * @x: the value to be aligned
 * @a: the alignment value
 */

#define round_up(x, a)          (((uintptr_t) (x) + ((uintptr_t) (a) - 1)) & ~((uintptr_t) (a) - 1))

/*
 * round_down - make a value to be aligned backward
 * @x: the value to be aligned
 * @a: the alignment value
 */

#define round_down(x, a)        ((x) & ~((uintptr_t) (a) - 1))

/*
 * min - get the min value of two
 * @x: the 1st value
 * @y: the 2nd value
 */

#ifndef min
#define min(x, y)               ((x) < (y) ? (x) : (y))
#endif

/*
 * min - get the max value of two
 * @x: the 1st value
 * @y: the 2nd value
 */

#ifndef max
#define max(x, y)               ((x) > (y) ? (x) : (y))
#endif

/*
 * STATIC_ASSERT - assert at compile time
 * @e: the expression checked
 */

#define STATIC_ASSERT(e)        typedef int __static_assert [(e) ? 1 : -1]

/*
 * ARRAY_SIZE - get the number of elements for an array
 * @a: the array name
 */

#define ARRAY_SIZE(a)           (sizeof (a) / sizeof ((a)[0]))

/*
 * __CONCAT - make a new symbol by join two sub-strings
 * @s1: string 1
 * @s2: string 2
 */

#ifndef __CONCAT
#define __CONCAT_RAW(s1, s2)    s1 ## s2
#define __CONCAT(s1, s2)        __CONCAT_RAW (s1, s2)
#endif

/*
 * __CVTSTR - create a string as string -> "string"
 * @s: the input string
 */

#define __CVTSTR_RAW(s)         #s
#define __CVTSTR(s)             __CVTSTR_RAW (s)

#endif /* __COMMON_H__ */

