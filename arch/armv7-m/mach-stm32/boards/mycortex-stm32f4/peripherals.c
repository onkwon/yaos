#include <kernel/module.h>
#include <pinmap.h>

REGISTER_DEVICE(uart, "uart", 1);

REGISTER_DEVICE(gpio, "gpio", PIN_STATUS_LED);

REGISTER_DEVICE(led, "led", 0);
