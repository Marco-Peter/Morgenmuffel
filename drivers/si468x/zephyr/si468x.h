/*
 * Copyright (c) 2020 Marco Peter
 */
#ifndef __VS1053_H__
#define __VS1053_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <device.h>

enum si4689_mode {
        si4689_MODE_DAB,
        si4689_MODE_FM,
        si4689_MODE_AM
};

struct si4689_api {
        int (*startup)(const struct device *dev, enum si4689_mode mode);
        int (*powerdown)(const struct device *dev);
};

static inline int si4689_startup(const struct device *dev, enum si4689_mode mode)
{
        const struct si4689_api *api = (const struct si4689_api *)dev->api;

        return api->startup(dev, mode);
}

static inline int si4689_powerdown(const struct device *dev)
{
        const struct si4689_api *api = (const struct si4689_api *)dev->api;

        return api->powerdown(dev);
}

#ifdef __cplusplus
}
#endif

#endif /* __VS1053_H__ */
