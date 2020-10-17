/*
 * Copyright (c) 2020 Marco Peter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT sensirion_sht2x

#include <drivers/i2c.h>
#include <drivers/sensor.h>
#include <logging/log.h>
#include <sys/crc.h>

LOG_MODULE_REGISTER(SHT2X, LOG_LEVEL_INF);

#define SHT2X_CMD_TRIGGER_MEAS_TEMP 0xF3
#define SHT2X_CMD_TRIGGER_MEAS_RH 0xF5
#define SHT2X_CMD_WRITE_USER_REG 0xE6
#define SHT2X_CMD_READ_USER_REG 0xE7
#define SHT2X_CMD_SOFT_RESET 0xFE
#define SHT2X_WAIT_TIME_TEMP_MS 86
#define SHT2X_WAIT_TIME_RH_MS 30

struct sht2x_config {
	char *i2c_bus_label;
	uint8_t i2c_addr;
};

struct sht2x_data {
	const struct device *i2c;
	int32_t rh;
	int32_t temp;
};

static inline const struct device *i2c_device(const struct device *dev)
{
	struct sht2x_data *data = dev->data;

	return data->i2c;
}

static inline const uint16_t i2c_address(const struct device *dev)
{
	const struct sht2x_config *config = dev->config;

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
	const struct sht2x_config *config = dev->config;
	struct sht2x_data *data = dev->data;
	int rc;

	data->i2c = device_get_binding(config->i2c_bus_label);
	if (data->i2c == NULL) {
		LOG_ERR("%s: device %s not found", dev->name,
			config->i2c_bus_label);
		return -ENODEV;
	}
	rc = send_command(dev, SHT2X_CMD_SOFT_RESET);
	if (rc != 0) {
		LOG_ERR("%s: reset failed with error %d", dev->name, rc);
	}
	return rc;
}

static int meas_temp(const struct device *dev)
{
	struct sht2x_data *data = dev->data;
	int rc;
	uint16_t value;
	uint8_t rx_data[3];
	uint8_t crc;

	rc = send_command(dev, SHT2X_CMD_TRIGGER_MEAS_TEMP);
	if (rc != 0) {
		LOG_ERR("%s: starting temp measurement failed with error %d",
			dev->name, rc);
		return rc;
	}
	k_sleep(K_MSEC(SHT2X_WAIT_TIME_TEMP_MS));
	rc = read_data(dev, rx_data, sizeof(rx_data));
	if (rc != 0) {
		LOG_ERR("%s: reading measurement data failed with error % d",
			dev->name, rc);
		return rc;
	}
	crc = crc8(rx_data, 2, 49, 0U, false);
	if (crc != rx_data[2]) {
		LOG_ERR("%s: CRC test failed. Expected: 0x%X, received: 0x%X",
			dev->name, crc, rx_data[2]);
		return -EIO;
	}
	if ((rx_data[1] & 0x02) == 0x02) {
		LOG_ERR("%s: received humidity instead of temperature",
			dev->name);
		return -EIO;
	}
	value = rx_data[0];
	value <<= 8;
	value |= rx_data[1];
	value &= 0xFFFC;
	LOG_DBG("%s: raw temperature value: 0x%X", dev->name, value);
	data->temp = 17572 * (uint32_t)value / (UINT16_MAX + 1) - 4685;
	LOG_DBG("%s: calculated temperature: %d mDeg.", dev->name, data->temp);
	return rc;
}

static int meas_rh(const struct device *dev)
{
	struct sht2x_data *data = dev->data;
	int rc;
	uint16_t value;
	uint8_t rx_data[3];
	uint8_t crc;

	rc = send_command(dev, SHT2X_CMD_TRIGGER_MEAS_RH);
	if (rc != 0) {
		LOG_ERR("%s: starting RH measurement failed with error %d",
			dev->name, rc);
		return rc;
	}
	k_sleep(K_MSEC(SHT2X_WAIT_TIME_RH_MS));
	rc = read_data(dev, rx_data, sizeof(rx_data));
	if (rc != 0) {
		LOG_ERR("%s: reading measurement data failed with error %d",
			dev->name, rc);
		return rc;
	}
	crc = crc8(rx_data, 2, 49, 0U, false);
	if (crc != rx_data[2]) {
		LOG_ERR("%s: CRC test failed. Expected: 0x%X, received: 0x%X",
			dev->name, crc, rx_data[2]);
		return -EIO;
	}
	if ((rx_data[1] & 0x02) == 0) {
		LOG_ERR("%s: received temperature instead of humidity",
			dev->name);
		return -EIO;
	}
	value = rx_data[0];
	value <<= 8;
	value |= rx_data[1];
	value &= 0xFFFC;
	LOG_DBG("%s: raw rh value: 0x%X", dev->name, value);
	data->rh = 12500 * (uint32_t)value / (UINT16_MAX + 1) - 600;
	return rc;
}

static int sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	int rc;

	if (chan == SENSOR_CHAN_ALL) {
		rc = meas_temp(dev);
		if (rc == 0) {
			rc = meas_rh(dev);
		}
	} else if (chan == SENSOR_CHAN_AMBIENT_TEMP) {
		rc = meas_temp(dev);
	} else if (chan == SENSOR_CHAN_HUMIDITY) {
		rc = meas_rh(dev);
	} else {
		LOG_ERR("%s: measurement channel %d is not supported.",
			dev->name, chan);
		rc = -ENOTSUP;
	}
	return rc;
}

static int channel_get(const struct device *dev, enum sensor_channel chan,
		       struct sensor_value *val)
{
	struct sht2x_data *data = dev->data;
	int rc = 0;

	if (chan == SENSOR_CHAN_AMBIENT_TEMP) {
		val->val1 = data->temp / 100;
		val->val2 = (data->temp % 100) * 10000;
	} else if (chan == SENSOR_CHAN_HUMIDITY) {
		val->val1 = data->rh / 100;
		val->val2 = (data->rh % 100) * 10000;
	} else {
		LOG_ERR("%s: measurement channel %d is not supported.",
			dev->name, chan);
		rc = -ENOTSUP;
	}
	return rc;
}

#define SHT32_DEVICE(id)                                                       \
	static struct sht2x_config sht2x_config_##id = {                       \
		.i2c_bus_label = DT_INST_BUS_LABEL(id),                        \
		.i2c_addr = DT_INST_REG_ADDR(id)                               \
	};                                                                     \
	static struct sht2x_data sht2x_data_##id;                              \
                                                                               \
	DEVICE_AND_API_INIT(                                                   \
		sht2x_##id, DT_INST_LABEL(id), init, &sht2x_data_##id,         \
		&sht2x_config_##id, POST_KERNEL,                               \
		CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                            \
		&((struct sensor_driver_api){ .attr_set = NULL,                \
					      .attr_get = NULL,                \
					      .trigger_set = NULL,             \
					      .sample_fetch = sample_fetch,    \
					      .channel_get = channel_get }));

DT_INST_FOREACH_STATUS_OKAY(SHT32_DEVICE)
