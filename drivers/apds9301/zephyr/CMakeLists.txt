# Copyright (c) 2020 Marco Peter
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_APDS9301)
zephyr_library()
zephyr_library_sources(
        apds9301.c
)
endif()
