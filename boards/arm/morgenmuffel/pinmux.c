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
#ifdef CONFIG_UART_1
	{STM32_PIN_PB6, STM32F2_PINMUX_FUNC_PB6_USART1_TX},
	{STM32_PIN_PB7, STM32F2_PINMUX_FUNC_PB7_USART1_RX},
#endif /* #ifdef CONFIG_UART_1 */
#ifdef CONFIG_UART_2
	{STM32_PIN_PD5, STM32F2_PINMUX_FUNC_PD5_USART2_TX},
	{STM32_PIN_PD6, STM32F2_PINMUX_FUNC_PD6_USART2_RX},
#endif /* #ifdef CONFIG_UART_2 */
#ifdef CONFIG_UART_3
	{STM32_PIN_PD8, STM32F2_PINMUX_FUNC_PD8_USART3_TX},
	{STM32_PIN_PD9, STM32F2_PINMUX_FUNC_PD9_USART3_RX},
#endif /* CONFIG_UART_6 */
#ifdef CONFIG_ETH_STM32_HAL
	{STM32_PIN_PC1, STM32F2_PINMUX_FUNC_PC1_ETH},
	{STM32_PIN_PC4, STM32F2_PINMUX_FUNC_PC4_ETH},
	{STM32_PIN_PC5, STM32F2_PINMUX_FUNC_PC5_ETH},
	{STM32_PIN_PC9, STM32F4_PINMUX_FUNC_PC9_MCO2},

	{STM32_PIN_PA1, STM32F2_PINMUX_FUNC_PA1_ETH},
	{STM32_PIN_PA2, STM32F2_PINMUX_FUNC_PA2_ETH},
	{STM32_PIN_PA7, STM32F2_PINMUX_FUNC_PA7_ETH},

	{STM32_PIN_PB11, STM32F2_PINMUX_FUNC_PB11_ETH},
	{STM32_PIN_PB12, STM32F2_PINMUX_FUNC_PB12_ETH},
	{STM32_PIN_PB13, STM32F2_PINMUX_FUNC_PB13_ETH},
#endif /* CONFIG_ETH_STM32_HAL */
#ifdef CONFIG_SPI
	{STM32_PIN_PA5, STM32F2_PINMUX_FUNC_PA5_SPI1_SCK},
	{STM32_PIN_PA6, STM32F2_PINMUX_FUNC_PA6_SPI1_MISO},
	{STM32_PIN_PB5, STM32F2_PINMUX_FUNC_PB5_SPI1_MOSI},
	{STM32_PIN_PB10, STM32F2_PINMUX_FUNC_PB10_SPI2_SCK},
	{STM32_PIN_PC2, STM32F2_PINMUX_FUNC_PC2_SPI2_MISO},
	{STM32_PIN_PC3, STM32F2_PINMUX_FUNC_PC3_SPI2_MOSI},
#endif /* CONFIG_SPI */
#ifdef CONFIG_I2C
	{STM32_PIN_PB8, STM32F2_PINMUX_FUNC_PB8_I2C1_SCL},
	{STM32_PIN_PB9, STM32F2_PINMUX_FUNC_PB9_I2C1_SDA},
#endif /* CONFIG_I2C */
#ifdef CONFIG_ETH_STM32_HAL
	{STM32_PIN_PC1, STM32F2_PINMUX_FUNC_PC1_ETH},
	{STM32_PIN_PC4, STM32F2_PINMUX_FUNC_PC4_ETH},
	{STM32_PIN_PC5, STM32F2_PINMUX_FUNC_PC5_ETH},

	{STM32_PIN_PA1, STM32F2_PINMUX_FUNC_PA1_ETH},
	{STM32_PIN_PA2, STM32F2_PINMUX_FUNC_PA2_ETH},
	{STM32_PIN_PA7, STM32F2_PINMUX_FUNC_PA7_ETH},

	{STM32_PIN_PG11, STM32F2_PINMUX_FUNC_PG11_ETH},
	{STM32_PIN_PG13, STM32F2_PINMUX_FUNC_PG13_ETH},
	{STM32_PIN_PB13, STM32F2_PINMUX_FUNC_PB13_ETH},
#endif /* CONFIG_ETH_STM32_HAL */
#ifdef CONFIG_USB_DC_STM32
	{STM32_PIN_PA11, STM32F2_PINMUX_FUNC_PA11_OTG_FS_DM},
	{STM32_PIN_PA12, STM32F2_PINMUX_FUNC_PA12_OTG_FS_DP},
#endif	/* CONFIG_USB_DC_STM32 */
#ifdef CONFIG_ADC_1
	{STM32_PIN_PA0, STM32F2_PINMUX_FUNC_PA0_ADC123_IN0},
#endif /* CONFIG_ADC_1 */
#ifdef CONFIG_USB
	{STM32_PIN_PA11, STM32F2_PINMUX_FUNC_PA11_OTG_FS_DM},
	{STM32_PIN_PA12, STM32F2_PINMUX_FUNC_PA12_OTG_FS_DP},
#endif /* CONFIG_USB */
};

static int pinmux_stm32_init(const struct device *port)
{
	ARG_UNUSED(port);

	stm32_setup_pins(pinconf, ARRAY_SIZE(pinconf));

	return 0;
}

SYS_INIT(pinmux_stm32_init, PRE_KERNEL_1,
		CONFIG_PINMUX_STM32_DEVICE_INITIALIZATION_PRIORITY);
