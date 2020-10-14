/*
 * Copyright (c) 2020 Marco Peter
 * 
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <init.h>
#include <drivers/gpio.h>

static int powersupply_morgenmuffel_init(const struct device *dev)
{
	int rc;
	const struct device *gpioPort;

	ARG_UNUSED(dev);

	gpioPort = device_get_binding("GPIOD");
	rc = gpio_pin_configure(gpioPort, 7, GPIO_OUTPUT_ACTIVE);

	return rc;
}

SYS_INIT(powersupply_morgenmuffel_init, APPLICATION,
	 CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
