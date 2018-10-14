/* hal_exc.h - exception abstract library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#ifndef __HAL_EXC_H__
#define __HAL_EXC_H__

#include <stdint.h>

/* typedefs */

typedef void (* hal_exc_handler_t) (void);

typedef struct hal_exc_methods
    {
    int (* setprio)           (unsigned int vec, unsigned int prio);
    } hal_exc_methods_t;

/* externs */

extern int hal_exc_connect    (unsigned int vec, hal_exc_handler_t handler);
extern int hal_exc_connect    (unsigned int vec, hal_exc_handler_t handler);
extern int hal_exc_disconnect (unsigned int vec);
extern int hal_exc_setprio    (unsigned int vec, unsigned int prio);
extern int hal_exc_register   (const hal_exc_methods_t * methods);

#endif  /* __HAL_EXC_H__ */

