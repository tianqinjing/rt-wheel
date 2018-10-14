/* task_arch.c - task library arch support */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,18aug18,cfm  writen
*/

#include <string.h>

#include <kernel/task.h>

#include <arch/regset.h>

/**
 * task_retval_set - set the return value of a task in its context
 * @task:   the victimized task
 * @retval: the new return value
 *
 * return: NA
 */

void task_retval_set (struct task * task, int retval)
    {
    struct regset * regset = (struct regset *) task->regset;

    regset->r0 = (uint32_t) retval;
    }

/**
 * task_pc_set - set the program counter of a task in its context
 * @task: the victimized task
 * @pc:   the new program counter
 *
 * return: NA
 */

void task_pc_set (struct task * task, uintptr_t pc)
    {
    struct regset * regset = (struct regset *) task->regset;

    regset->pc = (uint32_t) pc;
    }

/**
 * task_pc_get - get the program counter of a task in its context
 * @task: the task
 *
 * return: NA
 */

uintptr_t task_pc_get (struct task * task)
    {
    struct regset * regset = (struct regset *) task->regset;

    return (uintptr_t) regset->pc;
    }

/**
 * task_arg_set - set the argument of a task in its context
 * @task: the victimized task
 * @argn: which argument will be set, must be less than 4
 * @arg:  the argument value
 *
 * return: NA
 */

void task_arg_set (struct task * task, unsigned int argn, uintptr_t arg)
    {
    struct regset * regset = (struct regset *) task->regset;
    uint32_t      * pa = &regset->r0;

    if (argn > 4)
        {
        return;
        }

    pa [argn] = (uint32_t) arg;
    }

/**
 * task_ctx_init - initialize the context of a task
 * @task: the task being initialized
 *
 * return: NA
 */

void task_ctx_init (struct task * task)
    {
    char          * stack_top = task->stack_base + task->stack_size;
    struct regset * regset    = &((struct regset *) stack_top) [-1];

    memset (regset, 0, sizeof (struct regset));

    regset->epsr = EPSR_T;
    regset->pc   = (uint32_t) task_entry;
    regset->r0   = (uint32_t) task;

    task->regset = (uintptr_t) regset;
    }

