/*
 * Copyright (c) 2020 Marco Peter
 */

#define DT_DRV_COMPAT silabs_si4689

#include "si4689.h"
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include <logging/log.h>
#include "si468x_commands.h"

LOG_MODULE_REGISTER(si4689, LOG_LEVEL_DBG);

struct si4689_config {
	gpio_flags_t int_gpio_flags;
	gpio_flags_t reset_gpio_flags;
	gpio_flags_t cs_gpio_flags;
	char *spi_bus_label;
	char *int_gpio_label;
	char *reset_gpio_label;
	char *cs_gpio_label;
	gpio_pin_t int_gpio_pin;
	gpio_pin_t reset_gpio_pin;
	gpio_pin_t cs_gpio_pin;
};

struct si4689_data {
	struct device *spi;
	struct device *int_gpio;
	struct device *reset_gpio;
	struct device *cs_gpio;
};

static int init(struct device *dev)
{
	struct si4689_data *data = dev->data;
	struct si4689_config *config = dev->config;
	int rc;

	data->spi = device_get_binding(config->spi_bus_label);
	data->int_gpio = device_get_binding(config->int_gpio_label);
	data->reset_gpio = device_get_binding(config->reset_gpio_label);
	data->cs_gpio = device_get_binding(config->cs_gpio_label);

	rc = gpio_pin_configure(data->reset_gpio, config->reset_gpio_pin,
				GPIO_OUTPUT_ACTIVE | config->reset_gpio_flags);
	if (rc != 0) {
		LOG_ERR("%s: failed to configure reset gpio with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = gpio_pin_configure(data->cs_gpio, config->cs_gpio_pin,
				GPIO_OUTPUT | config->cs_gpio_flags);
	if (rc != 0) {
		LOG_ERR("%s: failed to configure chip select gpio with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = gpio_pin_configure(data->int_gpio, config->int_gpio_pin,
				GPIO_INPUT | config->int_gpio_flags);
	if (rc != 0) {
		LOG_ERR("%s: failed to configure interrupt gpio with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = gpio_pin_interrupt_configure(data->int_gpio, config->int_gpio_pin,
					  GPIO_INT_EDGE_TO_ACTIVE);
	if (rc != 0) {
		LOG_ERR("%s: failed to configure interrupt pin with rc %d",
			dev->name, rc);
		return rc;
	}
	return rc;
}

static int startup(const struct device *dev, enum si4689_mode mode)
{
	struct si4689_data *data = dev->data;
	struct si4689_config *config = dev->config;
	int rc;

	rc = gpio_pin_set(data->reset_gpio, config->reset_gpio_pin, 0);
	if(rc != 0) {
		LOG_ERR("%s: failed to release the reset line with rc %d", dev->name, rc);
		return rc;
	}
	k_sleep(K_MSEC(4));

	const uint8_t cmd[] = {
		SI468X_CMD_POWER_UP,
		0U,
		si468x_clk
	}

	return 0;
}

static int powerdown(const struct device *dev)
{
	return 0;
}

#define SI4689_DEVICE(id)                                                      \
	static struct si4689_config si4689_config_##id = {                     \
		.int_gpio_flags = DT_INST_GPIO_FLAGS(id, int_gpios),           \
		.reset_gpio_flags = DT_INST_GPIO_FLAGS(id, reset_gpios),       \
		.cs_gpio_flags = DT_INST_SPI_DEV_CS_GPIOS_FLAGS(id),           \
		.spi_bus_label = DT_INST_BUS_LABEL(id),                        \
		.int_gpio_label = DT_INST_GPIO_LABEL(id, int_gpios),           \
		.reset_gpio_label = DT_INST_GPIO_LABEL(id, reset_gpios),       \
		.cs_gpio_label = DT_INST_SPI_DEV_CS_GPIOS_LABEL(id),           \
		.int_gpio_pin = DT_INST_GPIO_PIN(id, int_gpios),               \
		.reset_gpio_pin = DT_INST_GPIO_PIN(id, reset_gpios),           \
		.cs_gpio_pin = DT_INST_SPI_DEV_CS_GPIOS_PIN(id)                \
	};                                                                     \
	static struct apds9301_data apds9301_data_##id;                        \
                                                                               \
	DEVICE_AND_API_INIT(si4689_##id, DT_INST_LABEL(id), init,              \
			    &si4689_data_##id, &si4689_config_##id,            \
			    POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,   \
			    &((struct si4689_api){ .startup = startup,         \
						   .powerdown = powerdown }));

DT_INST_FOREACH_STATUS_OKAY(SI4689_DEVICE)
