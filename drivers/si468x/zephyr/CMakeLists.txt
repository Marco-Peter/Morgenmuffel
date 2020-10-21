# SPDX-License-Identifier: Apache-2.0

if(CONFIG_SI468X)
# Add si468x.h to the set of global include paths.
zephyr_include_directories(.)

zephyr_library()
zephyr_library_sources(
        si468x.c
        si468x_commands.c
)
endif()
