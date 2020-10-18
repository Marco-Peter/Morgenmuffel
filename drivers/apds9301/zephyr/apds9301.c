/*
 * Copyright (c) 2020 Marco Peter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT avago_apds9301

#include <drivers/i2c.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(apds9301, LOG_LEVEL_INF);

#define APDS9301_REGISTER_ADDR_CONTROL 0x0
#define APDS9301_REGISTER_ADDR_TIMING 0x1
#define APDS9301_REGISTER_ADDR_THRESHLOW_LSB 0x2
#define APDS9301_REGISTER_ADDR_THRESHLOW_MSB 0x3
#define APDS9301_REGISTER_ADDR_THRESHHIGH_LSB 0x4
#define APDS9301_REGISTER_ADDR_THRESHHIGH_MSB 0x5
#define APDS9301_REGISTER_ADDR_INTERRUPT 0x6
#define APDS9301_REGISTER_ADDR_CRC 0x8
#define APDS9301_REGISTER_ADDR_ID 0xA
#define APDS9301_REGISTER_ADDR_DATA0_LSB 0xC
#define APDS9301_REGISTER_ADDR_DATA0_MSB 0xD
#define APDS9301_REGISTER_ADDR_DATA1_LSB 0xE
#define APDS9301_REGISTER_ADDR_DATA1_MSB 0xF

struct apds9301_config {
	char *i2c_bus_label;
	char *int_gpio_label;
	gpio_flags_t int_gpio_flags;
	uint8_t i2c_addr;
	uint8_t int_gpio_pin;
};

struct apds9301_data {
	const struct device *i2c;
	const struct device *int_gpio;
	struct gpio_callback int_gpio_cb;
	int32_t rh;
	int32_t temp;
};

static inline const struct device *i2c_device(const struct device *dev)
{
	struct apds9301_data *data = dev->data;

	return data->i2c;
}

static inline const uint16_t i2c_address(const struct device *dev)
{
	const struct apds9301_config *config = dev->config;

	return config->i2c_addr;
}

static inline int send_command(const struct device *dev, uint8_t cmd)
{
	return i2c_write(i2c_device(dev), &cmd, sizeof(cmd), i2c_address(dev));
}

static inline int read_data(const struct device *dev, uint8_t *data,
			    uint32_t len)
{
	return i2c_read(i2c_device(dev), data, len, i2c_address(dev));
}

static void gpio_callback(const struct device *dev, struct gpio_callback *cb,
			  uint32_t pins)
{
	struct apds9301_data *data = dev->data;
}

static int init(const struct device *dev)
{
	const struct apds9301_config *config = dev->config;
	struct apds9301_data *data = dev->data;
	int rc = 0;

	data->i2c = device_get_binding(config->i2c_bus_label);
	if (data->i2c == NULL) {
		LOG_ERR("%s: device %s not found", dev->name,
			config->i2c_bus_label);
		return -ENODEV;
	}
	if (config->int_gpio_label) {
		data->int_gpio = device_get_binding(config->int_gpio_label);
		if (data->int_gpio == NULL) {
			LOG_ERR("%s: device %s not found", dev->name,
				config->int_gpio_label);
		}
		rc = gpio_pin_configure(data->int_gpio, config->int_gpio_pin,
					GPIO_INPUT | config->int_gpio_flags);
		if (rc != 0) {
			LOG_ERR("%s: configuration of %s failed with rc %d",
				dev->name, config->int_gpio_label, rc);
		}
		gpio_init_callback(&data->int_gpio_cb, gpio_callback,
				   config->int_gpio_pin);
		rc = gpio_add_callback(data->int_gpio, &data->int_gpio_cb);
		if (rc != 0) {
			LOG_ERR("%s: adding callback failed with rc %d",
				dev->name, rc);
		}
	}
	return rc;
}

static int sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	return 0;
}

static int channel_get(const struct device *dev, enum sensor_channel chan,
		       struct sensor_value *val)
{
	return 0;
}

#define APDS9301_DEVICE(id)                                                    \
	static struct apds9301_config apds9301_config_##id = {                 \
		.i2c_bus_label = DT_INST_BUS_LABEL(id),                        \
		.int_gpio_label = DT_INST_GPIO_LABEL(id, int_gpios),           \
		.int_gpio_flags = DT_INST_GPIO_FLAGS(id, int_gpios),           \
		.i2c_addr = DT_INST_REG_ADDR(id),                              \
		.int_gpio_pin = DT_INST_GPIO_PIN(id, int_gpios),               \
	};                                                                     \
	static struct apds9301_data apds9301_data_##id;                        \
                                                                               \
	DEVICE_AND_API_INIT(                                                   \
		apds9301_##id, DT_INST_LABEL(id), init, &apds9301_data_##id,   \
		&apds9301_config_##id, POST_KERNEL,                            \
		CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                            \
		&((struct sensor_driver_api){ .attr_set = NULL,                \
					      .attr_get = NULL,                \
					      .trigger_set = NULL,             \
					      .sample_fetch = sample_fetch,    \
					      .channel_get = channel_get }));

DT_INST_FOREACH_STATUS_OKAY(APDS9301_DEVICE)
