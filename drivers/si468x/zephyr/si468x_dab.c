/*
 * Copyright (c) 2020 Marco Peter
 */

#include "si468x_private.h"
#include "si468x_commands.h"
#include <string.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(si468x_dab, LOG_LEVEL_DBG);

// Digital service interrupt source
#define SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE 0x8100
// Data package available --> GET DIGITAL_SERVICE_DATA
#define SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE_DSRVPKTINT 0x0001
// Input buffer overflow --> reading digital service data too slow
#define SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE_DSRVOVRFLINT 0x0002

#define SI468X_PROP_DIGITAL_SERVICE_RESTART_DELAY 0x8101
#define SI468X_PROP_DAB_DIGRAD_INTERRUPT_SOURCE 0xB000

#define SI468X_PROP_DAB_DIGRAD_INT_SOURCE_HARDMUTEINT 0x0010
#define SI468X_PROP_DAB_DIGRAD_INT_SOURCE_FICERRINT 0x0008
#define SI468X_PROP_DAB_DIGRAD_INT_SOURCE_ACQINT 0x0004
#define SI468X_PROP_DAB_DIGRAD_INT_SOURCE_RSSIHINT 0x0002
#define SI468X_PROP_DAB_DIGRAD_INT_SOURCE_RSSILINT 0x0001

#define SI468X_PROP_DAB_DIGRAD_RSSI_HIGH_THRESHOLD 0xB001
#define SI468X_PROP_DAB_DIGRAD_RSSI_LOW_THRESHOLD 0xB002
#define SI468X_PROP_DAB_VALID_RSSI_TIME 0xB200
#define SI468X_PROP_DAB_VALID_RSSI_THRESHOLD 0xB201
#define SI468X_PROP_DAB_VALID_ACQ_TIME 0xB202
#define SI468X_PROP_DAB_VALID_SYNC_TIME 0xB203
#define SI468X_PROP_DAB_VALID_DETECT_TIME 0xB204

// Event status change interrupt sources --> DAB_GET_EVENT_STATUS
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE 0xB300
// Digital service list interrupt
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_SRVLIST_INTEN 0x0001
// Frequency information interrupt
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_FREQINFO_INTEN 0x0002
// Reconfiguration warning interrupt
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_RECFGWRN_INTEN 0x0040
// Reconfiguration interrupt
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_RECFG_INTEN 0x0080

#define SI468X_PROP_DAB_EVENT_MIN_SVRLIST_PERIOD 0xB301
#define SI468X_PROP_DAB_EVENT_MIN_SVRLIST_PERIOD_RECONFIG 0xB302
#define SI468X_PROP_DAB_EVENT_MIN_FREQINFO_PERIOD 0xB303
// Select which PAD data application will be forwarded to the host
#define SI468X_PROP_DAB_XPAD_ENABLE 0xB400
// Enables PAD delivered DLS packets
#define SI468X_PROP_DAB_XPAD_ENABLE_DLS_ENABLE 0x0001
// Enables PAD delivered MOT packets
#define SI468X_PROP_DAB_XPAD_ENABLE_MOT_ENABLE 0x0002
// Enables PAD delivered TDC packets
#define SI468X_PROP_DAB_XPAD_ENABLE_TDC_ENABLE 0x0004

// Dynamic range control setting
#define SI468X_PROP_DAB_DRC_OPTION 0xB401
// Disable DRC function (no gain)
#define SI468X_PROP_DAB_DRC_OPTION_NONE 0x0000
// Apply 1/2 gain from specified value on PAD data
#define SI468X_PROP_DAB_DRC_OPTION_HALF 0x0001
// Apply full gain from specified value on PAD data
#define SI468X_PROP_DAB_DRC_OPTION_FULL 0x0010

#define SI468X_PROP_DAB_CTRL_DAB_MUTE_ENABLE 0xB500
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGNAL_LEVEL_THRESHOLD 0xB501
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_WIN_THRESHOLD 0xB502
#define SI468X_PROP_DAB_CTRL_DAB_UNMUTE_WIN_THRESHOLD 0xB503
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGLOSS_THRESHOLD 0xB504
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGLOW_THRESHOLD 0xB505
#define SI468X_PROP_DAB_TEST_BER_CONFIG 0xE800

const wchar_t ebu_chars[] = {
	0,    L'Ę', L'Į', L'Ų',	 L'Ă', L'Ė', L'Ď', L'Ș', L'Ț', L'Ċ', L'?', L'?',
	L'Ġ', L'Ĺ', L'Ż', L'Ń',	 L'ą', L'ę', L'į', L'ų', L'ă', L'ė', L'ď', L'ș',
	L'ț', L'ċ', L'Ň', L'Ě',	 L'ġ', L'ĺ', L'ż', L'?', L' ', L'!', L'"', L'#',
	L'ł', L'%', L'&', L'\'', L'(', L')', L'*', L'+', L',', L'-', L'.', L'/',
	L'0', L'1', L'2', L'3',	 L'4', L'5', L'6', L'7', L'8', L'9', L':', L';',
	L'<', L'=', L'>', L'?',	 L'@', L'A', L'B', L'C', L'D', L'E', L'F', L'G',
	L'H', L'I', L'J', L'K',	 L'L', L'M', L'N', L'O', L'P', L'Q', L'R', L'S',
	L'T', L'U', L'V', L'W',	 L'X', L'Y', L'Z', L'[', L'Ů', L']', L'Ł', L'_',
	L'Ą', L'a', L'b', L'c',	 L'd', L'e', L'f', L'g', L'h', L'i', L'j', L'k',
	L'l', L'm', L'n', L'o',	 L'p', L'q', L'r', L's', L't', L'u', L'v', L'w',
	L'x', L'y', L'z', L'«',	 L'ů', L'»', L'Ľ', L'Ħ', L'á', L'à', L'é', L'è',
	L'í', L'ì', L'ó', L'ò',	 L'ú', L'ù', L'Ñ', L'Ç', L'Ş', L'ß', L'¡', L'Ÿ',
	L'â', L'ä', L'ê', L'ë',	 L'î', L'ï', L'ô', L'ö', L'û', L'ü', L'ñ', L'ç',
	L'ş', L'ğ', L'ı', L'ÿ',	 L'Ķ', L'Ņ', L'©', L'Ģ', L'Ğ', L'ě', L'ň', L'ő',
	L'Ő', L'€', L'£', L'$',	 L'Ā', L'Ē', L'Ī', L'Ū', L'ķ', L'ņ', L'Ļ', L'ģ',
	L'ļ', L'İ', L'ń', L'ű',	 L'Ű', L'¿', L'ľ', L'°', L'ā', L'ē', L'ī', L'ū',
	L'Á', L'À', L'É', L'È',	 L'Í', L'Ì', L'Ó', L'Ò', L'Ú', L'Ù', L'Ř', L'Č',
	L'Š', L'Ž', L'Ð', L'Ŀ',	 L'Â', L'Ä', L'Ê', L'Ë', L'Î', L'Ï', L'Ô', L'Ö',
	L'Û', L'Ü', L'ř', L'č',	 L'š', L'ž', L'đ', L'ŀ', L'Ã', L'Å', L'Æ', L'Œ',
	L'ŷ', L'Ý', L'Õ', L'Ø',	 L'Þ', L'Ŋ', L'Ŕ', L'Ć', L'Ś', L'Ź', L'Ť', L'ð',
	L'ã', L'å', L'æ', L'œ',	 L'ŵ', L'ý', L'õ', L'ø', L'þ', L'ŋ', L'ŕ', L'ć',
	L'ś', L'ź', L'ť', L'ħ',	 L'?'
};

static int add_service(const struct device *dev, uint16_t service_id,
		       uint8_t component_id, uint8_t channel)
{
	struct si468x_dab_service *service = NULL;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	for (int i = 0; i < CONFIG_SI468X_DAB_SERVICE_LIST_SIZE; i++) {
		if (data->services[i].id == service_id) {
			service = &data->services[i];
			break;
		} else if (data->services[i].id == 0) {
			service = &data->services[i];
		}
	}
	if (service != NULL) {
		service->id = service_id;
		service->primary_comp_id = component_id;
		service->channel = channel;
		return 0;
	} else {
		return -ENOMEM;
	}
}

static const struct si468x_dab_service *get_service(const struct device *dev,
						    uint16_t service_id)
{
	struct si468x_dab_service *service = NULL;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	for (int i = 0; i < CONFIG_SI468X_DAB_SERVICE_LIST_SIZE; i++) {
		if (data->services[i].id == service_id) {
			service = &data->services[i];
			break;
		}
	}
	return service;
}

static void remove_services_on_channel(const struct device *dev,
				       uint8_t channel)
{
	struct si468x_data *data = (struct si468x_data *)dev->data;

	for (int i = 0; i < CONFIG_SI468X_DAB_SERVICE_LIST_SIZE; ++i) {
		if (data->services[i].channel == channel) {
			data->services[i].id = 0;
			data->services[i].primary_comp_id = 0;
			data->services[i].channel = 0;
		}
	}
}

static int dab_decode_ebu_string(wchar_t *string, const uint8_t *ebu_str,
				 size_t len)
{
	int i;
	for (i = 0; i < len; i++) {
		if (ebu_str[i] == 0) {
			break;
		}
		string[i] = ebu_chars[ebu_str[i]];
	}
	memset(&string[i], 0, (len - i) * sizeof(wchar_t));
	return i;
}

static int dab_tune(const struct device *dev, uint8_t channel)
{
	int rc;

	rc = si468x_dab_process_events(dev, true);
	if (rc != 0) {
		LOG_ERR("failed to acknowledge pending interrupts with rc %d",
			rc);
	}
	rc = si468x_cmd_dab_tune(dev, channel, 0);
	if (rc != 0) {
		LOG_ERR("failed to tune on DAB with rc %d", rc);
		return rc;
	}
	return 0;
}

static int dab_wait_for_tune_complete(const struct device *dev)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;
	struct si468x_config *config = (struct si468x_config *)dev->config;
	struct si468x_events events = { 0 };

	if (gpio_pin_get(data->int_gpio, config->int_gpio_pin) == 1) {
		LOG_DBG("Interrupt pin already active on tuning");
	}

	do {
		rc = k_sem_take(&data->sem, K_SECONDS(10));
		if (rc == -EAGAIN) {
			LOG_ERR("waiting for tuning timed out. Pin state: %d",
				gpio_pin_get(data->int_gpio,
					     config->int_gpio_pin));
			return rc;
		} else if (rc != 0) {
			LOG_ERR("generic error on waiting for semaphore rc %d",
				rc);
			return rc;
		}
		rc = si468x_cmd_rd_reply(dev, NULL, &events);
		if (rc != 0) {
			LOG_ERR("failed to read pending interrupts with rc %d",
				rc);
		}
		return rc;
	} while (events.stcint == false);
	if (gpio_pin_get(data->int_gpio, config->int_gpio_pin) == 1) {
		LOG_DBG("other interrupts active");
		k_sem_give(&data->sem);
	}
	return 0;
}

static int dab_is_acquisition_valid(const struct device *dev)
{
	int rc;
	struct si468x_dab_digrad_status digrad_status;

	rc = si468x_cmd_dab_digrad_status(dev, true, true, &digrad_status);
	if (rc != 0) {
		LOG_ERR("failed to acknowledge STC and digrad interrupt with rc %d",
			rc);
		return rc;
	}
	return (int)digrad_status.valid;
}

static int dab_wait_for_service_list(const struct device *dev)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;
	struct si468x_config *config = (struct si468x_config *)dev->config;
	struct si468x_events events = { 0 };

	if (gpio_pin_get(data->int_gpio, config->int_gpio_pin) == 1) {
		LOG_DBG("Interrupt pin already active when waiting for service list");
	}

	do {
		rc = k_sem_take(&data->sem, K_SECONDS(10));
		if (rc != -EAGAIN) {
			LOG_ERR("waiting for service list timed out. Pin state: %d",
				gpio_pin_get(data->int_gpio,
					     config->int_gpio_pin));
			return rc;
		} else if (rc != 0) {
			LOG_ERR("generic error on waiting for semaphore rc %d",
				rc);
			return rc;
		}
		rc = si468x_cmd_rd_reply(dev, NULL, &events);
		if (rc != 0) {
			LOG_ERR("failed to read pending interrupts with rc %d",
				rc);
		}
		return rc;
	} while (events.devntint == false);
	if (gpio_pin_get(data->int_gpio, config->int_gpio_pin) == 1) {
		k_sem_give(&data->sem);
	}
	return 0;
}

static int dab_is_service_list_ready(const struct device *dev)
{
	int rc;
	struct si468x_dab_event_status event_status;

	rc = si468x_cmd_dab_get_event_status(dev, true, &event_status);
	if (rc != 0) {
		LOG_ERR("failed to acknowledge event status with rc %d", rc);
		return rc;
	}
	return (int)event_status.svr_list;
}

static int dab_read_svc_component(const struct device *dev, uint8_t channel,
				  uint16_t svc_id, uint8_t *buffer)
{
	uint16_t cmp_id = (uint16_t)*buffer;
	if ((cmp_id & 0xC000) != 0U) {
		LOG_INF("data component ignored");
		return 0;
	}
	buffer += 2;
	if ((*buffer & 0x02) == 0U) {
		int rc;

		rc = add_service(dev, svc_id, (uint8_t)(cmp_id & 0xFF),
				 channel);
		if (rc != 0) {
			LOG_ERR("no memory for service");
			return -ENOMEM;
		}
		LOG_DBG("added service: svc_id = %d, cmp_id = %d", svc_id,
			cmp_id);
	} else {
		LOG_INF("secondary service ignored");
	}
	return 0;
}

static int dab_read_service_list(const struct device *dev, uint8_t channel,
				 uint8_t *buffer)
{
	int rc;

	rc = si468x_cmd_get_digital_service_list(dev, buffer);
	if (rc < 0) {
		LOG_ERR("failed to read service list with rc %d", rc);
		return rc;
	}
	LOG_INF("service list version %d", (uint16_t)*buffer);
	buffer += 2;
	uint8_t num_of_svc = *buffer;
	buffer += 4;
	for (int svc_num = 0; svc_num < num_of_svc; svc_num++) {
		uint32_t svc_id = (uint32_t)*buffer;
		buffer += 4;
		bool is_data_svc = (*buffer & 0x01) == 1U;
		buffer++;
		uint8_t num_of_cmp = *buffer & 0x0F;
		buffer += 3 + 16;
		if (is_data_svc == true) {
			LOG_INF("data service ignored");
			buffer += num_of_cmp * 4U;
			continue;
		}
		for (int cmp_num = 0; cmp_num < num_of_cmp; cmp_num++) {
			rc = dab_read_svc_component(dev, channel,
						    (uint16_t)(svc_id & 0xFFFF),
						    buffer);
			if (rc != 0) {
				return rc;
			}
			buffer += 4U;
		}
	}
	return 0;
}

int si468x_dab_startup(const struct device *dev)
{
	int rc;

	rc = si468x_cmd_set_property(
		dev, SI468X_PROP_INT_CTL_ENABLE,
		SI468X_PROP_INT_CTL_ENABLE_DEVNTIEN |
			(IS_ENABLED(CONFIG_SI468X_CTSIEN)
			 << SI468X_PROP_INT_CTL_ENABLE_CTSIEN_OFFS) |
			SI468X_PROP_INT_CTL_ENABLE_DSRVIEN |
			SI468X_PROP_INT_CTL_ENABLE_STCIEN);
	if (rc != 0) {
		LOG_ERR("failed to set dab interrupts with rc %d", rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev, SI468X_PROP_TUNE_FRONTEND_VARM,
				     CONFIG_SI468X_VARACTOR_SLOPE_DAB);
	if (rc != 0) {
		LOG_ERR("failed to set dab varactor slope with rc %d", rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev, SI468X_PROP_TUNE_FRONTEND_VARB,
				     CONFIG_SI468X_VARACTOR_INTCP_DAB);
	if (rc != 0) {
		LOG_ERR("failed to set dab varactor intercept with rc %d", rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev, SI468X_PROP_TUNE_FRONTEND_CFG,
				     IS_ENABLED(SI468X_VHFSW_DAB));
	if (rc != 0) {
		LOG_ERR("failed to set dab frontend configuration with rc %d",
			rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev,
				     SI468X_PROP_DAB_DIGRAD_INTERRUPT_SOURCE,
				     SI468X_PROP_DAB_DIGRAD_INT_SOURCE_ACQINT);
	if (rc != 0) {
		LOG_ERR("failed to set dab digrad interrupt sources with rc %d",
			rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev, SI468X_PROP_DAB_VALID_RSSI_THRESHOLD,
				     CONFIG_SI468X_DAB_VALID_RSSI_THRESHOLD);
	if (rc != 0) {
		LOG_ERR("failed to set dab valid RSSI threshold with rc %d",
			rc);
		return rc;
	}
	rc = si468x_cmd_set_property(
		dev, SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE,
		SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_SRVLIST_INTEN);
	if (rc != 0) {
		LOG_ERR("failed to set dab event interrupt sources with rc %d",
			rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev, SI468X_PROP_DAB_DRC_OPTION,
				     SI468X_PROP_DAB_DRC_OPTION_FULL);
	if (rc != 0) {
		LOG_ERR("failed to set dab drc with rc %d", rc);
		return rc;
	}
	return 0;
}

int si468x_dab_play_service(const struct device *dev, uint16_t service_id)
{
	int rc;
	const struct si468x_dab_service *service = get_service(dev, service_id);

	rc = dab_tune(dev, service->channel);
	if (rc != 0) {
		return rc;
	}
	rc = dab_wait_for_tune_complete(dev);
	if (rc != 0) {
		LOG_ERR("failed waiting for tuning to complete with rc %d", rc);
		return rc;
	}
	rc = dab_is_acquisition_valid(dev);
	if (rc < 0) {
		LOG_ERR("checking tuning validity failed with rc %d", rc);
		return rc;
	} else if (rc == 0) {
		LOG_ERR("No valid station found on channel %d",
			service->channel);
		return -ENOLINK;
	}
	rc = si468x_cmd_dab_start_service(dev, service->id,
					  service->primary_comp_id);
	if (rc != 0) {
		LOG_ERR("failed starting the service with rc %d", rc);
		return rc;
	}
	return rc;
}

int si468x_dab_process_events(const struct device *dev, bool ack_only)
{
	int rc = 0;

	return rc;
}

int si468x_dab_bandscan(const struct device *dev, uint8_t *buffer)
{
	int rc;
	uint8_t number_of_freqs;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	rc = si468x_cmd_dab_get_freq_list(dev, &number_of_freqs);
	if (rc != 0) {
		LOG_ERR("Failed to get frequency list");
		return rc;
	}
	memset(data->services, 0, sizeof(data->services));
	for (int i = 0; i < number_of_freqs; i++) {
		LOG_DBG("Scanning channel %d", i);
		rc = dab_tune(dev, i);
		if (rc != 0) {
			LOG_ERR("Failed to scan channel %d", i);
			return rc;
		}
		rc = dab_wait_for_tune_complete(dev);
		if (rc != 0) {
			LOG_ERR("failed waiting for tuning to complete with rc %d",
				rc);
			return rc;
		}
		rc = dab_is_acquisition_valid(dev);
		if (rc < 0) {
			LOG_ERR("checking tuning validity failed with rc %d",
				rc);
			return rc;
		} else if (rc == 0) {
			continue;
		}
		rc = dab_wait_for_service_list(dev);
		if (rc != 0) {
			LOG_ERR("failed waiting for service list with rc %d",
				rc);
			return rc;
		}
		rc = dab_is_service_list_ready(dev);
		if (rc < 0) {
			LOG_ERR("checking tuning validity failed with rc %d",
				rc);
			return rc;
		} else if (rc == 0) {
			LOG_ERR("service list not ready on channel %d", i);
			continue;
		}
		rc = dab_read_service_list(dev, i, buffer);
		if (rc != 0) {
			LOG_ERR("failed reading service list with rc %d", rc);
			return rc;
		}
	}
	return 0;
}

uint16_t si468x_dab_get_num_of_services(const struct device *dev)
{
	struct si468x_data *data = (struct si468x_data *)dev->data;
	uint16_t count = 0;

	for (int i = 0; i < CONFIG_SI468X_DAB_SERVICE_LIST_SIZE; i++) {
		if (data->services[i].id != 0U) {
			count++;
		}
	}
	return count;
}

uint16_t si468x_dab_get_service_id(const struct device *dev, uint16_t index)
{
	struct si468x_data *data = (struct si468x_data *)dev->data;
	uint16_t count = 0;

	for (int i = 0; i < CONFIG_SI468X_DAB_SERVICE_LIST_SIZE; i++) {
		if (data->services[i].id != 0U) {
			if (count == index) {
				return data->services[i].id;
			}
			count++;
		}
	}
	return 0U;
}
