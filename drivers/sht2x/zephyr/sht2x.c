/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT sensirion_sht2x

#include "sht2x.h"
#include <zephyr/types.h>
#include <syscall_handler.h>
#include <drivers/i2c.h>

#define LOG_LEVEL CONFIG_LED_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(sht2x);

#define SHT2X_CMD_TRIGGER_MEAS_TEMP 0xF3
#define SHT2X_CMD_TRIGGER_MEAS_RH 0xF5
#define SHT2X_CMD_WRITE_USER_REG 0xE6
#define SHT2X_CMD_READ_USER_REG 0xE7
#define SHT2X_CMD_SOFT_RESET 0xFE

struct sht2x_config {
	char *i2c_bus_label;
	uint8_t i2c_addr;
};

struct sht2x_data {
	const struct device *i2c;
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

static int init(const struct device *dev)
{
	const struct sht2x_config *config = dev->config;
	struct sht2x_data *data = dev->data;

	data->i2c = device_get_binding(config->i2c_bus_label);
	if (data->i2c == NULL) {
		LOG_ERR("%s: device %s not found", dev->name,
			config->i2c_bus_label);
		return -ENODEV;
	}
	return send_command(dev, SHT2X_CMD_SOFT_RESET);
}

static uint16_t meas_temp_impl(const struct device *dev)
{
	return 0U;
}

#ifdef CONFIG_USERSPACE
static inline void z_vrfy_sht2x_meas_temp(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_HELLO_WORLD(dev, print));

	z_impl_hello_world_print(dev);
}
#include <syscalls/sht2x_meas_temp_mrsh.c>
#endif /* CONFIG_USERSPACE */

static uint16_t meas_rh_impl(const struct device *dev)
{
	return 0U;
}

#ifdef CONFIG_USERSPACE
static inline void z_vrfy_sht2x_meas_rh(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_HELLO_WORLD(dev, print));

	z_impl_hello_world_print(dev);
}
#include <syscalls/sht2x_meas_rh_mrsh.c>
#endif /* CONFIG_USERSPACE */

#define SHT32_DEVICE(id)                                                       \
	static struct sht2x_config sht2x_config_##id = {                       \
		.i2c_bus_label = DT_INST_BUS_LABEL(id),                        \
		.i2c_addr = DT_INST_REG_ADDR(id)                               \
	};                                                                     \
	static struct sht2x_data sht2x_data_##id;                              \
                                                                               \
	DEVICE_AND_API_INIT(                                                   \
		sht2x_##id, DT_INST_LABEL(id), init, &sht2x_data_##id,         \
		&sht2x_config_##id, POST_KERNEL,                                \
		CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                            \
		&((struct sht2x_driver_api){ .meas_temp = meas_temp_impl,      \
					     .meas_rh = meas_rh_impl }));

DT_INST_FOREACH_STATUS_OKAY(SHT32_DEVICE)
