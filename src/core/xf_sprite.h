#ifndef xf_sprite_h
#define xf_sprite_h

#include <stdint.h>

typedef struct _XF_Sprite {
    uint32_t width;
    uint32_t height;

    uint32_t *data;
} XF_Sprite;

XF_Sprite* XF_LoadBMP(const char *path);
void XF_FreeSprite(XF_Sprite *o);

void XF_DrawSprite(const XF_Sprite *s, int x, int y);

#endif
