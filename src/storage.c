/**
 * This file implements all storage related functions, like file system
 * mount points.
 *
 *****************************************************************************/

#include "storage.h"
#include <fs/fs.h>
#include <fs/littlefs.h>

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);

/**
 * Check the return code variable rc and return it if it is not zero
 *
 *****************************************************************************/
#define checkRc()                                                              \
  if (rc != 0) {                                                               \
    return rc;                                                                 \
  }

/**
 * Mount point for flash storage
 *
 *****************************************************************************/
static struct fs_mount_t flash_storage_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &storage,
    .storage_dev = (void *)DT_FLASH_AREA_STORAGE_ID,
    .mnt_point = STORAGE_MOUNTPOINT_FLASH,
};

/**
 * Initialise all available storages
 *
 *****************************************************************************/
int storage_init(void) {
  int rc;
  unsigned int id = (uintptr_t)flash_storage_mnt.storage_dev;
  const struct flash_area *pfa;

  rc = flash_area_open(id, &pfa);
  checkRc();

  rc = fs_mount(&flash_storage_mnt);
  checkRc();

  return 0;
}

/**
 * Deinitialise and secure (mainly unmount) all available storeages
 *
 *****************************************************************************/
int storage_deinit(void) {
  int rc;
  rc = fs_unmount(&flash_storage_mnt);
  checkRc();

  return 0;
}
