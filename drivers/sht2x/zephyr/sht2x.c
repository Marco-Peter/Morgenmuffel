/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sht2x.h"
#include <zephyr/types.h>
#include <syscall_handler.h>
#include <drivers/i2c.h>

#define SHT2X_CMD_TRIGGER_MEAS_TEMP 0xF3
#define SHT2X_CMD_TRIGGER_MEAS_RH 0xF5
#define SHT2X_CMD_WRITE_USER_REG 0xE6
#define SHT2X_CMD_READ_USER_REG 0xE7
#define SHT2X_CMD_SOFT_RESET 0xFE

static struct hello_world_dev_data {
	uint32_t foo;
} data;

static int init(const struct device *dev)
{
	data.foo = 5;

	return 0;
}

static void meas_temp_impl(const struct device *dev)
{
	printk("Hello World from the kernel: %d\n", data.foo);

	__ASSERT(data.foo == 5, "Device was not initialized!");
}

#ifdef CONFIG_USERSPACE
static inline void z_vrfy_sht2x_meas_temp(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_HELLO_WORLD(dev, print));

	z_impl_hello_world_print(dev);
}
#include <syscalls/sht2x_meas_temp_mrsh.c>
#endif /* CONFIG_USERSPACE */

static void meas_rh_impl(const struct device *dev)
{
	printk("Hello World from the kernel: %d\n", data.foo);

	__ASSERT(data.foo == 5, "Device was not initialized!");
}

#ifdef CONFIG_USERSPACE
static inline void z_vrfy_sht2x_meas_rh(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_HELLO_WORLD(dev, print));

	z_impl_hello_world_print(dev);
}
#include <syscalls/sht2x_meas_rh_mrsh.c>
#endif /* CONFIG_USERSPACE */

DEVICE_AND_API_INIT(hello_world, "CUSTOM_DRIVER", init, &data, NULL,
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &((struct hello_world_driver_api){
			    .meas_temp = meas_temp_impl,
			    .meas_rh = meas_rh_impl }));
