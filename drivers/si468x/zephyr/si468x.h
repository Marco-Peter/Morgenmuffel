/*
 * Copyright (c) 2020 Marco Peter
 */
#ifndef __SI468X_H__
#define __SI468X_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <device.h>
#include <drivers/gpio.h>

enum si468x_mode {
        si468x_MODE_DAB = CONFIG_SI468X_FLASH_START_IMAGE_DAB,
        si468x_MODE_FM = CONFIG_SI468X_FLASH_START_IMAGE_FM,
        si468x_MODE_AM = CONFIG_SI468X_FLASH_START_IMAGE_AM
};

struct si468x_config {
	gpio_flags_t int_gpio_flags;
	gpio_flags_t reset_gpio_flags;
	gpio_flags_t cs_gpio_flags;
        uint16_t spi_slave_number;
	char *spi_bus_label;
	char *int_gpio_label;
	char *reset_gpio_label;
	char *cs_gpio_label;
	gpio_pin_t int_gpio_pin;
	gpio_pin_t reset_gpio_pin;
	gpio_pin_t cs_gpio_pin;
};

struct si468x_data {
	const struct device *spi;
	const struct device *int_gpio;
	const struct device *reset_gpio;
	const struct device *cs_gpio;
        bool clear_to_send;
};

struct si468x_api {
        int (*startup)(const struct device *dev, enum si468x_mode mode);
        int (*powerdown)(const struct device *dev);
};

static inline int si468x_startup(const struct device *dev, enum si468x_mode mode)
{
        const struct si468x_api *api = (const struct si468x_api *)dev->api;

        return api->startup(dev, mode);
}

static inline int si468x_powerdown(const struct device *dev)
{
        const struct si468x_api *api = (const struct si468x_api *)dev->api;

        return api->powerdown(dev);
}

#ifdef __cplusplus
}
#endif

#endif /* __SI468X_H__ */
