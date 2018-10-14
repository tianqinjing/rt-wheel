/* atomic.h - implement support for atomic type and operations */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,12aug18,cfm  writen
*/

#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include <stdbool.h>

#include <wheel/compiler.h>
#include <wheel/irq.h>

#include <arch/sync.h>

/* macros */

/* typedefs */

typedef struct
    {
    volatile int val;
    } atomic_t;

/* inlines */

/**
 * atomic_get - get the value of a atomic_t object
 * @a: address of the atomic_t object
 *
 * return: value of the atomic_t
 */

static inline int atomic_get (atomic_t * a)
    {
    return a->val;
    }

/**
 * atomic_set - set the value of a atomic_t object
 * @a: address of the atomic_t object
 *
 * return: NA
 */

static inline void atomic_set (atomic_t * a, int v)
    {
    a->val = v;
    }

/**
 * atomic_cas - compare-and-set the value of a atomic_t object
 * @a: address of the atomic_t object
 * @o: old value expected
 * @v: new value to set
 *
 * return: true if the swap is actually executed, FALSE otherwise
 */

static inline bool atomic_cas (atomic_t * a, int o, int v)
    {
    unsigned long flags;
    bool          ret;

    flags = int_lock ();

    if (likely (a->val == o))
        {
        ret = true;
        a->val = v;
        }
    else
        {
        ret = false;
        }

    int_unlock (flags);

    return ret;
    }

static inline bool atomic_add_unless (atomic_t * a, int v, int u)
    {
    unsigned long flags;
    bool          ret;

    flags = int_lock ();

    if (likely (a->val != u))
        {
        ret = true;
        a->val += v;
        }
    else
        {
        ret = false;
        }

    int_unlock (flags);

    return ret;
    }

static inline bool atomic_dec_ifnz (atomic_t * a)
    {
    return atomic_add_unless (a, -1, 0);
    }

#endif  /* __ATOMIC_H__ */

