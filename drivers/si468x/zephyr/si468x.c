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
		LOG_ERR("%s: failed to bind the reset gpio driver", dev->name);
	}
	rc = gpio_pin_configure(data->reset_gpio, config->reset_gpio_pin,
				GPIO_OUTPUT_ACTIVE | config->reset_gpio_flags);
	if (rc != 0) {
		LOG_ERR("%s: failed to configure reset gpio with rc %d",
			dev->name, rc);
		return rc;
	}
	if (config->cs_gpio_label != NULL) {
		data->cs_gpio = device_get_binding(config->cs_gpio_label);
		if (data->cs_gpio == NULL) {
			LOG_ERR("%s: failed to bind the cs gpio driver",
				dev->name);
		}
		rc = gpio_pin_configure(data->cs_gpio, config->cs_gpio_pin,
					GPIO_OUTPUT | config->cs_gpio_flags);
		if (rc != 0) {
			LOG_ERR("%s: failed to configure chip select gpio with rc %d",
				dev->name, rc);
			return rc;
		}
	}
	data->int_gpio = device_get_binding(config->int_gpio_label);
	if (data->int_gpio == NULL) {
		LOG_ERR("%s: failed to bind the int gpio driver", dev->name);
	}
	rc = gpio_pin_configure(data->int_gpio, config->int_gpio_pin,
				GPIO_INPUT | config->int_gpio_flags);
	if (rc != 0) {
		LOG_ERR("%s: failed to configure gpio pin interrupt with rc %d",
			dev->name, rc);
		return rc;
	}
	gpio_init_callback(&data->gpio_callback, gpio_callback_handler,
			   config->int_gpio_pin);
	rc = gpio_add_callback(data->int_gpio, &data->gpio_callback);
	if (rc != 0) {
		LOG_ERR("%s: failed to add gpio interrupt callback with rc %d",
			dev->name, rc);
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
		LOG_ERR("%s: failed to release the reset line with rc %d",
			dev->name, rc);
		return rc;
	}
	k_sleep(K_MSEC(4));
	rc = si468x_cmd_rd_reply(dev, NULL, NULL);
	if (rc != 0) {
		LOG_ERR("%s: reading state after reset failed with rc %d",
			dev->name, rc);
		return rc;
	}
	if (rc != si468x_PUP_RESET) {
		LOG_ERR("%s: wrong chip state after reset: %d", dev->name, rc);
		return -EIO;
	}
	rc = si468x_cmd_powerup(dev);
	if (rc != 0) {
		LOG_ERR("%s: failed to send powerup command with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_load_init(dev);
	if (rc != 0) {
		LOG_ERR("%s: load init command failed with rc %d", dev->name,
			rc);
		return rc;
	}
	rc = si468x_cmd_host_load(dev, minipatch, MINIPATCH_LENGTH);
	if (rc != 0) {
		LOG_ERR("%s: loading mini patch failed with rc %d", dev->name,
			rc);
		return rc;
	}
	rc = si468x_cmd_load_init(dev);
	if (rc != 0) {
		LOG_ERR("%s: load init command after mini patch failed with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_flash_load(dev, CONFIG_SI468X_FLASH_START_PATCH);
	if (rc != 0) {
		LOG_ERR("%s: flash load command for patch failed with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_load_init(dev);
	if (rc != 0) {
		LOG_ERR("%s: load init command after fw patch failed with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_flash_load(dev, mode);
	if (rc != 0) {
		LOG_ERR("%s: flash load command for firmware failed with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_boot(dev);
	if (rc != 0) {
		LOG_ERR("%s: booting failed with rc %d", dev->name, rc);
		return rc;
	}

	enum si468x_image image;
	rc = si468x_cmd_get_sys_state(dev, &image);
	if (rc != 0) {
		LOG_ERR("%s: getting image id failed with rc %d", dev->name,
			rc);
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
		LOG_ERR("%s: loaded wrong firmware image %d for mode %d",
			dev->name, image, mode);
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
		LOG_ERR("%s: failed to configure interrupt pin with rc %d",
			dev->name, rc);
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
		LOG_ERR("%s: failed to disable interrupt pin with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = gpio_pin_set(data->reset_gpio, config->reset_gpio_pin, 1);
	if (rc != 0) {
		LOG_ERR("%s: failed to pull the reset line with rc %d",
			dev->name, rc);
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

static int process_events(const struct device *dev)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	switch (data->current_mode) {
#if IS_ENABLED(CONFIG_SI468X_AM)
	case si468x_MODE_AM:
		rc = si468x_am_process_events(dev);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_DAB)
	case si468x_MODE_DAB:
		rc = si468x_dab_process_events(dev);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_FMHD)
	case si468x_MODE_FMHD:
		rc = si468x_fmhd_process_events(dev);
		break;
#endif
	case si468x_MODE_OFF:
		rc = 0;
	}
	return rc;
}

static int bandscan(const struct device *dev, enum si468x_mode mode)
{
	int rc;
	struct si468x_data *data = (struct si468x_data *)dev->data;

	switch (mode) {
#if IS_ENABLED(CONFIG_SI468X_AM)
	case si468x_MODE_AM:
		rc = si468x_am_bandscan(dev);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_DAB)
	case si468x_MODE_DAB:
		rc = si468x_dab_bandscan(dev);
		break;
#endif
#if IS_ENABLED(CONFIG_SI468X_FMHD)
	case si468x_MODE_FMHD:
		rc = si468x_fmhd_bandscan(dev);
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
				       .get_semaphore = get_semaphore }));

DT_INST_FOREACH_STATUS_OKAY(SI468X_DEVICE)
