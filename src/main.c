/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/printk.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/flash.h>
#include <fs/nvs.h>

#define FLASH_AREA_STORAGE_OFFSET 0;

static int initFlash(void);

static struct nvs_fs nvs;

/**
 * Application entry point
 */
void main(void)
{
  int rc;

  printk("Hello World! %s\n", CONFIG_BOARD);

  rc = initFlash();
  if (rc)
  {
    printk("Failed initialisation!\n");
  }
}

/**
 * Initialise all flash memory related functions, including nvs and file system.
 *
 * @return -ERRNO if something wrong happened.
 */

static int initFlash(void)
{
  struct flash_pages_info info;
  struct device *dev = NULL;
  int rc;

  dev = device_get_binding(DT_SPI_1_NAME);
  printk("SPI1 address: 0x%x\n", (uint32_t)dev);
  dev = device_get_binding("m25p16");

  nvs.offset = FLASH_AREA_STORAGE_OFFSET;
  rc = flash_get_page_info_by_offs(
      dev,
      nvs.offset,
      &info);

  if (rc)
  {
    printk("Unable to get page info\n");
    return rc;
  }

  printk("Sector size is %d bytes\n", info.size);
  /*
  nvs.sector_size = info.size;
  nvs.sector_count = 3U;

  rc = nvs_init(&nvs, DT_FLASH_DEV_NAME);
  if (rc)
  {
    printk("Flash initialisation failed\n");
    return rc;
  }
  */
  printk("Finished, all good!\n");
  return 0;
}
