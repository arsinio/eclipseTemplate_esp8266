#include "ets_sys.h"
#include "gpio.h"
#include "osapi.h"


void user_init(void)
{
	int pin = 0;
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	gpio_output_set(0, 0, 1 << pin, 0); // enable pin as output

	while(1)
	{
		gpio_output_set(1 << pin, 0, 0, 0); // set pin high
		os_delay_us(1000000);
		gpio_output_set(0, 1 << pin, 0, 0); // set pin low
		os_delay_us(1000000);
	}

	return;
}
