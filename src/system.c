#include "system.h"
#include <zephyr.h>
#include <device.h>
#include <sys/onoff.h>
#include <drivers/gpio.h>

static void start_5v(struct onoff_manager *mgr, onoff_notify_fn notify);
static void stop_5v(struct onoff_manager *mgr, onoff_notify_fn notify);
static void start_1v8(struct onoff_manager *mgr, onoff_notify_fn notify);
static void stop_1v8(struct onoff_manager *mgr, onoff_notify_fn notify);

static const struct onoff_transitions transitions_5v =
	ONOFF_TRANSITIONS_INITIALIZER(start_5v, stop_5v, NULL);
static const struct onoff_transitions transitions_1v8 =
	ONOFF_TRANSITIONS_INITIALIZER(start_1v8, stop_1v8, NULL);

static struct onoff_manager onoff_5v =
	ONOFF_MANAGER_INITIALIZER(&transitions_5v);
static struct onoff_manager onoff_1v8 =
	ONOFF_MANAGER_INITIALIZER(&transitions_1v8);

int system_init(void)
{
        return 0;
}

int system_request_5v(void)
{
        int retval;
	struct onoff_client client = {0};

        sys_notify_init_spinwait(&client.notify);

	retval = onoff_request(&onoff_5v, &client);
        if(retval != 0) {
                return retval;
        }
        while(sys_notify_fetch_result(&client.notify, &retval) != 0);
        retval &= ONOFF_STATE_MASK;
        if( retval & ONOFF_STATE_TO_ON) {
                return -EAGAIN;
        } else if( retval & ONOFF_STATE_ON) {
                return 0;
        } else {
                return -EFAULT;
        }
}

int system_release_5v(void)
{
        return onoff_release(&onoff_5v);
}

int system_request_1v8(void)
{
        int retval;
	struct onoff_client client = {0};

        sys_notify_init_spinwait(&client.notify);

	retval = onoff_request(&onoff_1v8, &client);
        if(retval != 0) {
                return retval;
        }
        while(sys_notify_fetch_result(&client.notify, &retval) != 0);
        retval &= ONOFF_STATE_MASK;
        if( retval & ONOFF_STATE_TO_ON) {
                return -EAGAIN;
        } else if( retval & ONOFF_STATE_ON) {
                return 0;
        } else {
                return -EFAULT;
        }
}

int system_release_1v8(void)
{
        return onoff_release(&onoff_1v8);
}

static void start_5v(struct onoff_manager *mgr, onoff_notify_fn notify)
{
	int retval;

	const struct device *portg = device_get_binding("PORTG");
	if (portg == NULL) {
		retval = -EIO;
	} else {
        	retval = gpio_pin_set(portg, 5, 1);
        }
        k_sleep(K_MSEC(10));
        notify(mgr, retval);
}

static void stop_5v(struct onoff_manager *mgr, onoff_notify_fn notify)
{
        int retval;

	const struct device *portg = device_get_binding("PORTG");
	if (portg == NULL) {
		retval = -EIO;
	} else {
        	retval = gpio_pin_set(portg, 5, 0);
        }
        notify(mgr, retval);
}

static void start_1v8(struct onoff_manager *mgr, onoff_notify_fn notify)
{
        int retval;

	const struct device *portg = device_get_binding("PORTF");
	if (portg == NULL) {
		retval = -EIO;
	} else {
        	retval = gpio_pin_set(portg, 7, 1);
        }
        k_sleep(K_MSEC(10));
        notify(mgr, retval);
}

static void stop_1v8(struct onoff_manager *mgr, onoff_notify_fn notify)
{
        int retval;

	const struct device *portg = device_get_binding("PORTF");
	if (portg == NULL) {
		retval = -EIO;
	} else {
        	retval = gpio_pin_set(portg, 7, 0);
        }
        notify(mgr, retval);
}
