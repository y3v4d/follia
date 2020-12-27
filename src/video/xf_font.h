#ifndef xf_font_h
#define xf_font_h

#include <stdint.h>

typedef struct _XF_CharBDF XF_CharBDF;

typedef struct _XF_FontBDF {
    uint32_t char_number;
    uint32_t start_char;

    int32_t fbbw, fbbh;

    XF_CharBDF **chars;
} XF_FontBDF;

XF_FontBDF* XF_LoadFontBDF(const char *path);
void XF_FreeFontBDF(XF_FontBDF *font);

void XF_setTextColor(uint32_t color);
void XF_DrawText(int x, int y, const char *text, int max_size, int max_width, XF_FontBDF *font);

#endif
