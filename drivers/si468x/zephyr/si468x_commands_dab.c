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

#define DIGRAD_STATUS_VALID(_buf) ((_buf)[1] & 0x01)
#define DIGRAD_STATUS_ACQ(_buf) (((_buf)[1] >> 2) & 0x01)
#define DIGRAD_STATUS_FICERR(_buf) (((_buf)[1] >> 3) & 0x01)
#define DIGRAD_STATUS_HARDMUTE(_buf) (((_buf)[1] >> 4) & 0x01)
#define DIGRAD_STATUS_RSSI(_buf) ((_buf)[2])
#define DIGRAD_STATUS_SNR(_buf) ((_buf)[3])
#define DIGRAD_STATUS_FIC_QUALITY(_buf) ((_buf)[4])

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

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command tune_freq failed with rc %d",
			dev->name, rc);
		return rc;
	}
	return 0;
}

int si468x_cmd_dab_start_service(const struct device *dev, uint16_t service_id,
				 uint8_t component_id)
{
	int rc;
	uint8_t cmd[] = { SI468X_CMD_START_DIGITAL_SERVICE,
			  0U,
			  0U,
			  0U,
			  service_id & 0xFF,
			  (service_id >> 8) & 0xFF,
			  0U,
			  0U,
			  component_id & 0xFF,
			  0U,
			  0U,
			  0U };
	struct spi_buf buf = { .buf = cmd, .len = sizeof(cmd) };
	struct spi_buf_set buf_set = { .buffers = &buf, .count = 1 };

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command start service failed with rc %d",
			dev->name, rc);
		return rc;
	}
	return 0;
}

int si468x_cmd_dab_get_freq_list(const struct device *dev, uint8_t *num_freqs)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;
	uint8_t cmd[] = { SI468X_CMD_DAB_GET_FREQ_LIST, 0U };
	struct spi_buf buf = { .buf = cmd, .len = sizeof(cmd) };
	struct spi_buf_set buf_set = { .buffers = &buf, .count = 1 };

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command get_freq_list failed with rc %d",
			dev->name, rc);
		return rc;
	}
	struct spi_buf ans_buf = { .buf = num_freqs, .len = sizeof(uint8_t) };
	struct spi_buf_set ans_buf_set = { .buffers = &ans_buf, .count = 1 };

	rc = si468x_cmd_rd_reply(dev, &ans_buf_set, NULL);
	if (rc != 0) {
		LOG_ERR("%s: waiting for CTS after getting freq list failed with rc %d",
			dev->name, rc);
		return rc;
	}
	return 0;
}

int si468x_cmd_dab_digrad_status(const struct device *dev, bool digrad_ack,
				 bool stc_ack,
				 struct si468x_dab_digrad_status *status)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;
	uint8_t cmd[] = { SI468X_CMD_DAB_DIGRAD_STATUS,
			  ((uint8_t)digrad_ack << 3) | (uint8_t)stc_ack };
	struct spi_buf buf = { .buf = cmd, .len = sizeof(cmd) };
	struct spi_buf_set buf_set = { .buffers = &buf, .count = 1 };
	struct spi_buf_set *ans_buf_set = NULL;

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command get_freq_list failed with rc %d",
			dev->name, rc);
		return rc;
	}
	uint8_t ans[5];
	struct spi_buf ans_buf = { .buf = ans, .len = sizeof(ans) };
	struct spi_buf_set ans_buf_set = { .buffers = &ans_buf, .count = 1 };
	rc = si468x_cmd_rd_reply(dev, ans_buf_set,
				 status == NULL ? NULL : &ans_buf_set);
	if (rc != 0) {
		LOG_ERR("%s: waiting for CTS after getting freq list failed with rc %d",
			dev->name, rc);
		return rc;
	}
	if (status != NULL) {
		status->valid = DIGRAD_STATUS_VALID(buf);
		status->acq = DIGRAD_STATUS_ACQ(buf);
		status->ficerr = DIGRAD_STATUS_FICERR(buf);
		status->hardmute = DIGRAD_STATUS_HARDMUTE(buf);
		status->rssi = DIGRAD_STATUS_RSSI(buf);
		status->fic_quality = DIGRAD_STATUS_FIC_QUALITY(buf);
	}
	return rc;
}
