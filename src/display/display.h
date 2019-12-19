/*
 * display.h
 *
 *  Created on: 08.01.2017
 *      Author: marco
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

/// Define the error codes
#define DISPLAY_OK 0 // operation successful
#define DISPLAY_ERR_SPI -1 // SPI transmission failed
#define DISPLAY_ERR_INV_CHSET -2 // Invalid character set
#define DISPLAY_TOO_MUCH_ROWS -3 // too much rows to write
#define DISPLAY_ERR_ALIGNMENT -4 // Wrong alignment (no multiple of page size)
#define DISPLAY_ERR_MEMORY -5 // No memory available

/// Define the display properties
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_PAGES 8
#define DISPLAY_PAGESIZE 8
#define DISPLAY_COMPLETE (DISPLAY_WIDTH * DISPLAY_PAGES)

#define DISPLAY_MIN_CHARWIDTH 5 // minimum character width
#define DISPLAY_MAX_CHARWIDTH 5 // maximum character width

/// The display instance
typedef struct Display {
    SpiDevice spi; // Instance of the SPI interface
    GPIO_TypeDef* rstPort; // Port of the reset signal
    GPIO_TypeDef* cmdPort; // Port of the command signal
    GPIO_TypeDef* pwrPort; // Port of the power supply enable signal
    uint16_t rstPin; // Pin with the reset signal
    uint16_t cmdPin; // Pin with the command signal
    uint16_t pwrPin; // Pin with the power supply enable signal
    uint8_t mirror_horiz; // mirror horizontally
    uint8_t mirror_vert; // mirror vertically
    uint8_t contrast;
} Display;

int displayInit(void);
int displayOn(void);
int displayOff(void);
int displaySetContrast(uint8_t contrast);
int displaySetPos(uint8_t posX, uint8_t posY);
int displayMemset(uint8_t value, uint8_t len);
int displayClear(void);

#endif /* INC_DISPLAY_H_ */
