/* heap.h - chunk based heap header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#ifndef __HEAP_H__
#define __HEAP_H__

#include <stddef.h>

#include <arch/config.h>

#include <wheel/common.h>
#include <wheel/rbtree.h>
#include <wheel/list.h>

#include <kernel/mutex.h>

#ifndef ALLOC_ALIGN
#define ALLOC_ALIGN             (8)
#endif

#define ALLOC_ALIGN_MASK        (ALLOC_ALIGN - 1)

#define MIN_HEAP_SIZE           (round_up (sizeof (block_t), ALLOC_ALIGN) + \
                                 sizeof (struct chunk) * 3 + ALLOC_ALIGN)

/* TODO: remove this, default NO, or connect with config system */
#define INCLUDE_MEM_STATISTICS

STATIC_ASSERT ((ALLOC_ALIGN != 0));
STATIC_ASSERT ((ALLOC_ALIGN & (sizeof (void *) - 1)) == 0);
STATIC_ASSERT ((ALLOC_ALIGN & ALLOC_ALIGN_MASK) == 0);
STATIC_ASSERT (sizeof (uintptr_t) <= ALLOC_ALIGN);

typedef struct size_node
    {
    rb_node_t          node;    /* 3 x ptr + 1 x int */
    size_t             size;    /* 1 x long */
    dlist_t            list;    /* 2 x ptr */
    } size_node_t;

typedef struct heap    heap_t;
typedef struct chunk   chunk_t;

struct chunk
    {
    size_t             prev_size;
    size_t             size;
    union
        {
        struct
            {
            heap_t   * heap;    /* used for allocated chunks */
            chunk_t  * head;    /* used for allocated chunks */
            };
        dlist_t        node;    /* used for free chunks */
        };
    };

typedef struct block
    {
    dlist_t            node;    /* link into heap->blocks */
    size_t             size;
    chunk_t          * chunk_head;
    } block_t;

#ifdef INCLUDE_MEM_STATISTICS
struct mem_stat
    {
    unsigned int       free_chunks;
    size_t             free_size;
    unsigned int       busy_chunks;
    size_t             busy_size;
    size_t             max_busy_size;
    unsigned long long cum_allocated;
    unsigned long long cum_size_allocated;
    unsigned long long cum_freed;
    unsigned long long cum_size_freed;
    };
#endif

struct heap
    {
    rb_tree_t          sizes;
    dlist_t            blocks;
    mutex_t            mux;

    /*
     * only when chunk->size < (sizeof (chunk_t) + sizeof (size_node_t))
     * size_nodes[n] will be used. And:
     *
     * 1) chunk->size is n * ALLOC_ALIGN
     * 2) chunks with chunk->size == sizeof (chunk_t) will not be inserted
     *
     * so size_nodes[0] for chunk->size = sizeof (chunk_t) + ALLOC_ALIGN
     *    size_nodes[1] for chunk->size = sizeof (chunk_t) + ALLOC_ALIGN * 2
     *    ...
     *
     * so the index in this array should be:
     *
     *    idx = (chunk->size - sizeof (chunk_t)) / ALLOC_ALIGN - 1;
     *
     * as chunks with chunk->size >= sizeof (chunk_t) + sizeof (size_node_t)
     * will not use this array, the max chunk size use this array be:
     *
     *    max_size = sizeof (chunk_t) + sizeof (size_node_t) - ALLOC_ALIGN;
     *
     * and so the max index it uses is:
     *
     *    max = (max_size - sizeof (chunk_t)) / ALLOC_ALIGN - 1;
     *
     * ==>
     *
     *    max = (sizeof (chunk_t) + sizeof (size_node_t) - ALLOC_ALIGN - sizeof (chunk_t)) / ALLOC_ALIGN - 1;
     *
     * ==>
     *
     *    max = (sizeof (size_node_t) - ALLOC_ALIGN) / ALLOC_ALIGN - 1;
     *
     * as the idx is [0 ~ (sizeof (size_node_t) - ALLOC_ALIGN) / ALLOC_ALIGN - 1],
     * the elements of this array is:
     *
     *    elements = (sizeof (size_node_t) - ALLOC_ALIGN) / ALLOC_ALIGN;
     *
     * which is equal to
     *
     *    elements = (sizeof (size_node_t) - 1) / ALLOC_ALIGN;
     */

    size_node_t        size_nodes [(sizeof (size_node_t) - 1) / ALLOC_ALIGN];

#ifdef HEAP_DEBUG
    size_node_t        sn_guard [2];
#endif

#ifdef INCLUDE_MEM_STATISTICS
    struct mem_stat    stat;
#endif
    };

STATIC_ASSERT ((sizeof (chunk_t) & ALLOC_ALIGN_MASK) == 0);

extern char * heap_alloc_align (heap_t * heap, size_t align, size_t bytes);
extern char * heap_alloc       (heap_t * heap, size_t bytes);
extern void   heap_free        (char * mem);
extern char * heap_realloc     (heap_t * heap, char * ptr, size_t size);

extern int    heap_init        (heap_t * heap);
extern int    heap_add         (heap_t * heap, char * buff, size_t size);

#endif  /* __HEAP_H__ */

