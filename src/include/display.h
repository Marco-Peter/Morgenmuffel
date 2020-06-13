/*
 * display.h
 *
 *  Created on: 08.01.2017
 *      Author: marco
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <drivers/gpio.h>

/** The display instance */
struct Display {
    struct device *dev;     /** The driver instance */
    struct device *rstPort; /** Port of the reset signal */
    struct device *cmdPort; /** Port of the command signal */
    struct device *pwrPort; /** Port of the power supply enable signal */
    gpio_pin_t rstPin;      /** Pin with the reset signal */
	gpio_pin_t cmdPin;      /** Pin with the command signal */
	gpio_pin_t pwrPin;      /** Pin with the power supply enable signal */
	uint8_t mirror_horiz;	    /** mirror horizontally */
	uint8_t mirror_vert;	    /** mirror vertically */
	uint8_t contrast;          /** Contrast strength */
};

int displayInit(struct Display *pInst);
int displayOn(struct Display *pInst);
int displayOff(struct Display *pInst);
int displaySetContrast(struct Display *pInst, uint8_t contrast);
int displaySetPos(struct Display *pInst, uint8_t posX, uint8_t posY);
int displayWriteData(struct Display *pInst, const uint8_t *data, size_t len);
int displayMemset(struct Display *pInst, uint8_t value, size_t len);
int displayClear(struct Display *pInst);

#endif // __DISPLAY_H__
