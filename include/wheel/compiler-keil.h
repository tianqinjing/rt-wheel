/* compiler-gnuc.h - compiler related defination for keil */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,12aug18,cfm  writen
*/

#ifndef __COMPILER_KEIL_H__
#define __COMPILER_KEIL_H__

/*
 * dummy - create a non-used variable
 * @type: the type of the new variable
 * @n: the id of this variable, used in the variable name
 */

#define dummy(type, n)      type dummy##n __attribute__ ((unused))

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

#define likely(exp)         (exp)

/*
 * unlikely - a condition is very unlikely to be true.
 * @exp: the bool exp.
 */

#define unlikely(exp)       (exp)

/*
 * __clz - counting leading zeros, keil just have __clz.
 */

#undef  __clz

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
    __CONCAT (name, $$Base)

/*
 * _RTW_SECTION_END - import to the end of a section
 * @name: the section name.
 */

#define _RTW_SECTION_END(name)          \
    __CONCAT (name, $$Limit)

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

#endif  /* __COMPILER_KEIL_H__ */

