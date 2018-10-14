/* systick.c - driver for ARM cortex-m systick timer */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,16sep18,cfm  writen
*/

#include <stdint.h>

#include <wheel/common.h>
#include <wheel/hal_timer.h>
#include <wheel/hal_exc.h>
#include <wheel/driver.h>

// TODO: move this driver to arch

#include <arch/exc.h>
#include <arch/config.h>

/* macros */

#define SYST_MAX_COUNT      0xffffff

#define SYST_CSR            (*(volatile uint32_t *) 0xe000e010)

#define SYST_CSR_ENABLE     (1 << 0)
#define SYST_CSR_TICKINT    (1 << 1)
#define SYST_CSR_CLKSOURCE  (1 << 2)
#define SYST_CSR_COUNTFLAG  (1 << 16)

#define SYST_RVR            (*(volatile uint32_t *) 0xe000e014)
#define SYST_CVR            (*(volatile uint32_t *) 0xe000e018)
#define SYST_CALIB          (*(volatile uint32_t *) 0xe000e01c)

#define SYST_IRQ            15

static int systick_enable (hal_timer_t * this, uint64_t max_count)
    {
    hal_exc_setprio (EXC_VEC_SYSTICK, (1 << NVIC_PRIO_BITS) - 1);

    SYST_RVR = max_count;
    SYST_CVR = 0;
    SYST_CSR = 7;

    return 0;
    }

static int systick_disable (hal_timer_t * this)
    {
    SYST_CSR = 0;

    return 0;
    }

static int systick_connect (hal_timer_t * this, void (* pfn) (uintptr_t),
                            uintptr_t arg)
    {
    return 0;
    }

static uint64_t systick_counter (hal_timer_t * this)
    {
    return (uint64_t) SYST_CVR;
    }

static const hal_timer_methods_t systick_methods =
    {
    .enable    = systick_enable,
    .disable   = systick_disable,
    .connect   = systick_connect,
    .counter   = systick_counter
    };

static hal_timer_t systick_timer =
    {
    .name      = "systick",
    .unit      = 0,
    .busy      = 0,
    .down      = true,
    .freq      = 100000000,         // TODO: FIXME
    .max_count = SYST_MAX_COUNT,
    .methods   = &systick_methods
    };

static int systick_init (void)
    {
    return hal_timer_register (&systick_timer);
    }

void systick_handler (void)
    {
    if (systick_timer.mode == HAL_TIMER_MODE_ONE_SHOT)
        {
        SYST_CSR = 0;
        }

    systick_timer.handler (systick_timer.arg);
    }

RTW_DRIVER_DEF (systick_init);

