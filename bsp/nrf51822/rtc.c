/* rtc.h - nrf51 rtc timer library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,22sep18,cfm  writen
*/

#include <stdint.h>

#include <wheel/hal_int.h>
#include <wheel/hal_timer.h>
#include <wheel/driver.h>

static const unsigned int rtc_irq [2] = {11, 17};

static struct
    {
    volatile uint32_t  tasks_hfclkstart;
    volatile uint32_t  tasks_hfclkstop;
    volatile uint32_t  tasks_lfclkstart;
    volatile uint32_t  tasks_lfclkstop;
    volatile uint32_t  tasks_cal;
    volatile uint32_t  tasks_ctstart;
    volatile uint32_t  tasks_ctstop;
    volatile uint32_t  pad0 [57];
    volatile uint32_t  events_hfclkstarted;
    volatile uint32_t  events_lfclkstarted;
    volatile uint32_t  pad1;
    volatile uint32_t  events_done;
    volatile uint32_t  events_ctto;
    volatile uint32_t  pad2 [124];
    volatile uint32_t  intenset;
    volatile uint32_t  intenclr;
    volatile uint32_t  pad3 [63];
    volatile uint32_t  hfclkrun;
    volatile uint32_t  hfclkstat;
    volatile uint32_t  pad4 [1];
    volatile uint32_t  lfclkrun;
    volatile uint32_t  lfclkstat;
    volatile uint32_t  lfclksrccopy;

    volatile uint32_t  pad5[62];
    volatile uint32_t  lfclksrc;
    volatile uint32_t  pad6 [7];
    volatile uint32_t  ctiv;
    volatile uint32_t  pad7 [5];
    volatile uint32_t  xtalfreq;
    } * const nrf_clock = (void *) 0x40000000;

static struct
    {
    volatile uint32_t  tasks_start;
    volatile uint32_t  tasks_stop;
    volatile uint32_t  tasks_clear;
    volatile uint32_t  tasks_trigovrflw;
    volatile uint32_t  pad0 [60];
    volatile uint32_t  events_tick;
    volatile uint32_t  events_ovrflw;
    volatile uint32_t  pad1 [14];
    volatile uint32_t  events_compare [4];
    volatile uint32_t  pad2 [109];
    volatile uint32_t  intenset;
    volatile uint32_t  intenclr;
    volatile uint32_t  pad3 [13];
    volatile uint32_t  evten;
    volatile uint32_t  evtenset;
    volatile uint32_t  evtenclr;
    volatile uint32_t  pad4 [110];
    volatile uint32_t  counter;
    volatile uint32_t  prescaler;
    volatile uint32_t  pad5 [13];
    volatile uint32_t  cc [4];
    volatile uint32_t  pad6 [683];
    volatile uint32_t  power;
    } * const nrf_rtc [2] = {(void *) 0x4000b000, (void *) 0x40001100};

static void rtc_handler (uintptr_t arg)
    {
    hal_timer_t * timer = (hal_timer_t *) arg;

    if (!nrf_rtc [timer->unit]->events_compare [0])
        {
        return;
        }

    nrf_rtc [timer->unit]->events_compare [0] = 0;  /* clear event */
    nrf_rtc [timer->unit]->tasks_clear        = 1;  /* clear counter */

    timer->handler (timer->arg);
    }

static int rtc_enable (hal_timer_t * this, uint64_t max_count)
    {
    hal_timer_t * timer = (hal_timer_t *) this;

    nrf_rtc [timer->unit]->cc [0]   = max_count;

    nrf_rtc [timer->unit]->evten    = 1 << 16;  /* compare0 event enable */
    nrf_rtc [timer->unit]->intenset = 1 << 16;  /* compare0 int enable */

    hal_int_setprio (rtc_irq [timer->unit], 3);

    hal_int_enable (rtc_irq [timer->unit]);

    nrf_rtc [timer->unit]->tasks_start = 1;

    return 0;
    }

static int rtc_disable (hal_timer_t * this)
    {
    hal_timer_t * timer = (hal_timer_t *) this;

    nrf_rtc [timer->unit]->tasks_start = 0;

    nrf_rtc [timer->unit]->intenclr = 1 << 16;  /* compare0 int enable */

    return 0;
    }

static int rtc_connect (hal_timer_t * this, void (* pfn) (uintptr_t),
                        uintptr_t arg)
    {
    return 0;                                   /* do nothing */
    }

static uint64_t rtc_counter (hal_timer_t * this)
    {
    hal_timer_t * timer = (hal_timer_t *) this;

    return (uint64_t) nrf_rtc [timer->unit]->counter;
    }


static int rtc_init (void)
    {
    static const hal_timer_methods_t rtc_methods =
        {
        .enable    = rtc_enable,
        .disable   = rtc_disable,
        .connect   = rtc_connect,
        .counter   = rtc_counter
        };

    static hal_timer_t rtc_timer [2] =
        {
            {
            .name      = "rtc",
            .unit      = 0,
            .busy      = 0,
            .down      = false,
            .freq      = 32768,
            .max_count = 0xffffff,
            .methods   = &rtc_methods
            },
            {
            .name      = "rtc",
            .unit      = 1,
            .busy      = 0,
            .down      = false,
            .freq      = 32768,
            .max_count = 0xffffff,
            .methods   = &rtc_methods
            },
        };

    nrf_clock->lfclksrc            = 1;         /* use xtal 32768hz */
    nrf_clock->events_lfclkstarted = 0;
    nrf_clock->tasks_lfclkstart    = 1;

    while (!nrf_clock->events_lfclkstarted);

    nrf_clock->events_lfclkstarted = 0;

    nrf_rtc [0]->prescaler = 0;
    nrf_rtc [1]->prescaler = 0;

    if (hal_int_connect (rtc_irq [0], rtc_handler, (uintptr_t) &rtc_timer [0]))
        {
        return -1;
        }

    if (hal_int_connect (rtc_irq [1], rtc_handler, (uintptr_t) &rtc_timer [1]))
        {
        hal_int_disconnect (rtc_irq [1]);

        return -1;
        }

    (void) hal_timer_register (&rtc_timer [0]);
    (void) hal_timer_register (&rtc_timer [1]);

    return 0;
    }

RTW_DRIVER_DEF (rtc_init);

