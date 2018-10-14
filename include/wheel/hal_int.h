/* hal_int.h - interrupt controller abstract library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#ifndef __HAL_INT_H__
#define __HAL_INT_H__

#include <stdint.h>

/* typedefs */

typedef void (* hal_int_handler_t) (uintptr_t);

typedef struct hal_int_methods
    {
    int (* enable)            (unsigned int irq);
    int (* disable)           (unsigned int irq);
    int (* setprio)           (unsigned int irq, unsigned int prio);
    } hal_int_methods_t;

/* externs */

extern int hal_int_connect    (unsigned int irq, hal_int_handler_t handler,
                               uintptr_t arg);
extern int hal_int_disconnect (unsigned int irq);
extern int hal_int_setprio    (unsigned int irq, unsigned int prio);
extern int hal_int_enable     (unsigned int irq);
extern int hal_int_disable    (unsigned int irq);
extern int hal_int_register   (const hal_int_methods_t * methods);

#endif  /* __HAL_INTC___ */

