#include "usb_device_handler.h"
#include <device.h>
#include <zephyr.h>
#include <drivers/uart.h>
#include <usb/usb_device.h>
#include <sys/ring_buffer.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(usb_device_handler, LOG_LEVEL_WRN);

RING_BUF_DECLARE(ringbuf_to_esp, 4096);
RING_BUF_DECLARE(ringbuf_to_host, 32);

static const struct device *uart_esp;
static const struct device *usb_cdc_esp;

static void usb_cdc_handler(const struct device *dev, void *user_data);
static void uart_esp_handler(const struct device *dev, void *user_data);

int usb_devicehandler_init(void)
{
	int rc;

	uart_esp = device_get_binding("UART_2");
	usb_cdc_esp = device_get_binding("CDC_ACM_0");
	if (uart_esp == NULL || usb_cdc_esp == NULL) {
		LOG_ERR("Failed to get devices");
		return -ENODEV;
	}
	rc = usb_enable(NULL);
	if (rc != 0) {
		LOG_ERR("Failed to enable USB with rc %d", rc);
	}

	uart_irq_callback_user_data_set(usb_cdc_esp, usb_cdc_handler, NULL);
	uart_irq_callback_user_data_set(uart_esp, uart_esp_handler, NULL);
	while (true) {
		uint32_t dtr = 0U;
		uart_line_ctrl_get(usb_cdc_esp, UART_LINE_CTRL_DTR, &dtr);
		if (dtr) {
			break;
		} else {
			/* Give CPU resources to low priority threads. */
			k_sleep(K_MSEC(100));
		}
	}

	LOG_INF("DTR set");
	rc = uart_line_ctrl_set(usb_cdc_esp, UART_LINE_CTRL_DCD, 1);
	if (rc != 0) {
		LOG_ERR("Failed to set DCD with rc %d", rc);
	}
	rc = uart_line_ctrl_set(usb_cdc_esp, UART_LINE_CTRL_DSR, 1);
	if (rc != 0) {
		LOG_ERR("Failed to set DSR with rc %d", rc);
	}
	uart_irq_rx_enable(uart_esp);
	uart_irq_rx_enable(usb_cdc_esp);
	return rc;
}

static int copy_to_buffer(const struct device *dev, struct ring_buf *ringbuf,
			  uint32_t available)
{
	int rc;
	uint32_t ready;
	int used = 0;
	int copied_total = 0;
	uint8_t *data;

	do {
		available -= used;
		ready = ring_buf_put_claim(ringbuf, &data, available);
		if (ready == 0U) {
			LOG_ERR("%s: Ringbuffer is full", dev->name);
		}
		used = uart_fifo_read(dev, data, ready);
		if (used == 0) {
			LOG_WRN("%s: No data in FIFO", dev->name);
		}
		rc = ring_buf_put_finish(ringbuf, used);
		if (rc != 0) {
			LOG_ERR("%s: invalid argument on ring_buf_put_finish!",
				dev->name);
		}
		copied_total += used;
	} while ((used == ready) && (ready < available));
	return copied_total;
}

static void uart_handler(const struct device *dev, const struct device *other,
			 struct ring_buf *out_buf, struct ring_buf *in_buf)
{
	while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {
		if (uart_irq_rx_ready(dev)) {
			uint32_t available;
			int used;

			available = ring_buf_space_get(out_buf);
			used = copy_to_buffer(dev, out_buf, available);
			LOG_DBG("received %d bytes from %s", used, dev->name);
			uart_irq_tx_enable(other);
		}

		if (uart_irq_tx_ready(dev)) {
			uint32_t available;
			int used;
			int rc;
			uint8_t *data;

			available =
				ring_buf_get_claim(in_buf, &data, UINT32_MAX);
			if (available == 0U) {
				uart_irq_tx_disable(dev);
				used = 0;
			} else {
				used = uart_fifo_fill(dev, data, available);
			}
			rc = ring_buf_get_finish(in_buf, used);
			if (rc != 0) {
				LOG_ERR("invalid argument on ring_buf_get_finish!");
			}
			LOG_DBG("sent %d bytes to %s", used, dev->name);
		}
	}
}

static void usb_cdc_handler(const struct device *dev, void *user_data)
{
	ARG_UNUSED(user_data);

	uart_handler(dev, uart_esp, &ringbuf_to_esp, &ringbuf_to_host);
}

static void uart_esp_handler(const struct device *dev, void *user_data)
{
	ARG_UNUSED(user_data);

	uart_handler(dev, usb_cdc_esp, &ringbuf_to_host, &ringbuf_to_esp);
}
