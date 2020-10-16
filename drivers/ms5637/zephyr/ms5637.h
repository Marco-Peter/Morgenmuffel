/*
 * Copyright (c) 2020 Marco Peter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MS5637_H__
#define __MS5637_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <device.h>

__subsystem struct ms5637_driver_api {
	int32_t (*meas_temp)(const struct device *dev);
	int32_t (*meas_press)(const struct device *dev);
};

__syscall     int32_t        ms5637_meas_temp(const struct device *dev);
static inline int32_t z_impl_ms5637_meas_temp(const struct device *dev)
{
	const struct ms5637_driver_api *api = dev->api;

	__ASSERT(api->meas_temp, "Callback pointer must not be NULL");
	return api->meas_temp(dev);
}

__syscall     int32_t        ms5637_meas_press(const struct device *dev);
static inline int32_t z_impl_ms5637_meas_press(const struct device *dev)
{
	const struct ms5637_driver_api *api = dev->api;

	__ASSERT(api->meas_temp, "Callback pointer must not be NULL");
	return api->meas_temp(dev);
}

#include <syscalls/sht2x.h>

#ifdef __cplusplus
}
#endif

#endif /* __MS637_H__*/
