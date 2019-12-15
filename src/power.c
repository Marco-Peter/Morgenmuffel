/*
 * power.c
 *
 * Implements the power supply functions
 *
 *  Created on: 20.01.2017
 *      Author: marco
 */

#include "power.h"
#include "gpio_handler.h"

Power power = {
    DISABLE_3V3_GPIO_Port,
    ENABLE_BKFD_GPIO_Port,
    ENABLE_1V8_GPIO_Port,
    ENABLE_5V_GPIO_Port,
    DISABLE_3V3_Pin,
    ENABLE_BKFD_Pin,
    ENABLE_1V8_Pin,
    ENABLE_5V_Pin
};

/**************************************************************************/ /**
 * Switches the 1.8V power supply on or off.
 *
 * @param pInst	Pointer to the power supply instance.
 * @param on	On-state of the power supply (0: off, else: on)
 *
 *****************************************************************************/
void powerSet1v8(Power* pInst, int on)
{
    HAL_GPIO_WritePin(pInst->en1v8Port, pInst->en1v8Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**************************************************************************/ /**
 * Switches the 5V power supply on or off.
 *
 * @param pInst	Pointer to the power supply instance.
 * @param on	On-state of the power supply (0: off, else: on)
 *
 *****************************************************************************/
void powerSet5v(Power* pInst, int on)
{
    HAL_GPIO_WritePin(pInst->en5vPort, pInst->en5vPin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**************************************************************************/ /**
 * Returns the current state of the 1.8V power supply.
 *
 * @param pInst	Pointer to the power supply instance.
 * @return	The on state of the 1.8V power supply.
 *
 *****************************************************************************/
int powerGet1v8(Power* pInst)
{
    return HAL_GPIO_ReadPin(pInst->en1v8Port, pInst->en1v8Pin);
}

/**************************************************************************/ /**
 * Returns the current state of the 5V power supply.
 *
 * @param pInst	Pointer to the power supply instance.
 * @return	The on state of the 5V power supply.
 *
 *****************************************************************************/
int powerGet5v(Power* pInst)
{
    return HAL_GPIO_ReadPin(pInst->en5vPort, pInst->en5vPin);
}
