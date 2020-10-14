# SPDX-License-Identifier: Apache-2.0

if((CONFIG_PINMUX) OR (CONFIG_DISPLAY))
zephyr_library()
zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
endif()

if(CONFIG_PINMUX)
zephyr_library_sources(pinmux.c)
endif()
if(CONFIG_DISPLAY)
zephyr_library_sources(display_powersupply.c)
endif()
