/* config.h - arch specific config header */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,11aug18,cfm  writen
*/

#ifndef __ARCH_CONFIG_H__
#define __ARCH_CONFIG_H__

#ifdef  filename
#undef  filename
#endif

#define filename            config.h

#include "arch-dispatch.h"

#undef  filename

#endif  /* __ARCH_CONFIG_H__ */

