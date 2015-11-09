/**
 * Copyright 2015 opencxa.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Christopher Armenio
 */
#include <cxa_assert.h>
#include <cxa_logger_implementation.h>
#include <cxa_esp8266_gpio.h>
#include <cxa_esp8266_usart.h>
#include <cxa_esp8266_timeBase.h>
#include <cxa_esp8266_wifiManager.h>

#include <cxa_timeDiff.h>


// ******** local macro definitions ********
#define LED_ONPERIOD_CONNECTING_MS			500
#define LED_OFFPERIOD_CONNECTING_MS			500
#define LED_ONPERIOD_CONNECTED_MS			1000
#define LED_OFFPERIOD_CONNECTED_MS			1000
#define LED_ONPERIOD_CONFIG_NOCONN			100
#define LED_OFFPERIOD_CONFIG_NOCONN			900
#define LED_ONPERIOD_CONFIG_CONN			900
#define LED_OFFPERIOD_CONFIG_CONN			100


// ******** local function prototoypes ********
static void updateLed(void);
static void wifiManCb_configMode_enter(void* userVarIn);
static void wifiManCb_numConnChange(uint8_t numConnStations, void* userVar);
static void wifiManCb_connecting(const char *const ssidIn, void* userVarIn);
static void wifiManCb_connected(const char *const ssidIn, void* userVarIn);


// ******** local variable declarations ********


static cxa_esp8266_gpio_t led_red;

static cxa_esp8266_usart_t usart;
static cxa_ioStream_t* ios_usart;
static cxa_timeBase_t tb_generalPurpose;

static uint32_t led_onPeriod_ms = 0;
static uint32_t led_offPeriod_ms = 0;
static cxa_timeDiff_t td_blink;


// ******** global function implementations ********
void setup(void)
{
	// setup our assert LED
	cxa_esp8266_gpio_init_output(&led_red, 0, CXA_GPIO_POLARITY_INVERTED, 0);
	cxa_assert_setAssertGpio(&led_red.super);


	// setup our debug usart (and delay so things can settle)
	cxa_esp8266_usart_init_noHH(&usart, CXA_ESP8266_USART_0, 115200);
	ios_usart = cxa_usart_getIoStream(&usart.super);
	cxa_assert_setIoStream(ios_usart);

	// setup our general-purpose timebase
	cxa_esp8266_timeBase_init(&tb_generalPurpose);

	// setup our logging system
	cxa_logger_setGlobalTimeBase(&tb_generalPurpose);
	cxa_logger_setGlobalIoStream(ios_usart);

	// setup our application specific code here
	cxa_timeDiff_init(&td_blink, &tb_generalPurpose, true);

	cxa_esp8266_wifiManager_init(NULL, &tb_generalPurpose);
	cxa_esp8266_wifiManager_addListener(wifiManCb_configMode_enter, wifiManCb_numConnChange, NULL, wifiManCb_connecting, wifiManCb_connected, NULL, NULL, NULL);
	cxa_esp8266_wifiManager_addStoredNetwork("yourSsid", "yourPassphrase");
	cxa_esp8266_wifiManager_start();
}


void loop(void)
{
	cxa_esp8266_wifiManager_update();
	updateLed();

	uint8_t rxByte;
	if( cxa_ioStream_readByte(ios_usart, &rxByte) == CXA_IOSTREAM_READSTAT_GOTDATA ) cxa_ioStream_writeByte(ios_usart, rxByte);
}


// ******** local function implementations ********
static void updateLed(void)
{
	if( led_offPeriod_ms == 0 ) cxa_gpio_setValue(&led_red.super, 1);
	else if( led_onPeriod_ms == 0 ) cxa_gpio_setValue(&led_red.super, 0);
	else if( cxa_gpio_getValue(&led_red.super) && cxa_timeDiff_isElapsed_recurring_ms(&td_blink, led_onPeriod_ms) ) cxa_gpio_setValue(&led_red.super, 0);
	else if( !cxa_gpio_getValue(&led_red.super) &&cxa_timeDiff_isElapsed_recurring_ms(&td_blink, led_offPeriod_ms) ) cxa_gpio_setValue(&led_red.super, 1);
}


static void wifiManCb_configMode_enter(void* userVarIn)
{
	led_onPeriod_ms = LED_ONPERIOD_CONFIG_NOCONN;
	led_offPeriod_ms = LED_OFFPERIOD_CONFIG_NOCONN;
}


static void wifiManCb_numConnChange(uint8_t numConnStations, void* userVar)
{
	if( numConnStations > 0 )
	{
		led_onPeriod_ms = LED_ONPERIOD_CONFIG_CONN;
		led_offPeriod_ms = LED_OFFPERIOD_CONFIG_CONN;
	}
	else
	{
		led_onPeriod_ms = LED_ONPERIOD_CONFIG_NOCONN;
		led_offPeriod_ms = LED_OFFPERIOD_CONFIG_NOCONN;
	}
}


static void wifiManCb_connecting(const char *const ssidIn, void* userVarIn)
{
	led_onPeriod_ms = LED_ONPERIOD_CONNECTING_MS;
	led_offPeriod_ms = LED_OFFPERIOD_CONNECTING_MS;
}


static void wifiManCb_connected(const char *const ssidIn, void* userVarIn)
{
	led_onPeriod_ms = LED_ONPERIOD_CONNECTED_MS;
	led_offPeriod_ms = LED_OFFPERIOD_CONNECTED_MS;
}
