# 2020 (c) by M. Peter
#
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_SHT2X)
  zephyr_include_directories(.)

  zephyr_library()
  zephyr_library_sources(
    sht2x.c
    )
endif()
