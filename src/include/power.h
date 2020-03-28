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

#pragma once

#include <stdbool.h>

int powerInit(void);

int powerSet1v8(bool on);
int powerSet3v3(bool on);
int powerSetLed(bool on);
int powerSetBackfeed(bool on);

int powerGet1v8(void);
int powerGet3v3(void);
int powerGetLed(void);
int powerGetBackfeed(void);
