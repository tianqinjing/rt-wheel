/* critical.c - critical job module */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,19aug18,cfm  writen
*/

#include <stdbool.h>
#include <stdint.h>

#include <wheel/common.h>
#include <wheel/irq.h>

#include <kernel/task.h>

#include <arch/sync.h>

/* defines */

#define CRITICAL_JOB_Q_ELEMENTS         64
#define CRITICAL_JOB_Q_MASK             (CRITICAL_JOB_Q_ELEMENTS - 1)

/* typedefs */

typedef struct critical_job
    {
    int    (* pfn) (uintptr_t, uintptr_t);
    uintptr_t arg1;
    uintptr_t arg2;
    int       pad;
    } critical_job_t;

/* externs */

/* globals */

volatile bool in_critical = false;

static critical_job_t        critical_job_q [CRITICAL_JOB_Q_ELEMENTS] = {0};
static volatile unsigned int head_idx = 0;
static volatile unsigned int tail_idx = 0;

/* inlines */

static inline int next_idx (int idx)
    {
    return (idx + 1) & CRITICAL_JOB_Q_MASK;
    }

static inline bool critical_job_q_is_full (void)
    {
    return next_idx (head_idx) == tail_idx;
    }

static inline int critical_job_q_add (int (* job) (uintptr_t, uintptr_t),
                                      uintptr_t arg1, uintptr_t arg2)
    {
    int idx = -1;

    int key = int_lock ();

    if (!critical_job_q_is_full ())
        {
        idx = head_idx;
        head_idx = next_idx (idx);
        }

    int_unlock (key);

    if (idx == -1)
        {
        // TODO: critical job queue full!
        return -1;
        }

    /*
     * it is safe do this out of int_lock/int_unlock protection:
     * 1) there are only two cases here:
     *    a) it is in task context now, one parent routine is just processing
     *       the critical queue
     *    b) it is in irq handler now, and this irq is just interruptted a task,
     *       or a lower priority irq and this task/irq handler is just procesing
     *       the critical queue
     * 2) the critical queue processing can only continue when current routine
     *    or irq handler return
     * 3) at that time, the followint assignment is done, of cause
     */

    critical_job_q [idx].pfn  = job;
    critical_job_q [idx].arg1 = arg1;
    critical_job_q [idx].arg2 = arg2;

    return 0;
    }

static inline void enter_critical (void)
    {
    in_critical = true;
    mb ();
    }

static inline void exit_critical (void)
    {
    in_critical = false;
    mb ();
    }

static inline int __do_critical (int (* job) (uintptr_t, uintptr_t),
                                 uintptr_t arg1, uintptr_t arg2)
    {
    int ret;
    int key;
    int idx;

    enter_critical ();

    ret = job (arg1, arg2);

    while (1)
        {
        key = int_lock ();

        if ((idx = tail_idx) == head_idx)
            {
            idx = -1;
            exit_critical ();
            }

        int_unlock (key);

        if (idx == -1)
            {
            break;
            }

        (void) critical_job_q [idx].pfn (critical_job_q [idx].arg1,
                                         critical_job_q [idx].arg2);

        tail_idx = next_idx (idx);

        mb ();
        }

    /*
     * schedule will just return the 'ret' passed to it, but this value
     * may be changed though task_retval_set
     */

    return schedule (ret);
    }

/**
 * do_critical - do critical work, can be invoked in all context
 * @job:  the job routine
 * @arg1: the first argument
 * @arg2: the second argument
 *
 * return: status
 */

int do_critical (int (* job) (uintptr_t, uintptr_t),
                 uintptr_t arg1, uintptr_t arg2)
    {
    if (in_critical)
        {
        critical_job_q_add (job, arg1, arg2);
        return 0;
        }

    return __do_critical (job, arg1, arg2);
    }

/**
 * do_critical_might_sleep - do critical that may cause the caller sleep
 * @job:  the job routine
 * @arg1: the first argument
 * @arg2: the second argument
 *
 * return: status
 */

int do_critical_might_sleep (int (* job) (uintptr_t, uintptr_t),
                             uintptr_t arg1, uintptr_t arg2)
    {
    // TODO: int_cnt -> macro or func
    if (task_lock_cnt || (int_cnt > 0) || (current == idle))
        {
        return -1;
        }

    return __do_critical (job, arg1, arg2);
    }

/**
 * do_critical_non_irq - do critical job in irq restricted context
 * @job:  the job routine
 * @arg1: the first argument
 * @arg2: the second argument
 *
 * return: status
 */

int do_critical_non_irq (int (* job) (uintptr_t, uintptr_t),
                         uintptr_t arg1, uintptr_t arg2)
    {
    if (int_cnt > 0)
        {
        return -1;
        }

    return __do_critical (job, arg1, arg2);
    }

