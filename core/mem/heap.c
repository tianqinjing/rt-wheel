/* heap.c - chunk based heap implementation */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#include <string.h>

#include <wheel/heap.h>

#include <kernel/mutex.h>

/**
 * __heap_compare_nk - compare size_node with a key
 */

static int __heap_compare_nk (bi_node_t * n, uintptr_t k)
    {
    size_node_t * sn = container_of (n, size_node_t, node.bin);

    if ((uintptr_t) sn->size == k)
        {
        return 0;
        }

    return (uintptr_t) sn->size > k ? 1 : -1;
    }

/**
 * __heap_compare_nn - compare two size_nodes
 */

static int __heap_compare_nn (bi_node_t * a, bi_node_t * b)
    {
    return __heap_compare_nk (a, (uintptr_t) container_of (b, size_node_t, node.bin)->size);
    }

/**
 * heap_init - initialize a heap struct
 * @heap: the given heap
 *
 * return: 0 on success, negtive value on error
 */

int heap_init (heap_t * heap)
    {
    if (heap == NULL)
        {
        return -1;
        }

#ifdef HEAP_DEBUG
    memset (heap->sn_guard, 0x5c, sizeof (struct size_node) * 2);
#endif

    rb_init (&heap->sizes, __heap_compare_nn, __heap_compare_nk);

    dlist_init (&heap->blocks);

    mutex_init (&heap->mux);

    return 0;
    }

/**
 * __is_free - check if a chunk is free
 * @chunk: the given chunk
 *
 * return: true if the chunk is free, false if not
 */

static inline int __is_free (chunk_t * chunk)
    {
    return chunk != chunk->head;
    }

/**
 * __get_next_chunk - get the next chunk
 * @chunk: the current chunk
 *
 * return: next chunk
 */

static inline chunk_t * __get_next_chunk (chunk_t * chunk)
    {
    return (chunk_t *) (((char *) chunk) + chunk->size);
    }

/**
 * __get_prev_chunk - get the previous chunk
 * @chunk: the current chunk
 *
 * return: previous chunk
 */

static inline chunk_t * __get_prev_chunk (chunk_t * chunk)
    {
    return (chunk_t *) (((char *) chunk) - chunk->prev_size);
    }

/**
 * __get_mem_block - get the memory block in a chunk
 * @chunk: the chunk
 *
 * return: the memory block in the chunk
 */

static inline char * __get_mem_block (chunk_t * chunk)
    {
    return (char *) (chunk + 1);
    }

/**
 * __get_size_node - get the size node for a given size
 * @heap:  the given heap
 * @chunk: the chunk contains the size
 *
 * return: the size node
 */

static inline size_node_t * __get_size_node (heap_t * heap, chunk_t * chunk)
    {
    if (chunk->size >= (sizeof (size_node_t) + sizeof (chunk_t)))
        {
        return (size_node_t *) __get_mem_block (chunk);
        }

    /* please refer to the comments for size_nodes in the struct defination */

    return &heap->size_nodes [(chunk->size - sizeof (chunk_t)) / ALLOC_ALIGN - 1];
    }

/**
 * __put_size_node - put the size node for a given size
 * @heap: the given heap
 * @sn:   the size node to put
 *
 * return: NA
 */

static inline void __put_size_node (heap_t * heap, size_node_t  * sn)
    {

    /* do nothing, refer to __get_size_node */

    return;
    }

/**
 * __new_chunk_for_mb - create a new chunk for a memory block
 * @mem: the memory block
 *
 * return: the new chunk containing the memory block
 */

static inline chunk_t * __new_chunk_for_mb (char * mem)
    {
    return &((chunk_t *) mem) [-1];
    }

/**
 * __get_chunk_for_mb - get the chunk of a memory block by reading mem [-1]
 * @mem: the memory block
 *
 * return: the chunk containing the memory block
 */

static inline chunk_t * __get_chunk_for_mb (char * mem)
    {

    /*
     * there are two possible cases here:
     *
     * 1) chunk->head is just behind the 'mem':
     *
     *    +------------+--------------+
     *    | chunk head | memory block |
     *    +------------+--------------+
     *
     * 2) after an aligned alloc, there are extra memory in the front but the
     *    size is less than sizeof (chunk_t), there will be a pointer
     *    behind 'mem':
     *
     *    +------------+-----------+----------------------+
     *    | chunk head | extra mem | aligned memory block |
     *    +------------+-----------+----------------------+
     *
     *    please refer to the routine of <__carve_head> for this case
     *
     * for both cases, this routine just works fine
     */

    return ((chunk_t **) mem) [-1];
    }

/**
 * __set_chunk_for_mb - set chunk in the front of memory block
 * @mem: the memory block
 *
 * return: NA
 */

static inline void __set_chunk_for_mb (char * mem, chunk_t * chunk)
    {
#ifdef HEAP_DEBUG
    chunk_t ** p = (&chunk->head) + 1;

    while (p < &((chunk_t **) mem) [-1])
        {
        *p++ = (chunk_t *) 0x55aa55aa;
        }
#endif

    ((chunk_t **) mem) [-1] = chunk;
    }

/**
 * __get_bi_node - bi_node create routine for binary search tree
 * @key: the key value
 * @arg: the chunk the node is created for (from)
 *
 * return: the bi_node for the chunk
 */

static bi_node_t * __get_bi_node (uintptr_t key, uintptr_t arg)
    {
    chunk_t     * chunk = (chunk_t *) arg;
    size_node_t * szn   = __get_size_node (chunk->heap, chunk);

    szn->size = key;
    dlist_init (&szn->list);

    return &szn->node.bin;
    }

/**
 * __put_chunk - put a free chunk to a heap
 * @heap:  the given heap
 * @chunk: the chunk to insert
 *
 * return: NA
 */

static inline void __put_chunk (heap_t * heap, chunk_t * chunk)
    {
    rb_node_t   * rbn;
    size_node_t * szn;

#ifdef HEAP_DEBUG
    if (chunk->size & ALLOC_ALIGN_MASK)
        {
        __bug ();
        }

    memset (__get_mem_block (chunk), 0xfe, chunk->size - sizeof (chunk_t));
#endif

    chunk->heap = heap;

    rbn = rb_node_get (&heap->sizes, (uintptr_t) chunk->size, __get_bi_node,
                       (uintptr_t) chunk);

    szn = container_of (rbn, size_node_t, node);

    dlist_add (&szn->list, &chunk->node);

#ifdef INCLUDE_MEM_STATISTICS
    heap->stat.free_chunks++;
    heap->stat.free_size += (chunk->size - sizeof (chunk_t));
#endif
    }

/**
 * __del_chunk - delete a chunk from a heap (for allocating or merging)
 * @heap:  the given heap
 * @chunk: the chunk to delete
 *
 * return: NA
 */

static inline void __del_chunk (heap_t * heap, chunk_t * chunk)
    {
    dlist_t     * prev = chunk->node.prev;
    size_node_t * sn;
    size_node_t * nsn;

    /* do not set 'chunk->heap' and 'chunk->head' it's not necessarily allocating */

#ifdef INCLUDE_MEM_STATISTICS
    heap->stat.free_chunks--;
    heap->stat.free_size -= (chunk->size - sizeof (chunk_t));
#endif

#ifdef HEAP_DEBUG
    if (chunk->size & ALLOC_ALIGN_MASK)
        {
        __bug ();
        }
#endif

    dlist_del (&chunk->node);

    if (dlist_empty (prev))
        {

        /* 'prev' must be &size_node->list */

        sn = container_of (prev, size_node_t, list);

        rb_delete (&heap->sizes, &sn->node);

        __put_size_node (heap, sn);

#ifdef HEAP_DEBUG
    memset (__get_mem_block (chunk), 0xac, chunk->size - sizeof (chunk_t));
#endif

        return;
        }

    /*
     * as most of size nodes are allocated in the memory block of chuck, we may
     * deleting a chunk that is just holding the size node, this can only happen
     * when merging free blocks, because chunk are always added from head, so the
     * chunk holds the size node are always the last one, but we are always get
     * new chunk from head
     *
     * here check this condition and carefully handle it
     */

    /* if size node is from heap->size_nodes [], do nothing */

    if (chunk->size <= sizeof (chunk_t) + sizeof (size_node_t))
        {
        return;
        }

    sn = (size_node_t *) __get_mem_block (chunk);

    /*
     * chunk are always linked to size node from head, and the chunk holds the
     * size node is just the first one added to the list, so it is always the
     * list tail, and the 'next' of tail is just the 'head', if this condition
     * is not true, we must not deleting chunk holding size node
     */

    if (chunk->node.next != &(sn->list))
        {
        return;
        }

    /*
     * the chunk being deleted is just holding the size_node and the size list
     * is not empty now
     *
     * we should find another size node in the size_node->list, every chunk in
     * the list can be used as all of them have the same size, here we just
     * pick the 'prev' chunk, because it is the new 'tail' after deleting this
     * chunk
     */

    /* delete the list head (which is in the size_node) */

    dlist_del (&sn->list);

    /* get a new size_node */

    nsn = (size_node_t *) __get_mem_block (container_of (prev, chunk_t, node));

    /*
     * made the new list head, 'prev' is now the new 'tail' and 'nsn->list' is
     * just the new 'head', so just add 'nsn->list' after 'prev'
     */

    dlist_add (prev, &nsn->list);

    nsn->size = sn->size;

    rb_replace (&heap->sizes, &sn->node, &nsn->node);

#ifdef HEAP_DEBUG
    memset (__get_mem_block (chunk), 0xac, chunk->size - sizeof (chunk_t));
#endif
    }

/**
 * __get_chunk - get a proper chunk in the heap
 * @heap:  the heap to allocate from
 * @bytes: size of memory in bytes to allocate
 *
 * return: the proper chunk found, or NULL if not found
 */

static inline chunk_t * __get_chunk (heap_t * heap, size_t bytes)
    {
    rb_node_t * rbn = rb_find_ge (&heap->sizes, bytes + sizeof (chunk_t));
    chunk_t   * chunk;

    if (rbn == NULL)
        {
        return NULL;
        }

    chunk = container_of (container_of (rbn, size_node_t, node)->list.next,
                          chunk_t, node);

    __del_chunk (heap, chunk);

    chunk->heap = heap;
    chunk->head = chunk;

    return chunk;
    }

/**
 * __get_avail_size - get available memory size in a memory block
 * @mem:   the given memory block, must be begining of the memory block of a chunk
 *
 * return: the size of the available memory size in the block
 */

static inline size_t __get_avail_size (char * mem)
    {
    return (size_t) (((char *) __get_next_chunk (__get_chunk_for_mb (mem))) - mem);
    }

/**
 * heap_add - add a block of memory to a heap
 * @heap: the heap to add in
 * @buff: the memory address to add
 * @size: the memory block size
 *
 * return: 0 on success, negtive value on error
 */

int heap_add (heap_t * heap, char * buff, size_t size)
    {
    int       ret;
    block_t * cb;
    chunk_t * chunk;

    if ((heap == NULL) || (buff == NULL))
        {
        return -1;
        }

    size -= (uintptr_t) buff & ALLOC_ALIGN_MASK;
    size &= ~ALLOC_ALIGN_MASK;
    buff  = (char *) ((uintptr_t) buff & ~ALLOC_ALIGN_MASK);

    if (size < MIN_HEAP_SIZE)
        {
        return -1;
        }

    if (buff > (buff + size))
        {
        return -1;
        }

    if ((ret = mutex_lock (&heap->mux)) != 0)
        {
        return ret;
        }

    /*
     * this block of memory will be initialized as:
     *
     * +----+-----+-----+-----+
     * | CB | ACH | FCH | ACH |
     * +----+-----+-----+-----+
     *
     * CB  - chunk block
     * ACH - allocated chunk header
     * FCH - free chunk header
     *
     * These two ACH at the beginning and ending are guards.
     */

    cb    = (block_t *) (buff);

    chunk = (chunk_t *) (buff + round_up (sizeof (block_t), ALLOC_ALIGN));

    cb->size         = size;
    cb->chunk_head   = chunk;

    dlist_add (&heap->blocks, &cb->node);

    chunk->prev_size = 0;
    chunk->size      = sizeof (chunk_t);
    chunk->heap      = heap;
    chunk->head      = chunk;

    chunk            = __get_next_chunk (chunk);

    chunk->prev_size = sizeof (chunk_t);
    chunk->size      = (size - round_up (sizeof (block_t), ALLOC_ALIGN) -
                        2 * sizeof (chunk_t));

    __put_chunk (heap, chunk);

    chunk            = __get_next_chunk (chunk);

    chunk->prev_size = (size - round_up (sizeof (block_t), ALLOC_ALIGN) -
                        2 * sizeof (chunk_t));
    chunk->size      = sizeof (chunk_t);
    chunk->heap      = heap;
    chunk->head      = chunk;

    (void) mutex_unlock (&heap->mux);

    return 0;
    }

/**
 * __carve_head - round up a block of memory and carve the unused memory at
 *                the beginning
 * @heap:  the heap used
 * @mem:   the memory block to round up and carve
 * @align: the alignment to round up
 *
 * return: the new aligned memory block
 */

static inline char * __carve_head (heap_t * heap, char * mem, size_t align)
    {
    chunk_t * chunk = __get_chunk_for_mb (mem);
    chunk_t * new_chunk;
    char    * new_mem;

#ifdef HEAP_DEBUG
    if ((uintptr_t) mem & ALLOC_ALIGN_MASK)
        {
        __bug ();
        }
#endif

    if (align == ALLOC_ALIGN)
        {
        return mem;
        }

    new_mem = (char *) round_up (mem, align);

    /*
     * the memory in front is not greater than a chunk head size, do not create
     * new chunk, and it is not worse to add logic save such memory block,
     * because for any method to save such memory block we must allocate a chunk
     * head for it, the chunk head itself waste more memory
     */

    if (new_mem - mem <= sizeof (chunk_t))
        {
        __set_chunk_for_mb (new_mem, chunk);
        return new_mem;
        }

    /*
     * there is extra memory ahead the aligned memory and greater than chunk size:
     *
     *                 .----- create a new chunk for allocated and aligned memory
     *                /
     * +-----+-----------+----------------+
     * | FCH | mem ahead | aligned memory |
     * +-----+-----------+----------------+
     * \     \           \
     *  \     \           `-- new_mem (aligned)
     *   \     `------------- mem
     *    `------------------ chunk
     *
     * ==>
     *
     *                 .----- new chunk for allocated and aligned memory
     *                /
     * +-----+-----+-----+----------------+
     * | FCH | mem | ACH | aligned memory |
     * +-----+-----+-----+----------------+
     * \     \           \
     *  \     \           `-- new_mem (aligned)
     *   \     `------------- mem (will be return to heap with it's chunk)
     *    `------------------ chunk (will be return to heap)
     */

    new_chunk            = __new_chunk_for_mb (new_mem);

    new_chunk->prev_size = new_mem - mem;
    new_chunk->size      = chunk->size - (new_mem - mem);
    new_chunk->heap      = heap;
    new_chunk->head      = new_chunk;

    __get_next_chunk (new_chunk)->prev_size = new_chunk->size;

    chunk->size          = new_mem - mem;

    __put_chunk (heap, chunk);

    return new_mem;
    }

/**
 * __carve_tail - carve the extra memory at the ending
 * @heap: the heap used
 * @mem:  the memory block to carve
 * @size: the size of required memory at the beginning
 *
 * return: NA
 */

static inline void __carve_tail (heap_t * heap, char * mem, size_t size)
    {
    chunk_t * chunk = ((chunk_t **) mem) [-1];
    chunk_t * new_chunk;
    chunk_t * next_chunk;
    size_t    usable_size;

    /*
     * there may be small memory gap (less than a chunk head size) betwen
     * __get_mem_block (chunk) and mem, see __carve_head for details
     *
     *                     .-- total size is chunk->size
     *                    /
     *  ..................+....................
     * /                                       \
     * +-----+-----+----------------------------+------------+ ~ ...
     * | ACH | GAP |        usable memory       | next chunk | ~ ...
     * +-----+-----+----------------------------+------------+ ~ ...
     * \            \
     *  \            `-------- mem
     *   `-------------------- chunk
     *
     * so, the usable memory size is (chunk->size - (addr (mem) - (addr) chunk))
     *
     * we are going to carve extra memory block at the end of the usable memory
     */

    usable_size = chunk->size - (mem - (char *) chunk);

    /*
     * the extra memory at the end is not greater than a chunk head size, do not
     * create new chunk, refer to the similar comment in __carve_head for details
     */

    if ((usable_size - size) <= sizeof (chunk_t))
        {
        return;
        }

    /*
     * there is extra memory at the end of usable memory greater than chunk size:
     *
     *                              .----- usable memory
     *                             /
     *                   .........+........
     *                  /                  \
     *                 /      .- size       \
     *                /      /               \
     *               .......+......           \
     *              /              \           \
     * +-----+-----+----------------+-----------+------------+ ~ ...
     * | ACH | GAP | wanted memory  | extra mem | next chunk | ~ ...
     * +-----+-----+----------------+-----------+------------+ ~ ...
     *  \           \                 \
     *   \           \                 `-- extra memory at the end
     *    \           `------------------- mem
     *     `------------------------------ chunk
     *
     * ==>
     *
     * +-----+-----+----------------+-----+-----+------------+ ~ ...
     * | FCH | GAP | wanted memory  | FCH | mem | next chunk | ~ ...
     * +-----+-----+----------------+-----+-----+------------+ ~ ...
     *  \           \                \
     *   \           \                `--- new_chunk, will return to heap
     *    \           `------------------- mem, will return to user
     *     `------------------------------ chunk, allocated
     */

    chunk->size         -= (usable_size - size);

    new_chunk            = (chunk_t *) (mem + size);
    new_chunk->prev_size = chunk->size;
    new_chunk->size      = (usable_size - size);

    next_chunk           = __get_next_chunk (new_chunk);

    /* only when invoked from heap_realloc, this might be true */

    if (unlikely (__is_free (next_chunk)))
        {
        new_chunk->size += next_chunk->size;
        __del_chunk (heap, next_chunk);
        next_chunk = __get_next_chunk (new_chunk);
        }

    next_chunk->prev_size = new_chunk->size;

    __put_chunk (heap, new_chunk);
    }

/**
 * heap_alloc_align - allocate a block of memory from a heap with alignment
 * @heap:  the heap to allocate from
 * @bytes: size of memory in bytes to allocate
 * @align: the expected alignment value
 *
 * return: the allocated memory block or NULL if fail
 */

char * heap_alloc_align (heap_t * heap, size_t align, size_t bytes)
    {
    chunk_t * chunk;
    char    * mem = NULL;

    if (heap == NULL)
        {
        return NULL;
        }

    /* align must be power of 2 */

    if (align & (align - 1))
        {
        return NULL;
        }

    bytes = unlikely (bytes == 0) ? 1 : bytes;

    align = unlikely (align < ALLOC_ALIGN) ? ALLOC_ALIGN : align;

    bytes = round_up (bytes, ALLOC_ALIGN);

    if (mutex_lock (&heap->mux))
        {
        return NULL;
        }

    /* find a big enough memory chunk */

    chunk = __get_chunk (heap, bytes + align - ALLOC_ALIGN);

    if (chunk == NULL)
        {
        goto out;
        }

    mem = __carve_head (heap, __get_mem_block (chunk), align);

    __carve_tail (heap, mem, bytes);

#ifdef INCLUDE_MEM_STATISTICS
    chunk = __get_chunk_for_mb (mem);

    heap->stat.busy_chunks++;
    heap->stat.busy_size += chunk->size;
    heap->stat.cum_allocated++;
    heap->stat.cum_size_allocated += chunk->size;

    if (heap->stat.busy_size > heap->stat.max_busy_size)
        {
        heap->stat.max_busy_size = heap->stat.busy_size;
        }
#endif

out:
    mutex_unlock (&heap->mux);

    return mem;
    }

/**
 * heap_alloc - allocate a block of memory from a heap
 * @heap:  the heap to allocate from
 * @bytes: size of memory in bytes to allocate
 *
 * return: the allocated memory block or NULL if fail
 */

char * heap_alloc (heap_t * heap, size_t bytes)
    {
    return heap_alloc_align (heap, ALLOC_ALIGN, bytes);
    }

/**
 * heap_free - free a block of memory
 * @heap:  the heap to allocate from
 *
 * return: NA
 */

void heap_free (char * mem)
    {
    chunk_t      * chunk;
    heap_t * heap;
    chunk_t      * prev_chunk;
    chunk_t      * next_chunk;

    /* ANSI C - free of NULL is OK */

    if (unlikely (mem == NULL))
        {
        return;
        }

    chunk = __get_chunk_for_mb (mem);
    heap  = chunk->heap;

#ifdef HEAP_DEBUG
    if (chunk->head != chunk)
        {
        __bug ();
        }
#endif

#ifdef INCLUDE_MEM_STATISTICS
    heap->stat.busy_chunks--;
    heap->stat.busy_size -= chunk->size;
    heap->stat.cum_freed++;
    heap->stat.cum_size_freed += chunk->size;
#endif

    prev_chunk = __get_prev_chunk (chunk);
    next_chunk = __get_next_chunk (chunk);

    if (__is_free (prev_chunk))
        {
        __del_chunk (heap, prev_chunk);
        prev_chunk->size += chunk->size;
        chunk = prev_chunk;
        }

    if (__is_free (next_chunk))
        {
        __del_chunk (heap, next_chunk);
        chunk->size += next_chunk->size;

        next_chunk = __get_next_chunk (chunk);
        }

    next_chunk->prev_size = chunk->size;

    __put_chunk (heap, chunk);
    }

/**
 * heap_realloc - realloc memory from a heap
 * @heap: the heap to allocate from
 * @ptr:  the original memory
 * @size: the new size
 *
 * return: the allocated memory block or NULL if fail
 */

char * heap_realloc (heap_t * heap, char * ptr, size_t size)
    {
    size_t align;
    char * mem;
    size_t usable_size;

    if (!ptr)
        {
        return heap_alloc (heap, size);
        }

    if (!size)
        {
        heap_free (ptr);
        return NULL;
        }

    size = round_up (size, ALLOC_ALIGN);

    usable_size = __get_avail_size (ptr);

    if (usable_size >= size)
        {
#ifdef  INCLUDE_MEM_STATISTICS
        size_t     carved_size;
#endif

        __carve_tail (heap, ptr, size);

#ifdef  INCLUDE_MEM_STATISTICS
        carved_size = usable_size - __get_avail_size (ptr);

        if (carved_size == 0)
            {
            return ptr;
            }

        heap->stat.busy_size -= carved_size;
        heap->stat.cum_freed++;
        heap->stat.cum_size_freed += carved_size;
#endif  /* INCLUDE_MEM_STATISTICS */

        return ptr;
        }

    /* try to get the original alignment */

    align = ((size_t) ptr) >> 1;
    align = (align ^ (align - 1)) + 1;

    mem   = heap_alloc_align (heap, align, size);

    memcpy (mem, ptr, __get_avail_size (ptr));

    heap_free (ptr);

    return mem;
    }

