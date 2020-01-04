/*
 * Copyright (c) 2019 Marco Peter
 *
 * This file contains all available parameter identifiers on the system.
 * Usage of the parameters are the task of the implementing modules.
 */

#pragma once

#include <stdint.h>

typedef struct parameters_t {
  uint16_t currentPlaylistItem;
  uint8_t volume;
} parameters_t;

extern parameters_t parameters;
