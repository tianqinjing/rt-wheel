/* timer.c - timer library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,18aug18,cfm  writen
*/

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <wheel/common.h>

#include <kernel/timer.h>
#include <kernel/task.h>
#include <kernel/tick.h>
#include <kernel/critical.h>

/**
 * timer_init - initialize a timer
 * @timer:    the timer to be initialized
 * @flag:     timer options
 * @interval: timer interval
 * @pfn:      timeout callback
 * @arg:      the argument for the timeout callback
 *
 * return: 0 on success, negtive value on error
 */

int timer_init (timer_id timer, uint16_t flag, unsigned long interval,
                void (*pfn) (uintptr_t), uintptr_t arg)
    {
    if ((flag > TIMER_FLAG_REPEATED) || (interval == 0) || (pfn == NULL))
        {
        return -1;
        }

    timer->status   = TIMER_STAT_INACTIVE;
    timer->flag     = flag;
    timer->interval = interval;
    timer->pfn      = pfn;
    timer->arg      = arg;

    return 0;
    }

/**
 * timer_create - create a timer
 * @flag:     timer options
 * @interval: timer interval
 * @pfn:      timeout callback
 * @arg:      the argument for the timeout callback
 *
 * return: 0 on success, negtive value on error
 */

timer_id timer_create (uint16_t flag, unsigned long interval,
                       void (*pfn) (uintptr_t), uintptr_t arg)
    {
    timer_id timer = (timer_id) malloc (sizeof (timer_t));

    if (timer == NULL)
        {
        return NULL;
        }

    if (timer_init (timer, flag, interval, pfn, arg))
        {
        free (timer);
        timer = NULL;
        }

    timer->flag |= TIMER_FLAG_MALLOC;

    return timer;
    }

static void __tick_q_callback_timer (struct tick_q_node * node, uintptr_t arg)
    {
    timer_id timer = container_of (node, timer_t, tq_node);

    (void) arg;

    /* prevent task switch in user timer callback */

    task_lock_cnt++;

    timer->pfn (timer->arg);

    task_lock_cnt--;

    if (timer->flag != TIMER_FLAG_REPEATED)
        {
        timer->status = TIMER_STAT_INACTIVE;
        return;
        }

    tick_q_add (&timer->tq_node, timer->interval, __tick_q_callback_timer, 0);
    }

int __timer_start (uintptr_t arg1, uintptr_t arg2)
    {
    timer_id timer = (timer_id) arg1;

    (void) arg2;

    if (timer->status == TIMER_STAT_ACTIVE)
        {
        tick_q_del (&timer->tq_node);
        }

    tick_q_add (&timer->tq_node, timer->interval, __tick_q_callback_timer, 0);

    timer->status = TIMER_STAT_ACTIVE;

    return 0;
    }

/**
 * timer_start - start a timer
 * @timer: the timer id to be started
 *
 * return: 0 on success, negtive value on error
 */

int timer_start (timer_id timer)
    {
    if (timer == NULL)
        {
        return -1;
        }

    return do_critical (__timer_start, (uintptr_t) timer, 0);
    }

int __timer_stop (uintptr_t arg1, uintptr_t arg2)
    {
    timer_id timer = (timer_id) arg1;

    (void) arg2;

    if (timer->status != TIMER_STAT_ACTIVE)
        {
        return 0;
        }

    tick_q_del (&timer->tq_node);

    timer->status = TIMER_STAT_INACTIVE;

    return 0;
    }

/**
 * timer_start - stop a timer
 * @timer: the timer id to be stopped
 *
 * return: 0 on success, negtive value on error
 */

int timer_stop (timer_id timer)
    {
    if (timer == NULL)
        {
        return -1;
        }

    return do_critical (__timer_stop, (uintptr_t) timer, 0);
    }

/**
 * timer_delete - delete a timer
 * @timer: the timer id to be deleted
 *
 * return: 0 on success, negtive value on error
 */

int timer_delete (timer_id timer)
    {
    int ret;

    if (timer == NULL)
        {
        return -1;
        }

    /*
     * use do_critical_non_irq to make sure that __timer_stop will be done
     * in current context, so we can free the timer, if use do_critical and
     * we are invoked in irq context, __timer_stop is not granteed to be
     * done when do_critical return
     * it is ok restrict this routine for irq context
     */

    ret = do_critical_non_irq (__timer_stop, (uintptr_t) timer, 0);

    if (ret)
        {
        return ret;
        }

    if (timer->flag & TIMER_FLAG_MALLOC)
        {
        free (timer);
        }

    return 0;
    }

