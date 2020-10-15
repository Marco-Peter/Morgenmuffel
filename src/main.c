/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <errno.h>
#include <logging/log.h>
#include <zephyr.h>
#include "display.h"
#include "../drivers/sht2x/zephyr/sht2x.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define abs(x) ((x) < 0 ? -(x) : (x))

static void show_initScreen(void);
static void show_temperature(void);

static int32_t temperature;
static int32_t humidity;

void main(void)
{
	const struct device *temp_sens = device_get_binding("SHT2X");

	if (temp_sens == NULL) {
		LOG_ERR("temperature sensor not found");
	}
	LOG_INF("send command initScreen");
	display_command(show_initScreen);

	LOG_INF("send command show_temperature");
	temperature = sht2x_meas_temp(temp_sens);
	humidity = sht2x_meas_rh(temp_sens);
	if (temperature < 0) {
		LOG_ERR("Temperature measurement failed with error %d",
			temperature);
	}
	if (humidity < 0) {
		LOG_ERR("Humidity measurement failed with error %d", humidity);
	}
	display_command(show_temperature);

	LOG_INF("finished");

	k_sleep(K_FOREVER);
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
	char text[] = "Nix zu sehen!";

	if (label == NULL) {
		label = lv_label_create(lv_scr_act(), NULL);
	}
	sprintf(text, "%d.%d°C  %d.%d%%", temperature / 100,
		abs(temperature) % 100, humidity / 100, abs(humidity) % 100);
	lv_label_set_text(label, text);
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 12);
}
