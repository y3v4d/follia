#ifndef xf_texture_h
#define xf_texture_h

#include <stdint.h>

typedef struct _XF_Texture {
    int width;
    int height;

    uint32_t *data;
} XF_Texture;

/*
 * XF_LoadTexture
 * Loads image file and stores width, height and data in newly allocated XF_Texture sturucture.
 * If it doesn't succeed (for whatever reason), will return NULL.
 *
 * path - path to the file relative to current directory (not always the directory with executable)
 */
XF_Texture* XF_LoadTexture(const char *path);
void XF_FreeTexture(XF_Texture *o);

/*
 * If in any function below user passes NULL as a texture, then good kurwa luck.
 */
void XF_DrawTexture(const XF_Texture *s, int x, int y);
void XF_DrawTextureScaled(const XF_Texture *s, int x, int y, int w, int h); // lower performance

#endif
