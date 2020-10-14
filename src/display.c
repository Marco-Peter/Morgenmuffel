#include "display.h"
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/display.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(disp);

#define DISPLAY_THREAD_PRIO 2
#define DISPLAY_THREAD_STACK 2048
#define DISPLAY_COMMAND_QUEUE_SIZE 3

K_MSGQ_DEFINE(display_msgq, sizeof(display_command_t),
	      DISPLAY_COMMAND_QUEUE_SIZE, sizeof(display_command_t));
static const struct device *display;

void display_command(display_command_t cmd)
{
	int rc;

	LOG_INF("enter display_command");
	rc = k_msgq_put(&display_msgq, &cmd, K_SECONDS(2));
	if(rc != 0) {
		LOG_INF("display_command failed with status %d", rc);
	}
	LOG_INF("leave display_command");
}

void display_clear(void)
{
	LOG_INF("enter display_clear");
	lv_obj_clean(lv_scr_act());
	LOG_INF("leave display_clear");
}

void display_off(void)
{
	LOG_INF("enter display_off");
	display_blanking_on(display);
	LOG_INF("leave display_off");
}

static void display_func(void)
{
	display = device_get_binding(CONFIG_LVGL_DISPLAY_DEV_NAME);
	if (display == NULL) {
		LOG_ERR("Display device not found");
		return;
	}
	display_blanking_off(display);

	for (;;) {
		display_command_t cmd;

		while (k_msgq_get(&display_msgq, &cmd, K_NO_WAIT) == 0) {
			LOG_INF("new command");
			cmd();
			LOG_INF("command done");
		}
		lv_task_handler();
		k_sleep(K_MSEC(10));
	}
}

K_THREAD_DEFINE(display_thread, DISPLAY_THREAD_STACK, display_func, NULL, NULL,
		NULL, DISPLAY_THREAD_PRIO, 0, 100);
