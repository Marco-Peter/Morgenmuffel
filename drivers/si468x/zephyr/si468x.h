/*
 * Copyright (c) 2020 Marco Peter
 */
#ifndef __SI468X_H__
#define __SI468X_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <device.h>

enum si468x_mode {
	si468x_MODE_OFF,
#if IS_ENABLED(CONFIG_SI468X_DAB)
	si468x_MODE_DAB = CONFIG_SI468X_FLASH_START_IMAGE_DAB,
#endif
#if IS_ENABLED(CONFIG_SI468X_FMHD)
	si468x_MODE_FMHD = CONFIG_SI468X_FLASH_START_IMAGE_FM,
#endif
#if IS_ENABLED(CONFIG_SI468X_AM)
	si468x_MODE_AM = CONFIG_SI468X_FLASH_START_IMAGE_AM,
#endif
};

struct si468x_api {
	int (*powerdown)(const struct device *dev);
	int (*play_service)(const struct device *dev, enum si468x_mode mode,
			    uint16_t service);
	int (*process_events)(const struct device *dev);
	int (*bandscan)(const struct device *dev, enum si468x_mode mode);
	struct k_sem *(*get_semaphore)(const struct device *dev);
};

static inline int si468x_powerdown(const struct device *dev)
{
	const struct si468x_api *api = (const struct si468x_api *)dev->api;

	return api->powerdown(dev);
}

static inline int si468x_play_service(const struct device *dev,
				      enum si468x_mode mode, uint16_t service)
{
	int rc;
	const struct si468x_api *api = (const struct si468x_api *)dev->api;

	if (mode == si468x_MODE_OFF) {
		rc = api->powerdown(dev);
	} else {
		rc = api->play_service(dev, mode, service);
	}
	return rc;
}

static inline int si468x_process_events(const struct device *dev)
{
	const struct si468x_api *api = (const struct si468x_api *)dev->api;

	return api->process_events(dev);
}

static inline int si468x_bandscan(const struct device *dev,
				  enum si468x_mode mode)
{
	int rc;
	const struct si468x_api *api = (const struct si468x_api *)dev->api;

	if (mode == si468x_MODE_OFF) {
		rc = api->powerdown(dev);
	} else {
		rc = api->bandscan(dev, mode);
	}
	return rc;
}

static inline struct k_sem *si468x_get_semaphore(const struct device *dev)
{
	const struct si468x_api *api = (const struct si468x_api *)dev->api;

	return api->get_semaphore(dev);
}

#ifdef __cplusplus
}
#endif

#endif /* __SI468X_H__ */
