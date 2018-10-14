/* compiler-gnuc.h - compiler related definations */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,12aug18,cfm  writen
*/

#include <wheel/common.h>       /* as compiler-xxxx.h used __CONCAT */

#if   defined (__CC_ARM)
#include "compiler-keil.h"
#elif defined (__GNUC__)
#include "compiler-gnuc.h"
#elif defined (_MSC_VER)
#include "compiler-msvc.h"
#endif

