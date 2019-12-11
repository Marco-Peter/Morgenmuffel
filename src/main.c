/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/printk.h>
#include <zephyr.h>
#include <device.h>
#include <spi.h>
#include <drivers/flash.h>
#include <fs/nvs.h>
//#include <include/generated_dts_board.h>

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
  struct device *spi1;
  struct device *spi2;
  struct device *spiflash;
  int rc;

  struct spi_cs_control spiControl_flash =
      {
          .gpio_dev = device_get_binding(DT_INST_0_JEDEC_SPI_NOR_CS_GPIOS_CONTROLLER),
          .gpio_pin = DT_INST_0_JEDEC_SPI_NOR_CS_GPIOS_PIN,
          .delay = 0};

  struct spi_config spiConfig_flash =
      {
          .frequency = 75000000,
          .operation =
              SPI_OP_MODE_MASTER |
              SPI_TRANSFER_MSB |
              SPI_WORD_SET(8) |
              SPI_LINES_SINGLE,
          .slave = 0,
          .cs = &spiControl_flash};

  uint8_t query = 0x9F;
  uint8_t ans[3];
  struct spi_buf buffers[] = {
      {.buf = &query,
       .len = sizeof(query)},
      {.buf = &ans,
       .len = sizeof(ans)}};

  struct spi_buf_set bufSet =
      {
          .buffers = buffers,
          .count = sizeof(buffers) / sizeof(buffers[0])};

  spi1 = device_get_binding(DT_SPI_1_NAME);
  printk("SPI1 address: 0x%x\n", (uint32_t)spi1);
  spi2 = device_get_binding(DT_SPI_2_NAME);
  printk("SPI2 address: 0x%x\n", (uint32_t)spi2);

  rc = spi_transceive(spi2, &spiConfig_flash, &bufSet, &bufSet);

  spiflash = device_get_binding(DT_INST_0_JEDEC_SPI_NOR_LABEL);
  printk("SPI FLASH address: 0x%x\n", (uint32_t)spiflash);

  nvs.offset = FLASH_AREA_STORAGE_OFFSET;
  rc = flash_get_page_info_by_offs(
      spiflash,
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
