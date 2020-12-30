#ifndef xf_font_h
#define xf_font_h

#include <stdint.h>

typedef struct _XF_CharBDF XF_CharBDF;

typedef struct _XF_FontBDF {
    uint32_t char_number; // amount of characters in font
    uint32_t start_char; // first character in font

    int32_t fbbw, fbbh; // general width and height of the individual character

    XF_CharBDF **chars; // user don't have to know about character implementation
} XF_FontBDF;

/*
 * XF_LoadFontBDF
 * Load .bdf font and stores everything in newly allocated XF_FontBDF structure.
 * If it doesn't succeed (for whatever reason), will return NULL.
 *
 * path - path to the file relative to current directory (not always the directory with executable)
 */
XF_FontBDF* XF_LoadFontBDF(const char *path);
void XF_FreeFontBDF(XF_FontBDF *font);

void XF_SetTextColor(uint32_t color);

/*
 * XF_DrawText
 * Support multiline text rendering (properly wraps text and reads \n character).
 *
 * x, y - coordinates
 * text - text to render
 * max_size - function will render MAXIMUM max_size characters
 * max_width - text will render only in range from x to x + max_width. Additional text will be displaced to the next line.
 * font - font to use for rendering
 */
void XF_DrawText(int x, int y, const char *text, int max_size, int max_width, XF_FontBDF *font);

#endif
