/* context-gnuc.s - context related routines for gnuc */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,13oct18,cfm  writen
*/

#include <arch/aarch-m/asm.h>

#define SCB_ICSR                0xE000ED04
#define ICSR_PENDSVSET          0x10000000

        IMPORT  (in_critical)
        IMPORT  (ready_q)
        IMPORT  (current)
        IMPORT  (task_lock_cnt)
        IMPORT  (idle)

        IMPORT  (task_entry)

        EXPORT  (pendsv_handler)
        EXPORT  (sched_start)
        EXPORT  (schedule)
        EXPORT  (int_lock)
        EXPORT  (int_unlock)

        .text

/**
 * sched_start - start up the task scheduling
 *
 * return: NA, will not return
 */

PROC (sched_start)
        MOVS    r0, #2
        MSR     control, r0

        MRS     r0, msp
        MOV     sp, r0

        /* set current to idle and then send pendsv to launch the highest task */

        LDR     r0, =idle
        LDR     r0, [r0]
        LDR     r1, =current
        STR     r0, [r1]

        LDR     r1, [r0, #4 * 2]
        LDR     r2, [r0, #4 * 3]
        ADD     r1, r2
        MOV     sp, r1

        BL      _send_pendsv

        BL      task_entry

ENDP (sched_start)

/**
 * pendsv_handler - pendsv handler used for task switching
 *
 * return: NA
 */

PROC (pendsv_handler)

        /*
         * must be returning to task, interrupt needless to
         * be disabled here
         */

        LDR     r1, =current
        LDR     r3, [r1]

        MRS     r0, psp

        SUBS    r0, r0, #4 * 8

        /* stm not used for better interrupt response */

        STR     r4, [r0, #4 * 0]
        STR     r5, [r0, #4 * 1]
        STR     r6, [r0, #4 * 2]
        STR     r7, [r0, #4 * 3]

        MOV     r4, r8
        MOV     r5, r9
        MOV     r6, r10
        MOV     r7, r11

        STR     r4, [r0, #4 * 4]
        STR     r5, [r0, #4 * 5]
        STR     r6, [r0, #4 * 6]
        STR     r7, [r0, #4 * 7]

        STR     r0, [r3]

        LDR     r0, =ready_q            /* &ready_q.highest */

        /* interrupt needless be disabled here. */

        LDR     r0, [r0]                /* next task */
        STR     r0, [r1]                /* current = ready_q->highest */
        LDR     r0, [r0]                /* r0 = task->regset */

        LDR     r4, [r0, #4 * 4]
        LDR     r5, [r0, #4 * 5]
        LDR     r6, [r0, #4 * 6]
        LDR     r7, [r0, #4 * 7]

        MOV     r8, r4
        MOV     r9, r5
        MOV     r10, r6
        MOV     r11, r7

        LDR     r4, [r0, #4 * 0]
        LDR     r5, [r0, #4 * 1]
        LDR     r6, [r0, #4 * 2]
        LDR     r7, [r0, #4 * 3]

        ADDS    r0, r0, #4 * 8

        MSR     psp, r0

        BX      lr                      /* do exception return */
ENDP (pendsv_handler)

/*
 * schedule - check if reshcedule is needed at the end of do_critical
 *
 * return: status
 */

PROC (schedule)
        LDR     r1, =current
        LDR     r2, [r1]
        LDR     r3, =ready_q
        LDR     r3, [r3]
        CMP     r2, r3
        BEQ     0f

        LDR     r3, =task_lock_cnt
        LDR     r3, [r3]
        CMP     r3, #0
        BNE     0f

        CMP     r2, #0                  /* r2 = current, pre-kernel, do not schedule */
        BEQ     0f

_send_pendsv:

        LDR     r3, =SCB_ICSR
        LDR     r2, =ICSR_PENDSVSET
        STR     r2, [r3]
0:
        BX      lr
ENDP (schedule)

/*
 * int_lock - disable irq
 *
 * return: the original primask
 */

PROC (int_lock)
        MRS     r0, primask
        CPSID   i
        BX      lr
ENDP (int_lock)

/*
 * int_unlock - restore primask
 * @flags: the original primask value
 *
 * return: NA
 */

PROC (int_unlock)
        MSR     primask, r0
        BX      lr
ENDP (int_unlock)

