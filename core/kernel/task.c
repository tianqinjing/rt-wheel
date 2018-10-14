/* task.c - task library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,18aug18,cfm  writen
*/

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#include <wheel/common.h>
#include <wheel/list.h>
#include <wheel/mem.h>
#include <wheel/defer.h>
#include <wheel/cmder.h>

#include <arch/sync.h>

#include <kernel/task.h>
#include <kernel/critical.h>

#undef putchar

/* externs */

_RTW_IMPORT_SECTION_START (TASK_SECTION_NAME);
_RTW_IMPORT_SECTION_END   (TASK_SECTION_NAME);

extern void task_ctx_init   (struct task * task);
extern void task_retval_set (struct task * task, int retval);

/* globals */

task_id current = NULL;

unsigned int task_lock_cnt = 0;

struct ready_q ready_q = {0};

static dlist_t all_tasks = DLIST_INIT (all_tasks);

/**
 * idle_entry - the idle task loop
 *
 * return: NA, never return
 */

static void idle_entry (void)
    {
    for (;;)
        {
        }
    }

/* prio of idle is not used */

RTW_TASK_DEF (idle, 0, 0, 0x50, idle_entry, 0);

/**
 * static_task_init - driver initialization routine
 *
 * return: 0 on success, negtive value on error
 */

int static_task_init (void)
    {
    task_t * task;

    for (task  = (task_t *) _RTW_SECTION_START (TASK_SECTION_NAME);
         task != (task_t *) _RTW_SECTION_END   (TASK_SECTION_NAME);
         task++)
        {
        if (task->c_prio > TASK_PRIO_MAX)
            {
            task->c_prio = task->o_prio = TASK_PRIO_MAX;
            }

        task_ctx_init (task);

        memset (task->stack_base, 0xee, task->stack_size - sizeof (struct regset));

        dlist_add (&all_tasks, &task->node);

        task_resume (task);
        }

    return 0;
    }

/**
 * task_create - create a task
 * @name:       the name of the task being created
 * @prio:       the priority of the task
 * @options:    options
 * @stack_size: the stack size the task required
 * @entry:      the entry point of the task
 * @arg:        the argument of the task
 *
 * return: the created task handler or NULL of fail
 */

task_id task_create (const char * name, uint8_t prio, uint32_t options,
                     size_t stack_size, int (* entry) (uintptr_t),
                     uintptr_t arg)
    {
    task_id task;
    char  * stack;
    size_t  alloc_size;

    if (name == NULL)
        {
        return NULL;
        }

    if (prio > TASK_PRIO_MAX)
        {
        return NULL;
        }

    /* TODO: options */

    stack_size = round_up (stack_size, STACK_ALIGN);

    alloc_size = stack_size + sizeof (struct task) + sizeof (struct regset);

    if ((stack_size == 0) || (stack_size > alloc_size))
        {
        return NULL;
        }

    stack = (char *) malloc (alloc_size);

    if (!stack)
        {
        return NULL;
        }

    task = (task_id) (stack + alloc_size - sizeof (struct task));

    memset (task, 0, sizeof (task_t));

    task->status           = TASK_STATUS_SUSPEND;

    task->entry            = entry;
    task->arg              = arg;

    task->stack_base       = stack;
    task->stack_size       = stack_size + sizeof (struct regset);

    strncpy (task->name, name, MAX_TASK_NAME_LEN);

    task->name [MAX_TASK_NAME_LEN - 1] = '\0';

    /* init priority feilds */

    task->c_prio = prio;
    task->o_prio = prio;

    dlist_init (&task->mutex_owned);

    task_ctx_init (task);

    dlist_add (&all_tasks, &task->node);

    memset (task->stack_base, 0xee, task->stack_size - sizeof (struct regset));

    return task;
    }

static void __task_delete_clean (struct deferred_job * job)
    {
    task_id task = (task_id) job->pdata;

    free (task->stack_base);
    }

static int __task_delete (uintptr_t arg1, uintptr_t arg2)
    {
    dlist_t        * itr, * next;
    deferred_job_t * job;
    task_id          task = (task_id) arg1;

    (void) arg2;

    if (task != current)
        {
        task_pc_set (task, (uintptr_t) task_delete);
        task_arg_set (task, 0, (uintptr_t) task);
        task_resume (task);

        return 0;
        }

    dlist_del (&task->node);

    /* delete current task from ready queue */

    task_ready_q_del (current);

    current->status = TASK_STATUS_DEAD;

    /* unlock all the lock owned by current */

    dlist_foreach_safe (itr, next, &task->mutex_owned)
        {
        mutex_id mutex = container_of (itr, mutex_t, node);

        mutex_unlock (mutex);
        }

    /* create deferred job struct at the end of the stack (stack base) */

    job = (deferred_job_t *) task->stack_base;

    if ((char *) (job + 1) > (char *) &task)
        {

        /* stack is not enough, just use the tcb as it is useless now */

        job = (deferred_job_t *) task;
        }

    job->job   = __task_delete_clean;
    job->pdata = (uintptr_t) current;

    do_deferred (job, DEFERRED_PRIO_LOWEST);

    return 0;
    }

/**
 * task_delete - delete a task
 * @task: the task to delete
 *
 * return: 0 for success, negtive value if fail
 */

int task_delete (task_id task)
    {
    return do_critical (__task_delete, (uintptr_t) task, 0);
    }

/**
 * __task_resume - resume a task in critical region
 */

static int __task_resume (uintptr_t arg1, uintptr_t arg2)
    {
    task_id task = (task_id) arg1;

    (void) arg2;

    if (task->status == TASK_STATUS_READY)
        {
        return 0;
        }

    task->status &= ~TASK_STATUS_SUSPEND;

    if (task != idle)
        {
        task_ready_q_add (task);
        }

    return 0;
    }

/**
 * task_resume - resume a task
 * @task: the task to resume
 *
 * return: 0 for success, negtive value if fail
 */

int task_resume (task_id task)
    {

    /* resuming current task, current task is running */

    if (task == NULL)
        {
        return 0;
        }

    return do_critical (__task_resume, (uintptr_t) task, 0);
    }

static int __task_suspend (uintptr_t arg1, uintptr_t arg2)
    {
    task_id task = (task_id) arg1;

    (void) arg2;

    /* delete current task from ready queue */

    if (task->status == TASK_STATUS_READY)
        {
        task_ready_q_del (task);
        }

    task->status |= TASK_STATUS_SUSPEND;

    return 0;
    }

/**
 * task_suspend - suspend a task
 * @task: the task to suspend
 *
 * return: 0 for success, negtive value if fail
 */

int task_suspend (task_id task)
    {

    /* resuming current task, current task is running */

    if (task == NULL)
        {
        task = current;
        }

    return do_critical (__task_suspend, (uintptr_t) task, 0);
    }

/**
 * task_spawn - create a task and then resume it
 * @name:       the name of the task being created
 * @prio:       the priority of the task
 * @options:    options
 * @stack_size: the stack size the task required
 * @entry:      the entry point of the task
 * @arg:        the argument of the task
 *
 * return: the created task handler or NULL of fail
 */

task_id task_spawn (const char * name, uint8_t prio, uint32_t options,
                    size_t stack_size, int (* entry) (uintptr_t), uintptr_t arg)
    {
    task_id tid = task_create (name, prio, options, stack_size, entry, arg);

    if (tid != NULL)
        {
        (void) task_resume (tid);
        }

    return tid;
    }

/**
 * task_status_get - get the status of a task
 * @task: the given task if NULL current will be selected
 *
 * return: the status of the given task
 */

int task_status_get (task_id task)
    {
    if (task == NULL)
        {
        task = current;
        }

    return task->status;
    }

/**
 * task_prio_get - get the priority of a task
 * @task: the given task if NULL current will be selected
 *
 * return: the priority of the given task
 */

uint8_t task_prio_get (task_id task)
    {
    task = task == NULL ? current : task;

    return task->c_prio;
    }

static int __task_prio_set (uintptr_t arg1, uintptr_t arg2)
    {
    task_id   task = (task_id) arg1;
    uint8_t   prio = (uint8_t) arg2;

    if (prio == task->o_prio)
        {
        return 0;
        }

    if (task->mutex_wanted != NULL)
        {
        return -1;
        }

    task->o_prio = prio;

    if (dlist_empty (&task->mutex_owned))
        {
        task->c_prio = prio;

        return 0;
        }

    if (task->c_prio == task->o_prio)
        {
        uint8_t   lowest_mutex_prio = TASK_PRIO_MAX;
        dlist_t * itr;

        dlist_foreach (itr, &task->mutex_owned)
            {
            mutex_id mutex = container_of (itr, mutex_t, node);

            if (lowest_mutex_prio > mutex->prio)
                {
                lowest_mutex_prio = mutex->prio;
                }
            }

        task->c_prio = (prio <= lowest_mutex_prio) ? prio : lowest_mutex_prio;
        }
    else
        {
        if (prio < task->c_prio)
            {
            task->c_prio  = prio;
            }
        }

    return 0;
    }

/**
 * task_prio_set - set the priority of a task
 * @task: the given task if NULL current will be selected
 * @prio: the new priority
 *
 * return: 0 on success, negtive value on fail when the prio is not valid or the
 *         task is pending on a mutex
 */

int task_prio_set (task_id task, uint8_t prio)
    {
    if (prio > TASK_PRIO_MAX)
        {
        return -1;
        }

    task = task == NULL ? current : task;

    return do_critical (__task_prio_set, (uintptr_t) task, (uintptr_t) prio);
    }

static void __tick_q_callback_task (struct tick_q_node * node, uintptr_t arg)
    {
    task_id task = container_of (node, struct task, tq_node);

    if (task->status & TASK_STATUS_PEND)
        {
        task_retval_set (task, -1);
        }

    task_ready_q_add (task);

    if (arg)
        {
        ((void (*) (task_id)) arg) (task);
        }
    }

int __task_delay (uintptr_t arg1, uintptr_t arg2)
    {
    unsigned int ticks = (unsigned int) arg1;

    (void) arg2;

    task_ready_q_del (current);

    if (ticks == 0)
        {
        task_ready_q_add (current);
        return 0;
        }

    current->status |= TASK_STATUS_DELAY;

    tick_q_add (&current->tq_node, ticks, __tick_q_callback_task, 0);

    return 0;
    }

/**
 * task_delay - delay a task from executing
 * @ticks: number of ticks to delay task
 *
 * return: 0, or negtive value if error
 */

int task_delay (unsigned int ticks)
    {
    return do_critical_might_sleep (__task_delay, (uintptr_t) ticks, 0);
    }

/**
 * task_lock - disable the task preemptive
 *
 * return: NA
 */

void task_lock (void)
    {
    task_lock_cnt++;
    wmb ();
    }

static int __task_unlock (uintptr_t arg1, uintptr_t arg2)
    {
    task_lock_cnt--;
    wmb ();

    return 0;
    }

/**
 * task_unlock - enable the task preemptive
 *
 * return: NA
 */

void task_unlock (void)
    {

    /* use do_critical for the reschedule in the end of the function */

    (void) do_critical (__task_unlock, 0, 0);
    }

/**
 * task_entry - the entry point for new created tasks
 * @task: the new created task id
 *
 * return: NA
 */

void task_entry (task_id task)
    {
    task->entry (task->arg);

    task_lock_cnt = 0;

    task_delete (current);
    }

/**
 * task_ready_q_init - initialize the task ready queue
 *
 * return: NA
 */

void task_ready_q_init (void)
    {
    int i;

    ready_q.highest = idle;

    for (i = 0; i < 32; i++)
        {
        dlist_init (&ready_q.heads [i]);
        }
    }

void __ready_q_put (struct task * task, bool head)
    {
    uint8_t prio = task->c_prio;

    if (task->status & TASK_STATUS_PEND)
        {
        dlist_del (&task->pq_node);
        }

    if (task->status & TASK_STATUS_DELAY)
        {
        dlist_del (&task->tq_node.node);
        }

    task->status &= ~(TASK_STATUS_PEND | TASK_STATUS_DELAY);

    if (task->status != TASK_STATUS_READY)
        {
        return;
        }

    if ((ready_q.highest == idle) || (prio < ready_q.highest->c_prio))
        {
        ready_q.highest = task;
        }

    ready_q.bmap |= 1 << (31 - prio);

    if (unlikely (head))
        {
        dlist_add (&ready_q.heads [prio], &task->rq_node);
        }
    else
        {
        task->tick_slices = 0;
        dlist_add_tail (&ready_q.heads [prio], &task->rq_node);
        }
    }

/**
 * task_ready_q_add - add a task to the tail of the ready queue with its priority
 * @task: the task to add
 *
 * return: NA
 */

void task_ready_q_add (struct task * task)
    {
    __ready_q_put (task, false);
    }

/**
 * task_ready_q_ins - add a task to the head of the ready queue with its priority
 * @task: the task to add
 *
 * return: NA
 */

void task_ready_q_ins (struct task * task)
    {
    __ready_q_put (task, true);
    }

/**
 * task_ready_q_del - remove a task from ready queue
 * @task: the task to remove
 *
 * return: NA
 */

void task_ready_q_del (struct task * task)
    {
    int           idx;
    uint8_t       prio = task->c_prio;

    dlist_del (&task->rq_node);

    if (dlist_empty (&ready_q.heads [prio]))
        {
        ready_q.bmap &= ~(1 << (31 - task->c_prio));
        }

    if (&ready_q.highest->rq_node != &task->rq_node)
        {
        return;
        }

    if (ready_q.bmap == 0)
        {
        ready_q.highest = idle;
        return;
        }

    idx = __clz (ready_q.bmap);

    ready_q.highest = container_of (ready_q.heads [idx].next, struct task, rq_node);

    return;
    }

static inline void __task_q_xwait_timed (dlist_t * q, unsigned int timeout,
                                         void (* callback) (task_id task))
    {
    task_ready_q_del (current);

    current->status |= TASK_STATUS_PEND;

    if (timeout != UINT_MAX)
        {
        current->status |= TASK_STATUS_DELAY;
        tick_q_add (&current->tq_node, timeout, __tick_q_callback_task,
                    (uintptr_t) callback);
        }
    }

/**
 * task_fwait_q_add - add current task to a fifo wait queue
 * @q:        the fifo wait queue
 * @timeout:  the max ticks the task wait in the queue
 * @callback: the timeout call back
 *
 * return: NA
 */

void task_fwait_q_add (dlist_t * q, unsigned int timeout,
                       void (* callback) (task_id task))
    {
    dlist_add_tail (q, &current->pq_node);

    __task_q_xwait_timed (q, timeout, callback);
    }

static void __pwait_q_add (dlist_t * q, task_id task)
    {
    dlist_t * itr;

    dlist_foreach (itr, q)
        {
        if (task->c_prio < container_of (itr, task_t, pq_node)->c_prio)
            {
            break;
            }
        }

    /*
     * 1) if q is empty, itr will just be itr
     * 2) if all tasks in have higher priority than current, itr will be q
     * 3) otherwise, itr will be the first node that priority lower than current
     *
     * for all of these three conditions, just add current in front of q
     */

    dlist_add_tail (itr, &task->pq_node);
    }

/**
 * task_fwait_q_add - add current task to a priority wait queue
 * @q:        the priority wait queue
 * @timeout:  the max ticks the task wait in the queue
 * @callback: the timeout call back
 *
 * return: NA
 */

void task_pwait_q_add (dlist_t * q, unsigned int timeout,
                       void (* callback) (task_id task))
    {
    __pwait_q_add (q, current);

    __task_q_xwait_timed (q, timeout, callback);
    }

/**
 * task_pwait_q_adj - correct the location in a priority queue when the priority
 *                    of a task changed
 * @q:    the priority wait queue
 * @task: the task whose priority changed and need correct loction
 *
 * return: NA
 */

void task_pwait_q_adj (dlist_t * q, task_id task)
    {
    dlist_del (&task->pq_node);
    __pwait_q_add (q, task);
    }

static void __task_show (cmder_t * cmder, task_id task)
    {
    char buff [12];
    char * status;

    cmder_print (cmder, task->name, MAX_TASK_NAME_LEN - 1, CMDER_PRINT_LALIGN);

    sprintf (buff, " 0x%08x", (uintptr_t) task->entry);
    cmder->putstr (cmder->arg, buff);

    sprintf (buff, " 0x%08x", (uintptr_t) task);
    cmder->putstr (cmder->arg, buff);

    cmder->putchar (cmder->arg, ' ');

    sprintf (buff, "%d ", task->o_prio);
    cmder_print (cmder, buff, 7, CMDER_PRINT_RALIGN);

    sprintf (buff, "%d ", task->c_prio);
    cmder_print (cmder, buff, 7, CMDER_PRINT_RALIGN);

    if (task->status == TASK_STATUS_READY)
        {
        status = "READY";
        }
    else if (task->status & TASK_STATUS_SUSPEND)
        {
        status = "SUSPEND";
        }
    else if (task->status & TASK_STATUS_PEND)
        {
        status = "PEND";
        }
    else if (task->status & TASK_STATUS_DELAY)
        {
        status = "DELAY";
        }
    else
        {
        status = "UNKNOWN";
        }

    cmder_print (cmder, status, 8, CMDER_PRINT_LALIGN);

    sprintf (buff, " 0x%08x", task_pc_get (task));
    cmder->putstr (cmder->arg, buff);

    cmder->putchar (cmder->arg, '\n');
    }

static int task_show (cmder_t * cmder, int argc, char * argv [])
    {
    dlist_t * itr;
    task_id   task;

    /* print out the header */

    cmder->putstr (cmder->arg,
                   "\nNAME    ENTRY      TASK_ID    O_PRIO C_PRIO STATUS   PC\n");

    cmder->putstr (cmder->arg,
                   "======= ========== ========== ====== ====== ======== ==========\n");

    dlist_foreach (itr, &all_tasks)
        {
        task = container_of (itr, task_t, node);
        __task_show (cmder, task);
        }

    return 0;
    }

RTW_CMDER_CMD_DEF ("i", "show task info", task_show);

