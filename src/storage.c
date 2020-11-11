/**
 * This file implements all storage related functions, like file system
 * mount points.
 *
 */

#include "storage.h"
#include <drivers/flash.h>
#include <fs/fs.h>
#include <fs/littlefs.h>
#include <settings/settings.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(storage, LOG_LEVEL_DBG);

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);

/**
 * Mount point for flash storage
 *
 */
static struct fs_mount_t flash_storage_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &storage,
	.storage_dev = (void *)FLASH_AREA_ID(storage),
	.mnt_point = STORAGE_MOUNTPOINT_FLASH,
};

/**
 * Initialise all available storages
 *
 */
int storage_init(void)
{
	int rc;
	struct fs_mount_t *mp = &flash_storage_mnt;
	unsigned int id = (uintptr_t)mp->storage_dev;
	const struct flash_area *pfa;
	struct fs_statvfs sbuf;

	rc = flash_area_open(id, &pfa);
	if (rc != 0) {
		LOG_ERR("failed to open flash area with rc %d", rc);
		return rc;
	}

	rc = fs_mount(mp);
	if (rc != 0) {
		LOG_ERR("failed to mount file system with rc %d", rc);
		return rc;
	}

	rc = fs_statvfs(mp->mnt_point, &sbuf);
	if (rc != 0) {
		LOG_ERR("failed to get file system status with rc %d", rc);
		return rc;
	}

	LOG_INF("%s: bsize = %lu; frsize = %lu;"
		" blocks = %lu; bfree = %lu\n",
		mp->mnt_point, sbuf.f_bsize, sbuf.f_frsize, sbuf.f_blocks,
		sbuf.f_bfree);

	rc = settings_subsys_init();
	if (rc != 0) {
		LOG_ERR("failed to initialise settings subsystem with rc %d",
			rc);
		return rc;
	}

	rc = settings_load();
	if (rc != 0) {
		LOG_ERR("failed to load settings with rc %d", rc);
	}
	return 0;
}

/**
 * Deinitialise and secure (mainly unmount) all available storeages
 *
 */
int storage_deinit(void)
{
	int rc;
	rc = fs_unmount(&flash_storage_mnt);
	if(rc != 0) {
		LOG_ERR("failed to deinitialise file system with rc %d", rc);
	}
	return 0;
}

/**
 * Read a single parameter from storage space
 * 
 * @param id	The parameter identifier
 * @param data	The parameter data
 * @param len	The parameter dat length
 * @return		The number of read bytes or an error code
 */
int storage_read_parameter(uint16_t id, void *data, size_t len)
{
	return 0;
}

/**
 * Write a single parameter to storage space
 * 
 * @param id	The parameter identifier
 * @param data	The parameter data
 * @param len	The parameter dat length
 * @return		The number of written bytes or an error code
 */
int storage_write_parameter(uint16_t id, void *data, size_t len)
{
	return 0;
}
