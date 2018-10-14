/* sync.h - arm-m sync library, including support for atomic */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,11aug18,cfm  writen
*/

#ifndef __AARCH_M_SYNC_H__
#define __AARCH_M_SYNC_H__

#if   defined (__CC_ARM)
#include "sync-keil.h"
#elif defined (__GNUC__)
#include "sync-gnuc.h"
#endif

#endif  /* __AARCH_M_SYNC_H__ */
