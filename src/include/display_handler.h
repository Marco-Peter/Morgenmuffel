/* Main include file for display related functions
 *
 *  Created on: 28.05.2017
 *      Author: marco
 */

#pragma once

#include "display.h"
#include "font.h"
#include <zephyr.h>

// scroll/refresh cycle in ms
#define DISP_SCROLL_CYCLE 200

/// Define the display properties
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_PAGES 8
#define DISPLAY_PAGESIZE 8
#define DISPLAY_COMPLETE (DISPLAY_WIDTH * DISPLAY_PAGES)

// Frame around text fields
#define DISP_FRAME_NONE 0x00
#define DISP_FRAME_BOTTOM 0x01
#define DISP_FRAME_TOP 0x02
#define DISP_FRAME_LEFT 0x04
#define DISP_FRAME_RIGHT 0x08

#define DISPLAY_MIN_CHARWIDTH 5 // minimum character width
#define DISPLAY_MAX_CHARWIDTH 5 // maximum character width

/**
The Text field instance. This contains all information to draw and handle
a text field.
*/
typedef struct TextField {
  const Font *font;
  const wchar_t *text;
  uint16_t length;
  uint16_t offset;
  uint8_t posX;
  uint8_t posY;
  uint8_t alignLeft : 1, alignRight : 1, inverted : 1, blinking : 1, frame : 4;
  bool updated;
} TextField;

/**
The instance of a complete display page.
*/
typedef struct DisplayPage {
  uint32_t nFields;  /// number of text field objects
  TextField *fields; /// Pointer to text field objects
} DisplayPage;

// Menu display functions
DisplayPage *dispShowPage(DisplayPage *page);
DisplayPage *dispClearPage(void);

// Handling functions for text fields
TextField *dispInitTextField(TextField *field, const wchar_t *text,
                             const Font *font, uint8_t posX, uint8_t posY,
                             uint16_t fLength, char align, bool inverted,
                             bool blinking, uint8_t frame);

const wchar_t *dispSetText(TextField *field, const wchar_t *text);
void dispSetInversion(TextField *field, uint8_t inversion);
