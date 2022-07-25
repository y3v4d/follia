#ifndef __FL_TEXTURE_H__
#define __FL_TEXTURE_H__

#include <stdint.h>

typedef struct {
    int width;
    int height;

    uint32_t *data;
} FL_Texture;

/*
 * FL_LoadTexture
 * Loads image file and stores width, height and data in newly allocated FL_Texture sturucture.
 * If it doesn't succeed (for whatever reason), will return NULL.
 *
 * path - path to the file relative to current directory (not always the directory with executable)
 */
FL_Texture* FL_LoadTexture(const char *path);
void FL_FreeTexture(FL_Texture *o);

/*
 * If in any function below user passes NULL as a texture, then good kurwa luck.
 */
void FL_DrawTexture(const FL_Texture *s, int x, int y);
void FL_DrawTextureScaled(const FL_Texture *s, int x, int y, int w, int h); // lower performance

#endif
