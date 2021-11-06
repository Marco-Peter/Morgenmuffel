/*
 * Copyright (c) 2020 Marco Peter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT meas_ms5637

#include <drivers/i2c.h>
#include <drivers/sensor.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(MS5637, LOG_LEVEL_ERR);

#define MS5637_CMD_RESET 0x1E

#define MS5637_CMD_CONV_P_256 0x40
#define MS5637_CMD_CONV_P_512 0x42
#define MS5637_CMD_CONV_P_1024 0x44
#define MS5637_CMD_CONV_P_2048 0x46
#define MS5637_CMD_CONV_P_4096 0x48
#define MS5637_CMD_CONV_P_8192 0x4A

#define MS5637_CMD_CONV_T_256 0x50
#define MS5637_CMD_CONV_T_512 0x52
#define MS5637_CMD_CONV_T_1024 0x54
#define MS5637_CMD_CONV_T_2048 0x56
#define MS5637_CMD_CONV_T_4096 0x58
#define MS5637_CMD_CONV_T_8192 0x5A

#define MS5637_CMD_CONV_READ_ADC 0x00

#define MS5637_PROM_CMD_OFFSET 0xA0
#define MS5637_PROM_REG_COUNT 7
#define MS5637_CMD_READ_CRC 0xA0
#define MS5637_CMD_READ_C1 0xA2
#define MS5637_CMD_READ_C2 0xA4
#define MS5637_CMD_READ_C3 0xA6
#define MS5637_CMD_READ_C4 0xA8
#define MS5637_CMD_READ_C5 0xAA
#define MS5637_CMD_READ_C6 0xAC

#define MS5637_CONV_TIMER_OSR8192_MS 20
#define MS5637_CONV_TIMER_OSR4096_MS 12
#define MS5637_CONV_TIMER_OSR2048_MS 7
#define MS5637_CONV_TIMER_OSR1024_MS 5
#define MS5637_CONV_TIMER_OSR512_MS 3
#define MS5637_CONV_TIMER_OSR256_MS 2

struct ms5637_config {
	const struct device *i2c_bus;
	uint8_t i2c_addr;
};

struct ms5637_data {
	uint16_t sens_t1;
	uint16_t off_t1;
	uint16_t tcs;
	uint16_t tco;
	uint16_t t_ref;
	uint16_t temp_sens;
	int32_t dt;
	int32_t pressure;
	uint8_t conv_cmd_temp;
	uint8_t conv_cmd_press;
	uint8_t conv_time_temp;
	uint8_t conv_time_press;
};

static inline const struct device *i2c_device(const struct device *dev)
{
	struct ms5637_config *cnf = dev->config;

	return cnf->i2c_bus;
}

static inline const uint16_t i2c_address(const struct device *dev)
{
	const struct ms5637_config *config = dev->config;

	return config->i2c_addr;
}

static inline int send_command(const struct device *dev, uint8_t cmd)
{
	LOG_DBG("Send command 0x%X to address 0x%X", cmd, i2c_address(dev));
	return i2c_write(i2c_device(dev), &cmd, sizeof(cmd), i2c_address(dev));
}

static inline int read_data(const struct device *dev, uint8_t *data,
			    uint32_t len)
{
	LOG_DBG("Read %d bytes from address 0x%X", len, i2c_address(dev));
	return i2c_read(i2c_device(dev), data, len, i2c_address(dev));
}

static uint8_t calc_crc4(uint8_t *data)
{
	uint32_t rem = 0;

	data[0] &= 0x0F;
	data[14] = 0;
	data[15] = 0;
	for (int i = 0; i < (2 * (MS5637_PROM_REG_COUNT + 1)); i++) {
		rem ^= data[i];
		for (int j = 8; j > 0; j--) {
			if (rem & 0x8000) {
				rem = (rem << 1) ^ 0x3000;
			} else {
				rem = (rem << 1);
			}
		}
	}
	rem = (rem >> 12) & 0x000F;
	return rem ^ 0x00;
}

static int init(const struct device *dev)
{
	const struct ms5637_config *config = dev->config;
	struct ms5637_data *data = dev->data;
	int rc;
	uint8_t prom[2 * (MS5637_PROM_REG_COUNT + 1)] = {
		0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE,
		0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE
	};
	uint8_t crc_exp;
	uint8_t crc_calc;

	data->conv_cmd_temp = MS5637_CMD_CONV_T_8192;
	data->conv_cmd_press = MS5637_CMD_CONV_P_8192;
	data->conv_time_temp = MS5637_CONV_TIMER_OSR8192_MS;
	data->conv_time_press = MS5637_CONV_TIMER_OSR8192_MS;

	rc = send_command(dev, MS5637_CMD_RESET);
	if (rc != 0) {
		rc = send_command(dev, MS5637_CMD_RESET);
		if (rc != 0) {
			LOG_ERR("%s: reset failed with error %d", dev->name,
				rc);
		}
	}
	for (uint8_t i = 0; i < MS5637_PROM_REG_COUNT; i++) {
		rc = send_command(dev, 2 * i + MS5637_PROM_CMD_OFFSET);
		if (rc != 0) {
			LOG_ERR("%s: prom read command failed with error %d",
				dev->name, rc);
			return rc;
		}
		rc = read_data(dev, &prom[2 * i], 2U);
		if (rc != 0) {
			LOG_ERR("%s: reading prom failed with error %d",
				dev->name, rc);
			return rc;
		}
	}
	crc_exp = prom[0] >> 4;
	crc_calc = calc_crc4(prom);
	if (crc_calc != crc_exp) {
		LOG_ERR("%s: crc check failed. Expected: 0x%X, calculated: 0x%X",
			dev->name, crc_exp, crc_calc);
		return -EIO;
	}

	data->sens_t1 = prom[2];
	data->sens_t1 <<= 8;
	data->sens_t1 |= prom[3];
	data->off_t1 = prom[4];
	data->off_t1 <<= 8;
	data->off_t1 |= prom[5];
	data->tcs = prom[6];
	data->tcs <<= 8;
	data->tcs |= prom[7];
	data->tco = prom[8];
	data->tco <<= 8;
	data->tco |= prom[9];
	data->t_ref = prom[10];
	data->t_ref <<= 8;
	data->t_ref |= prom[11];
	data->temp_sens = prom[12];
	data->temp_sens <<= 8;
	data->temp_sens |= prom[13];
	LOG_INF("%s: sens_t1 = %u", dev->name, data->sens_t1);
	LOG_INF("%s: off_t1 = %u", dev->name, data->off_t1);
	LOG_INF("%s: tcs = %u", dev->name, data->tcs);
	LOG_INF("%s: tco = %u", dev->name, data->tco);
	LOG_INF("%s: t_ref = %u", dev->name, data->t_ref);
	LOG_INF("%s: temp_sens = %u", dev->name, data->temp_sens);
	return rc;
}

static int convert(const struct device *dev, uint8_t cmd, uint8_t wait,
		   uint32_t *value)
{
	int rc;
	uint8_t rx_data[3];

	rc = send_command(dev, cmd);
	if (rc != 0) {
		LOG_ERR("%s: starting conversion failed with error %d",
			dev->name, rc);
		return rc;
	}
	k_sleep(K_MSEC(wait));
	rc = send_command(dev, MS5637_CMD_CONV_READ_ADC);
	if (rc != 0) {
		LOG_ERR("%s: requesting adc data failed with error %d",
			dev->name, rc);
		return rc;
	}
	rc = read_data(dev, rx_data, sizeof(rx_data));
	if (rc != 0) {
		LOG_ERR("%s: reading measurement data failed with error %d",
			dev->name, rc);
		return rc;
	}
	*value = (rx_data[0] << 16) | (rx_data[1] << 8) | rx_data[2];
	return 0;
}

static int32_t calc_pressure(const struct device *dev, uint32_t value)
{
	struct ms5637_data *data = dev->data;

	int64_t offs;
	int64_t sens;
	int32_t press;

	offs = ((int64_t)data->off_t1 << 17) +
	       (((int64_t)data->tco * (int64_t)data->dt) >> 6);
	sens = ((int64_t)data->sens_t1 << 16) +
	       (((int64_t)data->tcs * (int64_t)data->dt) >> 7);
	press = ((((int64_t)value * sens) >> 21) - offs) >> 15;
	return press;
}

static int sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	struct ms5637_data *data = dev->data;
	int rc;
	uint32_t value;

	if (chan == SENSOR_CHAN_ALL || chan == SENSOR_CHAN_PRESS) {
		rc = convert(dev, data->conv_cmd_temp, data->conv_time_temp,
			     &value);
		if (rc == 0) {
			data->dt = (int32_t)value - (data->t_ref << 8);
			LOG_DBG("%s: value = %d, dt = %d", dev->name, value,
				data->dt);
			rc = convert(dev, data->conv_cmd_press,
				     data->conv_time_press, &value);
			if (rc == 0) {
				data->pressure = calc_pressure(dev, value);
				LOG_DBG("%s: value = %d, press = %d", dev->name,
					value, data->pressure);
			}
		}
	} else if (chan == SENSOR_CHAN_AMBIENT_TEMP) {
		rc = convert(dev, data->conv_cmd_temp, data->conv_time_temp,
			     &value);
		if (rc == 0) {
			data->dt = (int32_t)value - (data->t_ref << 8);
		}
	} else {
		rc = -ENOTSUP;
	}
	return rc;
}

static int channel_get(const struct device *dev, enum sensor_channel chan,
		       struct sensor_value *val)
{
	struct ms5637_data *data = dev->data;
	int rc = 0;

	if (chan == SENSOR_CHAN_AMBIENT_TEMP) {
		uint16_t temp_sens;
		int32_t dt;
		uint8_t div;

		if (data->dt > UINT16_MAX) {
			dt = data->dt >> 1;
			temp_sens = data->temp_sens >> 1;
			div = 21;
		} else {
			dt = data->dt;
			temp_sens = data->temp_sens;
			div = 23;
		}
		int32_t temp = 2000 + ((dt * temp_sens) >> div);
		LOG_DBG("%s: temp = %d", dev->name, temp);
		val->val1 = temp / 100;
		val->val2 = (temp % 100) * 10000;
	} else if (chan == SENSOR_CHAN_PRESS) {
		val->val1 = data->pressure / 1000;
		val->val2 = (data->pressure % 1000) * 1000;
	} else {
		rc = -ENOTSUP;
	}
	return rc;
}

#define MS5637_DEVICE(inst)                                                    \
	static const struct ms5637_config ms5637_config_##inst = {             \
		.i2c_bus = DEVICE_DT_GET(DT_INST_BUS(inst)),                   \
		.i2c_addr = DT_INST_REG_ADDR(inst)                             \
	};                                                                     \
	static struct ms5637_data ms5637_data_##inst;                          \
                                                                               \
	DEVICE_DT_INST_DEFINE(                                                 \
		inst, init, NULL, &ms5637_data_##inst, &ms5637_config_##inst,  \
		POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,                      \
		&((struct sensor_driver_api){ .attr_set = NULL,                \
					      .attr_get = NULL,                \
					      .trigger_set = NULL,             \
					      .sample_fetch = sample_fetch,    \
					      .channel_get = channel_get }));

DT_INST_FOREACH_STATUS_OKAY(MS5637_DEVICE)
