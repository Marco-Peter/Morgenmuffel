#include "lvgl/lvgl.h"

/*******************************************************************************
 * Size: 32 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifndef FREE_SANS_32
#define FREE_SANS_32 1
#endif

#if FREE_SANS_32

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+20 " " */
    0x0,

    /* U+25 "%" */
    0x0, 0x0, 0x18, 0x7, 0xc0, 0xc, 0x3, 0xf8,
    0x3, 0x1, 0xc7, 0x1, 0x80, 0xe0, 0xe0, 0xc0,
    0x30, 0x18, 0x30, 0xc, 0x6, 0x18, 0x3, 0x1,
    0x86, 0x0, 0xe0, 0xe3, 0x0, 0x1c, 0x70, 0xc0,
    0x3, 0xf8, 0x60, 0x0, 0x7c, 0x18, 0x0, 0x0,
    0xc, 0x0, 0x0, 0x7, 0x7, 0x80, 0x1, 0x83,
    0xf0, 0x0, 0xe1, 0x86, 0x0, 0x30, 0xc0, 0xc0,
    0x1c, 0x30, 0x30, 0x6, 0xc, 0xc, 0x3, 0x3,
    0x3, 0x0, 0xc0, 0xe1, 0xc0, 0x60, 0x1f, 0xe0,
    0x18, 0x3, 0xf0, 0xc, 0x0, 0x0,

    /* U+2D "-" */
    0xff, 0xff,

    /* U+30 "0" */
    0x7, 0xc0, 0x3f, 0xe0, 0xf1, 0xe1, 0xc1, 0xe7,
    0x1, 0xce, 0x3, 0xb8, 0x3, 0xf0, 0x7, 0xe0,
    0xf, 0xc0, 0x1f, 0x80, 0x3f, 0x0, 0x7e, 0x0,
    0xfc, 0x1, 0xf8, 0x3, 0xf0, 0x7, 0x70, 0x1c,
    0xe0, 0x39, 0xc0, 0x71, 0xc1, 0xc1, 0xff, 0x1,
    0xfc, 0x0, 0x40, 0x0,

    /* U+31 "1" */
    0x3, 0x7, 0x7, 0xf, 0xff, 0xff, 0x7, 0x7,
    0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7,
    0x7, 0x7, 0x7, 0x7, 0x7, 0x7,

    /* U+32 "2" */
    0xf, 0xe0, 0x7f, 0xf1, 0xe0, 0xf3, 0x80, 0xee,
    0x0, 0xfc, 0x1, 0xf8, 0x3, 0x80, 0x7, 0x0,
    0x1e, 0x0, 0x38, 0x1, 0xe0, 0xf, 0x80, 0x3e,
    0x1, 0xf0, 0x7, 0x80, 0x1c, 0x0, 0x70, 0x0,
    0xc0, 0x3, 0x80, 0x7, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xc0,

    /* U+33 "3" */
    0xf, 0xc0, 0x7f, 0xe1, 0xe1, 0xe3, 0x81, 0xee,
    0x1, 0xdc, 0x3, 0xb8, 0x7, 0x0, 0xe, 0x0,
    0x78, 0x7, 0xe0, 0xf, 0xc0, 0x1, 0xe0, 0x1,
    0xc0, 0x1, 0xc0, 0x3, 0xf0, 0x7, 0xe0, 0xf,
    0xc0, 0x1f, 0xc0, 0x73, 0x81, 0xe3, 0xff, 0x83,
    0xfe, 0x0, 0x0, 0x0,

    /* U+34 "4" */
    0x0, 0x38, 0x0, 0x38, 0x0, 0x78, 0x0, 0xf8,
    0x1, 0xf8, 0x1, 0xb8, 0x3, 0xb8, 0x7, 0x38,
    0xe, 0x38, 0xc, 0x38, 0x1c, 0x38, 0x38, 0x38,
    0x70, 0x38, 0x60, 0x38, 0xff, 0xff, 0xff, 0xff,
    0x0, 0x38, 0x0, 0x38, 0x0, 0x38, 0x0, 0x38,
    0x0, 0x38, 0x0, 0x38,

    /* U+35 "5" */
    0x3f, 0xfc, 0x7f, 0xf8, 0xff, 0xf3, 0x80, 0x7,
    0x0, 0xe, 0x0, 0x1c, 0x0, 0x38, 0x0, 0x67,
    0xe0, 0xff, 0xe1, 0xe1, 0xe7, 0x80, 0xe0, 0x1,
    0xe0, 0x1, 0xc0, 0x3, 0x80, 0x7, 0x0, 0xf,
    0x80, 0x1f, 0x80, 0x3f, 0x0, 0xe7, 0x3, 0x87,
    0xff, 0x7, 0xf8, 0x0, 0x0,

    /* U+36 "6" */
    0x7, 0xe0, 0x1f, 0xe0, 0xf0, 0xe1, 0xc0, 0xe7,
    0x1, 0xce, 0x0, 0x18, 0x0, 0x70, 0x0, 0xe7,
    0xe1, 0xdf, 0xf3, 0xf0, 0xf7, 0x80, 0xef, 0x1,
    0xfc, 0x1, 0xf8, 0x3, 0xf0, 0x7, 0xe0, 0xe,
    0xc0, 0x1d, 0xc0, 0x71, 0xc1, 0xe3, 0xff, 0x81,
    0xfc, 0x0, 0x0, 0x0,

    /* U+37 "7" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x0, 0x60,
    0x1, 0xc0, 0x7, 0x0, 0xc, 0x0, 0x38, 0x0,
    0xe0, 0x1, 0x80, 0x7, 0x0, 0xc, 0x0, 0x38,
    0x0, 0x60, 0x1, 0xc0, 0x3, 0x80, 0xe, 0x0,
    0x1c, 0x0, 0x38, 0x0, 0x60, 0x1, 0xc0, 0x3,
    0x80, 0x0,

    /* U+38 "8" */
    0x7, 0xc0, 0x3f, 0xe0, 0xf1, 0xe3, 0xc1, 0xe7,
    0x1, 0xce, 0x3, 0x9c, 0x7, 0x3c, 0x1e, 0x3c,
    0x78, 0x3f, 0xe0, 0x7f, 0x81, 0xc1, 0xc7, 0x1,
    0xdc, 0x1, 0xf8, 0x3, 0xf0, 0x7, 0xe0, 0xf,
    0xc0, 0x1d, 0xc0, 0x73, 0xc1, 0xe3, 0xff, 0x83,
    0xfe, 0x0, 0x0, 0x0,

    /* U+39 "9" */
    0xf, 0xc0, 0x3f, 0xe0, 0xe1, 0xe3, 0x80, 0xef,
    0x1, 0xdc, 0x1, 0xb8, 0x3, 0xf0, 0x7, 0xe0,
    0xf, 0xc0, 0x3d, 0xc0, 0x7b, 0xc3, 0xf3, 0xfe,
    0xe1, 0xf9, 0xc0, 0x3, 0x80, 0x7, 0x0, 0x1c,
    0xe0, 0x39, 0xc0, 0x63, 0xc1, 0xc3, 0xff, 0x3,
    0xfc, 0x0, 0x0, 0x0,

    /* U+3A ":" */
    0xff, 0x80, 0x0, 0x0, 0x0, 0x3f, 0xe0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 128, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 455, .box_w = 26, .box_h = 24, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 79, .adv_w = 170, .box_w = 8, .box_h = 2, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 81, .adv_w = 285, .box_w = 15, .box_h = 23, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 125, .adv_w = 285, .box_w = 8, .box_h = 22, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 285, .box_w = 15, .box_h = 22, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 189, .adv_w = 285, .box_w = 15, .box_h = 23, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 233, .adv_w = 285, .box_w = 16, .box_h = 22, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 285, .box_w = 15, .box_h = 24, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 322, .adv_w = 285, .box_w = 15, .box_h = 23, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 366, .adv_w = 285, .box_w = 15, .box_h = 22, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 408, .adv_w = 285, .box_w = 15, .box_h = 23, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 452, .adv_w = 285, .box_w = 15, .box_h = 23, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 496, .adv_w = 128, .box_w = 3, .box_h = 17, .ofs_x = 3, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x5, 0xd
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 14, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 3, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    },
    {
        .range_start = 48, .range_length = 11, .glyph_id_start = 4,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

/*Store all the custom data of the font*/
static lv_font_fmt_txt_dsc_t font_dsc = {
    .glyph_bitmap = gylph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t free_sans_32 = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 24,          /*The maximum line height required by the font*/
    .base_line = 1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0)
    .underline_position = -6,
    .underline_thickness = 2,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if FREE_SANS_32*/

