/* mutex.c - mutex lock library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,26aug18,cfm  writen
*/

#include <stdint.h>
#include <limits.h>
#include <stdbool.h>

#include <wheel/common.h>

#include <kernel/mutex.h>
#include <kernel/critical.h>

/**
 * mutex_init - initialize a mutex
 * @mutex: the mutex to be initialized
 *
 * return: 0 on success, negtive value on error
 */

int mutex_init (mutex_id mutex)
    {
    if (mutex == NULL)
        {
        return -1;
        }

    mutex->owner   = NULL;

    dlist_init (&mutex->pend_q);

    return 0;
    }

static inline void __try_raise_mutex_prio (mutex_id mutex, uint8_t prio)
    {
    if (mutex->prio <= prio)
        {
        return;
        }

    mutex->prio = prio;

    /* owner's prio may be higher */

    if (prio >= mutex->owner->c_prio)
        {
        return;
        }

    /* owner in ready_q and prio changed */

    if (mutex->owner->status == TASK_STATUS_READY)
        {
        task_ready_q_del (mutex->owner);
        }

    mutex->owner->c_prio = prio;

    if (mutex->owner->status == TASK_STATUS_READY)
        {
        task_ready_q_add (mutex->owner);
        /* must not pend on mutex, just return */
        return;
        }

    if (mutex->owner->mutex_wanted != NULL)
        {
        task_pwait_q_adj (&mutex->owner->mutex_wanted->pend_q, mutex->owner);
        __try_raise_mutex_prio (mutex->owner->mutex_wanted, prio);
        }
    }

static void __recalc_mutex_prio (mutex_id mutex)
    {
    if (dlist_empty (&mutex->pend_q))
        {
        mutex->prio = TASK_PRIO_MAX;
        }
    else
        {
        mutex->prio = container_of (mutex->pend_q.next, task_t, pq_node)->c_prio;
        }
    }

static void __mutex_set_owner (mutex_id mutex, task_id owner)
    {
    mutex->recurse = 1;
    mutex->owner   = owner;

    __recalc_mutex_prio (mutex);

    dlist_add (&owner->mutex_owned, &mutex->node);
    }

static inline bool __recalc_task_prio (task_id task)
    {
    mutex_id  mutex;
    dlist_t * itr;
    uint8_t   prio = task->o_prio;

    if (task->c_prio == prio)
        {
        return false;
        }

    dlist_foreach (itr, &task->mutex_owned)
        {
        mutex = container_of (itr, mutex_t, node);

        if (mutex->prio < prio)
            {
            prio = mutex->prio;
            }
        }

    if (prio == task->c_prio)
        {
        return false;
        }

    if (task->status == TASK_STATUS_READY)
        {
        task_ready_q_del (task);
        task->c_prio = prio;
        task_ready_q_ins (task);
        }
    else
        {
        task->c_prio = prio;
        }

    return true;
    }

static inline void __try_lower_mutex_prio (mutex_id mutex)
    {
    uint8_t prio = mutex->prio;

    __recalc_mutex_prio (mutex);

    if (prio == mutex->prio)
        {
        return;
        }

    if (!mutex->owner)
        {
        return;
        }

    if (!__recalc_task_prio (mutex->owner))
        {
        return;
        }

    if (mutex->owner->mutex_wanted)
        {
        task_pwait_q_adj (&mutex->owner->mutex_wanted->pend_q, mutex->owner);
        __try_lower_mutex_prio (mutex->owner->mutex_wanted);
        }
    }

static void __tick_q_callback_mutex (task_id task)
    {
    mutex_id mutex = task->mutex_wanted;
    uint8_t  prio  = mutex->prio;

    task->mutex_wanted = NULL;

    /* current task is not the highest priority task pending in this mutex */

    if (prio != task->c_prio)
        {
        return;
        }

    /* task->c_prio needless change */

    __try_lower_mutex_prio (mutex);
    }

static int __mutex_lock (uintptr_t arg1, uintptr_t arg2)
    {
    mutex_id     mutex   = (mutex_id) arg1;
    unsigned int timeout = arg2;

    if (current == NULL)
        {
        return 0;       /* pre-kernel, no racing */
        }

    if (mutex->recurse == 0)
        {
        __mutex_set_owner (mutex, current);

        return 0;
        }

    if (mutex->owner == current)
        {
        mutex->recurse++;

        return 0;
        }

    if (timeout == 0)
        {
        return -1;
        }

    current->mutex_wanted = mutex;

    __try_raise_mutex_prio (mutex, current->c_prio);

    task_pwait_q_add (&mutex->pend_q, timeout, __tick_q_callback_mutex);

    return 0;
    }

/**
 * mutex_lock - lock a mutex
 * @mutex:   the mutex to be locked
 *
 * return: 0 on success, negtive value on error
 */

int mutex_lock (mutex_id mutex)
    {
    return do_critical_might_sleep (__mutex_lock, (uintptr_t) mutex, UINT_MAX);
    }

/**
 * mutex_trylock - try to lock a mutex
 * @mutex:   the mutex to be locked
 *
 * return: 0 on success, negtive value on error
 */

int mutex_trylock (mutex_id mutex)
    {
    return do_critical_non_irq (__mutex_lock, (uintptr_t) mutex, 0);
    }

/**
 * mutex_timedlock - lock a mutex with timeout
 * @mutex:   the mutex to be locked
 * @timeout: the max number of waiting ticks
 *
 * return: 0 on success, negtive value on error
 */

int mutex_timedlock (mutex_id mutex, unsigned int timeout)
    {
    return do_critical_might_sleep (__mutex_lock, (uintptr_t) mutex,
                                    (uintptr_t) timeout);
    }

int __mutex_unlock (uintptr_t arg1, uintptr_t arg2)
    {
    mutex_id mutex = (mutex_id) arg1;
    task_id  next_task;

    (void) arg2;

    if (current == NULL)
        {
        return 0;       /* pre-kernel, no racing */
        }

    if (mutex->owner != current)
        {
        return -1;
        }

    if (--mutex->recurse != 0)
        {
        return 0;
        }

    /* remove mutex form current->mutex_owned */

    dlist_del (&mutex->node);

    (void) __recalc_task_prio (current);

    if (dlist_empty (&mutex->pend_q))
        {
        mutex->owner = NULL;
        return 0;
        }

    next_task = container_of (mutex->pend_q.next, task_t, pq_node);

    next_task->mutex_wanted = NULL;

    /* next_task->c_prio needless change */

    task_ready_q_add (next_task);

    __mutex_set_owner (mutex, next_task);

    return 0;
    }

/**
 * mutex_unlock - unlock a mutex
 * @mutex: the mutex to be unlocked
 *
 * return: 0 on success, negtive value on error
 */

int mutex_unlock (mutex_id mutex)
    {
    return do_critical_non_irq (__mutex_unlock, (uintptr_t) mutex, 0);
    }

