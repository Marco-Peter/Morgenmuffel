/******************************************************************************
* font.c
*
* Implementation of font related functions
*
* 2017 by M. Peter
*
******************************************************************************/

#include "font.h"
#include <stdio.h>

/**************************************************************************/ /**
* Find and return the glyph data to a given character.
*
* @param character	The character to find within the given glyph set.
* @param glyphs		A pointer to the glyph set to use.
* @return			A pointer to the glyph set for the character or zero
*					when the character could not be found.
*
******************************************************************************/
const Glyph* getGlyph(wchar_t character, const Font* font)
{
    const Glyph* curGlyph = font->glyphs;
    while (curGlyph->character != 0) {
        if (curGlyph->character == character) {
            return curGlyph;
        }
        curGlyph++;
    }
    // Character not in glyph set
    return 0;
}

/**************************************************************************/ /**
* Get the text length on display of the given string
*
* @param text		The text to be measured.
* @param glyphSet	The glyph set to apply for the measurement
*
******************************************************************************/
uint32_t textLength(const wchar_t* text, const Font* font)
{
    uint32_t length = 0;

    if (text == NULL) {
        return 0;
    }

    for (; *text != 0; ++text) {
        const Glyph* glyph = getGlyph(*text, font);
        if (glyph != 0) {
            length += glyph->desc.advance;
        }
    }
    return length;
}
