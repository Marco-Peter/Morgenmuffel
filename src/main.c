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
#include "../drivers/sht2x/zephyr/sht2x.h"

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(main);

static void show_initScreen(void);
static void show_temperature(void);

static int32_t temperature;

void main(void)
{
	const struct device *temp_sens = device_get_binding("SHT2X");

	if(temp_sens == NULL) {
		LOG_ERR("temperature sensor not found");
	}
	LOG_INF("send command initScreen");
	display_command(show_initScreen);

	k_sleep(K_SECONDS(5));

	LOG_INF("send command display_clear");
	temperature = sht2x_meas_temp(temp_sens);
	display_command(show_temperature);

	k_sleep(K_SECONDS(5));

	LOG_INF("send command display_off");
	display_command(display_off);

	LOG_INF("finished");

	k_sleep(K_FOREVER);

	for(;;);
}

static void show_initScreen(void)
{
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(label, "Morgenmuffel");
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
	//lv_obj_set_auto_realign(label, true);
}

static void show_temperature(void)
{
	static lv_obj_t *label;
	char *text = "";

	if(label == NULL) {
		label = lv_label_create(lv_scr_act(), NULL);
	}
	sprintf(text, "%d.%d°C", temperature / 1000, temperature % 1000);
	lv_label_set_text(label, text);
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 12);
}
