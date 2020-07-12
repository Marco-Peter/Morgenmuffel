/*
 * Copyright (c) 2020 Marco Peter
 */

#include "vs1053.h"
#include <zephyr/types.h>
#include <syscall_handler.h>
#include <drivers/spi.h>

struct vs1053_data {
	struct device *spi;
	struct device *reset;
	struct device *dreq;
	struct spi_cs_control cs_control;
	struct spi_cs_control dcs_control;
	struct spi_config cs_config;
	struct spi_config dcs_config;
};

static struct vs1053_data vs1053_data;

static int init(struct device *dev)
{
	struct vs1053_data *data = dev->driver_data;

	return 0;
}

DEVICE_INIT(vs1053, "VS1053", init, &vs1053_data, NULL,
			POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);
