# Copyright (c) 2020 Marco Peter
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_MS5637)
zephyr_include_directories(.)
zephyr_library()
zephyr_library_sources(
        ms5637.c
)
endif()
