/* mutex.h - mutex lock library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,26aug18,cfm  writen
*/

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <stdint.h>

#include <wheel/list.h>

#include <kernel/task.h>

/* typedefs */

typedef struct mutex
    {
    uint16_t recurse;
    uint8_t  prio;      /* the max prio of the tasks pend on this mutex */
    task_t * owner;
    dlist_t  pend_q;
    dlist_t  node;      /* linked in task_t->owned_mutex */
    } mutex_t, * mutex_id;

/* defines */

#define MUTEX_INIT(name)        \
    { 0, 0, NULL, { &(name).pend_q, &(name).pend_q }, { NULL, NULL } }

/* externs */

extern int mutex_init      (mutex_id mutex);
extern int mutex_lock      (mutex_id mutex);
extern int mutex_lock      (mutex_id mutex);
extern int mutex_trylock   (mutex_id mutex);
extern int mutex_timedlock (mutex_id mutex, unsigned int timeout);
extern int mutex_unlock    (mutex_id mutex);

#endif /* __MUTEX_H__ */

