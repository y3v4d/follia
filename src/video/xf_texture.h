#ifndef xf_texture_h
#define xf_texture_h

#include <stdint.h>

typedef struct _XF_Texture {
    uint32_t width;
    uint32_t height;

    uint32_t *data;
} XF_Texture;

XF_Texture* XF_LoadBMP(const char *path);
void XF_FreeTexture(XF_Texture *o);

void XF_DrawTexture(const XF_Texture *s, int x, int y);
void XF_DrawTextureScaled(const XF_Texture *s, int x, int y, int w, int h);

#endif
