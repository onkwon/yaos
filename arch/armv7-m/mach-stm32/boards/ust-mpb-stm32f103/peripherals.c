#include <kernel/module.h>
#include <pinmap.h>

REGISTER_DEVICE(uart, "uart", 1);

REGISTER_DEVICE(gpio, "gpio", PIN_STATUS_LED);

REGISTER_DEVICE(timer, "tim", 2);
REGISTER_DEVICE(timer, "tim", 3);
REGISTER_DEVICE(timer, "tim", 4);
REGISTER_DEVICE(timer, "tim", 5);

REGISTER_DEVICE(led, "led", 0);
