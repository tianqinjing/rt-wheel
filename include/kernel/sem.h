/* sem.h - semaphore library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,26aug18,cfm  writen
*/

#ifndef __SEM_H__
#define __SEM_H__

#include <stdint.h>

#include <wheel/list.h>

typedef struct sem
    {
    unsigned int count;
    dlist_t      pend_q;
    } sem_t, * sem_id;

/* defines */

#define SEM_INIT(name, count)       \
    { count, { &(name).pend_q, &(name).pend_q } }

extern int sem_init      (sem_t * sem, uintptr_t value);
extern int sem_wait      (sem_t * sem);
extern int sem_trywait   (sem_t * sem);
extern int sem_timedwait (sem_t * sem, unsigned int timeout);
extern int sem_post      (sem_t * sem);

#endif  /* __SEM_H__ */

