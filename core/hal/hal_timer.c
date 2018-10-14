/* hal_timer.h - hardware timer abstract library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,16sep18,cfm  writen
*/

#include <string.h>

#include <wheel/common.h>
#include <wheel/hal_timer.h>

/* locals */

static dlist_t hal_timers = DLIST_INIT (hal_timers);

/**
 * hal_timer_enable - enable a timer
 * @timer:     the timer to be enabled
 * @max_count: the timer max rollover count
 *
 * return: 0 on success, negtive value on error
 */

int hal_timer_enable (hal_timer_t * timer, uint64_t cmp_rld)
    {
    if (!timer || !timer->methods)
        {
        return -1;
        }

    if (cmp_rld > timer->max_count)
        {
        return -1;
        }

    timer->cmp_rld = cmp_rld;

    return timer->methods->enable (timer, cmp_rld);
    }

/**
 * hal_timer_disable - disable a timer
 * @timer:     the timer to be disabled
 *
 * return: 0 on success, negtive value on error
 */

int hal_timer_disable (hal_timer_t * timer)
    {
    if (!timer || !timer->methods || !timer->methods->disable)
        {
        return -1;
        }

    return timer->methods->disable (timer);
    }

/**
 * hal_timer_connect - connect a callback routine to a timer
 * @timer: the timer to be connected
 * @pfn:   the callback routine
 * @arg:   the argument to the callback routine
 *
 * return: 0 on success, negtive value on error
 */

int hal_timer_connect (hal_timer_t * timer, void (* pfn) (uintptr_t),
                       uintptr_t arg)
    {
    if (!timer || !timer->methods)
        {
        return -1;
        }

    timer->handler = pfn;
    timer->arg     = arg;

    return timer->methods->connect (timer, pfn, arg);
    }

/**
 * hal_timer_counter - get the current counter of a timer
 * @timer: the timer to read
 *
 * return: the counter for now (timestamp)
 */

uint64_t hal_timer_counter (hal_timer_t * timer)
    {
    uint64_t counter;

    if (!timer || !timer->methods)
        {
        return 0;
        }

    counter = timer->methods->counter (timer);

    if (timer->down)
        {
        counter = timer->cmp_rld - counter;
        }

    return counter;
    }

/**
 * hal_timer_register - register a timer to the hal
 * @timer: the timer to register
 *
 * return: 0 on success, negtive value on error
 */

int hal_timer_register (hal_timer_t * timer)
    {
    if (!timer->methods)
        {
        return -1;
        }

    if (!timer->methods->enable || !timer->methods->connect ||
        !timer->methods->counter)
        {
        return -1;
        }

    dlist_add_tail (&hal_timers, &timer->node);

    return 0;
    }

/**
 * hal_timer_get - get (allocate) a timer by name
 * @name: the timer name
 * @mode: the timer mode
 *
 * return: 0 on success, negtive value on error
 */

hal_timer_t * hal_timer_get (const char * name, uint8_t mode)
    {
    dlist_t     * itr;

    dlist_foreach (itr, &hal_timers)
        {
        hal_timer_t * timer = container_of (itr, hal_timer_t, node);

        if (strncmp (name, timer->name, HAL_TIMER_MAX_NAME_LEN))
            {
            continue;
            }

        timer->busy = true;
        timer->mode = mode;

        return timer;
        }

    return NULL;
    }

