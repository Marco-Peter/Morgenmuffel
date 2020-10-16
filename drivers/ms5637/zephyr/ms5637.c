/*
 * Copyright (c) 2020 Marco Peter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT meas_ms5637

#include "ms5637.h"
#include <syscall_handler.h>
#include <drivers/i2c.h>
#include <logging/log.h>
#include <sys/crc.h>

LOG_MODULE_REGISTER(MS5637, LOG_LEVEL_INF);

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
#define MS5637_CMD_READ_C1 0xA1
#define MS5637_CMD_READ_C2 0xA2
#define MS5637_CMD_READ_C3 0xA3
#define MS5637_CMD_READ_C4 0xA4
#define MS5637_CMD_READ_C5 0xA5
#define MS5637_CMD_READ_C6 0xA6

struct ms5637_config {
	char *i2c_bus_label;
	uint8_t i2c_addr;
};

struct ms5637_data {
	const struct device *i2c;
	uint16_t sens_t1;
	uint16_t off_t1;
	uint16_t tcs;
	uint16_t tco;
	uint16_t t_ref;
	uint16_t temp_sens;
};

static inline const struct device *i2c_device(const struct device *dev)
{
	struct ms5637_data *data = dev->data;

	return data->i2c;
}

static inline const uint16_t i2c_address(const struct device *dev)
{
	const struct ms5637_config *config = dev->config;

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

static uint8_t crc4(uint8_t *data)
{
	int cnt;
	uint32_t rem = 0;
	uint8_t bit;

	data[0] &= 0x0FFF;
	data[7] = 0U;
	for (int i = 0; i < 8; i +) {
		rem ^= data[2 * i + 1];
		rem ^= data[2 * i];
		for (int j = 8; j > 0; j--) {
			if (rem & 0x8000) {
				rem = (rem << 1) ^ 0x3000;
			} else {
				rem <<= 1;
			}
		}
	}
	rem = ((rem >> 12) & 0x000F);
	return rem;
}

static int init(const struct device *dev)
{
	const struct ms5637_config *config = dev->config;
	struct ms5637_data *data = dev->data;
	int rc;
	uint8_t prom[2 * MS5637_PROM_REG_COUNT];
	uint8_t crc;

	data->i2c = device_get_binding(config->i2c_bus_label);
	if (data->i2c == NULL) {
		LOG_ERR("%s: device %s not found", dev->name,
			config->i2c_bus_label);
		return -ENODEV;
	}
	rc = send_command(dev, MS5637_CMD_RESET);
	if (rc != 0) {
		LOG_ERR("%s: reset failed with error %d", dev->name, rc);
	}
	for (uint8_t i = 0; i < MS5637_PROM_REG_COUNT; i++) {
		rc = send_command(dev, i + MS5637_PROM_CMD_OFFSET);
		if (rc != 0) {
			LOG_ERR("%s: prom read command failed with error %d",
				dev->name);
			return rc;
		}
		rc = read_data(dev, &prom[2 * i], 2U);
		if (rc != 0) {
			LOG_ERR("%s: reading prom failed with error %d",
				dev->name);
			return rc;
		}
	}
	crc = crc4(prom);
	if (crc != (prom[0] >> 4)) {
		LOG_ERR("%s: crc check failed. Expected: 0x%X, received: 0x%X",
			dev->name, prom[0] >> 4, crc);
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
	return rc;
}

static int32_t meas_temp_impl(const struct device *dev)
{
}

#ifdef CONFIG_USERSPACE
static inline void z_vrfy_sht2x_meas_temp(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_HELLO_WORLD(dev, print));

	z_impl_hello_world_print(dev);
}
#include <syscalls/sht2x_meas_temp_mrsh.c>
#endif /* CONFIG_USERSPACE */

static int32_t meas_rh_impl(const struct device *dev)
{
}

#ifdef CONFIG_USERSPACE
static inline void z_vrfy_sht2x_meas_rh(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_HELLO_WORLD(dev, print));

	z_impl_hello_world_print(dev);
}
#include <syscalls/sht2x_meas_rh_mrsh.c>
#endif /* CONFIG_USERSPACE */

#define MS5637_DEVICE(id)                                                      \
	static struct ms5637_config ms5637_config_##id = {                     \
		.i2c_bus_label = DT_INST_BUS_LABEL(id),                        \
		.i2c_addr = DT_INST_REG_ADDR(id)                               \
	};                                                                     \
	static struct ms5637_data ms5637_data_##id;                            \
                                                                               \
	DEVICE_AND_API_INIT(                                                   \
		ms5637_##id, DT_INST_LABEL(id), init, &ms5637_data_##id,       \
		&ms5637_config_##id, POST_KERNEL,                              \
		CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                            \
		&((struct ms5637_driver_api){ .meas_temp = meas_temp_impl,     \
					      .meas_rh = meas_rh_impl }));

DT_INST_FOREACH_STATUS_OKAY(MS5637_DEVICE)

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(spi)
int ms5607_spi_init(const struct device *dev);
#else
/* I2c Interface not implemented yet */
BUILD_ASSERT(1, "I2c interface not implemented yet");
#endif

struct ms5607_config {
	char *ms5607_device_name;
};

struct ms5607_data {
	const struct device *ms5607_device;
	const struct ms5607_transfer_function *tf;
	/* Calibration values */
	uint16_t sens_t1;
	uint16_t off_t1;
	uint16_t tcs;
	uint16_t tco;
	uint16_t t_ref;
	uint16_t tempsens;

	/* Measured values */
	int32_t pressure;
	int32_t temperature;

	/* conversion commands */
	uint8_t pressure_conv_cmd;
	uint8_t temperature_conv_cmd;

	uint8_t pressure_conv_delay;
	uint8_t temperature_conv_delay;
};

struct ms5607_transfer_function {
	int (*reset)(const struct ms5607_data *data);
	int (*read_prom)(const struct ms5607_data *data, uint8_t cmd,
			 uint16_t *val);
	int (*start_conversion)(const struct ms5607_data *data, uint8_t cmd);
	int (*read_adc)(const struct ms5607_data *data, uint32_t *val);
};

static void ms5607_compensate(struct ms5607_data *data,
			      const int32_t adc_temperature,
			      const int32_t adc_pressure)
{
	int64_t dT;
	int64_t OFF;
	int64_t SENS;
	int64_t temp_sq;
	int64_t Ti;
	int64_t OFFi;
	int64_t SENSi;

	/* first order compensation as per datasheet
	 * (https://www.te.com/usa-en/product-CAT-BLPS0035.html) section
	 * PRESSURE AND TEMPERATURE CALCULATION
	 */

	dT = adc_temperature - ((uint32_t)(data->t_ref) << 8);
	data->temperature = 2000 + (dT * data->tempsens) / (1ll << 23);
	OFF = ((int64_t)(data->off_t1) << 17) + (dT * data->tco) / (1ll << 6);
	SENS = ((int64_t)(data->sens_t1) << 16) + (dT * data->tcs) / (1ll << 7);

	/* Second order compensation as per datasheet
	 * (https://www.te.com/usa-en/product-CAT-BLPS0035.html) section
	 * SECOND ORDER TEMPERATURE COMPENSATION
	 */

	temp_sq = (int64_t)(data->temperature - 2000) *
		  (int64_t)(data->temperature - 2000);
	if (data->temperature < 2000) {
		Ti = (dT * dT) / (1ll << 31);
		OFFi = (61ll * temp_sq) / (1ll << 4);
		SENSi = 2ll * temp_sq;
		if (data->temperature < -1500) {
			temp_sq = (int64_t)(data->temperature + 1500) *
				  (int64_t)(data->temperature + 1500);
			OFFi += 15ll * temp_sq;
			SENSi += 8ll * temp_sq;
		}
	} else {
		SENSi = 0;
		OFFi = 0;
		Ti = 0;
	}

	OFF -= OFFi;
	SENS -= SENSi;

	data->temperature -= Ti;
	data->pressure = (SENS * (int64_t)adc_pressure / (1ll << 21) - OFF) /
			 (1ll << 15);
}

static int ms5607_read_prom(const struct ms5607_data *data, uint8_t cmd,
			    uint16_t *val)
{
	int err;

	err = data->tf->read_prom(data, cmd, val);
	if (err < 0) {
		LOG_ERR("Error reading prom");
		return err;
	}

	return 0;
}

static int ms5607_get_measurement(const struct ms5607_data *data, uint32_t *val,
				  uint8_t cmd, uint8_t delay)
{
	int err;

	*val = 0U;

	err = data->tf->start_conversion(data, cmd);
	if (err < 0) {
		return err;
	}

	k_msleep(delay);

	err = data->tf->read_adc(data, val);
	if (err < 0) {
		return err;
	}

	return 0;
}

static int ms5607_sample_fetch(const struct device *dev,
			       enum sensor_channel channel)
{
	struct ms5607_data *data = dev->data;
	int err;
	uint32_t adc_pressure, adc_temperature;

	__ASSERT_NO_MSG(channel == SENSOR_CHAN_ALL);

	err = ms5607_get_measurement(data, &adc_pressure,
				     data->pressure_conv_cmd,
				     data->pressure_conv_delay);
	if (err < 0) {
		return err;
	}

	err = ms5607_get_measurement(data, &adc_temperature,
				     data->temperature_conv_cmd,
				     data->temperature_conv_delay);
	if (err < 0) {
		return err;
	}

	ms5607_compensate(data, adc_temperature, adc_pressure);
	return 0;
}

static int ms5607_channel_get(const struct device *dev,
			      enum sensor_channel chan,
			      struct sensor_value *val)
{
	const struct ms5607_data *data = dev->data;

	switch (chan) {
	case SENSOR_CHAN_AMBIENT_TEMP:
		val->val1 = data->temperature / 100;
		val->val2 = data->temperature % 100 * 10000;
		break;
	case SENSOR_CHAN_PRESS:
		val->val1 = data->pressure / 100;
		val->val2 = data->pressure % 100 * 10000;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int ms5607_attr_set(const struct device *dev, enum sensor_channel chan,
			   enum sensor_attribute attr,
			   const struct sensor_value *val)
{
	struct ms5607_data *data = dev->data;
	uint8_t p_conv_cmd, t_conv_cmd, conv_delay;

	if (attr != SENSOR_ATTR_OVERSAMPLING) {
		return -ENOTSUP;
	}

	switch (val->val1) {
	case 4096:
		p_conv_cmd = MS5637_CMD_CONV_P_4096;
		t_conv_cmd = MS5637_CMD_CONV_T_4096;
		conv_delay = 9U;
		break;
	case 2048:
		p_conv_cmd = MS5637_CMD_CONV_P_2048;
		t_conv_cmd = MS5637_CMD_CONV_T_2048;
		conv_delay = 5U;
		break;
	case 1024:
		p_conv_cmd = MS5637_CMD_CONV_P_1024;
		t_conv_cmd = MS5637_CMD_CONV_T_1024;
		conv_delay = 3U;
		break;
	case 512:
		p_conv_cmd = MS5637_CMD_CONV_P_512;
		t_conv_cmd = MS5637_CMD_CONV_T_512;
		conv_delay = 2U;
		break;
	case 256:
		p_conv_cmd = MS5637_CMD_CONV_P_256;
		t_conv_cmd = MS5637_CMD_CONV_T_256;
		conv_delay = 1U;
		break;
	default:
		LOG_ERR("invalid oversampling rate %d", val->val1);
		return -EINVAL;
	}

	switch (chan) {
	case SENSOR_CHAN_ALL:
		data->pressure_conv_cmd = p_conv_cmd;
		data->temperature_conv_cmd = t_conv_cmd;
		data->temperature_conv_delay = conv_delay;
		data->pressure_conv_delay = conv_delay;
		break;
	case SENSOR_CHAN_PRESS:
		data->pressure_conv_cmd = p_conv_cmd;
		data->pressure_conv_delay = conv_delay;
		break;
	case SENSOR_CHAN_AMBIENT_TEMP:
		data->temperature_conv_cmd = t_conv_cmd;
		data->temperature_conv_delay = conv_delay;
		break;
	default:
		return -ENOTSUP;
	}

	return 0;
}

static const struct ms5607_config ms5607_config = {
	.ms5607_device_name = DT_INST_BUS_LABEL(0),
};

static int ms5607_init(const struct device *dev)
{
	const struct ms5607_config *const config = dev->config;
	struct ms5607_data *data = dev->data;
	struct sensor_value val;
	int err;

	data->ms5607_device = device_get_binding(config->ms5607_device_name);
	if (!data->ms5607_device) {
		LOG_ERR("master not found: %s", config->ms5607_device_name);
		return -EINVAL;
	}

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(spi)
	ms5607_spi_init(dev);
#else
	BUILD_ASSERT(1, "I2c interface not implemented yet");
#endif

	data->pressure = 0;
	data->temperature = 0;

	val.val1 = MS5607_PRES_OVER_DEFAULT;
	err = ms5607_attr_set(dev, SENSOR_CHAN_PRESS, SENSOR_ATTR_OVERSAMPLING,
			      &val);
	if (err < 0) {
		return err;
	}

	val.val1 = MS5607_TEMP_OVER_DEFAULT;
	err = ms5607_attr_set(dev, SENSOR_CHAN_AMBIENT_TEMP,
			      SENSOR_ATTR_OVERSAMPLING, &val);
	if (err < 0) {
		return err;
	}

	err = data->tf->reset(data);
	if (err < 0) {
		return err;
	}

	k_sleep(K_MSEC(2));

	err = ms5607_read_prom(data, MS5637_CMD_CONV_READ_OFF_T1,
			       &data->off_t1);
	if (err < 0) {
		return err;
	}

	LOG_DBG("OFF_T1: %d", data->off_t1);

	err = ms5607_read_prom(data, MS5637_CMD_CONV_READ_SENSE_T1,
			       &data->sens_t1);
	if (err < 0) {
		return err;
	}

	LOG_DBG("SENSE_T1: %d", data->sens_t1);

	err = ms5607_read_prom(data, MS5637_CMD_CONV_READ_T_REF, &data->t_ref);
	if (err < 0) {
		return err;
	}

	LOG_DBG("T_REF: %d", data->t_ref);

	err = ms5607_read_prom(data, MS5637_CMD_CONV_READ_TCO, &data->tco);
	if (err < 0) {
		return err;
	}

	LOG_DBG("TCO: %d", data->tco);

	err = ms5607_read_prom(data, MS5637_CMD_CONV_READ_TCS, &data->tcs);
	if (err < 0) {
		return err;
	}

	LOG_DBG("TCS: %d", data->tcs);

	err = ms5607_read_prom(data, MS5637_CMD_CONV_READ_TEMPSENS,
			       &data->tempsens);
	if (err < 0) {
		return err;
	}

	LOG_DBG("TEMPSENS: %d", data->tempsens);

	return 0;
}

static const struct sensor_driver_api ms5607_api_funcs = {
	.attr_set = ms5607_attr_set,
	.sample_fetch = ms5607_sample_fetch,
	.channel_get = ms5607_channel_get,
};

static struct ms5607_data ms5607_data;

DEVICE_AND_API_INIT(ms5607, DT_INST_LABEL(0), ms5607_init, &ms5607_data,
		    &ms5607_config, POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,
		    &ms5607_api_funcs);
