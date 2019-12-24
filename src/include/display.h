/*
 * display.h
 *
 *  Created on: 08.01.2017
 *      Author: marco
 */

#pragma once

int displayInit(void);
int displayOn(void);
int displayOff(void);
int displaySetContrast(uint8_t contrast);
int displaySetPos(uint8_t posX, uint8_t posY);
int displayWriteData(const uint8_t *data, size_t len);
int displayMemset(uint8_t value, size_t len);
int displayClear(void);
