/*
 * power.h
 *
 * Declares power the power supply handling.
 *
 * Display power handling is done in the display module.
 *
 *  Created on: 20.01.2017
 *      Author: marco
 */

#ifndef INC_POWER_H_
#define INC_POWER_H_

#include "stm32f2xx_hal.h"

typedef struct Power {
    GPIO_TypeDef* dis3v3Port;
    GPIO_TypeDef* enBkFdPort;
    GPIO_TypeDef* en1v8Port;
    GPIO_TypeDef* en5vPort;
    uint16_t dis3v3Pin;
    uint16_t enBkFdPin;
    uint16_t en1v8Pin;
    uint16_t en5vPin;
} Power;

extern Power power;

void powerSet1v8(Power* pInst, int on);
void powerSet5v(Power* pInst, int on);

int powerGet1v8(Power* pInst);
int powerGet5v(Power* pInst);

#endif /* INC_POWER_H_ */
