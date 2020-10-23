/*
 * Copyright (c) 2020 Marco Peter
 */

#include "si468x_commands.h"
#include "si468x_private.h"
#include <logging/log.h>

LOG_MODULE_REGISTER(si468x_commands_dab, LOG_LEVEL_DBG);

#define SI468X_CMD_DAB_TUNE_FREQ 0xB0
#define SI468X_CMD_DAB_DIGRAD_STATUS 0xB2
#define SI468X_CMD_DAB_GET_EVENT_STATUS 0xB3
#define SI468X_CMD_DAB_GET_ENSEMBLE_INFO 0xB4
#define SI468X_CMD_DAB_GET_ANNOUNCEMENT_SUPPORT_INFO 0xB5
#define SI468X_CMD_DAB_GET_ANNOUNCEMENT_INFO 0xB6
#define SI468X_CMD_DAB_GET_SERVICE_LINKING_INFO 0xB7
#define SI468X_CMD_DAB_SET_FREQ_LIST 0xB8
#define SI468X_CMD_DAB_GET_FREQ_LIST 0xB9
#define SI468X_CMD_DAB_GET_COMPONENT_INFO 0xBB
#define SI468X_CMD_DAB_GET_TIME 0xBC
#define SI468X_CMD_DAB_GET_AUDIO_INFO 0xBD
#define SI468X_CMD_DAB_GET_SUBCHAN_INFO 0xBE
#define SI468X_CMD_DAB_GET_FREQ_INFO 0xBF
#define SI468X_CMD_DAB_GET_SERVICE_INFO 0xC0
#define SI468X_CMD_DAB_GET_OE_SERVICES_INFO 0xC1
#define SI468X_CMD_DAB_ACF_STATUS 0xC2

static int wait_for_stcint(const struct device *dev,
			   const struct spi_buf_set *spi_buf_set)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	do {
		rc = si468x_cmd_wait_for_cts(dev, spi_buf_set);
	} while (rc >= 0 && data->seek_tune_complete == false);
	return rc;
}

int si468x_cmd_dab_tune(const struct device *dev, uint8_t channel,
			uint16_t ant_cap)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;
	uint8_t cmd[] = {
		SI468X_CMD_DAB_TUNE_FREQ, 0U, channel, 0U, ant_cap & 0xFFU,
		(ant_cap >> 8) & 0xFFU
	};
	struct spi_buf buf = { .buf = cmd, .len = sizeof(cmd) };
	struct spi_buf_set buf_set = { .buffers = &buf, .count = 1 };

        data->seek_tune_complete = false;
	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command tune_freq failed with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = wait_for_stcint(dev, NULL);
	if (rc < 0) {
		LOG_ERR("%s: waiting for STC failed with rc %d", dev->name, rc);
		return rc;
	}
	return 0;
}
