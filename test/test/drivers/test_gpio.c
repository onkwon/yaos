#include "unity.h"
#include "drivers/gpio.h"
#include "bitmap.h"
#include "dict.h"
#include <stdint.h>

#include "mock_hw_gpio.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static void cb(int vector)
{
	printf("Callback called %d\n", vector);
}

void test_gpio(void)
{
	uint16_t pin;

	pin = 1;
	TEST_ASSERT_EQUAL(-ENOENT, gpio_get(pin));
	hw_gpio_init_IgnoreAndReturn(0);
	TEST_ASSERT_EQUAL(0, gpio_init(pin, GPIO_MODE_OUTPUT, NULL));
	TEST_ASSERT_EQUAL(-EEXIST, gpio_init(pin, GPIO_MODE_OUTPUT, NULL));
	hw_gpio_get_IgnoreAndReturn(1);
	TEST_ASSERT_EQUAL(1, gpio_get(pin));

	pin = 0;
	TEST_ASSERT_EQUAL(-ENOENT, gpio_get(pin));
	hw_gpio_init_IgnoreAndReturn(1);
	TEST_ASSERT_EQUAL(0, gpio_init(pin,
				GPIO_MODE_INPUT
				| GPIO_CONF_PULLUP
				| GPIO_INT_FALLING,
				NULL));
	TEST_ASSERT_EQUAL(-EEXIST, gpio_init(pin, GPIO_MODE_OUTPUT, NULL));
	hw_gpio_get_IgnoreAndReturn(1);
	TEST_ASSERT_EQUAL(1, gpio_get(pin));
}
