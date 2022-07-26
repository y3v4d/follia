#ifndef __FL_FONT_H__
#define __FL_FONT_H__

#include "video/fl_texture.h"
#include <stdint.h>

typedef struct _FL_CharBDF {
    uint32_t id;
  
    // bounding box 
    uint32_t bbw, bbh;
    int32_t bbxoff, bbyoff;

    // dwidth (distance between origin to the next character origin
    uint32_t dwx, dwy;

    uint8_t *data;
} FL_CharBDF;

typedef struct {
    uint32_t char_number; // amount of characters in font
    uint32_t start_char; // first character in font

    int32_t fbbw, fbbh; // general width and height of the individual character

    FL_CharBDF **chars; // user don't have to know about character implementation
} FL_FontBDF;

typedef struct {
    uint16_t id;

    uint16_t x, y; // position in texture
    uint16_t w, h; // size in texture

    uint16_t x_off, y_off;
    uint16_t x_adv;
} FL_CharFNT;

typedef struct {
    uint16_t count;
    uint16_t start_char;

    int size;

    FL_Texture *texture;
    FL_CharFNT *chars;
} FL_FontFNT;

/*
 * FL_LoadFontBDF
 * Load .bdf font and stores everything in newly allocated FL_FontBDF structure.
 * If it doesn't succeed (for whatever reason), will return NULL.
 *
 * path - path to the file relative to current directory (not always the directory with executable)
 */
FL_FontBDF* FL_LoadFontBDF(const char *path);
void FL_FreeFontBDF(FL_FontBDF *font);

void FL_SetTextColor(uint32_t color);

FL_FontFNT* FL_LoadFontFNT(const char *path);
void FL_FreeFontFNT(FL_FontFNT *o);

/*
 * FL_DrawText
 * Support multiline text rendering (properly wraps text and reads \n character).
 *
 * x, y - coordinates
 * text - text to render
 * max_size - function will render MAXIMUM max_size characters
 * max_width - text will render only in range from x to x + max_width. Additional text will be displaced to the next line.
 * font - font to use for rendering
 */
void FL_DrawTextBDF(int x, int y, const char *text, int max_size, int max_width, FL_FontBDF *font);
void FL_DrawTextFNT(int x, int y, const char *text, int size, int max_width, FL_FontFNT *font);
#endif
