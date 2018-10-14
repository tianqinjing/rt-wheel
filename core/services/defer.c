/* defer.c - deferred job module */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,03oct18,cfm  writen
*/

#include <stdlib.h>
#include <limits.h>

#include <wheel/common.h>
#include <wheel/list.h>
#include <wheel/defer.h>
#include <wheel/irq.h>

#include <kernel/event.h>
#include <kernel/task.h>

static dlist_t deferred_jobs [NR_DEFERRED_PRIOS];

static event_t deferred_event = EVENT_INIT (deferred_event);

/* typedefs */

static uint32_t __prio_to_event (uint8_t prio)
    {
    return 1 << (31 - prio);
    }

/**
 * do_deferred - do deferred job
 * @job:  the event id
 * @prio: the priority of the job
 *
 * return: 0 on success, negtive value on error
 */

int do_deferred (deferred_job_t * job, uint8_t prio)
    {
    unsigned long flags;

    if ((job == NULL) || (prio > DEFERRED_PRIO_LOWEST))
        {
        return -1;
        }

    flags = int_lock ();
    dlist_add_tail (&deferred_jobs [prio], &job->node);
    int_unlock (flags);

    event_send (&deferred_event, __prio_to_event (prio));

    return 0;
    }

static void deferred_task (void)
    {
    int i;

    for (i = 0; i < NR_DEFERRED_PRIOS; i++)
        {
        dlist_init (&deferred_jobs [i]);
        }

    while (1)
        {
        uint32_t job_bit_map = 0;

        event_recv (&deferred_event, 0xffffffff, EVENT_WAIT_ANY, UINT_MAX,
                    &job_bit_map);

        while (job_bit_map)
            {
            uint8_t       prio  = __clz (job_bit_map);
            unsigned long flags;

            job_bit_map &= ~(__prio_to_event (prio));

            while (!dlist_empty (&deferred_jobs [prio]))
                {
                dlist_t        * dlist = deferred_jobs [prio].next;
                deferred_job_t * job;

                flags = int_lock ();
                dlist_del (dlist);
                int_unlock (flags);

                job = container_of (dlist, deferred_job_t, node);

                job->job (job);
                }
            }
        }
    }

RTW_TASK_DEF (defer, 0, 0, 0x200, deferred_task, 0);

