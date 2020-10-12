#include "display.h"
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/display.h>
#include <lvgl.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(disp);

#define POWER_ON_PIN 7U
#define DISPLAY_THREAD_PRIO 2
#define DISPLAY_THREAD_STACK 256

static const struct device *display;
static const struct device *power_on;

int display_off(void)
{
	int retval;

	retval = display_blanking_on(display);
	return retval;
}

static void display_func(void)
{
	int retval;

	display = device_get_binding("DISPLAY");
	if (display == NULL) {
		LOG_ERR("Display device not found");
                return;
	}
	power_on = device_get_binding("GPIOD");
	if (power_on == NULL) {
		LOG_ERR("Power supply pin not found");
                return;
	}
	retval = gpio_pin_set(power_on, POWER_ON_PIN, 1);
	if (retval != 0) {
		LOG_ERR("Failed to set power supply pin");
                return;
	}

        for(;;) {
                k_sleep(K_MSEC(10));
                lv_task_handler();
        }
}

K_THREAD_DEFINE(display_thread, DISPLAY_THREAD_STACK, display_func, NULL, NULL,
		NULL, DISPLAY_THREAD_PRIO, 0, 100);
