/*
 * Copyright (c) 2020 Marco Peter
 */
#ifndef __VS1053_H__
#define __VS1053_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <device.h>

void vs1053_play_data( struct device *dev, void *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __VS1053_H__ */
