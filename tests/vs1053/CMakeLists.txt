cmake_minimum_required(VERSION 3.13.1)

# Define the custom board
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(BOARD morgenmuffel)
set(ZEPHYR_BOARD_FLASH_RUNNER jlink)

find_package(Zephyr)

list(APPEND ZEPHYR_EXTRA_MODULES
	${CMAKE_CURRENT_SOURCE_DIR}/../../drivers/vs1053
)

project(VS1053)

target_sources(app PRIVATE
	src/main.c
)
