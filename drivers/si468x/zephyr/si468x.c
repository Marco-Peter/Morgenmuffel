/*
 * Copyright (c) 2020 Marco Peter
 */

#define DT_DRV_COMPAT silabs_si468x

#include "si468x.h"
#include <drivers/spi.h>
#include <logging/log.h>
#include "si468x_commands.h"

LOG_MODULE_REGISTER(si468x, LOG_LEVEL_DBG);

static int init(const struct device *dev)
{
	struct si468x_data *data = dev->data;
	const struct si468x_config *config = dev->config;
	int rc;

	data->spi = device_get_binding(config->spi_bus_label);

	data->reset_gpio = device_get_binding(config->reset_gpio_label);
	if(data->reset_gpio == NULL) {
		LOG_ERR("%s: failed to bind the reset gpio driver", dev->name);
	}
	rc = gpio_pin_configure(data->reset_gpio, config->reset_gpio_pin,
				GPIO_OUTPUT_ACTIVE | config->reset_gpio_flags);
	if (rc != 0) {
		LOG_ERR("%s: failed to configure reset gpio with rc %d",
			dev->name, rc);
		return rc;
	}
	if(config->cs_gpio_label != NULL) {
		data->cs_gpio = device_get_binding(config->cs_gpio_label);
		if(data->cs_gpio == NULL) {
			LOG_ERR("%s: failed to bind the cs gpio driver", dev->name);
		}
		rc = gpio_pin_configure(data->cs_gpio, config->cs_gpio_pin,
					GPIO_OUTPUT | config->cs_gpio_flags);
		if (rc != 0) {
			LOG_ERR("%s: failed to configure chip select gpio with rc %d",
				dev->name, rc);
			return rc;
		}
	}
	data->int_gpio = device_get_binding(config->int_gpio_label);
	if(data->int_gpio == NULL) {
		LOG_ERR("%s: failed to bind the int gpio driver", dev->name);
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

static int startup(const struct device *dev, enum si468x_mode mode)
{
	struct si468x_data *data = dev->data;
	const struct si468x_config *config = dev->config;
	int rc;

	rc = gpio_pin_set(data->reset_gpio, config->reset_gpio_pin, 1);
	if (rc != 0) {
		LOG_ERR("%s: failed to pull the reset line with rc %d",
			dev->name, rc);
		return rc;
	}
	k_sleep(K_MSEC(1));
	rc = gpio_pin_set(data->reset_gpio, config->reset_gpio_pin, 0);
	if (rc != 0) {
		LOG_ERR("%s: failed to release the reset line with rc %d",
			dev->name, rc);
		return rc;
	}
	k_sleep(K_MSEC(4));
	data->clear_to_send = true;

	rc = si468x_cmd_powerup(dev);
	if(rc != 0){
		LOG_ERR("%s: failed to send powerup command with rc %d", dev->name, rc);
		return rc;
	}

	return 0;
}

static int powerdown(const struct device *dev)
{
	return 0;
}

#define SI468X_DEVICE(id)                                                      \
	static struct si468x_config si468x_config_##id = {                     \
		.int_gpio_flags = DT_INST_GPIO_FLAGS(id, int_gpios),           \
		.reset_gpio_flags = DT_INST_GPIO_FLAGS(id, reset_gpios),       \
		.cs_gpio_flags = DT_INST_SPI_DEV_CS_GPIOS_FLAGS(id),           \
		.spi_slave_number = DT_INST_REG_ADDR(id),                      \
		.spi_bus_label = DT_INST_BUS_LABEL(id),                        \
		.int_gpio_label = DT_INST_GPIO_LABEL(id, int_gpios),           \
		.reset_gpio_label = DT_INST_GPIO_LABEL(id, reset_gpios),       \
		.cs_gpio_label = DT_INST_SPI_DEV_CS_GPIOS_LABEL(id),           \
		.int_gpio_pin = DT_INST_GPIO_PIN(id, int_gpios),               \
		.reset_gpio_pin = DT_INST_GPIO_PIN(id, reset_gpios),           \
		.cs_gpio_pin = DT_INST_SPI_DEV_CS_GPIOS_PIN(id)                \
	};                                                                     \
	static struct si468x_data si468x_data_##id;                            \
                                                                               \
	DEVICE_AND_API_INIT(si468x_##id, DT_INST_LABEL(id), init,              \
			    &si468x_data_##id, &si468x_config_##id,            \
			    POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,   \
			    &((struct si468x_api){ .startup = startup,         \
						   .powerdown = powerdown }));

DT_INST_FOREACH_STATUS_OKAY(SI468X_DEVICE)
