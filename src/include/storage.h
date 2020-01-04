/**
 * This file declares all storage related functions, like file system
 * mount points.
 *
 *****************************************************************************/

#pragma once

#define MAX_PATH_LEN 255
#define STORAGE_MOUNTPOINT_FLASH "/flash"

int storage_init(void);
int storage_deinit(void);
