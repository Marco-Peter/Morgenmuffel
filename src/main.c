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

static struct sensor_value temperature1;
static struct sensor_value temperature2;
static struct sensor_value humidity;
static struct sensor_value pressure;

static inline int abs(int value)
{
	return value < 0 ? -value : value;
}

void main(void)
{
	const struct device *rh_sens = device_get_binding("SHT2X");
	const struct device *pr_sens = device_get_binding("MS5637");

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
					&temperature1);
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
		rc = sensor_sample_fetch(pr_sens);
		if(rc != 0) {
			LOG_ERR("Fetching data from MS5637 failed with error %d", rc);
		}
		rc = sensor_channel_get(pr_sens, SENSOR_CHAN_AMBIENT_TEMP, &temperature2);
		if (rc != 0) {
			LOG_ERR("Getting temperature from MS5637 failed with eror %d",
				rc);
		}
		rc = sensor_channel_get(pr_sens, SENSOR_CHAN_PRESS, &pressure);
		if (rc != 0) {
			LOG_ERR("Getting temperature from SHT21 failed with eror %d",
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
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	//lv_obj_set_auto_realign(label, true);
}

static void show_temperature(void)
{
	static lv_obj_t *label1;
	static lv_obj_t *label2;
	static lv_obj_t *label3;
	char text[] = "Hier gibt es nix zu sehen!";

	if (label1 == NULL) {
		label1 = lv_label_create(lv_scr_act(), NULL);
	}
	if (label2 == NULL) {
		label2 = lv_label_create(lv_scr_act(), NULL);
	}
	if (label3 == NULL) {
		label3 = lv_label_create(lv_scr_act(), NULL);
	}
	snprintf(text, sizeof(text), "%s%d.%dC %d.%d%%",
		 temperature1.val1 < 0 || temperature1.val2 < 0 ? "-" : "",
		 abs(temperature1.val1), abs(temperature1.val2) / 10000,
		 humidity.val1, humidity.val2 / 10000);
	lv_label_set_text(label1, text);
	lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_MID, 0, 12);
	snprintf(text, sizeof(text), "%s%d.%dC",
		 temperature2.val1 < 0 || temperature2.val2 < 0 ? "-" : "",
		 abs(temperature2.val1), abs(temperature2.val2) / 10000);
	lv_label_set_text(label2, text);
	lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_MID, 0, 24);
	snprintf(text, sizeof(text), "%d.%dKpa",
		 pressure.val1, pressure.val2 / 1000);
	lv_label_set_text(label3, text);
	lv_obj_align(label3, NULL, LV_ALIGN_IN_TOP_MID, 0, 36);
}
