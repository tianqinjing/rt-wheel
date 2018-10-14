/* compiler-gnuc.h - compiler related defination for msvc */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,12aug18,cfm  writen
*/

#ifndef __COMPILER_MSVC_H__
#define __COMPILER_MSVC_H__

/*
 * __nonreturn - create a non-used variable
 * @type: the type of the new variable
 * @n: the id of this variable, used in the variable name
 */

#define __noreturn__

/*
 * likely - a condition is very likely to be true.
 * @exp: the bool exp
 */

#define likely(exp)     (exp)

/*
 * unlikely - a condition is very unlikely to be true.
 * @exp: the bool exp.
 */

#define unlikely(exp)   (exp)

/*
 * built-in find first set
 * https://en.wikipedia.org/wiki/Find_first_set
 */

#define __clz           __lzcnt

#endif  /* __COMPILER_MSVC_H__ */

