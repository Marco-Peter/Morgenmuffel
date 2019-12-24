/**************************************************************************/ /**
 * hardwareconfig.h
 *
 * Hardware configuration of the project
 *
 * 2019 by M. Peter
 *
 *****************************************************************************/

#pragma once

// Designation of used pins
#define POWER_1V8_Pin 7
#define POWER_1V8_GPIO_Port "GPIOF"
#define POWER_5V_Pin 5
#define POWER_5V_GPIO_Port "GPIOG"
#define POWER_13V_Pin 7
#define POWER_13V_GPIO_Port "GPIOD"
#define POWER_3V3_Pin 0
#define POWER_3V3_GPIO_Port "GPIOA"
#define POWER_BKFD_Pin 10
#define POWER_BKFD_GPIO_Port "GPIOA"

#define BUTTON_PWR_Pin 14
#define BUTTON_PWR_GPIO_Port "GPIOE"
#define BUTTON_MENU_Pin 0
#define BUTTON_MENU_GPIO_Port "GPIOF"
#define BUTTON_SELECT_Pin 9
#define BUTTON_SELECT_GPIO_Port "GPIOG"
#define BUTTON_PLAY_Pin 13
#define BUTTON_PLAY_GPIO_Port "GPIOC"
#define BUTTON_FWD_Pin 2
#define BUTTON_FWD_GPIO_Port "GPIOE"
#define BUTTON_BACK_Pin 0
#define BUTTON_BACK_GPIO_Port "GPIOC"
#define BUTTON_ALARM1_Pin 8
#define BUTTON_ALARM1_GPIO_Port "GPIOD"
#define BUTTON_ALARM2_Pin 12
#define BUTTON_ALARM2_GPIO_Port "GPIOD"
#define BUTTON_ALARM3_Pin 13
#define BUTTON_ALARM3_GPIO_Port "GPIOD"

#define ENC_VOL_A_Pin 8
#define ENC_VOL_A_GPIO_Port "GPIOA"
#define ENC_VOL_B_Pin 11
#define ENC_VOL_B_GPIO_Port "GPIOE"

#define ENC_SELECT_A_Pin 4
#define ENC_SELECT_A_GPIO_Port "GPIOB"
#define ENC_SELECT_B_Pin 7
#define ENC_SELECT_B_GPIO_Port "GPIOC"

#define BT_RST_Pin 3
#define BT_RST_GPIO_Port "GPIOE"
#define BT_TX_Pin 10
#define BT_TX_GPIO_Port "GPIOC"
#define BT_RX_Pin 11
#define BT_RX_GPIO_Port "GPIOC"

#define AUDIO_MUX_B_Pin 4
#define AUDIO_MUX_B_GPIO_Port "GPIOE"
#define AUDIO_MUX_A_Pin 5
#define AUDIO_MUX_A_GPIO_Port "GPIOE"
#define AUDIO_MUX_EN_Pin 6
#define AUDIO_MUX_EN_GPIO_Port "GPIOE"
#define AUDIO_THERM_SUB_Pin 11
#define AUDIO_THERM_SUB_GPIO_Port "GPIOG"
#define AUDIO_SHDN_Pin 12
#define AUDIO_SHDN_GPIO_Port "GPIOG"
#define AUDIO_THERM_TOP_Pin 13
#define AUDIO_THERM_TOP_GPIO_Port "GPIOG"

#define VS1053_RST_Pin 1
#define VS1053_RST_GPIO_Port "GPIOF"
#define VS1053_DREQ_Pin 2
#define VS1053_DREQ_GPIO_Port "GPIOF"

#define DISPLAY_CS_Pin 3
#define DISPLAY_CS_GPIO_Port "GPIOF"
#define DISPLAY_RST_Pin 4
#define DISPLAY_RST_GPIO_Port "GPIOF"
#define DISPLAY_DC_Pin 5
#define DISPLAY_DC_GPIO_Port "GPIOF"

#define BUZZER_Pin 8
#define BUZZER_GPIO_Port "GPIOF"

#define WLAN_UPDATE_Pin 3
#define WLAN_UPDATE_GPIO_Port "GPIOA"
#define WLAN_CHIP_EN_Pin 4
#define WLAN_CHIP_EN_GPIO_Port "GPIOA"
#define WLAN_RST_Pin 0
#define WLAN_RST_GPIO_Port "GPIOB"

#define DECT_PON_Pin 15
#define DECT_PON_GPIO_Port "GPIOE"
#define DECT_INT_Pin 1
#define DECT_INT_GPIO_Port "GPIOB"
#define DECT_RST_Pin 15
#define DECT_RST_GPIO_Port "GPIOF"

#define ETH_RST_Pin 11
#define ETH_RST_GPIO_Port "GPIOF"
#define ETH_RXER_Pin 14
#define ETH_RXER_GPIO_Port "GPIOF"
#define ETH_INCLK_Pin 9
#define ETH_INCLK_GPIO_Port "GPIOC"

#define SI4689_INT_Pin 10
#define SI4689_INT_GPIO_Port "GPIOD"
#define SI4689_RST_Pin 11
#define SI4689_RST_GPIO_Port "GPIOD"

#define INT_BRIGHTNESS_Pin 0
#define INT_BRIGHTNESS_GPIO_Port "GPIOE"

#define SPI_PERIPH "SPI_2"
#define SPI_MEMORY "SPI_1"
