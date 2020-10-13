# Boilerplate code, which pulls in the Zephyr build system.
cmake_minimum_required(VERSION 3.13.1)

# Define the custom board
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD morgenmuffel)
set(ZEPHYR_BOARD_FLASH_RUNNER jlink)

find_package(Zephyr)

project(MorgenMuffel)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add your source file to the "app" target. This must come after
# the boilerplate code, which defines the target.
target_sources(app PRIVATE
    src/main.c
    src/storage.h
    src/storage.c
    src/events.c
    src/userinput.c
    src/display.h
    src/display.c
    src/free_sans_16.c
    src/free_sans_32.c
    src/powersupply.c
)
target_include_directories( app PRIVATE src/include)
