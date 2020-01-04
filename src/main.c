/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

//#include "display_handler.h"
#include "storage.h"
#include <device.h>
#include <drivers/flash.h>
#include <errno.h>
#include <logging/log.h>
#include <spi.h>
#include <storage/flash_map.h>
#include <sys/printk.h>
#include <zephyr.h>

#define MAX_PATH_LEN 255

struct device *spi_device = NULL;
struct device *flash_device = NULL;

/**
 * Application entry point
 */
void main(void) {
  int rc = 0;

  // TextField initField;
  // wchar_t initTitle[] = L"Morgenmuffel";
  // DisplayPage initPage = {1, &initField};

  spi_device = device_get_binding("SPI_1");
  flash_device = device_get_binding("M25P16");

  rc = storage_init();
  if (rc) {
    printk("Failed initialisation with errno %d!\n", rc);
  }

  // dispInitTextField(&initField, initTitle, &fontFreeSans16, 0, 16, 128, 'c',
  //                  false, false, DISP_FRAME_NONE);

  // dispShowPage(&initPage);

  k_sleep(K_SECONDS(5));

  // displayOff();

  rc = storage_deinit();
  if (rc) {
    printk("Failed deinitialisation with errno %d!\n", rc);
  }

  printk("Finished, all good!\n");
}
