/* mem.c - memory managment library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#include <wheel/mem.h>
#include <wheel/heap.h>

heap_t kernel_heap [1] = {0,};

/**
 * mem_init - initialize the memory management
 *
 * return: 0 on success, negtive value on error
 */

int mem_init (void)
    {
    struct phys_mem * spm = system_phys_mem;
    int               block_added = 0;

    heap_init (kernel_heap);

    while (spm->end)
        {
        if (heap_add (kernel_heap, spm->start, spm->end - spm->start) == 0)
            {
            block_added++;
            }

        spm++;
        }

    return block_added == 0 ? -1 : 0;
    }

void * malloc (size_t size)
    {
    return heap_alloc (kernel_heap, size);
    }

void free (void * ptr)
    {
    heap_free (ptr);
    }

void * memalign (size_t alignment,  size_t size)
    {
    return heap_alloc_align (kernel_heap, alignment, size);
    }

