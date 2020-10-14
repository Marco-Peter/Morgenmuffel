/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

//#include "display_handler.h"
#include <device.h>
#include <drivers/flash.h>
#include <errno.h>
#include <logging/log.h>
#include <drivers/spi.h>
#include <storage/flash_map.h>
#include <sys/printk.h>
#include <zephyr.h>
#include "display.h"

static void show_initScreen(void);

void main(void)
{
	display_command(show_initScreen);

	k_sleep(K_SECONDS(5));

	display_off();

	for(;;);
}

static void show_initScreen(void)
{
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(label, "Morgenmuffel");
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
	//lv_obj_set_auto_realign(label, true);
}
