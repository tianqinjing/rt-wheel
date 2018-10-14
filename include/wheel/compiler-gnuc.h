/* compiler-gnuc.h - compiler related defination for GNUC */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,12aug18,cfm  writen
*/

#ifndef __COMPILER_GNUC_H__
#define __COMPILER_GNUC_H__

/*
 * dummy - create a non-used variable
 * @type: the type of the new variable
 * @n: the id of this variable, used in the variable name
 */

#define dummy(type, n)  type dummy##n __attribute__ ((unused))

/*
 * __nonreturn - create a non-used variable
 * @type: the type of the new variable
 * @n: the id of this variable, used in the variable name
 */

#define __noreturn__    __attribute__ ((noreturn))

/*
 * likely - a condition is very likely to be true.
 * @exp: the bool exp
 */

#define likely(exp)     __builtin_expect(!!(exp), 1)

/*
 * unlikely - a condition is very unlikely to be true.
 * @exp: the bool exp.
 */

#define unlikely(exp)   __builtin_expect(!!(exp), 0)

/*
 * __clz - counting leading zeros, keil just have __clz.
 */

#define __clz           __builtin_clz

/*
 * _RTW_SECTION - place a symbol in a specific section
 * @name: the section name.
 */

#define _RTW_SECTION(name)  __attribute__ ((__section__ (__CVTSTR (name))))

/*
 * _RTW_SECTION_START - import to the start of a section
 * @name: the section name.
 */

#define _RTW_SECTION_START(name)        \
    __CONCAT (__CONCAT (__, name), __CONCAT (_start, __))

/*
 * _RTW_SECTION_END - import to the end of a section
 * @name: the section name.
 */

#define _RTW_SECTION_END(name)          \
    __CONCAT (__CONCAT (__, name), __CONCAT (_end, __))

/*
 * _RTW_IMPORT_SECTION_START - reference to the start of a section
 * @name: the section name.
 */

#define _RTW_IMPORT_SECTION_START(name) \
    extern char _RTW_SECTION_START (name) []

/*
 * _RTW_IMPORT_SECTION_END - reference to the end of a section
 * @name: the section name.
 */

#define _RTW_IMPORT_SECTION_END(name)   \
    extern char _RTW_SECTION_END (name) []

#endif  /* __COMPILER_GNUC_H__ */

