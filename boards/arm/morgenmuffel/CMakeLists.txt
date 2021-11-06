# SPDX-License-Identifier: Apache-2.0

if(CONFIG_DISPLAY)
zephyr_library()
zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
zephyr_library_sources(display_powersupply.c)
endif()
