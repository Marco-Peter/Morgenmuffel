/*
 * Copyright (c) 2019 Marco Peter
 * 
 * This file contains all available parameter identifiers on the system.
 * Usage of the parameters are the task of the implementing modules.
 */

#pragma once

#define PARAM_SYSTEM_VOLUME 0
#define PARAM_SYSTERM_POWERSTATE 1

#define PARAM_PLAYLIST_CURRENT_ENTRY 1000
#define PARAM_PLAYLIST_START 1001

/// Define the display properties
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_PAGES 8
#define DISPLAY_PAGESIZE 8
#define DISPLAY_COMPLETE (DISPLAY_WIDTH * DISPLAY_PAGES)

#define DISPLAY_MIN_CHARWIDTH 5 // minimum character width
#define DISPLAY_MAX_CHARWIDTH 5 // maximum character width
