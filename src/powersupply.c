#include "powersupply.h"
#include <zephyr.h>
#include <device.h>
#include <sys/onoff.h>
#include <drivers/gpio.h>

/* TODO: Try to use DeviceTree for those settings */
#define POWER_1V8_Pin 7
#define POWER_1V8_GPIO_Port "GPIOF"
#define POWER_5V_Pin 5
#define POWER_5V_GPIO_Port "GPIOG"

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

int powersupply_request_5v(void)
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

int powersupply_release_5v(void)
{
        return onoff_release(&onoff_5v);
}

int powersupply_request_1v8(void)
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

int powersupply_release_1v8(void)
{
        return onoff_release(&onoff_1v8);
}

static void start_5v(struct onoff_manager *mgr, onoff_notify_fn notify)
{
	int retval;
	const struct device *port = device_get_binding(POWER_5V_GPIO_Port);
	if (port == NULL) {
		retval = -EIO;
	} else {
        	retval = gpio_pin_set(port, POWER_5V_Pin, 1);
        }
        k_sleep(K_MSEC(10));
        notify(mgr, retval);
}

static void stop_5v(struct onoff_manager *mgr, onoff_notify_fn notify)
{
        int retval;
	const struct device *port = device_get_binding(POWER_5V_GPIO_Port);
	if (port == NULL) {
		retval = -EIO;
	} else {
        	retval = gpio_pin_set(port, POWER_5V_Pin, 0);
        }
        notify(mgr, retval);
}

static void start_1v8(struct onoff_manager *mgr, onoff_notify_fn notify)
{
        int retval;
	const struct device *port = device_get_binding(POWER_1V8_GPIO_Port);
	if (port == NULL) {
		retval = -EIO;
	} else {
        	retval = gpio_pin_set(port, POWER_1V8_Pin, 1);
        }
        k_sleep(K_MSEC(10));
        notify(mgr, retval);
}

static void stop_1v8(struct onoff_manager *mgr, onoff_notify_fn notify)
{
        int retval;
	const struct device *port = device_get_binding(POWER_1V8_GPIO_Port);
	if (port == NULL) {
		retval = -EIO;
	} else {
        	retval = gpio_pin_set(port, POWER_1V8_Pin, 0);
        }
        notify(mgr, retval);
}

static int init(const struct device *dev)
{
	int rc;
	const struct device *gpioPort;

        ARG_UNUSED(dev);

        gpioPort = device_get_binding(POWER_1V8_GPIO_Port);
        if(gpioPort == NULL) {
                return -ENODEV;
        }
	rc = gpio_pin_configure(gpioPort, POWER_1V8_Pin, GPIO_OUTPUT);
	if (rc != 0) {
		return rc;
	}
	gpioPort = device_get_binding(POWER_5V_GPIO_Port);
        if(gpioPort == NULL) {
                return -ENODEV;
        }
	rc = gpio_pin_configure(gpioPort, POWER_5V_Pin, GPIO_OUTPUT);
	return rc;
}

//SYS_INIT(init, APPLICATION, 99);
