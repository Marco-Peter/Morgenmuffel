/*
 * Copyright (c) 2019 Marco Peter
 *
 * This file contains all available parameter identifiers on the system.
 * Usage of the parameters are the task of the implementing modules.
 */

#pragma once

#include <zephyr/types.h>

#define PARAMETERS_BASE_PLAYLIST 1000

typedef struct parameters_t {
	u16_t currentPlaylistItem;
	u8_t volume;
} parameters_t;

extern parameters_t parameters;
