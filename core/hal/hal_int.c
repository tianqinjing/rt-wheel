/* hal_int.c - interrupt controller abstract library */

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
#include <wheel/hal_int.h>

/* globals */

unsigned int int_cnt = 0;

/* statics */

static struct
    {
    hal_int_handler_t handler;
    uintptr_t         arg;
    } hal_int_vector [RTW_NR_IRQS];

static const hal_int_methods_t * hal_int_methods = NULL;

/**
 * hal_int_dispatch - run interrupt routine for an irq
 * @irq: the irq number
 *
 * return: NA
 */

void hal_int_dispatch (unsigned int irq)
    {
    if (irq >= RTW_NR_IRQS)
        {
        return;
        }

    if (hal_int_vector [irq].handler == NULL)
        {
        return;
        }

    hal_int_vector [irq].handler (hal_int_vector [irq].arg);
    }

/**
 * hal_int_connect - connect a routine to a hardware interrupt
 * @irq:     the irq number to attach to
 * @handler: irq handler to be installed
 * @arg:     argument for the handler
 *
 * return: 0 on success, negtive value on error
 */

int hal_int_connect (unsigned int irq, hal_int_handler_t handler, uintptr_t arg)
    {
    if (irq >= RTW_NR_IRQS)
        {
        return -1;
        }

    if (hal_int_vector [irq].handler != NULL)
        {
        return -1;
        }

    hal_int_vector [irq].arg     = arg;
    hal_int_vector [irq].handler = handler;

    return 0;
    }

/**
 * hal_int_disconnect - disconnect a routine to a hardware interrupt
 * @irq: the irq number to be attached
 *
 * return: 0 on success, negtive value on error
 */

int hal_int_disconnect (unsigned int irq)
    {
    if (irq >= RTW_NR_IRQS)
        {
        return -1;
        }

    hal_int_vector [irq].handler = NULL;

    return 0;
    }

/**
 * hal_int_setprio - set the priority of a specific irq
 * @irq:  the irq number
 * @prio: the priority of the irq
 *
 * return: 0 on success, negtive value on error
 */

int hal_int_setprio (unsigned int irq, unsigned int prio)
    {
    if (!hal_int_methods || !hal_int_methods->setprio)
        {
        return -1;
        }

    return hal_int_methods->setprio (irq, prio);
    }

/**
 * hal_int_enable - enable a specific irq
 * @irq:  the irq number to be enabled
 *
 * return: 0 on success, negtive value on error
 */

int hal_int_enable (unsigned int irq)
    {
    if (!hal_int_methods || !hal_int_methods->enable)
        {
        return -1;
        }

    return hal_int_methods->enable (irq);
    }

/**
 * hal_int_disable - disable a specific irq
 * @irq: the irq number to be disabled
 *
 * return: 0 on success, negtive value on error
 */

int hal_int_disable (unsigned int irq)
    {
    if (!hal_int_methods || !hal_int_methods->disable)
        {
        return -1;
        }

    return hal_int_methods->disable (irq);
    }

/**
 * hal_int_register - register an interrupt controler
 * @methods:   the interrupt controler methods
 *
 * return: 0 on success, negtive value on error
 */

int hal_int_register (const hal_int_methods_t * methods)
    {
    if ((!methods) || (!methods->enable))
        {
        return -1;
        }

    hal_int_methods = methods;

    // TODO: devfs register

    return 0;
    }

