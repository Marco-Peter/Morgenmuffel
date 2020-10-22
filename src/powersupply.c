#include "powersupply.h"
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(power, LOG_LEVEL_ERR);

/* TODO: Try to use DeviceTree for those settings */
#define POWER_1V8_Pin 7
#define POWER_1V8_GPIO_Port "GPIOF"
#define POWER_5V_Pin 5
#define POWER_5V_GPIO_Port "GPIOG"

static int start_5v(void);
static int stop_5v(void);
static int start_1v8(void);
static int stop_1v8(void);

const struct device *gpio_5v;
const struct device *gpio_1v8;
static uint32_t requests_5v;
static uint32_t requests_1v8;

int powersupply_request_5v(void)
{
	if (requests_5v == 0U) {
		int rc;
		rc = start_5v();
		if (rc != 0) {
			return rc;
		}
	}
	if (requests_5v != UINT32_MAX) {
		requests_5v++;
	}
	return 0;
}

int powersupply_release_5v(void)
{
	if (requests_5v == 1U) {
		int rc;
		rc = stop_5v();
		if (rc != 0) {
			return rc;
		}
	}
	if (requests_5v != 0U) {
		requests_5v--;
	}
	return 0;
}

int powersupply_request_1v8(void)
{
	if (requests_1v8 == 0U) {
		int rc;
		rc = start_1v8();
		if (rc != 0) {
			return rc;
		}
	}
	if (requests_1v8 != UINT32_MAX) {
		requests_1v8++;
	}
	return 0;
}

int powersupply_release_1v8(void)
{
	if (requests_1v8 == 1U) {
		int rc;
		rc = stop_1v8();
		if (rc != 0) {
			return rc;
		}
	}
	if (requests_1v8 != 0U) {
		requests_1v8--;
	}
	return 0;
}

static int start_5v(void)
{
	int rc;

	rc = gpio_pin_set(gpio_5v, POWER_5V_Pin, 1);
	k_sleep(K_MSEC(1));
	return rc;
}

static int stop_5v(void)
{
	int rc;
	rc = gpio_pin_set(gpio_5v, POWER_5V_Pin, 0);
	return rc;
}

static int start_1v8(void)
{
	int rc;
	rc = gpio_pin_set(gpio_1v8, POWER_1V8_Pin, 1);
	k_sleep(K_MSEC(1));
	return rc;
}

static int stop_1v8(void)
{
	int rc;
	rc = gpio_pin_set(gpio_1v8, POWER_1V8_Pin, 0);
	return rc;
}

static int init(const struct device *dev)
{
	int rc;

	ARG_UNUSED(dev);

	LOG_DBG("power: Configuring power pins");
	gpio_1v8 = device_get_binding(POWER_1V8_GPIO_Port);
	if (gpio_1v8 == NULL) {
		LOG_ERR("power: GPIO for 1.8V power not found");
		return -ENODEV;
	}
	rc = gpio_pin_configure(gpio_1v8, POWER_1V8_Pin,
				GPIO_OUTPUT | GPIO_PUSH_PULL);
	if (rc != 0) {
		LOG_ERR("power: Failed to configure 1.8V power pin");
		return rc;
	}
	gpio_5v = device_get_binding(POWER_5V_GPIO_Port);
	if (gpio_5v == NULL) {
		LOG_ERR("power: GPIO for 5V power not found");
		return -ENODEV;
	}
	rc = gpio_pin_configure(gpio_5v, POWER_5V_Pin,
				GPIO_OUTPUT | GPIO_PUSH_PULL);
	if (rc != 0) {
		LOG_ERR("power: Failed to configure 5V power pin");
	}
	return rc;
}

SYS_INIT(init, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);
