/*
 * power.c
 *
 * Implements the power supply functions
 *
 *  Created on: 20.01.2017
 *      Author: marco
 */

#include "power.h"
#include <device.h>
#include <drivers/gpio.h>

#include "hardwareconfig.h"

/**
 * Initialise GPIO related to the power supply control pins.
 */
int powerInit(void)
{
	const struct device *gpioPort = device_get_binding(POWER_1V8_GPIO_Port);
	int rc;

	rc = gpio_pin_configure(gpioPort, POWER_1V8_Pin, GPIO_OUTPUT);
	if (rc != 0) {
		return rc;
	}
	gpioPort = device_get_binding(POWER_3V3_GPIO_Port);
	rc = gpio_pin_configure(gpioPort, POWER_3V3_Pin, GPIO_OUTPUT);
	if (rc != 0) {
		return rc;
	}
	gpioPort = device_get_binding(POWER_5V_GPIO_Port);
	rc = gpio_pin_configure(gpioPort, POWER_5V_Pin, GPIO_OUTPUT);
	if (rc != 0) {
		return rc;
	}
	gpioPort = device_get_binding(POWER_BKFD_GPIO_Port);
	rc = gpio_pin_configure(gpioPort, POWER_BKFD_Pin, GPIO_OUTPUT);
	return rc;
}

/**
 * Switches the 1.8V power supply on or off.
 *
 * @param on	On-state of the power supply (0: off, else: on)
 */
int powerSet1v8(bool on)
{
	const struct device *gpioPort = device_get_binding(POWER_1V8_GPIO_Port);
	int rc;

	rc = gpio_pin_set(gpioPort, POWER_1V8_Pin, 1);
	return rc;
}

/**
 * Switches the 3.3V power supply on or off.
 *
 * @param on	On-state of the power supply (0: off, else: on)
 */
int powerSet3v3(bool on)
{
	const struct device *gpioPort = device_get_binding(POWER_3V3_GPIO_Port);
	int rc;

	rc = gpio_pin_set(gpioPort, POWER_3V3_Pin, 1);
	return rc;
}

/**
 * Switches the 5V LED power supply on or off.
 *
 * @param on	On-state of the power supply (0: off, else: on)
 */
int powerSetLed(bool on)
{
	const struct device *gpioPort;
	int rc;

	gpioPort = device_get_binding(POWER_5V_GPIO_Port);
	rc = gpio_pin_set(gpioPort, POWER_5V_Pin, 1);
	return rc;
}

/**
 * Switches the backfeed power supply on or off.
 *
 * @param on	On-state of the power supply (0: off, else: on)
 */
int powerSetBackfeed(bool on)
{
	const struct device *gpioPort = device_get_binding(POWER_BKFD_GPIO_Port);
	int rc;

	rc = gpio_pin_set(gpioPort, POWER_BKFD_Pin, 1);
	return rc;
}

/**
 * Returns the current state of the 1.8V power supply.
 *
 * @return	The on state of the 1.8V power supply.
 */
int powerGet1v8(void)
{
	const struct device *gpioPort = device_get_binding(POWER_1V8_GPIO_Port);
	int pinState;

	pinState = gpio_pin_get(gpioPort, POWER_1V8_Pin);
	return pinState;
}

/**
 * Returns the current state of the 3.3V power supply.
 *
 * @return	The on state of the 3.3V power supply.
 */
int powerGet3v3(void)
{
	const struct device *gpioPort = device_get_binding(POWER_3V3_GPIO_Port);
	int pinState;

	pinState = gpio_pin_get(gpioPort, POWER_3V3_Pin);
	return pinState;
}

/**
 * Returns the current state of the 5V LED power supply.
 *
 * @return	The on state of the 5V power supply.
 */
int powerGetLed(void)
{
	const struct device *gpioPort = device_get_binding(POWER_5V_GPIO_Port);
	int pinState;

	pinState = gpio_pin_get(gpioPort, POWER_5V_Pin);
	return pinState;
}

/**
 * Returns the current state of the backfeed power supply.
 *
 * @return	The on state of the backfeed power supply.
 */
int powerGetBackfeed(void)
{
	const struct device *gpioPort = device_get_binding(POWER_BKFD_GPIO_Port);
	int pinState;

	pinState = gpio_pin_get(gpioPort, POWER_BKFD_Pin);
	return pinState;
}
