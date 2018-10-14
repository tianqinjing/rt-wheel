/* hal_uart.h - hardware uart abstract library header file header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,23sep18,cfm  writen
*/

#ifndef __HAL_UART_H__
#define __HAL_UART_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <wheel/list.h>
#include <wheel/ring.h>

#include <kernel/sem.h>
#include <kernel/mutex.h>

/* macros */

#define HAL_UART_RING_SIZE      256

#define HAL_UART_MAX_NAME_LEN   16

#define HAL_UART_MODE_POLL      (1 << 0)
#define HAL_UART_MODE_INT       (1 << 1)

/* typedefs */

typedef struct hal_uart hal_uart_t;

typedef struct hal_uart_methods
    {
    int          (* ioctl)      (hal_uart_t * uart, int cmd, void * arg);
    size_t       (* poll_read)  (hal_uart_t * uart, unsigned char * ch);
    size_t       (* poll_write) (hal_uart_t * uart, unsigned char ch);
    int          (* tx_start)   (hal_uart_t * uart);
    } hal_uart_methods_t;

struct hal_uart
    {
    dlist_t      node;              /* node to be inserted in the timer list */
    const char * name;
    uint8_t      mode;
    uint8_t      unit;              /* unit number */
    bool         deferred_isr;
    uint32_t     baudrate;

    ring_t     * rxring;
    ring_t     * txring;

    sem_t        rxsem;
    sem_t        txsem;
    mutex_t      rxmux;
    mutex_t      txmux;

    const hal_uart_methods_t * methods;
    };

/* externs */

extern hal_uart_t * hal_uart_get        (const char * name, uint8_t unit);
extern size_t       hal_uart_poll_read  (hal_uart_t * uart, unsigned char * buff, size_t len);
extern size_t       hal_uart_read       (hal_uart_t * uart, unsigned char * buff, size_t len);
extern size_t       hal_uart_poll_write (hal_uart_t * uart, unsigned char * buff, size_t len);
extern size_t       hal_uart_write      (hal_uart_t * uart, unsigned char * buff, size_t len);
extern void         hal_rx_putc         (hal_uart_t * uart, unsigned char ch);
extern size_t       hal_tx_getc         (hal_uart_t * uart, unsigned char * ch);
extern int          hal_uart_register   (hal_uart_t * uart);

#endif  /* __HAL_UART_H__ */

