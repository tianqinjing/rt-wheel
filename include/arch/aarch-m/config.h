/* config.h - arm-m arch specific config header */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,12aug18,cfm  writen
*/

#ifndef __AARCH_M_CONFIG_H__
#define __AARCH_M_CONFIG_H__

/* macros */

#define RTW_NR_EXCS                 16

// TODO: determain NVIC_PRIO_BITS according to m0, m1 etc.

#define NVIC_PRIO_BITS              2u

#define ALLOC_ALIGN                 8
#define STACK_ALIGN                 8

/* typedefs */

typedef unsigned int pa_t;      /* TODO: */
typedef char       * va_t;      /* TODO: */

#endif  /* __AARCH_M_CONFIG_H__ */

