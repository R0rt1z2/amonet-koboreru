#include <platform.h>
#include <mmio.h>

#include <drivers/uart.h>

#define UART_THR_REG   (UART_BASE + 0x00)
#define UART_LSR_REG   (UART_BASE + 0x14)
#define UART_LSR_THRE  BIT(5)

static void low_uart_put(int ch)
{
    while (!(readl(UART_LSR_REG) & UART_LSR_THRE))
        ;
    writel(ch, UART_THR_REG);
}

void uart_putc(int c, void *ctx)
{
    (void)ctx;
    if (c == '\n') low_uart_put('\r');
    low_uart_put(c);
}