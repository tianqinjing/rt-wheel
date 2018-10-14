/* hal_exc.c - exception abstract library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#include <stddef.h>

#include <wheel/config.h>
#include <wheel/hal_exc.h>

/* globals */

/* statics */

// TODO: init with default handler, that will deal with signal on map, sig disp..

static hal_exc_handler_t hal_exc_vector [RTW_NR_EXCS];

static const hal_exc_methods_t * hal_exc_methods;

/**
 * hal_exc_setprio - set the priority of a specific exception
 * @exc:  the irq number
 * @prio: the priority of the irq
 *
 * return: 0 on success, negtive value on error
 */

int hal_exc_setprio (unsigned int exc, unsigned int prio)
    {
    if (!hal_exc_methods || !hal_exc_methods->setprio)
        {
        return -1;
        }

    return hal_exc_methods->setprio (exc, prio);
    }

/**
 * hal_exct_dispatch - run callback routine for an exception
 * @vec: the exception vector number
 *
 * return: NA
 */

void hal_exc_dispatch (unsigned int vec)
    {
    if (vec >= RTW_NR_EXCS)
        {
        return;
        }

    if (hal_exc_vector [vec] == NULL)
        {
        return;
        }

    hal_exc_vector [vec] ();
    }

/**
 * hal_exc_connect - connect a callback to an exception
 * @vec:     the irq number to attach to
 * @handler: irq handler to be installed
 * @arg:     argument for the handler
 *
 * return: 0 on success, negtive value on error
 */

int hal_exc_connect (unsigned int vec, hal_exc_handler_t handler)
    {
    if (vec >= RTW_NR_EXCS)
        {
        return -1;
        }

    if (hal_exc_vector [vec] != NULL)
        {
        return -1;
        }

    hal_exc_vector [vec] = handler;

    return 0;
    }

/**
 * hal_exc_disconnect - disconnect a routine to a hardware interrupt
 * @vec: the irq number to be attached
 *
 * return: 0 on success, negtive value on error
 */

int hal_exc_disconnect (unsigned int vec)
    {
    if (vec >= RTW_NR_EXCS)
        {
        return -1;
        }

    hal_exc_vector [vec] = NULL;

    return 0;
    }

/**
 * hal_exc_register - register an exception controler
 * @methods:   the interrupt controler methods
 *
 * return: 0 on success, negtive value on error
 */

int hal_exc_register (const hal_exc_methods_t * methods)
    {
    if (methods == NULL)
        {
        return -1;
        }

    hal_exc_methods = methods;

    return 0;
    }

