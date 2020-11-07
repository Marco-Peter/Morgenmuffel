/**
 * This file declares all storage related functions, like file system
 * mount points.
 *
 *****************************************************************************/

#ifndef STORAGE_H_
#define STORAGE_H_

#define MAX_PATH_LEN 255
#define STORAGE_MOUNTPOINT_FLASH "/flash"

int storage_init(void);
int storage_deinit(void);
int storage_read_parameter(uint16_t id, void *data, size_t len);
int storage_write_parameter(uint16_t id, void *data, size_t len);

#endif /* STORAGE_H_ */