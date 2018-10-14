/* arch-dispatch.h - include file dispatcher */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,11aug18,cfm  writen
*/

#include <wheel/common.h>

#define __MKPATH(dir, file)              dir/file
#define __INCFILE(arch, file)           __CVTSTR(__MKPATH(arch, file))

#ifdef _MSC_VER
#define archdir                         msvc
#endif

#ifdef __AARCH_M__
#define archdir                         aarch-m
#endif

#include __INCFILE (archdir, filename)

