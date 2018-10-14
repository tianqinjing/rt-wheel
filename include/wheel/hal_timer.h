/* hal_timer.h - hardware timer abstract library header file header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,16sep18,cfm  writen
*/

#include <stdbool.h>
#include <stdint.h>

#include <wheel/list.h>

#ifndef __HAL_TIMER_H__
#define __HAL_TIMER_H__

/* macros */

#define HAL_TIMER_MAX_NAME_LEN      16

#define HAL_TIMER_MODE_ONE_SHOT     0
#define HAL_TIMER_MODE_REPEATED     1

/* typedefs */

typedef struct hal_timer hal_timer_t;

typedef struct hal_timer_methods
    {
    int          (* enable)  (hal_timer_t * timer, uint64_t cmp_rld);
    int          (* disable) (hal_timer_t * timer);
    int          (* connect) (hal_timer_t * timer, void (* pfn) (uintptr_t),
                              uintptr_t arg);
    uint64_t     (* counter) (hal_timer_t * timer);
    } hal_timer_methods_t;

struct hal_timer
    {
    dlist_t      node;              /* node to be inserted in the timer list */
    const char * name;
    uint8_t      unit;              /* unit number */
    uint8_t      mode;
    bool         busy;              /* timer allocated */
    bool         down;              /* timer counting down */
    uint32_t     freq;
    uint64_t     cmp_rld;           /* compare value or reload value */
    uint64_t     max_count;
    void         (* handler) (uintptr_t);
    uintptr_t    arg;

    const hal_timer_methods_t * methods;
    };

/* externs */

extern int           hal_timer_enable   (hal_timer_t * timer, uint64_t cmp_rld);
extern int           hal_timer_disable  (hal_timer_t * timer);
extern int           hal_timer_connect  (hal_timer_t * timer,
                                         void (* pfn) (uintptr_t),
                                         uintptr_t arg);
extern uint64_t      hal_timer_counter  (hal_timer_t * timer);
extern int           hal_timer_register (hal_timer_t * timer);
extern hal_timer_t * hal_timer_get      (const char * name, uint8_t mode);

#endif  /* __HAL_TIMER_H__ */

