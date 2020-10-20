# SPDX-License-Identifier: Apache-2.0

if(CONFIG_SI4689)
# Add si4689.h to the set of global include paths.
zephyr_include_directories(.)

zephyr_library()
zephyr_library_sources(si4689.c)
endif()
