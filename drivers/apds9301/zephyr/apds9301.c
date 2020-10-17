/*
 * Copyright (c) 2020 Marco Peter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT sensirion_apds9301

#include <drivers/i2c.h>
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
	uint8_t i2c_addr;
};

struct apds9301_data {
	const struct device *i2c;
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

static int init(const struct device *dev)
{
	const struct apds9301_config *config = dev->config;
	struct apds9301_data *data = dev->data;
	int rc;

	data->i2c = device_get_binding(config->i2c_bus_label);
	if (data->i2c == NULL) {
		LOG_ERR("%s: device %s not found", dev->name,
			config->i2c_bus_label);
		return -ENODEV;
	}
}

static int sample_fetch(const struct device *dev, enum sensor_channel chan)
{
}

static int channel_get(const struct device *dev, enum sensor_channel chan,
		       struct sensor_value *val)
{
}

#define APDS9301_DEVICE(id)                                                       \
	static struct apds9301_config apds9301_config_##id = {                       \
		.i2c_bus_label = DT_INST_BUS_LABEL(id),                        \
		.i2c_addr = DT_INST_REG_ADDR(id)                               \
	};                                                                     \
	static struct apds9301_data apds9301_data_##id;                              \
                                                                               \
	DEVICE_AND_API_INIT(                                                   \
		apds9301_##id, DT_INST_LABEL(id), init, &apds9301_data_##id,         \
		&apds9301_config_##id, POST_KERNEL,                               \
		CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                            \
		&((struct sensor_driver_api){ .attr_set = NULL,            \
					      .attr_get = NULL,            \
					      .trigger_set = NULL,             \
					      .sample_fetch = sample_fetch,    \
					      .channel_get = channel_get }));

DT_INST_FOREACH_STATUS_OKAY(APDS9301_DEVICE)
