/* driver.h - driver related definations */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,16sep18,cfm  writen
*/

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <wheel/compiler.h>

/* typedefs */

typedef int (* driver_init_t) (void);

/* macros */

#define DRIVER_SECTION_NAME         driver_init

#define RTW_DRIVER_DEF(init)        \
    driver_init_t __driver_##init _RTW_SECTION (DRIVER_SECTION_NAME) = init

#endif  /* __DRIVER_H__ */

