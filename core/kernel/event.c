/* event.c - event library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,19aug18,cfm  writen
*/

#include <stdlib.h>
#include <stdint.h>

#include <wheel/common.h>
#include <wheel/list.h>

#include <kernel/event.h>
#include <kernel/task.h>
#include <kernel/critical.h>

/**
 * event_init - initialize an event
 * @event: the event id to be initialized
 *
 * return: 0 on success, negtive value on error
 */

int event_init (event_id event)
    {
    if (event == NULL)
        {
        return -1;
        }

    event->event_set = 0;
    dlist_init (&event->pend_q);

    return 0;
    }

/**
 * event_create - creat and then initialize an event
 *
 * return: event id on success, NULL on error
 */

event_id event_create (void)
    {
    event_id event = (event_id) malloc (sizeof (event_t));

    if (event == NULL)
        {
        return NULL;
        }

    event->event_set = 0;
    dlist_init (&event->pend_q);

    return event;
    }

/**
 * __event_feed - feed a task waiting for events from an event
 * @task:    the task to feed, the task may be current (invoked from event_recv)
 *           or a task in the pend_q (invoked form event_send)
 * @event:   the event id, now holding event sets, and is the event that the task
 *           just want to read from
 * @timeout: max wait ticks, always 0 if invoked form event_send, trying to wake
 *           up a task
 *
 * return: 0 on success, negtive value on error
 */

static int __event_feed (task_id task, event_id event, unsigned int timeout)
    {
    uint32_t wanted  = task->event_wanted;

    switch (task->event_option)
        {
        case EVENT_WAIT_ALL:
            if ((event->event_set & wanted) == wanted)
                goto got;
            break;
        case EVENT_WAIT_ANY:
            if ((event->event_set & wanted) != 0)
                goto got;
            break;
        default:
            return -1;
        }

    if (!timeout)
        {
        return -1;
        }

    task_fwait_q_add (&event->pend_q, timeout, NULL);

    return 0;

got:

    task->event_recved = event->event_set &= wanted;

    event->event_set &= ~wanted;

    return 0;
    }

static int __event_recv (uintptr_t arg1, uintptr_t arg2)
    {
    return __event_feed (current, (event_id) arg1, (unsigned int) arg2);
    }

/**
 * event_recv - receive event(s) from an event object
 * @event:   the event id
 * @wanted:  wanted event set
 * @option:  the receive option, EVENT_WAIT_ALL or EVENT_WAIT_ANY
 * @timeout: max ticks wait events
 * @recved:  event set(s) received
 *
 * return: 0 on success, negtive value on error
 */

int event_recv (event_id event, uint32_t wanted, uint32_t option,
                unsigned int timeout, uint32_t * recved)
    {
    current->event_wanted = wanted;
    current->event_option = option;

    if (do_critical_might_sleep (__event_recv, (uintptr_t) event,
                                 (uintptr_t) timeout))
        {
        return -1;
        }

    if (recved)
        {
        *recved = current->event_recved;
        }

    return 0;
    }

static int __event_send (uintptr_t arg1, uintptr_t arg2)
    {
    event_id  event  = (event_id) arg1;
    uint32_t  events = (uint32_t) arg2;
    dlist_t * itr;

    event->event_set |= events;

    dlist_foreach (itr, &event->pend_q)
        {
        task_id task = container_of (itr, task_t, pq_node);

        if (__event_feed (task, event, 0))
            {
            continue;
            }

        task_ready_q_add (task);

        if (event->event_set == 0)
            {
            break;
            }
        }

    return 0;
    }

/**
 * event_send - send event(s) to an event object
 * @event:  the event id
 * @events: event set
 *
 * return: 0 on success, negtive value on error
 */

int event_send (event_id event, uint32_t events)
    {
    if (!event || !events)
        {
        return -1;
        }

    return do_critical (__event_send, (uintptr_t) event, (uintptr_t) events);
    }

