/*
 * Copyright (c) 2020 Marco Peter
 */

#define DT_DRV_COMPAT silabs_si468x

#include "si468x_private.h"
#include <drivers/spi.h>
#include <logging/log.h>
#include "si468x_commands.h"
#include "si46xx_rom00_mini_patch.h"

LOG_MODULE_REGISTER(si468x, LOG_LEVEL_DBG);

static int init(const struct device *dev);
static int startup(const struct device *dev, enum si468x_mode mode);
static int powerdown(const struct device *dev);

static void gpio_callback_handler(const struct device *dev,
				  struct gpio_callback *cb,
				  gpio_port_pins_t pins)
{
	struct si468x_data *data =
		CONTAINER_OF(cb, struct si468x_data, gpio_callback);

	k_sem_give(&data->sem);
}

static int init(const struct device *dev)
{
	struct si468x_data *data = dev->data;
	const struct si468x_config *config = dev->config;
	int rc;

	data->spi = device_get_binding(config->spi_bus_label);

	data->reset_gpio = device_get_binding(config->reset_gpio_label);
	if (data->reset_gpio == NULL) {
		LOG_ERR("failed to bind the reset gpio driver");
	}
	rc = gpio_pin_configure(data->reset_gpio, config->reset_gpio_pin,
				GPIO_OUTPUT_ACTIVE | config->reset_gpio_flags);
	if (rc != 0) {
		LOG_ERR("failed to configure reset gpio with rc %d", rc);
		return rc;
	}
	if (config->cs_gpio_label != NULL) {
		data->cs_gpio = device_get_binding(config->cs_gpio_label);
		if (data->cs_gpio == NULL) {
			LOG_ERR("failed to bind the cs gpio driver");
		}
		rc = gpio_pin_configure(data->cs_gpio, config->cs_gpio_pin,
					GPIO_OUTPUT | config->cs_gpio_flags);
		if (rc != 0) {
			LOG_ERR("failed to configure chip select gpio with rc %d",
				rc);
			return rc;
		}
	}
	data->int_gpio = device_get_binding(config->int_gpio_label);
	if (data->int_gpio == NULL) {
		LOG_ERR("failed to bind the int gpio driver");
	}
	rc = gpio_pin_configure(data->int_gpio, config->int_gpio_pin,
				GPIO_INPUT | config->int_gpio_flags);
	if (rc != 0) {
		LOG_ERR("failed to configure gpio pin interrupt with rc %d",
			rc);
		return rc;
	}
	gpio_init_callback(&data->gpio_callback, gpio_callback_handler,
			   BIT(config->int_gpio_pin));
	rc = gpio_add_callback(data->int_gpio, &data->gpio_callback);
	if (rc != 0) {
		LOG_ERR("failed to add gpio interrupt callback with rc %d", rc);
		return rc;
	}
	rc = k_sem_init(&data->sem, 0, 1);
	return rc;
}

static int startup(const struct device *dev, enum si468x_mode mode)
{
	struct si468x_data *data = (struct si468x_data *)dev->data;
	const struct si468x_config *config =
		(struct si468x_config *)dev->config;
	int rc;

	rc = powerdown(dev);
	if (rc != 0) {
		return rc;
	}
	k_sleep(K_MSEC(1));
	rc = gpio_pin_set(data->reset_gpio, config->reset_gpio_pin, 0);
	if (rc != 0) {
		LOG_ERR("failed to release the reset line with rc %d", rc);
		return rc;
	}
	k_sleep(K_MSEC(4));
	rc = si468x_cmd_rd_reply(dev, NULL, NULL);
	if (rc != 0) {
		LOG_ERR("reading state after reset failed with rc %d", rc);
		return rc;
	}
	if (rc != si468x_PUP_RESET) {
		LOG_ERR("wrong chip state after reset: %d", rc);
		return -EIO;
	}
	rc = si468x_cmd_powerup(dev);
	if (rc != 0) {
		LOG_ERR("failed to send powerup command with rc %d", rc);
		return rc;
	}
	rc = si468x_cmd_load_init(dev);
	if (rc != 0) {
		LOG_ERR("load init command failed with rc %d", rc);
		return rc;
	}
	rc = si468x_cmd_host_load(dev, minipatch, MINIPATCH_LENGTH);
	if (rc != 0) {
		LOG_ERR("loading mini patch failed with rc %d", rc);
		return rc;
	}
	rc = si468x_cmd_load_init(dev);
	if (rc != 0) {
		LOG_ERR("load init command after mini patch failed with rc %d",
			rc);
		return rc;
	}
	rc = si468x_cmd_flash_load(dev, CONFIG_SI468X_FLASH_START_PATCH);
	if (rc != 0) {
		LOG_ERR("flash load command for patch failed with rc %d", rc);
		return rc;
	}
	rc = si468x_cmd_load_init(dev);
	if (rc != 0) {
		LOG_ERR("load init command after fw patch failed with rc %d",
			rc);
		return rc;
	}
	rc = si468x_cmd_flash_load(dev, mode);
	if (rc != 0) {
		LOG_ERR("flash load command for firmware failed with rc %d",
			rc);
		return rc;
	}
	rc = si468x_cmd_boot(dev);
	if (rc != 0) {
		LOG_ERR("booting failed with rc %d", rc);
		return rc;
	}

	enum si468x_image image;
	rc = si468x_cmd_get_sys_state(dev, &image);
	if (rc != 0) {
		LOG_ERR("getting image id failed with rc %d", rc);
		return rc;
	}
	if (!(
#if IS_ENABLED(CONFIG_SI468X_AM)
		    ((mode == si468x_MODE_AM) && (image == si468x_IMG_AMHD)) ||
#endif
#if IS_ENABLED(CONFIG_SI468X_DAB)
		    ((mode == si468x_MODE_DAB) && (image == si468x_IMG_DAB)) ||
#endif
#if IS_ENABLED(CONFIG_SI468X_FMHD)
		    ((mode == si468x_MODE_FMHD) && (image == si468x_IMG_FMHD)) ||
#endif
		    false)) {
		LOG_ERR("loaded wrong firmware image %d for mode %d", image,
			mode);
		return -EIO;
	}
	switch (mode) {
#if IS_ENABLED(CONFIG_SI468X_AM)
	case si468x_MODE_AM:
		rc = si468x_am_startup(dev);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_DAB)
	case si468x_MODE_DAB:
		rc = si468x_dab_startup(dev);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_FMHD)
	case si468x_MODE_FMHD:
		rc = si468x_fmhd_startup(dev);
		break;
#endif
	default:
		rc = -ENOTSUP;
	}
	if (rc == 0) {
		data->current_mode = mode;
	}
	rc = gpio_pin_interrupt_configure(data->int_gpio, config->int_gpio_pin,
					  GPIO_INT_EDGE_TO_ACTIVE);
	if (rc != 0) {
		LOG_ERR("failed to configure interrupt pin with rc %d", rc);
		return rc;
	}
	return rc;
}

static int powerdown(const struct device *dev)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;
	const struct si468x_config *config =
		(struct si468x_config *)dev->config;

	rc = gpio_pin_interrupt_configure(data->int_gpio, config->int_gpio_pin,
					  GPIO_INT_DISABLE);
	if (rc != 0) {
		LOG_ERR("failed to disable interrupt pin with rc %d", rc);
		return rc;
	}
	rc = gpio_pin_set(data->reset_gpio, config->reset_gpio_pin, 1);
	if (rc != 0) {
		LOG_ERR("failed to pull the reset line with rc %d", rc);
		return rc;
	}
	data->pup_state = si468x_PUP_RESET;
	data->current_mode = si468x_MODE_OFF;
	return rc;
}

static int play_service(const struct device *dev, enum si468x_mode mode,
			uint16_t service)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	if (mode != data->current_mode) {
		rc = startup(dev, mode);
		if (rc != 0) {
			return rc;
		}
	}
	switch (mode) {
#if IS_ENABLED(CONFIG_SI468X_AM)
	case si468x_MODE_AM:
		rc = si468x_am_play_service(dev, service);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_DAB)
	case si468x_MODE_DAB:
		rc = si468x_dab_play_service(dev, service);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_FMHD)
	case si468x_MODE_FMHD:
		rc = si468x_fmhd_play_service(dev, service);
		break;
#endif
	default:
		return -ENOTSUP;
	}
	return rc;
}

static int process_events(const struct device *dev, bool ack_only)
{
	int rc = 0;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	switch (data->current_mode) {
#if IS_ENABLED(CONFIG_SI468X_AM)
	case si468x_MODE_AM:
		rc = si468x_am_process_events(dev, ack_only);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_DAB)
	case si468x_MODE_DAB:
		rc = si468x_dab_process_events(dev, ack_only);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_FMHD)
	case si468x_MODE_FMHD:
		rc = si468x_fmhd_process_events(dev, ack_only);
		break;
#endif
	case si468x_MODE_OFF:
		rc = 0;
		break;
	}
	return rc;
}

static int bandscan(const struct device *dev, enum si468x_mode mode,
		    uint8_t *buffer)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	if (mode != data->current_mode) {
		rc = startup(dev, mode);
		if (rc != 0) {
			return rc;
		}
	}
	switch (mode) {
#if IS_ENABLED(CONFIG_SI468X_AM)
	case si468x_MODE_AM:
		rc = si468x_am_bandscan(dev, buffer);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_DAB)
	case si468x_MODE_DAB:
		rc = si468x_dab_bandscan(dev, buffer);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_FMHD)
	case si468x_MODE_FMHD:
		rc = si468x_fmhd_bandscan(dev, buffer);
		break;
#endif
	default:
		rc = -ENOTSUP;
	}
	return rc;
}

static struct k_sem *get_semaphore(const struct device *dev)
{
	struct si468x_data *data = (struct si468x_data *)dev->data;

	return &data->sem;
}

static uint16_t get_num_of_services(const struct device *dev)
{
	uint16_t num_of_services;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	switch (data->current_mode) {
#if IS_ENABLED(CONFIG_SI468X_AM)
	case si468x_MODE_AM:
		num_of_services = si468x_am_get_num_of_services(dev);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_DAB)
	case si468x_MODE_DAB:
		num_of_services = si468x_dab_get_num_of_services(dev);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_FMHD)
	case si468x_MODE_FMHD:
		num_of_services = si468x_fmhd_get_num_of_services(dev);
		break;
#endif
	default:
		num_of_services = 0U;
	}
	return num_of_services;
}

static uint16_t get_service_id(const struct device *dev, uint16_t index)
{
	uint16_t service_id;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	switch (data->current_mode) {
#if IS_ENABLED(CONFIG_SI468X_AM)
	case si468x_MODE_AM:
		service_id = si468x_am_get_service_id(dev, index);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_DAB)
	case si468x_MODE_DAB:
		service_id = si468x_dab_get_service_id(dev, index);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_FMHD)
	case si468x_MODE_FMHD:
		service_id = si468x_fmhd_get_service_id(dev, index);
		break;
#endif
	default:
		service_id = 0U;
	}
	return service_id;
}

#define SI468X_DEVICE(id)                                                      \
	static struct si468x_config si468x_config_##id = {                     \
		.int_gpio_flags = DT_INST_GPIO_FLAGS(id, int_gpios),           \
		.reset_gpio_flags = DT_INST_GPIO_FLAGS(id, reset_gpios),       \
		.cs_gpio_flags = DT_INST_SPI_DEV_CS_GPIOS_FLAGS(id),           \
		.spi_slave_number = DT_INST_REG_ADDR(id),                      \
		.spi_bus_label = DT_INST_BUS_LABEL(id),                        \
		.int_gpio_label = DT_INST_GPIO_LABEL(id, int_gpios),           \
		.reset_gpio_label = DT_INST_GPIO_LABEL(id, reset_gpios),       \
		.cs_gpio_label = DT_INST_SPI_DEV_CS_GPIOS_LABEL(id),           \
		.int_gpio_pin = DT_INST_GPIO_PIN(id, int_gpios),               \
		.reset_gpio_pin = DT_INST_GPIO_PIN(id, reset_gpios),           \
		.cs_gpio_pin = DT_INST_SPI_DEV_CS_GPIOS_PIN(id)                \
	};                                                                     \
	static struct si468x_data si468x_data_##id;                            \
                                                                               \
	DEVICE_AND_API_INIT(                                                   \
		si468x_##id, DT_INST_LABEL(id), init, &si468x_data_##id,       \
		&si468x_config_##id, POST_KERNEL,                              \
		CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                            \
		&((struct si468x_api){ .powerdown = powerdown,                 \
				       .play_service = play_service,           \
				       .process_events = process_events,       \
				       .bandscan = bandscan,                   \
				       .get_semaphore = get_semaphore,         \
				       .get_num_of_services =                  \
					       get_num_of_services,            \
				       .get_service_id = get_service_id }));

DT_INST_FOREACH_STATUS_OKAY(SI468X_DEVICE)
