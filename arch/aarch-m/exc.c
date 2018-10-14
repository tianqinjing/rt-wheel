/* exc.h - arm-m exception abstract library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,22sep18,cfm  writen
*/

#include <wheel/hal_exc.h>

// TODO:

static int exc_setprio (unsigned int vec, unsigned int prio)
    {
    return -1;
    }

int exc_init (void)
    {
    static const hal_exc_methods_t nvic_methods =
        {
        .setprio = exc_setprio
        };

    return hal_exc_register (&nvic_methods);
    }

