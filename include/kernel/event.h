/* event.h - event library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,19aug18,cfm  writen
*/

#ifndef __EVENT_H__
#define __EVENT_H__

#include <stdint.h>

#define EVENT_WAIT_ALL  1
#define EVENT_WAIT_ANY  2

typedef struct event
    {
    uint32_t event_set;
    dlist_t  pend_q;
    } event_t, * event_id;

/* macros */

#define EVENT_INIT(name)    \
    { 0, { &(name).pend_q, &(name).pend_q } }

extern int      event_init   (event_id event);
extern event_id event_create (void);
extern int      event_recv   (event_id event, uint32_t wanted, uint32_t option,
                              unsigned int timeout, uint32_t * recved);
extern int      event_send   (event_id event, uint32_t events);

#endif  /* __EVENT_H__ */

