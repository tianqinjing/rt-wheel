/* tick.h - tick library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,18aug18,cfm  writen
*/

#ifndef __TICK_H__
#define __TICK_H__

#include <stdint.h>

#include <wheel/list.h>

struct tick_q_node
    {
    dlist_t      node;
    unsigned int ticks_left;
    void      (* pfn) (struct tick_q_node *, uintptr_t);
    uintptr_t    arg;
    };

extern void tick_q_del  (struct tick_q_node * node);
extern void tick_q_add  (struct tick_q_node * node, unsigned int ticks,
                         void (*pfn) (struct tick_q_node *, uintptr_t),
                         uintptr_t arg);
extern void tick_shot_n (unsigned int ticks);
extern void tick_shot   (void);

#endif  /* __TICK_H__ */

