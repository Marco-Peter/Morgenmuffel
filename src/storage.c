/**
 * This file implements all storage related functions, like file system
 * mount points.
 *
 *****************************************************************************/

#include "storage.h"
#include <drivers/flash.h>
#include <fs/nvs.h>
#include <fs/fs.h>
#include <fs/littlefs.h>

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);

/**
 * Check the return code variable rc and return it if it is not zero
 *
 *****************************************************************************/
#define checkRc()                                                              \
	if (rc != 0) {                                                         \
		return rc;                                                     \
	}

/**
 * Mount point for flash storage
 *
 *****************************************************************************/
static struct fs_mount_t flash_storage_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &storage,
	.storage_dev = (void *)FLASH_AREA_ID(storage),
	.mnt_point = STORAGE_MOUNTPOINT_FLASH,
};

static struct nvs_fs nvs;

/**
 * Initialise all available storages
 *
 *****************************************************************************/
int storage_init(void)
{
	int rc;
	struct fs_mount_t *mp = &flash_storage_mnt;
	unsigned int id = (uintptr_t)mp->storage_dev;
	const struct flash_area *pfa;
	struct fs_statvfs sbuf;
	struct flash_pages_info info;
	struct device *spiflash;

	spiflash = device_get_binding("M25P16");
	rc = flash_get_page_info_by_offs(spiflash, nvs.offset, &info);
	checkRc();

	nvs.offset = 0;
	nvs.sector_size = info.size;
	nvs.sector_count = 8U;

	rc = nvs_init(&nvs, "M25P16");
	checkRc();

	rc = flash_area_open(id, &pfa);
	checkRc();

	rc = fs_mount(mp);
	checkRc();

	rc = fs_statvfs(mp->mnt_point, &sbuf);
	checkRc();

	printk("%s: bsize = %lu; frsize = %lu;"
	       " blocks = %lu; bfree = %lu\n",
	       mp->mnt_point, sbuf.f_bsize, sbuf.f_frsize, sbuf.f_blocks,
	       sbuf.f_bfree);
	return 0;
}

/**
 * Deinitialise and secure (mainly unmount) all available storeages
 *
 *****************************************************************************/
int storage_deinit(void)
{
	int rc;
	rc = fs_unmount(&flash_storage_mnt);
	checkRc();

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
int storage_read_parameter(u16_t id, void *data, size_t len)
{
	return nvs_read(&nvs, id, data, len);
}

/**
 * Write a single parameter to storage space
 * 
 * @param id	The parameter identifier
 * @param data	The parameter data
 * @param len	The parameter dat length
 * @return		The number of written bytes or an error code
 */
int storage_write_parameter(u16_t id, void *data, size_t len)
{
	return nvs_write(&nvs, id, data, len);
}
