/* startup.s - startup file, including vector table and irq, exception entries */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,20aug18,cfm  writen
*/

#include <arch/aarch-m/asm.h>

#include "../hw_config.h"

#define NRF_POWER_RAMON_ADDRESS                 0x40000524
#define NRF_POWER_RAMONB_ADDRESS                0x40000554
#define NRF_POWER_RAMONx_RAMxON_ONMODE_Msk      0x3

        IMPORT  (pendsv_handler)
        IMPORT  (exc_handler)
        IMPORT  (irq_handler)

        EXPORT  (reset_handler)
        EXPORT  (__stack)       /* will be used in _start */

        .bss

        .fill   0x200, 1, 0
__msp_top:
__stack:

        .section .vectors

vectors:
        .long   __msp_top
        .long   reset_handler   /* reset        */
        .long   exc_handler     /* NMI          */
        .long   exc_handler     /* HardFault    */
        .long   exc_handler     /* MemManage    */
        .long   exc_handler     /* BusFault     */
        .long   exc_handler     /* UsageFault   */
        .long   0
        .long   0
        .long   0
        .long   0
        .long   exc_handler     /* SVCall       */
        .long   exc_handler     /* DebugMonitor */
        .long   0
        .long   pendsv_handler  /* PendSV       */
        .long   0               /* SysTick, NA  */

#ifdef RTW_CONFIG_IRQ_DISPATCH
        .rept   NR_IRQS
        .long   irq_handler
        .endr
#else
        /* customer irq handlers goes here */
#endif

        .text

PROC (reset_handler)

        /* make sure ALL RAM banks are powered on */

        MOVS    r1, #NRF_POWER_RAMONx_RAMxON_ONMODE_Msk

        LDR     r0, =NRF_POWER_RAMON_ADDRESS
        LDR     r2, [r0]
        ORRS    r2, r1
        STR     r2, [r0]

        LDR     r0, =NRF_POWER_RAMONB_ADDRESS
        LDR     r2, [r0]
        ORRS    r2, r1
        STR     r2, [r0]

        /* copy data section, .bss will be zeroed in _start */

        LDR     r1, =__etext
        LDR     r2, =__data_start__
        LDR     r3, =__data_end__

        SUBS    r3, r2
        BLE     1f
0:
        SUBS    r3, 4
        LDR     r0, [r1,r3]
        STR     r0, [r2,r3]
        BGT     0b
1:
        LDR     r0, =_start
        BX      r0
ENDP (reset_handler)

