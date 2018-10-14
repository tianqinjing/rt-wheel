/* sysclc.h - system clock library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,16sep18,cfm  writen
*/

#include <stddef.h>

#include <kernel/tick.h>

#include <wheel/config.h>
#include <wheel/hal_timer.h>

/* locals */

static hal_timer_t * systim = NULL;

/**
 * sysclk_init - system clock init
 *
 * return: NA
 */

int sysclk_init (void)
    {
    systim = hal_timer_get (RTW_TICK_TIME_NAME, HAL_TIMER_MODE_REPEATED);

    if (!systim)
        {
        return -1;
        }

    hal_timer_connect (systim, tick_shot_n, 1);

    hal_timer_enable (systim, systim->freq / RTW_SYS_TICK_HZ);

    return 0;
    }

/**
 * sysclk_timestamp - system clock timestamp get
 *
 * return: NA
 */

uint64_t sysclk_timestamp (void)
    {
    return hal_timer_counter (systim);
    }

