/* hal_uart.h - hardware uart abstract library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,23sep18,cfm  writen
*/

#include <limits.h>
#include <string.h>

#include <wheel/common.h>
#include <wheel/hal_uart.h>
#include <wheel/irq.h>

#include <kernel/mutex.h>

/* locals */

static dlist_t hal_uarts     = DLIST_INIT (hal_uarts);
static mutex_t hal_uarts_mux = MUTEX_INIT (hal_uarts_mux);

/**
 * hal_uart_get - get (allocate) a uart by name
 * @name: the uart name
 * @unit: the uart unit (0, 1 ...)
 *
 * return: a pointer to the uart control struct or NULL if device not found
 */

hal_uart_t * hal_uart_get (const char * name, uint8_t unit)
    {
    dlist_t    * itr;
    hal_uart_t * uart;
    hal_uart_t * ret = NULL;

    mutex_lock (&hal_uarts_mux);

    dlist_foreach (itr, &hal_uarts)
        {
        uart = container_of (itr, hal_uart_t, node);

        if (strncmp (name, uart->name, HAL_UART_MAX_NAME_LEN))
            {
            continue;
            }

        if (uart->unit == unit)
            {
            ret = uart;
            break;
            }
        }

    mutex_unlock (&hal_uarts_mux);

    return ret;
    }

/**
 * hal_uart_poll_read - read from uart by polling mode
 * @uart: the uart device
 * @buff: the receiver buffer address
 * @len:  the receiver buffer length
 *
 * return: the number of bytes really read
 */

size_t hal_uart_poll_read (hal_uart_t * uart, unsigned char * buff, size_t len)
    {
    size_t i;

    if ((uart == NULL) || (buff == NULL) || (uart->methods->poll_read == NULL))
        {
        return 0;
        }

    for (i = 0; i < len; i++)
        {
        if (uart->methods->poll_read (uart, &buff [i]) != 0)
            {
            break;
            }
        }

    return i;
    }

static inline size_t __uart_getc (hal_uart_t * uart, unsigned char * buff,
                                  unsigned int timeout)
    {
    unsigned long flags = 0;
    size_t        ret;

    if (sem_timedwait (&uart->rxsem, timeout) != 0)
        {
        return 0;
        }

    if (mutex_timedlock (&uart->rxmux, timeout) != 0)
        {
        sem_post (&uart->rxsem);
        return 0;
        }

    if (!uart->deferred_isr)
        {
        flags = int_lock ();
        }

    ret = ring_getc (uart->rxring, buff);

    if (!uart->deferred_isr)
        {
        int_unlock (flags);
        }

    mutex_unlock (&uart->rxmux);

    return ret;
    }

/**
 * hal_uart_read - read from uart by asynchronous mode
 * @uart: the uart device
 * @buff: the receiver buffer address
 * @len:  the receiver buffer length
 *
 * return: the number of bytes really read
 */

size_t hal_uart_read (hal_uart_t * uart, unsigned char * buff, size_t len)
    {
    size_t i;

    if (uart->mode == HAL_UART_MODE_POLL)
        {
        return hal_uart_poll_read (uart, buff, len);
        }

    if ((uart == NULL) || (buff == NULL))
        {
        return 0;
        }

    for (i = 0; i < len; i++)
        {
        if (__uart_getc (uart, &buff [i], UINT_MAX) == 0)
            {
            break;
            }
        }

    return len;
    }

// TODO: hal_uart_timedread

/**
 * hal_uart_poll_write - write buffer to uart by polling mode
 * @uart: the uart device
 * @buff: the writer buffer address
 * @len:  the writer buffer length
 *
 * return: the number of bytes really write
 */

size_t hal_uart_poll_write (hal_uart_t * uart, unsigned char * buff, size_t len)
    {
    size_t i;

    if ((uart == NULL) || (buff == NULL) || (uart->methods->poll_write == NULL))
        {
        return 0;
        }

    for (i = 0; i < len; i++)
        {
        if (uart->methods->poll_write (uart, buff [i]) != 0)
            {
            break;
            }
        }

    return i;
    }

static inline size_t __uart_putc (hal_uart_t * uart, unsigned char ch,
                                  unsigned int timeout)
    {
    unsigned long flags = 0;
    size_t        ret;
    bool          empty;

    if (sem_timedwait (&uart->txsem, timeout) != 0)
        {
        return 0;
        }

    if (mutex_timedlock (&uart->txmux, timeout) != 0)
        {
        sem_post (&uart->txsem);
        return 0;
        }

    if (!uart->deferred_isr)
        {
        flags = int_lock ();
        }

    empty = ring_empty (uart->txring);

    ret = ring_putc (uart->txring, ch);

    if (!uart->deferred_isr)
        {
        int_unlock (flags);
        }

    mutex_unlock (&uart->txmux);

    if (empty)
        {
        uart->methods->tx_start (uart);
        }

    return ret;
    }

/**
 * hal_uart_write - write buffer to uart by asynchronous mode
 * @uart: the uart device
 * @buff: the writer buffer address
 * @len:  the writer buffer length
 *
 * return: the number of bytes really write
 */

size_t hal_uart_write (hal_uart_t * uart, unsigned char * buff, size_t len)
    {
    size_t i;

    if (uart->mode == HAL_UART_MODE_POLL)
        {
        return hal_uart_poll_write (uart, buff, len);
        }

    if ((uart == NULL) || (buff == NULL))
        {
        return 0;
        }

    for (i = 0; i < len; i++)
        {
        __uart_putc (uart, buff [i], UINT_MAX);
        }

    return len;
    }

// TODO: hal_uart_timedwrite

/**
 * hal_rx_putc - put char by uart rx-ISR or a deferred rx-ISR task
 * @uart: the uart device
 * @ch:   the char to put to the rxring
 *
 * return: NA
 */

void hal_rx_putc (hal_uart_t * uart, unsigned char ch)
    {
    struct ring * ring = uart->rxring;
    bool          full;

    if (uart->deferred_isr)
        {
        mutex_lock (&uart->rxmux);
        }

    full = ring_full (ring);

    ring_putc_force (ring, ch);

    if (uart->deferred_isr)
        {
        mutex_unlock (&uart->rxmux);
        }

    if (!full)
        {
        sem_post (&uart->rxsem);
        }
    }

/**
 * hal_tx_getc - get char by uart tx-ISR or a deferred tx-ISR task
 * @uart: the uart device
 * @ch:   a pointer to char where put the char get from txring
 *
 * return: 1 if got a char from txring, 0 if the txring is empty
 */

size_t hal_tx_getc (hal_uart_t * uart, unsigned char * ch)
    {
    size_t        ret;
    struct ring * ring = uart->txring;

    if (uart->deferred_isr)
        {
        mutex_lock (&uart->txmux);
        }

    ret = ring_empty (ring) ? 0 : ring_getc (ring, ch);

    if (uart->deferred_isr)
        {
        mutex_unlock (&uart->txmux);
        }

    if (ret != 0)
        {
        sem_post (&uart->txsem);
        }

    return ret;
    }

/**
 * hal_uart_register - register a uart hal device
 * @uart: the uart device to register
 *
 * return: 0 on success, negtive value on error
 */

int hal_uart_register (hal_uart_t * uart)
    {
    if ((uart->name == NULL) || (uart->methods == NULL))
        {
        return -1;
        }

    switch (uart->mode)
        {
        case HAL_UART_MODE_INT:
            if (uart->methods->tx_start == NULL)
                {
                return -1;
                }

            break;
        case HAL_UART_MODE_POLL:
            if ((uart->methods->poll_write == NULL) ||
                (uart->methods->poll_read  == NULL))
                {
                return -1;
                }

            break;
        default:
            return -1;
        }

    if (uart->baudrate == 0)
        {
        return -1;
        }

    if (sem_init (&uart->rxsem, 0))
        {
        return -1;
        }

    if (sem_init (&uart->txsem, HAL_UART_RING_SIZE))
        {
        return -1;
        }

    if (mutex_init (&uart->rxmux))
        {
        return -1;
        }

    if (mutex_init (&uart->txmux))
        {
        return -1;
        }

    uart->rxring = ring_create (HAL_UART_RING_SIZE);

    if (uart->rxring == NULL)
        {
        return -1;
        }

    uart->txring = ring_create (HAL_UART_RING_SIZE);

    if (uart->txring == NULL)
        {
        ring_destroy (uart->rxring);

        return -1;
        }

    dlist_add_tail (&hal_uarts, &uart->node);

    return 0;

    // TODO: devfs
    }

