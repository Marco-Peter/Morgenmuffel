# SPDX-License-Identifier: Apache-2.0

if(CONFIG_SI468X)
zephyr_include_directories(.)

zephyr_library()
zephyr_library_sources(
        si468x.c
        si468x_commands.c
)

if(CONFIG_SI468X_DAB)
zephyr_library_sources(
        si468x_dab.c
        si468x_commands_dab.c
)

endif() # CONFIG_SI468X_DAB
endif() # CONFIG_SI468X
