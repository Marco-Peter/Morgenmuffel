/*
 * Copyright (c) 2019 Marco Peter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <device.h>
#include <init.h>
#include <drivers/pinmux.h>
#include <sys/sys_io.h>

#include <pinmux/stm32/pinmux_stm32.h>

/* pin assignments for MORGENMUFFEL board */
static const struct pin_config pinconf[] = {
#ifdef CONFIG_ETH_STM32_HAL
	/* MCO2 configuration of pin PC9 */
	{ STM32_PIN_PC9, (STM32_PINMUX_ALT_FUNC_0 | STM32_PUSHPULL_NOPULL |
			  STM32_OSPEEDR_VERY_HIGH_SPEED) },
#endif /* CONFIG_ETH_STM32_HAL */
};

static int pinmux_stm32_init(const struct device *port)
{
	ARG_UNUSED(port);

	stm32_setup_pins(pinconf, ARRAY_SIZE(pinconf));

	return 0;
}

SYS_INIT(pinmux_stm32_init, PRE_KERNEL_1,
	 CONFIG_PINMUX_STM32_DEVICE_INITIALIZATION_PRIORITY);
