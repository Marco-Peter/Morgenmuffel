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
#include <math.h>

LOG_MODULE_REGISTER(apds9301, LOG_LEVEL_DBG);

#define REGISTER_CONTROL 0x0U
#define REGISTER_TIMING 0x1U
#define REGISTER_THRESHLOW_LSB 0x2U
#define REGISTER_THRESHLOW_MSB 0x3U
#define REGISTER_THRESHHIGH_LSB 0x4U
#define REGISTER_THRESHHIGH_MSB 0x5U
#define REGISTER_INTERRUPT 0x6U
#define REGISTER_ID 0xAU
#define REGISTER_DATA_VIS_LSB 0xCU
#define REGISTER_DATA_VIS_MSB 0xDU
#define REGISTER_DATA_IR_LSB 0xEU
#define REGISTER_DATA_IR_MSB 0xFU

#define MASK_COMMAND_CMD 0x80U
#define OFFS_COMMAND_CMD 7
#define MASK_COMMAND_CLEAR 0x40U
#define OFFS_COMMAND_CLEAR 6
#define MASK_COMMAND_WORD 0x20U
#define OFFS_COMMAND_WORD 5
#define MASK_COMMAND_ADDRESS 0x0FU
#define OFFS_COMMAND_ADDRESS 0

#define MASK_CONTROL_POWER 0x03U
#define OFFS_CONTROL_POWER 0
#define CONTROL_POWER_ON 0x03U
#define CONTROL_POWER_OFF 0x00U

#define MASK_TIMING_GAIN 0x10U
#define OFFS_TIMING_GAIN 4
#define MASK_TIMING_MANUAL 0x08U
#define OFFS_TIMING_MANUAL 3
#define MASK_TIMING_INTEG 0x03U
#define OFFS_TIMING_INTEG 0
#define TIMING_INTEG_MS_13_7 0x00U
#define TIMING_INTEG_SCALE_13_7 (1U / 29U)
#define TIMING_INTEG_MS_101 0x01U
#define TIMING_INTEG_SCALE_101 (1U / 4U)
#define TIMING_INTEG_MS_402 0x02U
#define TIMING_INTEG_SCALE_402 1U

#define MASK_INTERRUPT_LEVEL 0x10U
#define OFFS_INTERRUPT_LEVEL 4
#define MASK_INTERRUPT_PERSIST 0x0FU
#define OFFS_INTERRUPT_PERSIST 0
#define INTERRUPT_EVERY_CYCLE 0U
#define INTERRUPT_OUT_OF_RANGE(_x) ((_x)&MASK_INTERRUPT_PERSIST)

#define MASK_ID_PARTNUMBER 0xF0U
#define OFFS_ID_PARTNUMBER 0x4
#define MASK_ID_REVNO 0x0FU
#define OFFS_ID_REVNO 0
#define ID_PARTNUMBER ((0x5U << OFFS_ID_PARTNUMBER) & MASK_ID_PARTNUMBER)

enum apds9301_gain {
	gain_low,
	gain_high
};

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
	uint32_t vis;
	uint32_t ir;
	enum apds9301_gain gain;
};

static int sample_fetch(const struct device *dev, enum sensor_channel chan);

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

static inline uint8_t init_cmd(uint8_t addr, bool word, bool clear_irq)
{
	uint8_t cmd;

	cmd = MASK_COMMAND_CMD | addr;
	if (clear_irq == true) {
		cmd |= MASK_COMMAND_CLEAR;
	}
	if (word == true) {
		cmd |= MASK_COMMAND_WORD;
	}
	return cmd;
}

static inline uint8_t init_len(bool word)
{
	uint8_t len;

	if (word == true) {
		len = 2;
	} else {
		len = 1;
	}
	return len;
}

static inline int write(const struct device *dev, uint8_t addr, uint16_t data,
			bool word, bool clear_irq)
{
	uint8_t cmd = init_cmd(addr, word, clear_irq);
	uint8_t len = init_len(word);

	return i2c_burst_write(i2c_device(dev), i2c_address(dev), cmd,
			       (uint8_t *)&data, len);
}

static inline int read(const struct device *dev, uint8_t addr, uint16_t *data,
		       bool word, bool clear_irq)
{
	uint8_t cmd = init_cmd(addr, word, clear_irq);
	uint8_t len = init_len(word);

	return i2c_burst_read(i2c_device(dev), i2c_address(dev), cmd,
			      (uint8_t *)data, len);
}

static void gpio_callback(const struct device *dev, struct gpio_callback *cb,
			  uint32_t pins)
{
	//struct apds9301_data *data = dev->data;
}

static int init(const struct device *dev)
{
	const struct apds9301_config *config = dev->config;
	struct apds9301_data *data = dev->data;
	int rc = 0;
	uint16_t reg_data;

	data->i2c = device_get_binding(config->i2c_bus_label);
	if (data->i2c == NULL) {
		LOG_ERR("%s: device %s not found", dev->name,
			config->i2c_bus_label);
		return -ENODEV;
	}
	data->gain = gain_low;
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
	rc = read(dev, REGISTER_ID, &reg_data, false, false);
	if (rc != 0) {
		LOG_ERR("%s: failed to read ID register with rc %d", dev->name,
			rc);
	}
	if ((reg_data & MASK_ID_PARTNUMBER) != ID_PARTNUMBER) {
		LOG_ERR("%s: wrong part number", dev->name);
	}
	rc = write(dev, REGISTER_TIMING, TIMING_INTEG_MS_402, false, false);
	if (rc != 0) {
		LOG_ERR("%s: failed to set the sample timing", dev->name);
	}
	rc = write(dev, REGISTER_CONTROL, CONTROL_POWER_ON, false, false);
	if (rc != 0) {
		LOG_ERR("%s: failed to enable the sensor", dev->name);
	}
	return rc;
}

static int sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	struct apds9301_data *data = dev->data;
	int rc = 0;
	uint16_t ir;
	uint16_t vis = 0;

	if (chan == SENSOR_CHAN_LIGHT || chan == SENSOR_CHAN_ALL) {
		rc = read(dev, REGISTER_DATA_VIS_LSB, &vis, true, false);
		if (rc != 0) {
			LOG_ERR("%s: fetching visible data failed with rc %d",
				dev->name, rc);
			return rc;
		}
		rc = read(dev, REGISTER_DATA_IR_LSB, &ir, true, false);
		if (rc != 0) {
			LOG_ERR("%s: fetching infrared data failed with rc %d",
				dev->name, rc);
			return rc;
		}
	} else {
		return -ENOTSUP;
	}
	data->vis = vis;
	data->ir = ir;
	if(data->gain == gain_low) {
		data->vis *= 16;
		data->ir *= 16;
	}
	LOG_DBG("%s: fetched values: vis = %u, ir = %u", dev->name, vis, ir);
	if (data->gain == gain_high && (ir >= 50000U || vis >= 50000U)) {
		data->gain = gain_low;
		LOG_INF("%s: changing to low gain", dev->name);
		rc = write(dev, REGISTER_TIMING, TIMING_INTEG_MS_402, false,
			   false);
		if (rc != 0) {
			LOG_ERR("%s: changing gain failed with rc %d",
				dev->name, rc);
			return rc;
		}
	} else if (data->gain == gain_low && ir <= 1000U && vis <= 1000U) {
		data->gain = gain_high;
		LOG_INF("%s: changing to high gain", dev->name);
		rc = write(dev, REGISTER_TIMING,
			   TIMING_INTEG_MS_402 | MASK_TIMING_GAIN, false,
			   false);
		if (rc != 0) {
			LOG_ERR("%s: changing gain failed with rc %d",
				dev->name, rc);
			return rc;
		}
	}
	return rc;
}

static int channel_get(const struct device *dev, enum sensor_channel chan,
		       struct sensor_value *val)
{
	struct apds9301_data *data = dev->data;
	int rc = 0;

	if (chan == SENSOR_CHAN_LIGHT) {
		int32_t value;

		value = data->vis - data->ir;
		val->val1 = value / 40;
		val->val2 = (value % 40) * 25000;
	} else {
		rc = -ENOTSUP;
	}
	return rc;
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
