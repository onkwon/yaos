#include <kernel/module.h>
#include <pinmap.h>

REGISTER_DEVICE(uart, "uart", 1);

REGISTER_DEVICE(timer, "tim", 2);
REGISTER_DEVICE(timer, "tim", 3);

REGISTER_DEVICE(gpio, "gpio", PIN_DEBUG);
