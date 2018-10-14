/* sem.c - semaphore library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,26aug18,cfm  writen
*/

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include <wheel/common.h>

#include <kernel/sem.h>
#include <kernel/task.h>
#include <kernel/critical.h>

/**
 * sem_init - initialize a semahpore
 * @sem:   the semaphore to be initialized
 * @value: the initialized semaphore value
 *
 * return: 0 on success, negtive value on error
 */

int sem_init (sem_t * sem, uintptr_t value)
    {
    if (sem == NULL)
        {
        return -1;
        }

    sem->count = value;

    dlist_init (&sem->pend_q);

    return 0;
    }

int __sem_wait (uintptr_t arg1, uintptr_t arg2)
    {
    sem_t      * sem     = (sem_t *) arg1;
    unsigned int timeout = arg2;

    if (sem->count > 0)
        {
        sem->count--;
        return 0;
        }

    if (timeout == 0)
        {
        return -1;
        }

    task_fwait_q_add (&sem->pend_q, timeout, NULL);

    return 0;
    }

/**
 * sem_wait - lock a semaphore
 * @sem: the semaphore to be locked
 *
 * return: 0 on success, negtive value on error
 */

int sem_wait (sem_t * sem)
    {
    return do_critical_might_sleep (__sem_wait, (uintptr_t) sem, UINT_MAX);
    }

/**
 * sem_trywait - try to lock a semaphore
 * @sem:   the semaphore to be locked
 *
 * return: 0 on success, negtive value on error
 */

int sem_trywait (sem_t * sem)
    {
    return do_critical_non_irq (__sem_wait, (uintptr_t) sem, 0);
    }

/**
 * sem_timedwait - lock a semaphore with timeout
 * @sem:     the semaphore to be locked
 * @timeout: the max number of waiting ticks
 *
 * return: 0 on success, negtive value on error
 */

int sem_timedwait (sem_t * sem, unsigned int timeout)
    {
    return do_critical_might_sleep (__sem_wait, (uintptr_t) sem,
                                    (uintptr_t) timeout);
    }

int __sem_post (uintptr_t arg1, uintptr_t arg2)
    {
    sem_t       * sem = (sem_t *) arg1;
    struct task * task;

    (void) arg2;

    if (sem->count == UINT_MAX)
        {
        return -1;      /* overflow */
        }

    if (dlist_empty (&sem->pend_q))
        {
        sem->count++;
        }
    else
        {
        task = container_of (sem->pend_q.next, struct task, pq_node);
        task_ready_q_add (task);
        }

    return 0;
    }

/**
 * sem_post - unlock a semaphore
 * @sem:     the semaphore to be unlocked
 *
 * return: 0 on success, negtive value on error
 */

int sem_post (sem_t * sem)
    {
    return do_critical (__sem_post, (uintptr_t) sem, 0);
    }

