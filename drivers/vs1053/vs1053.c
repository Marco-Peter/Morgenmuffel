/*
 * Copyright (c) 2020 Marco Peter
 */

#include "vs1053.h"
#include <zephyr/types.h>
#include <syscall_handler.h>

static struct vs1053_data {
	uint32_t foo;
};

static int init(struct device *dev)
{

}
