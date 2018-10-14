/* tick.c - tick library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,18aug18,cfm  writen
*/

#include <wheel/common.h>
#include <wheel/list.h>

#include <kernel/critical.h>
#include <kernel/task.h>
#include <kernel/tick.h>

volatile uint64_t tick_count;

static unsigned int rr_slices = 5;          // TODO: correct value configiralbe

dlist_t tick_q = DLIST_INIT (tick_q);

/**
 * tick_q_add - add a tick queue node to tick queue
 * @node:  the tick queue to be added, usually come from a timer or task
 * @ticks: the max number of ticks the node stay in the tick queue
 * @pfn:   the timeout callback
 * @arg:   the argument for the timeout callback
 *
 * return: NA
 */

void tick_q_add (struct tick_q_node * node, unsigned int ticks,
                 void (*pfn) (struct tick_q_node *, uintptr_t), uintptr_t arg)
    {
    dlist_t * itr;
    dlist_t * ins = NULL;

    if (!dlist_empty (&tick_q))
        {
        dlist_foreach (itr, &tick_q)
            {
            struct tick_q_node * n = container_of (itr, struct tick_q_node, node);

            if (ticks <= n->ticks_left)
                {
                n->ticks_left -= ticks;
                ins = itr;
                break;
                }

            ticks -= n->ticks_left;
            }
        }

    node->ticks_left = ticks;
    node->pfn        = pfn;
    node->arg        = arg;

    dlist_add_tail (ins == NULL ? &tick_q : ins, &node->node);
    }

/**
 * tick_q_del - remove a tick queue node from tick queue
 * @node:  the tick queue to be removed, usually come from a timer or task
 *
 * return: NA
 */

void tick_q_del (struct tick_q_node * node)
    {
    struct tick_q_node * next;

    dlist_del (&node->node);

    if (dlist_empty (&tick_q))
        {
        return;
        }

    /* if deleting the last node, just return */

    if (node->node.next == &tick_q)
        {
        return;
        }

    next = container_of (node->node.next, struct tick_q_node, node);

    next->ticks_left += node->ticks_left;
    }

static inline void tick_q_shot (unsigned int ticks)
    {
    struct tick_q_node * node;

    if (dlist_empty (&tick_q))
        {
        return;
        }

    node = container_of (tick_q.next, struct tick_q_node, node);

    if (node->ticks_left > ticks)
        {
        node->ticks_left -= ticks;
        return;
        }

    node->ticks_left = 0;

    do
        {
        dlist_del (&node->node);

        node->pfn (node, node->arg);

        if (dlist_empty (&tick_q))
            {
            break;
            }

        node = container_of (tick_q.next, struct tick_q_node, node);
        } while (node->ticks_left == 0);
    }

static int __tick_shot_n (uintptr_t arg1, uintptr_t arg2)
    {
    unsigned int ticks = (unsigned int) arg1;

    (void) arg2;

    tick_count += ticks;

    tick_q_shot (ticks);

    /*
     * even for shotting N ticks, tick slice just need to be added once because
     * the system is just sleeped for at least (N-1) ticks
     */

    if (++current->tick_slices >= rr_slices)
        {
        current->tick_slices = 0;

        task_ready_q_del (current);
        task_ready_q_add (current);
        }

    return 0;
    }

/**
 * tick_shot_n - advance the tick queue by <n>
 * @ticks: the advanced tick numbers
 *
 * return: NA
 */

void tick_shot_n (unsigned int ticks)
    {
    (void) do_critical (__tick_shot_n, (uintptr_t) ticks, 0);
    }

/**
 * tick_shot - advance the tick queue by 1
 *
 * return: NA
 */

void tick_shot (void)
    {
    tick_shot_n (1);
    }

