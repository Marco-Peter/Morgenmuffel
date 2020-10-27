/*
 * Copyright (c) 2020 Marco Peter
 */

#ifndef __SI468X_PRIVATE_H__
#define __SI468X_PRIVATE_H__

#include "si468x.h"

#include <drivers/gpio.h>

//Enable top level interrupt sources
#define SI468X_PROP_INT_CTL_ENABLE 0x0000
// seek / tune complete --> Safe for next Seek/Tune command
#define SI468X_PROP_INT_CTL_ENABLE_STCIEN 0x0001
// ACF information changed (FM mode)
#define SI468X_PROP_INT_CTL_ENABLE_ACFIEN 0x0002
// RDS information updated (FM mode)
#define SI468X_PROP_INT_CTL_ENABLE_RDSIEN 0x0004
// Received signal quality interrupt
#define SI468X_PROP_INT_CTL_ENABLE_RSQIEN 0x0008
// enabled data component needs attention --> GET_DIGITAL_SERVICE_DATA
#define SI468X_PROP_INT_CTL_ENABLE_DSRVIEN 0x0010
// digital radio link change interrupt --> DAB_DIGRAD_STATUS
#define SI468X_PROP_INT_CTL_ENABLE_DACQIEN 0x0020
// Command error
#define SI468X_PROP_INT_CTL_ENABLE_ERR_CMDIEN 0x0040
// Clear to send next command
#define SI468X_PROP_INT_CTL_ENABLE_CTSIEN 0x0080
#define SI468X_PROP_INT_CTL_ENABLE_CTSIEN_OFFS 7
// digital radio event change interrupt
#define SI468X_PROP_INT_CTL_ENABLE_DEVNTIEN 0x2000

#define SI468X_PROP_INT_CTL_REPEAT 0x0001
#define SI468X_PROP_DIGITAL_IO_OUTPUT_SELECT 0x0200
#define SI468X_PROP_DIGITAL_IO_OUTPUT_SAMPLE_RATE 0x0201
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT 0x0202
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVRDS1 0x0203
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVRDS2 0x0204
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVRDS3 0x0205
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVRDS4 0x0206
#define SI468X_PROP_AUDIO_ANALOG_VOLUME 0x0300
#define SI468X_PROP_AUDIO_MUTE 0x0301
#define SI468X_PROP_AUDIO_OUTPUT_CONFIG 0x0302
#define SI468X_PROP_PIN_CONFIG_ENABLE 0x0800
#define SI468X_PROP_WAKE_TONE_ENABLE 0x0900
#define SI468X_PROP_WAKE_TONE_PERIOD 0x0901
#define SI468X_PROP_WAKE_TONE_FREQ 0x0902
#define SI468X_PROP_WAKE_TONE_AMPLITUDE 0x0903
#define SI468X_PROP_TUNE_FRONTEND_VARM 0x1710 // slope
#define SI468X_PROP_TUNE_FRONTEND_VARB 0x1711 // intercept
#define SI468X_PROP_TUNE_FRONTEND_CFG 0x1712

#if IS_ENABLED(CONFIG_SI468X_DAB)
struct si468x_dab_service {
	uint16_t id;
	uint8_t primary_comp_id;
	uint8_t channel;
};
#endif

enum si468x_pup_state {
	si468x_PUP_RESET = 0,
	si468x_PUP_BOOTLOADER = 2,
	si468x_PUP_APPLICATION = 3
};

struct si468x_config {
	gpio_flags_t int_gpio_flags;
	gpio_flags_t reset_gpio_flags;
	gpio_flags_t cs_gpio_flags;
	uint16_t spi_slave_number;
	char *spi_bus_label;
	char *int_gpio_label;
	char *reset_gpio_label;
	char *cs_gpio_label;
	gpio_pin_t int_gpio_pin;
	gpio_pin_t reset_gpio_pin;
	gpio_pin_t cs_gpio_pin;
};

struct si468x_data {
	const struct device *spi;
	const struct device *int_gpio;
	const struct device *reset_gpio;
	const struct device *cs_gpio;
	struct gpio_callback gpio_callback;
	struct k_sem sem;
#if IS_ENABLED(CONFIG_SI468X_DAB)
	struct si468x_dab_service services[CONFIG_SI468X_DAB_SERVICE_LIST_SIZE];
	uint8_t current_channel;
#endif
	uint16_t current_service;
	enum si468x_mode current_mode;
	enum si468x_pup_state pup_state;
};

int si468x_dab_startup(const struct device *dev);
int si468x_dab_play_service(const struct device *dev, uint16_t service);
int si468x_dab_process_events(const struct device *dev, bool ack_only);
int si468x_dab_bandscan(const struct device *dev);

int si468x_fmhd_startup(const struct device *dev);
int si468x_fmhd_play_service(const struct device *dev, uint16_t service);
int si468x_fmhd_process_events(const struct device *dev, bool ack_only);
int si468x_fmhd_bandscan(const struct device *dev);

int si468x_am_startup(const struct device *dev);
int si468x_am_play_service(const struct device *dev, uint16_t service);
int si468x_am_process_events(const struct device *dev, bool ack_only);
int si468x_am_bandscan(const struct device *dev);

#endif /* __SI468X_PRIVATE_H__ */
