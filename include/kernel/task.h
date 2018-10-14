/* task.h - task library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,18aug18,cfm  writen
*/

#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <stddef.h>

#include <wheel/list.h>

#include <kernel/tick.h>

#include <arch/regset.h>

/* defines */

#define MAX_TASK_NAME_LEN       8

#define TASK_STATUS_READY       0
#define TASK_STATUS_SUSPEND     1
#define TASK_STATUS_PEND        2
#define TASK_STATUS_DELAY       4
#define TASK_STATUS_DEAD        8

#define NR_TASK_PRIOS           32

#define TASK_PRIO_MAX           (NR_TASK_PRIOS - 1)
#define TASK_PRIO_MIN           0
#define TASK_PRIO_INV           (NR_TASK_PRIOS)

#define TASK_SECTION_NAME       static_task

typedef struct mutex * mutex_id;

typedef struct task
    {
    uintptr_t              regset;
    volatile int           status;

    char                 * stack_base;
    size_t                 stack_size;

    /* priority values */

    uint8_t                c_prio;
    uint8_t                o_prio;

    unsigned int           tick_slices;

    int                 (* entry) (uintptr_t);
    uintptr_t              arg;

    uint32_t               error;

    union
        {
        dlist_t            rq_node;
        struct tick_q_node tq_node;
        };

    dlist_t                pq_node;

    /* ipc related feilds */

    dlist_t                mutex_owned;
    mutex_id               mutex_wanted;

#if 1   // TODO: ifdef RTW_CONFIG_EVENT
    uint32_t               event_wanted;
    uint32_t               event_recved;
    uint32_t               event_option;
#endif

    char                   name [MAX_TASK_NAME_LEN];

    /* link all tasks with this node */

    dlist_t                node;
    } task_t, * task_id;

/**
 * RTW_TASK_DEF - define a task object at compile time
 * @n: the task name
 * @p: the task priority
 * @o: the task option
 * @s: the task stack size
 * @e: the task entry point
 * @a: the argument for the entry point
 *
 * return: NA
 */

#define RTW_TASK_DEF(n, p, o, s, e, a)                                      \
                                                                            \
STATIC_ASSERT (s > 0);                                                      \
STATIC_ASSERT ((s & 7) == 0);                                               \
STATIC_ASSERT (p >= TASK_PRIO_MIN);                                         \
STATIC_ASSERT (p <= TASK_PRIO_MAX);                                         \
                                                                            \
static long   __static_task_##n##_stack [(s + sizeof (struct regset)) /     \
                                         sizeof (long)];                    \
                                                                            \
static task_t __static_task_##n##_tcb _RTW_SECTION (TASK_SECTION_NAME) =    \
    {                                                                       \
    .status      = TASK_STATUS_SUSPEND,                                     \
    .c_prio      = p,                                                       \
    .o_prio      = p,                                                       \
    .stack_base  = ((char *) __static_task_##n##_stack),                    \
    .stack_size  = s + sizeof (struct regset),                              \
    .entry       = (int (*) (uintptr_t)) e,                                 \
    .arg         = a,                                                       \
    .mutex_owned = DLIST_INIT (__static_task_##n##_tcb.mutex_owned),        \
    .name        = __CVTSTR (n),                                            \
    };                                                                      \
                                                                            \
task_id n = &__static_task_##n##_tcb

struct ready_q
    {
    struct task     * highest;
    uint32_t          bmap;
    dlist_t           heads [NR_TASK_PRIOS];
    };

extern task_id        current;
extern task_id        idle;
extern unsigned int   task_lock_cnt;
extern struct ready_q ready_q;

extern task_id        task_create       (const char * name, uint8_t prio,
                                         uint32_t options, size_t stack_size,
                                         int (* entry) (uintptr_t),
                                         uintptr_t arg);
extern int            task_delete       (task_id task);
extern int            task_resume       (task_id task);
extern int            task_suspend      (task_id task);
extern task_id        task_spawn        (const char * name, uint8_t prio,
                                         uint32_t options, size_t stack_size,
                                         int (* entry) (uintptr_t),
                                         uintptr_t arg);
extern uint8_t        task_prio_get     (task_id task);
extern void           task_entry        (task_id task);
extern int            task_delay        (unsigned int ticks);
extern void           task_lock         (void);
extern void           task_unlock       (void);
extern void           task_retval_set   (struct task * task, int retval);
extern void           task_pc_set       (struct task * task, uintptr_t pc);
extern uintptr_t      task_pc_get       (struct task * task);
extern void           task_arg_set      (struct task * task, unsigned int argn,
                                         uintptr_t arg);
extern int            schedule          (int);
extern void           task_ready_q_init (void);
extern void           task_ready_q_add  (struct task * task);
extern void           task_ready_q_ins  (struct task * task);
extern void           task_ready_q_del  (struct task * task);
extern void           task_fwait_q_add  (dlist_t * q, unsigned int timeout,
                                         void (* callback) (task_id task));
extern void           task_pwait_q_add  (dlist_t * q, unsigned int timeout,
                                         void (* callback) (task_id task));
extern void           task_pwait_q_adj  (dlist_t * q, task_id task);
#endif  /* __TASK_H__ */

