/*
 * Copyright (c) 2020 Marco Peter
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __SHT2X_H__
#define __SHT2X_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <device.h>

__subsystem struct sht2x_driver_api {
	int32_t (*meas_temp)(const struct device *dev);
	int32_t (*meas_rh)(const struct device *dev);
};

/**
 * Return the relative humidity in 1/100 %.
 * Divide by 100 to get the RH% value in percent (e.g. 45.34%).
 * Divide by 10000 to get the RH value (e.g. 0.4534).
 */
__syscall     int32_t        sht2x_meas_temp(const struct device *dev);
static inline int32_t z_impl_sht2x_meas_temp(const struct device *dev)
{
	const struct sht2x_driver_api *api = dev->api;

	__ASSERT(api->meas_temp, "Callback pointer must not be NULL");
	return api->meas_temp(dev);
}

/**
 * Return the temperature in 1/100 °C.
 * 
 * Divide by 100 to get the temperature in °C.
 */
__syscall     int32_t        sht2x_meas_rh(const struct device *dev);
static inline int32_t z_impl_sht2x_meas_rh(const struct device *dev)
{
	const struct sht2x_driver_api *api = dev->api;

	__ASSERT(api->meas_rh, "Callback pointer must not be NULL");
	return api->meas_rh(dev);
}

#ifdef __cplusplus
}
#endif

#include <syscalls/sht2x.h>

#endif /* __SHT2X_H__ */