/*
 * si468x_commands.c
 *
 * Implements the commands which are sent to the SI468x radio receiver
 *
 *  Created on: 11.02.2017
 *      Author: marco
 */

#include "si468x_commands.h"
#include "si468x_private.h"
#include <string.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(si468x_commands, LOG_LEVEL_DBG);

#define SI468X_CMD_POWER_UP 0x01
#define SI468X_CMD_HOST_LOAD 0x04
#define SI468X_CMD_FLASH_LOAD 0x05
#define SI468X_CMD_LOAD_INIT 0x06
#define SI468X_CMD_BOOT 0x07
#define SI468X_CMD_GET_PART_INFO 0x08
#define SI468X_CMD_GET_SYS_STATE 0x09
#define SI468X_CMD_GET_POWER_UP_ARGS 0x0A
#define SI468X_CMD_READ_OFFSET 0x10
#define SI468X_CMD_GET_FUNC_INFO 0x12
#define SI468X_CMD_SET_PROPERTY 0x13
#define SI468X_CMD_GET_PROPERTY 0x14
#define SI468X_CMD_WRITE_STORAGE 0x15
#define SI468X_CMD_READ_STORAGE 0x16

#define SI468X_SPI_MAX_FREQ 10000000
#define SI468X_HOST_LOAD_DATA_LIMIT 4096
#define SI468X_CTSIEN(_x) (_x << 7)

#define STATUS0_MASK_CTS 0x80
#define STATUS0_OFFS_CTS 7
#define STATUS0_CTS(_buf)                                                      \
	((((uint8_t *)(_buf))[1] & STATUS0_MASK_CTS) >> STATUS0_OFFS_CTS)

#define STATUS0_MASK_ERR_CMD 0x40
#define STATUS0_OFFS_ERR_CMD 6
#define STATUS0_ERR_CMD(_buf)                                                  \
	((((uint8_t *)(_buf))[1] & STATUS0_MASK_ERR_CMD) >>                    \
	 STATUS0_OFFS_ERR_CMD)

#define STATUS0_MASK_DACQINT 0x20
#define STATUS0_OFFS_DACQINT 5
#define STATUS0_DACQINT(_buf)                                                  \
	((((uint8_t *)(_buf))[1] & STATUS0_MASK_DACQINT) >>                    \
	 STATUS0_OFFS_DACQINT)

#define STATUS0_MASK_DSRVINT 0x10
#define STATUS0_OFFS_DSRVINT 4
#define STATUS0_DSRVINT(_buf)                                                  \
	((((uint8_t *)(_buf))[1] & STATUS0_MASK_DSRVINT) >>                    \
	 STATUS0_OFFS_DSRVINT)

#define STATUS0_MASK_STCINT 0x01
#define STATUS0_OFFS_STCINT 0
#define STATUS0_STCINT(_buf)                                                   \
	((((uint8_t *)(_buf))[1] & STATUS0_MASK_STCINT) >> STATUS0_OFFS_STCINT)

#define STATUS1_MASK_DEVNTINT 0x20
#define STATUS1_OFFS_DEVNTINT 5
#define STATUS1_DEVNTINT(_buf)                                                 \
	((((uint8_t *)(_buf))[2] & STATUS1_MASK_DEVNTINT) >>                   \
	 STATUS1_OFFS_DEVNTINT)

#define STATUS3_MASK_PUP_STATE 0xC0
#define STATUS3_OFFS_PUP_STATE 6
#define STATUS3_PUP_STATE(_buf)                                                \
	((((uint8_t *)(_buf))[4] & STATUS3_MASK_PUP_STATE) >>                  \
	 STATUS3_OFFS_PUP_STATE)

#define STATUS3_MASK_DSPERR 0x10
#define STATUS3_OFFS_DSPERR 4
#define STATUS3_DSPERR(_buf)                                                   \
	((((uint8_t *)(_buf))[4] & STATUS3_MASK_DSPERR) >> STATUS3_OFFS_DSPERR)

#define STATUS3_MASK_REPOFERR 0x08
#define STATUS3_OFFS_REPOFERR 3
#define STATUS3_REPOFERR(_buf)                                                 \
	((((uint8_t *)(_buf))[4] & STATUS3_MASK_REPOFERR) >>                   \
	 STATUS3_OFFS_REPOFERR)

#define STATUS3_MASK_CMDOFERR 0x04
#define STATUS3_OFFS_CMDOFERR 2
#define STATUS3_CMDOFERR(_buf)                                                 \
	((((uint8_t *)(_buf))[4] & STATUS3_MASK_CMDOFERR) >>                   \
	 STATUS3_OFFS_CMDOFERR)

#define STATUS3_MASK_ARBERR 0x02
#define STATUS3_OFFS_ARBERR 1
#define STATUS3_ARBERR(_buf)                                                   \
	((((uint8_t *)(_buf))[4] & STATUS3_MASK_ARBERR) >> STATUS3_OFFS_ARBERR)

#define STATUS3_MASK_ERRNR 0x01
#define STATUS3_OFFS_ERRNR 0
#define STATUS3_ERRNR(_buf)                                                    \
	((((uint8_t *)(_buf))[4] & STATUS3_MASK_ERRNR) >> STATUS3_OFFS_ERRNR)

#define STATUS_ERROR_CODE(_buf) (((uint8_t *)(_buf))[5])

#define ERR_CMD_UNSPECIFIED 0x01
#define ERR_CMD_REPLY_OVERFLOW 0x02
#define ERR_CMD_NOT_AVAILABLE 0x03
#define ERR_CMD_NOT_SUPPORTED 0x04
#define ERR_CMD_BAD_FREQUENCY 0x05
#define ERR_CMD_COMMAND_NOT_FOUND 0x10
#define ERR_CMD_BAD_ARG1 0x11
#define ERR_CMD_BAD_ARG2 0x12
#define ERR_CMD_BAD_ARG3 0x13
#define ERR_CMD_BAD_ARG4 0x14
#define ERR_CMD_BAD_ARG5 0x15
#define ERR_CMD_BAD_ARG6 0x16
#define ERR_CMD_BAD_ARG7 0x17
#define ERR_CMD_COMMAND_BUSY 0x18
#define ERR_CMD_AT_BAND_LIMIT 0x19
#define ERR_CMD_BAD_NVM 0x20
#define ERR_CMD_BAD_PATCH 0x30
#define ERR_CMD_BAD_BOOTMODE 0x31
#define ERR_CMD_BAD_PROPERTY 0x40
#define ERR_CMD_NOT_ACQUIRED 0x50
#define ERR_CMD_APP_NOT_SUPPORTED 0xFF

static int read_status(const struct device *dev,
		       const struct spi_buf_set *spi_buf_set,
		       struct si468x_events *events)
{
	struct si468x_data *data = dev->data;
	enum si468x_pup_state pup_state;

	if (STATUS0_ERR_CMD(spi_buf_set->buffers->buf)) {
		LOG_ERR("%s: received command error with code %d", dev->name,
			STATUS_ERROR_CODE(spi_buf_set->buffers->buf));
		return -EIO;
	}
	if (STATUS3_PUP_STATE(spi_buf_set->buffers->buf) != data->pup_state) {
		LOG_ERR("%s: wrong pup_state! %d instead of %d", dev->name,
			STATUS3_PUP_STATE(spi_buf_set->buffers->buf),
			data->pup_state);
		return -EIO;
	}
	if (STATUS0_CTS(spi_buf_set->buffers->buf) == false) {
		LOG_DBG("%s: not clear to send", dev->name);
	}
	if (events != NULL) {
		events->dacqint = STATUS0_DACQINT(spi_buf_set->buffers->buf);
		events->dsrvint = STATUS0_DSRVINT(spi_buf_set->buffers->buf);
		events->stcint = STATUS0_STCINT(spi_buf_set->buffers->buf);
		events->devntint = STATUS1_DEVNTINT(spi_buf_set->buffers->buf);
	}
	return pup_state;
}

int si468x_send_command(const struct device *dev,
			const struct spi_buf_set *spi_buf_set)
{
	int rc;

	struct si468x_data *data = dev->data;
	const struct si468x_config *config = dev->config;
	struct spi_cs_control spi_cs_control = {
		.gpio_dev = data->cs_gpio,
		.delay = 0,
		.gpio_pin = config->cs_gpio_pin,
		.gpio_dt_flags = config->cs_gpio_flags
	};
	struct spi_config spi_config = {
		.frequency = SI468X_SPI_MAX_FREQ,
		.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB |
			     SPI_WORD_SET(8) | SPI_LINES_SINGLE,
		.slave = config->spi_slave_number,
		.cs = data->cs_gpio != NULL ? &spi_cs_control : NULL
	};
	rc = spi_write(data->spi, &spi_config, spi_buf_set);
	return rc;
}

int si468x_cmd_rd_reply(const struct device *dev,
			const struct spi_buf_set *spi_buf_set,
			struct si468x_events *events)
{
	int rc;
	size_t count;
	uint8_t status[5];
	uint8_t errcmd;

	struct si468x_data *data = dev->data;
	const struct si468x_config *config = dev->config;
	struct spi_cs_control spi_cs_control = {
		.gpio_dev = data->cs_gpio,
		.delay = 0,
		.gpio_pin = config->cs_gpio_pin,
		.gpio_dt_flags = config->cs_gpio_flags
	};
	struct spi_config spi_config = {
		.frequency = SI468X_SPI_MAX_FREQ,
		.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB |
			     SPI_WORD_SET(8) | SPI_LINES_SINGLE,
		.slave = config->spi_slave_number,
		.cs = data->cs_gpio != NULL ? &spi_cs_control : NULL
	};

	if (spi_buf_set != NULL) {
		count = spi_buf_set->count + 1;
	} else {
		count = 2;
	}
	struct spi_buf buf[count];
	buf[0].buf = status;
	buf[0].len = sizeof(status);
	if (spi_buf_set != NULL) {
		memcpy(&buf[1], spi_buf_set->buffers,
		       spi_buf_set->count * sizeof(struct spi_buf));
	} else {
		buf[1].buf = &errcmd;
		buf[1].len = sizeof(errcmd);
	}
	struct spi_buf_set buf_set = { .buffers = buf, .count = count };

	rc = spi_read(data->spi, &spi_config, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: failed to read response with rc %d", dev->name,
			rc);
		return rc;
	}

	rc = read_status(dev, &buf_set, events);
	if (rc < 0) {
		LOG_ERR("%s: reading status failed with rc %d", dev->name, rc);
		return rc;
	}

	return rc;
}

int si468x_cmd_powerup(const struct device *dev)
{
	int rc;
	struct si468x_data *data = dev->data;

	uint8_t cmd[] = { SI468X_CMD_POWER_UP,
			  SI468X_CTSIEN(IS_ENABLED(CONFIG_SI468X_CTSIEN)),
			  (CONFIG_SI468X_CLK_MODE << 4) | CONFIG_SI468X_TR_SIZE,
			  CONFIG_SI468X_IBIAS,
			  CONFIG_SI468X_XTALFREQ & 0xFF,
			  (CONFIG_SI468X_XTALFREQ >> 8) & 0xFF,
			  (CONFIG_SI468X_XTALFREQ >> 16) & 0xFF,
			  (CONFIG_SI468X_XTALFREQ >> 24) & 0xFF,
			  CONFIG_SI468X_CTUN,
			  0x10,
			  0,
			  0,
			  0,
			  CONFIG_SI468X_IBIASRUN,
			  0,
			  0 };
	struct spi_buf buf = { .buf = cmd, .len = sizeof(cmd) };
	struct spi_buf_set buf_set = { .buffers = &buf, .count = 1 };

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command powerup failed with rc %d",
			dev->name, rc);
		return rc;
	}
	k_sleep(K_MSEC(2));
	data->pup_state = si468x_PUP_BOOTLOADER;
	rc = si468x_cmd_rd_reply(dev, NULL, NULL);
	if (rc < 0) {
		LOG_ERR("%s: reading state after power up command failed with rc %d",
			dev->name, rc);
		return rc;
	}
	return 0;
}

int si468x_cmd_load_init(const struct device *dev)
{
	int rc;
	struct si468x_data *data = dev->data;
	uint8_t cmd[] = { SI468X_CMD_LOAD_INIT, 0 };
	struct spi_buf buf = { .buf = cmd, .len = sizeof(cmd) };
	struct spi_buf_set buf_set = { .buffers = &buf, .count = 1 };

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command load_init failed with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_rd_reply(dev, NULL, NULL);
	if (rc < 0) {
		LOG_ERR("%s: waiting for CTS after load_init failed with rc %d",
			dev->name, rc);
		return rc;
	}
	return 0;
}

int si468x_cmd_host_load(const struct device *dev, const uint8_t *buffer,
			 uint16_t len)
{
	int rc;
	struct si468x_data *data = dev->data;

	if (len > SI468X_HOST_LOAD_DATA_LIMIT) {
		LOG_ERR("%s: Too much data for host load command", dev->name);
		return -EINVAL;
	}

	uint8_t cmd[] = { SI468X_CMD_HOST_LOAD, 0, 0, 0 };
	struct spi_buf buf[] = { { .buf = cmd, .len = sizeof(cmd) },
				 { .buf = (void *)buffer, .len = len } };
	struct spi_buf_set buf_set = { .buffers = buf, .count = 2 };

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command host_load failed with rc %d",
			dev->name, rc);
		return rc;
	}
	k_sleep(K_MSEC(4));
	rc = si468x_cmd_rd_reply(dev, NULL, NULL);
	if (rc < 0) {
		LOG_ERR("%s: waiting for CTS after host load failed with rc %d",
			dev->name, rc);
		return rc;
	}
	return 0;
}

int si468x_cmd_flash_load(const struct device *dev, uint32_t start_addr)
{
	int rc;
	struct si468x_data *data = dev->data;
	uint8_t cmd[] = { SI468X_CMD_FLASH_LOAD,
			  0,
			  0,
			  0,
			  start_addr & 0xFF,
			  (start_addr >> 8) & 0xFF,
			  (start_addr >> 16) & 0xFF,
			  (start_addr >> 24) & 0xFF,
			  0,
			  0,
			  0,
			  0 };
	struct spi_buf buf = { .buf = cmd, .len = sizeof(cmd) };
	struct spi_buf_set buf_set = { .buffers = &buf, .count = 1 };

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command flash_load failed with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_rd_reply(dev, NULL, NULL);
	if (rc < 0) {
		LOG_ERR("%s: waiting for CTS after flash load failed with rc %d",
			dev->name, rc);
		return rc;
	}
	return 0;
}

int si468x_cmd_boot(const struct device *dev)
{
	int rc;
	struct si468x_data *data = dev->data;
	uint8_t cmd[] = { SI468X_CMD_BOOT, 0 };
	struct spi_buf buf = { .buf = cmd, .len = sizeof(cmd) };
	struct spi_buf_set buf_set = { .buffers = &buf, .count = 1 };

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command boot failed with rc %d", dev->name,
			rc);
		return rc;
	}
	data->pup_state = si468x_PUP_APPLICATION;
	rc = si468x_cmd_rd_reply(dev, NULL, NULL);
	if (rc < 0) {
		LOG_ERR("%s: waiting for CTS after booting failed with rc %d",
			dev->name, rc);
		return rc;
	}
	return 0;
}

int si468x_cmd_get_sys_state(const struct device *dev, enum si468x_image *image)
{
	int rc;
	struct si468x_data *data = dev->data;
	uint8_t cmd[] = { SI468X_CMD_GET_SYS_STATE, 0 };
	struct spi_buf buf = { .buf = cmd, .len = sizeof(cmd) };
	struct spi_buf_set buf_set = { .buffers = &buf, .count = 1 };

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command boot failed with rc %d", dev->name,
			rc);
		return rc;
	}
	uint8_t ans;
	struct spi_buf ans_buf = { .buf = &ans, .len = sizeof(ans) };
	struct spi_buf_set ans_buf_set = { .buffers = &ans_buf, .count = 1 };
	rc = si468x_cmd_rd_reply(dev, &ans_buf_set, NULL);
	if (rc < 0) {
		LOG_ERR("%s: waiting for CTS after getting system state failed with rc %d",
			dev->name, rc);
		return rc;
	}
	*image = (enum si468x_image)ans;
	return 0;
}

int si468x_cmd_set_property(const struct device *dev, uint16_t id, uint16_t val)
{
	int rc;
	struct si468x_data *data = dev->data;
	uint8_t cmd[] = {
		SI468X_CMD_SET_PROPERTY, 0,	     id & 0xFF,
		(id >> 8) & 0xFF,	 val & 0xFF, (val >> 8) & 0xFF
	};
	struct spi_buf buf = { .buf = cmd, .len = sizeof(cmd) };
	struct spi_buf_set buf_set = { .buffers = &buf, .count = 1 };

	rc = si468x_send_command(dev, &buf_set);
	if (rc != 0) {
		LOG_ERR("%s: sending command set property failed with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_rd_reply(dev, NULL, NULL);
	if (rc < 0) {
		LOG_ERR("%s: waiting for CTS after set property failed with rc %d",
			dev->name, rc);
		return rc;
	}
	return 0;
}
