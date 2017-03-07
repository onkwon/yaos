#include <kernel/module.h>
#include <pinmap.h>

REGISTER_DEVICE(usart, "usart", 1);

REGISTER_DEVICE(timer, "tim", 2);
REGISTER_DEVICE(timer, "tim", 3);

REGISTER_DEVICE(gpio, "gpio", 2);
