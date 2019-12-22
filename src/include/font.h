/******************************************************************************
* font.h
*
* Declaration of structures to draw characters in a defined font
*
* 2017 by M. Peter
*
******************************************************************************/

#ifndef __font_h__
#define __font_h__

#include <stdint.h>
#include <wchar.h>

/******************************************************************************
* Glyph descriptor with the required key to find the glyph using linear search
*
******************************************************************************/
typedef struct Glyph {
	wchar_t character;		// represented character
	struct {
		const uint8_t *data;		// Pointer to the glyph data
		uint8_t width;		// Width of the character
		uint8_t height;		// height of the character
		uint8_t bearingX;	// Distance from origin to actual beginning of the character
		uint8_t bearingY;	// Disatnce from origin to actual beginning of the character
		uint8_t advance;	// Number of pixels to advance the cursor after the character is drawn
	} desc;
} Glyph;

/******************************************************************************
* Font data container
*
******************************************************************************/
typedef struct Font {
	uint16_t size;
	const Glyph *glyphs;
} Font;

/******************************************************************************
* Structures containing data to describe a given character set
*
******************************************************************************/
extern const Font fontFreeSans16;
extern const Font fontFreeSans32;

/******************************************************************************
* Font related functions
*
******************************************************************************/
const Glyph *getGlyph(wchar_t character, const Font *font);
uint32_t textLength(const wchar_t *text, const Font *font);


#endif // __font_h__

