#include <wheel/hal_uart.h>
#include <wheel/driver.h>
#include <wheel/hal_int.h>

static struct
    {
    volatile uint32_t tasks_startrx;
    volatile uint32_t tasks_stoprx;
    volatile uint32_t tasks_starttx;
    volatile uint32_t tasks_stoptx;
    volatile uint32_t pad0 [3];
    volatile uint32_t tasks_suspend;
    volatile uint32_t pad1 [56];
    volatile uint32_t events_cts;
    volatile uint32_t events_ncts;
    volatile uint32_t events_rxdrdy;
    volatile uint32_t pad2 [4];
    volatile uint32_t events_txdrdy;
    volatile uint32_t pad3;
    volatile uint32_t events_error;
    volatile uint32_t pad4 [7];
    volatile uint32_t events_rxto;
    volatile uint32_t pad5 [46];
    volatile uint32_t shorts;
    volatile uint32_t pad6 [64];
    volatile uint32_t intenset;
    volatile uint32_t intenclr;
    volatile uint32_t pad7 [93];
    volatile uint32_t errorsrc;
    volatile uint32_t pad8 [31];
    volatile uint32_t enable;
    volatile uint32_t pad9;
    volatile uint32_t pselrts;
    volatile uint32_t pseltxd;
    volatile uint32_t pselcts;
    volatile uint32_t pselrxd;
    volatile uint32_t rxd;
    volatile uint32_t txd;
    volatile uint32_t pad10;
    volatile uint32_t baudrate;
    volatile uint32_t pad11 [17];
    volatile uint32_t config;
    volatile uint32_t pad12 [675];
    volatile uint32_t power;
    } * const nrf_uart = (void *) 0x40002000;

static struct
    {
    volatile uint32_t pad0 [321];
    volatile uint32_t out;
    volatile uint32_t outset;
    volatile uint32_t outclr;
    volatile uint32_t in;
    volatile uint32_t dir;
    volatile uint32_t dirset;
    volatile uint32_t dirclr;
    volatile uint32_t pad1 [120];
    volatile uint32_t pin_cnf [32];
    } * const nrf_gpio = (void *) 0x50000000;

#if 1
void debug_out (char ch)
{
    while (!nrf_uart->events_txdrdy);

    nrf_uart->txd = ch;
}
#endif

static void nrf_uart_handler (uintptr_t arg)
    {
    hal_uart_t * uart = (hal_uart_t *) arg;

    if (nrf_uart->events_rxdrdy != 0)
        {
        nrf_uart->events_rxdrdy = 0;

        hal_rx_putc (uart, (char) nrf_uart->rxd);
        }

    if (nrf_uart->events_txdrdy != 0)
        {
        unsigned char ch;

        if (hal_tx_getc (uart, &ch) != 0)
            {
            nrf_uart->events_txdrdy = 0;
            nrf_uart->txd = ch;
            }
        else
            {
            nrf_uart->intenclr = 1 << 7;    /* tx-stop */
            }
        }
    }

static int nrf_uart_ioctl (hal_uart_t * uart, int cmd, void * arg)
    {
    return -1;  // TODO:
    }

static size_t nrf_uart_poll_write (hal_uart_t * uart, unsigned char outchar)
    {
    return 0;  // TODO:
    }

static int nrf_uart_tx_start (hal_uart_t * uart)
    {
    nrf_uart->intenset = 1 << 7;
    return 0;
    }

static int nrf_uart_init (void)
    {
    static hal_uart_t uart;

    static const hal_uart_methods_t nrf_uart_methods =
        {
        nrf_uart_ioctl,
        NULL,
        nrf_uart_poll_write,
        nrf_uart_tx_start,
        };

    // TODO: gpio driver frame
    nrf_gpio->pin_cnf [9]  = 1;     // tx, 9 output
    nrf_gpio->pin_cnf [11] = 0;     // rx, 11, in, nopull

    nrf_uart->pseltxd = 9;
    nrf_uart->pselrxd = 11;

    nrf_uart->baudrate = 0x01d7e000;
    nrf_uart->enable   = 4;

    nrf_uart->tasks_startrx = 1;
    nrf_uart->tasks_starttx = 1;
    nrf_uart->events_rxdrdy = 0;
    nrf_uart->events_txdrdy = 0;
    nrf_uart->intenclr = 0xffffffff;
    nrf_uart->intenset = 1 << 2;    // enable rx int

    uart.name         = "nrf_uart";
    uart.mode         = HAL_UART_MODE_INT;
    uart.unit         = 0;
    uart.deferred_isr = false;      // TODO:
    uart.baudrate     = 115200;
    uart.methods      = &nrf_uart_methods;

    if (hal_int_connect (2, nrf_uart_handler, (uintptr_t) &uart))
        {
        return -1;
        }

    hal_int_setprio (2, 3);

    hal_int_enable (2);

    nrf_uart->txd = 0;

    return hal_uart_register (&uart);
    }

 RTW_DRIVER_DEF (nrf_uart_init);

