#include <kernel/task.h>
#include <kernel/sem.h>
#include <kernel/mutex.h>

#include <wheel/hal_uart.h>

// TODO: initialization process needs to be improved

int main (void)
{
    /* page frame mgr init, and phys/virt page init */

    extern int mmu_init (void);
    mmu_init ();

    extern int mem_init (void);
    mem_init ();

    task_ready_q_init ();

    extern int exc_init ();
    exc_init ();

    extern int driver_init ();
    driver_init ();

    extern int uart_cmder_init (void);
    uart_cmder_init ();

    extern void static_task_init (void);
    static_task_init ();

    extern int sysclk_init ();
    sysclk_init ();

    extern void sched_start (void);
    sched_start ();

    return 0;
}

