

#include <cxa_arduino_gpio.h>
#include <cxa_delay.h>


static cxa_arduino_gpio_t led_red;


void setup()
{
	cxa_arduino_gpio_init_output(&led_red, 0, CXA_GPIO_POLARITY_NONINVERTED, 0);
}


void loop()
{
	cxa_gpio_toggle(&led_red.super);
	cxa_delay_ms(250);
}
