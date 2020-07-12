# SPDX-License-Identifier: Apache-2.0

if(CONFIG_VS1053_DRIVER)
	# Add vs1053.h to the set of global include paths.
	zephyr_include_directories(.)
	
	zephyr_library()
	zephyr_library_sources(
		vs1053.c
	)
endif()
