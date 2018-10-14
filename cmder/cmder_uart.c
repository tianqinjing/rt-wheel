/* cmder_uart.c - uart cmder shell */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,07oct18,cfm  writen
*/

#include <stdio.h>

#include <wheel/hal_uart.h>
#include <wheel/cmder.h>

#include <kernel/task.h>

static int uart_getchar (uintptr_t arg)
    {
    unsigned char ch;

    hal_uart_t * uart = (hal_uart_t *) arg;

    do
        {
        hal_uart_read (uart, &ch, 1);
        } while (ch == '\0');

    if (ch != 0x1b)
        {
        return ch;
        }

    hal_uart_read (uart, &ch, 1);

    if (ch != 0x5b)
        {
        return ch;
        }

    hal_uart_read (uart, &ch, 1);

    switch (ch)
        {
        case 0x31:
            hal_uart_read (uart, &ch, 1);   /* eat 0x7e */
            return cmder_vk_home;
        case 0x33:
            hal_uart_read (uart, &ch, 1);   /* eat 0x7e */
            return cmder_vk_del;
        case 0x34:
            hal_uart_read (uart, &ch, 1);   /* eat 0x7e */
            return cmder_vk_end;
        case 0x41:
            return cmder_vk_up;
        case 0x42:
            return cmder_vk_down;
        case 0x43:
            return cmder_vk_right;
        case 0x44:
            return cmder_vk_left;
        default:
            return '\0';
        }
    }

static void uart_putchar (uintptr_t arg, char ch)
    {
    hal_uart_t * uart = (hal_uart_t *) arg;

    hal_uart_write (uart, (unsigned char *) &ch, 1);

    if (ch == '\n')
        {
        ch = '\r';
        hal_uart_write (uart, (unsigned char *) &ch, 1);
        }
    }

static int uart_putstr (uintptr_t arg, const char * str)
    {
    char ch;
    int  i = 0;

    while ((ch = *str++) != '\0')
        {
        uart_putchar (arg, ch);
        i++;
        }

    return i;
    }

cmder_t uart_cmder =
    {
    "", 0, 0, 0, 0, 0, {0}, {0}, uart_getchar, uart_putchar, uart_putstr
    };

int uart_cmder_init (void)
    {
    hal_uart_t * uart;

    static unsigned char his_content [256];
    static unsigned char his_indexes [64];

    ring_init (&uart_cmder.his_cmd, his_content, 256);
    ring_init (&uart_cmder.his_idx, his_indexes, 64);

    uart = hal_uart_get ("nrf_uart", 0);

    if (uart == NULL)
        {
        return -1;
        }

    uart_cmder.arg = (uintptr_t) uart;

    task_spawn ("t0", 0, 0, 0x1000, (int (*) (uintptr_t)) cmder_loop,
                (uintptr_t) &uart_cmder);

    return 0;
    }

