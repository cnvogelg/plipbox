#include "hardware/uart.h"
#include "hardware/gpio.h"

#include "types.h"

#include "hw_uart.h"

#define HW_UART_TX_PIN    0
#define HW_UART_RX_PIN    1

void hw_uart_init(void)
{
  uart_init(uart0, CONFIG_BAUD_RATE);

  gpio_set_function(HW_UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(HW_UART_RX_PIN, GPIO_FUNC_UART);
}

void hw_uart_send(u08 data)
{
  uart_putc_raw(uart0, data);
}
