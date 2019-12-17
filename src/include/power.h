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

void powerSet1v8(bool on);
void powerSet5v(bool on);

int powerGet1v8(void);
int powerGet5v(void);
