/**
 * This file declares all storage related functions, like file system
 * mount points.
 *
 *****************************************************************************/

#pragma once

#include <stddef.h>
#include <zephyr/types.h>

#define MAX_PATH_LEN 255
#define STORAGE_MOUNTPOINT_FLASH "/flash"

int storage_init(void);
int storage_deinit(void);
int storage_read_parameter(u16_t id, void *data, size_t len);
int storage_write_parameter(u16_t id, void *data, size_t len);
