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
#include <drivers/sensor.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static void show_initScreen(void);
static void show_temperature(void);

static struct sensor_value temperature;
static struct sensor_value humidity;

static inline int abs(int value)
{
	return value < 0 ? -value : value;
}

void main(void)
{
	const struct device *rh_sens = device_get_binding("SHT2X");

	if (rh_sens == NULL) {
		LOG_ERR("temperature sensor not found");
	}
	LOG_DBG("send command initScreen");
	display_command(show_initScreen);

	for (;;) {
		int rc;
		LOG_DBG("send command show_temperature");
		rc = sensor_sample_fetch(rh_sens);
		if (rc != 0) {
			LOG_ERR("Fetching data from SHT21 failed with error %d",
				rc);
		}
		rc = sensor_channel_get(rh_sens, SENSOR_CHAN_AMBIENT_TEMP,
					&temperature);
		if (rc != 0) {
			LOG_ERR("Getting temperature from SHT21 failed with eror %d",
				rc);
		}
		rc = sensor_channel_get(rh_sens, SENSOR_CHAN_HUMIDITY,
					&humidity);
		if (rc != 0) {
			LOG_ERR("Getting humidity from SHT21 failed with error %d",
				rc);
		}
		display_command(show_temperature);
		LOG_DBG("Cycle finished");
		k_sleep(K_SECONDS(1));
	}
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
	snprintf(text, sizeof(text), "%s%d.%dC %d.%d%%",
		 temperature.val1 < 0 || temperature.val2 < 0 ? "-" : "",
		 abs(temperature.val1), abs(temperature.val2) / 10000,
		 abs(humidity.val1), abs(humidity.val2) / 10000);
	lv_label_set_text(label, text);
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 12);
}
