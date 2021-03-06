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
#include <drivers/gpio.h>
#include <si468x.h>
#include "powersupply.h"
#include "usb_device_handler.h"
#include "events.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static void show_initScreen(void);
static void show_sht21(void);
static void show_ms5637(void);
static void show_apds9301(void);

static const struct device *rh_sens;
static const struct device *pr_sens;
static const struct device *lx_sens;
static const struct device *tuner;
static const struct device *wlan_chip_en_port;
static const struct device *wlan_update_port;
static const struct device *wlan_rst;

uint8_t buffer[2048];

static inline int abs(int value)
{
	return value < 0 ? -value : value;
}

void main(void)
{
	int rc;

	rc = powersupply_request_1v8();
	if (rc != 0) {
		LOG_ERR("Requesting 1.8V failed with rc %d", rc);
	}
	k_sleep(K_MSEC(10));

	rh_sens = device_get_binding("SHT2X");
	if (rh_sens == NULL) {
		LOG_ERR("humidity sensor not found");
	}
	pr_sens = device_get_binding("MS5637");
	if (rh_sens == NULL) {
		LOG_ERR("pressure sensor not found");
	}
	lx_sens = device_get_binding("APDS9301");
	if (rh_sens == NULL) {
		LOG_ERR("lux sensor not found");
	}
	tuner = device_get_binding("TUNER");
	if (rh_sens == NULL) {
		LOG_ERR("tuner not found");
	}
	/*
	wlan_chip_en_port = device_get_binding("GPIOA");
	if (wlan_chip_en_port == NULL) {
		LOG_ERR("WLan enable port not found");
	}
	wlan_update_port = wlan_chip_en_port;
	wlan_rst = device_get_binding("GPIOB");
	if (wlan_rst == NULL) {
		LOG_ERR("WLan reset port not found");
	}

	rc = gpio_pin_configure(wlan_chip_en_port, 4, GPIO_OUTPUT_INACTIVE);
	if (rc != 0) {
		LOG_ERR("Failed to configure enable output");
	}
	rc = gpio_pin_configure(wlan_rst, 0,
				GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);
	if (rc != 0) {
		LOG_ERR("Failed to configure reset output");
	}
	rc = gpio_pin_configure(wlan_update_port, 3,
				GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);
	if (rc != 0) {
		LOG_ERR("Failed to configure update output");
	}
	*/

	LOG_DBG("send command initScreen");
	display_command(show_initScreen);
	/*
	rc = si468x_bandscan(tuner, si468x_MODE_DAB, buffer);
	if (rc != 0) {
		LOG_ERR("Bandscan failed with rc %d", rc);
	}

	rc = si468x_play_service(tuner, si468x_MODE_DAB, 42);
	if (rc != 0) {
		LOG_ERR("Failed to start the tuner with rc %d", rc);
	}
	*/
	/*
	k_sleep(K_MSEC(100));
	rc = gpio_pin_set(wlan_chip_en_port, 4, 1);
	if (rc != 0) {
		LOG_ERR("Failed to switch on enable pin");
	}
	k_sleep(K_MSEC(100));
	rc = gpio_pin_set(wlan_rst, 0, 0);
	if (rc != 0) {
		LOG_ERR("Failed to release reset pin");
	}
	k_sleep(K_MSEC(100));

	rc = usb_devicehandler_init();
	if (rc != 0) {
		LOG_ERR("Initialisation of USB failed");
	}
	
	rc = usb_enable(NULL);
	if (rc != 0) {
		LOG_ERR("Failed to enable USB with rc %d", rc);
	}
	*/
	for (;;) {
		int event;
		event = wait_for_user_event();
		LOG_DBG("Got user input 0x%08X", event);
/*
		k_sleep(K_SECONDS(1));
		display_command(show_sht21);
		display_command(show_apds9301);*/
	}
}

static void show_initScreen(void)
{
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(label, "Morgenmuffel");
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	//lv_obj_set_auto_realign(label, true);
}

static void show_sht21(void)
{
	struct sensor_value temperature1;
	struct sensor_value humidity;
	static lv_obj_t *lbl_sht21;
	int rc;
	char text[] = "Hier gibt es nix zu sehen!";

	rc = sensor_sample_fetch(rh_sens);
	if (rc != 0) {
		LOG_ERR("Fetching data from SHT21 failed with error %d", rc);
	}
	rc = sensor_channel_get(rh_sens, SENSOR_CHAN_AMBIENT_TEMP,
				&temperature1);
	if (rc != 0) {
		LOG_ERR("Getting temperature from SHT21 failed with eror %d",
			rc);
	}
	rc = sensor_channel_get(rh_sens, SENSOR_CHAN_HUMIDITY, &humidity);
	if (rc != 0) {
		LOG_ERR("Getting humidity from SHT21 failed with error %d", rc);
	}

	if (lbl_sht21 == NULL) {
		lbl_sht21 = lv_label_create(lv_scr_act(), NULL);
	}
	snprintf(text, sizeof(text), "%s%d.%dC %d.%d%%",
		 temperature1.val1 < 0 || temperature1.val2 < 0 ? "-" : "",
		 abs(temperature1.val1), abs(temperature1.val2) / 10000,
		 humidity.val1, humidity.val2 / 10000);
	lv_label_set_text(lbl_sht21, text);
	lv_obj_align(lbl_sht21, NULL, LV_ALIGN_IN_TOP_MID, 0, 12);
}

static void show_ms5637(void)
{
	int rc;
	struct sensor_value temperature2;
	struct sensor_value pressure;
	static lv_obj_t *lbl_ms5637_1;
	static lv_obj_t *lbl_ms5637_2;
	char text[] = "Hier gibt es nix zu sehen!";

	rc = sensor_sample_fetch(pr_sens);
	if (rc != 0) {
		LOG_ERR("Fetching data from MS5637 failed with error %d", rc);
	}
	rc = sensor_channel_get(pr_sens, SENSOR_CHAN_AMBIENT_TEMP,
				&temperature2);
	if (rc != 0) {
		LOG_ERR("Getting temperature from MS5637 failed with eror %d",
			rc);
	}
	rc = sensor_channel_get(pr_sens, SENSOR_CHAN_PRESS, &pressure);
	if (rc != 0) {
		LOG_ERR("Getting temperature from MS5637 failed with eror %d",
			rc);
	}

	if (lbl_ms5637_1 == NULL) {
		lbl_ms5637_1 = lv_label_create(lv_scr_act(), NULL);
	}
	if (lbl_ms5637_2 == NULL) {
		lbl_ms5637_2 = lv_label_create(lv_scr_act(), NULL);
	}

	snprintf(text, sizeof(text), "%s%d.%dC",
		 temperature2.val1 < 0 || temperature2.val2 < 0 ? "-" : "",
		 abs(temperature2.val1), abs(temperature2.val2) / 10000);
	lv_label_set_text(lbl_ms5637_1, text);
	lv_obj_align(lbl_ms5637_1, NULL, LV_ALIGN_IN_TOP_MID, 0, 24);
	snprintf(text, sizeof(text), "%d.%dKpa", pressure.val1,
		 pressure.val2 / 1000);
	lv_label_set_text(lbl_ms5637_2, text);
	lv_obj_align(lbl_ms5637_2, NULL, LV_ALIGN_IN_TOP_MID, 0, 36);
}

static void show_apds9301(void)
{
	int rc;
	struct sensor_value light_value;
	static lv_obj_t *lbl_apds9301_1;
	char text[] = "Hier gibt es nix zu sehen!";

	rc = sensor_sample_fetch(lx_sens);
	if (rc != 0) {
		LOG_ERR("Fetching data from APDS9301 failed with error %d", rc);
	}
	rc = sensor_channel_get(lx_sens, SENSOR_CHAN_LIGHT, &light_value);
	if (rc != 0) {
		LOG_ERR("Getting illuminance from APDS9301 failed with eror %d",
			rc);
	}
	if (lbl_apds9301_1 == NULL) {
		lbl_apds9301_1 = lv_label_create(lv_scr_act(), NULL);
	}
	snprintf(text, sizeof(text), "%d.%d lux", light_value.val1,
		 light_value.val2 / 1000);
	lv_label_set_text(lbl_apds9301_1, text);
	lv_obj_align(lbl_apds9301_1, NULL, LV_ALIGN_IN_TOP_MID, 0, 24);
}
