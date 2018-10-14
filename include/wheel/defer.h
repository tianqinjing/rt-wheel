/* defer.h - deferred job module header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,03oct18,cfm  writen
*/

#ifndef __DEFER_H__
#define __DEFER_H__

#include <stdint.h>

/* macros */

#define NR_DEFERRED_PRIOS       32

#define DEFERRED_PRIO_HIGHEST   0
#define DEFERRED_PRIO_LOWEST    (NR_DEFERRED_PRIOS - 1)

/* typedefs */

typedef struct deferred_job
    {
    dlist_t   node;
    void   (* job) (struct deferred_job *);     /* the job routine */
    uintptr_t pdata;                            /* private data */
    } deferred_job_t;

/* externs */

extern int do_deferred (deferred_job_t * job, uint8_t prio);

#endif  /* __DEFER_H__ */

