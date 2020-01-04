# Boilerplate code, which pulls in the Zephyr build system.
cmake_minimum_required(VERSION 3.13.1)

# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory.
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD morgenmuffel)
set(ZEPHYR_BOARD_FLASH_RUNNER jlink)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(MorgenMuffel)

# Add your source file to the "app" target. This must come after
# the boilerplate code, which defines the target.
target_sources(app PRIVATE
    src/main.c
    src/include/storage.h
    src/storage.c
    src/include/display.h
    src/display/display.c
    src/include/font.h
    src/display/font.c
    src/display/font_FreeSans_16.c
    src/display/font_FreeSans_32.c
    src/include/display_handler.h
    src/display/display_handler.c
    src/power.c
)
target_include_directories( app PRIVATE src/include)
