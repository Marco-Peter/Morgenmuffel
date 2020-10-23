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
        int (*startup)(const struct device *dev, enum si468x_mode mode);
        int (*powerdown)(const struct device *dev);
};

static inline int si468x_startup(const struct device *dev, enum si468x_mode mode)
{
        const struct si468x_api *api = (const struct si468x_api *)dev->api;

        return api->startup(dev, mode);
}

static inline int si468x_powerdown(const struct device *dev)
{
        const struct si468x_api *api = (const struct si468x_api *)dev->api;

        return api->powerdown(dev);
}

#ifdef __cplusplus
}
#endif

#endif /* __SI468X_H__ */
