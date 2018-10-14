/* timer.h - timer library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,18aug18,cfm  writen
*/

#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

#include <kernel/tick.h>

#define TIMER_STAT_INACTIVE     0   /* not in tick_q */
#define TIMER_STAT_ACTIVE       1   /* in tick_q */

#define TIMER_FLAG_ONE_SHOT     0
#define TIMER_FLAG_REPEATED     1
#define TIMER_FLAG_MALLOC       16

typedef struct timer
    {
    uint16_t           status;
    uint16_t           flag;
    struct tick_q_node tq_node;
    unsigned long      interval;
    void            (* pfn) (uintptr_t);
    uintptr_t          arg;
    } timer_t, * timer_id;

extern int timer_init        (timer_id timer, uint16_t mode, unsigned long interval,
                              void (*pfn)(uintptr_t), uintptr_t arg);
extern timer_id timer_create (uint16_t flag, unsigned long interval,
                              void (*pfn) (uintptr_t), uintptr_t arg);
extern int timer_start       (timer_id timer);
extern int timer_stop        (timer_id timer);
extern int timer_delete      (timer_id timer);

#endif  /* __TIMER_H__ */
