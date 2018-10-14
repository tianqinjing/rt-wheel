/* mem.h - memory related defination header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#ifndef __MEM_H__
#define __MEM_H__

#include <stddef.h>

#include <wheel/heap.h>

struct phys_mem
    {
    char * start;
    char * end;
    };

extern heap_t          kernel_heap [1];
extern struct phys_mem system_phys_mem [];

#endif /* __MEM_H__ */

