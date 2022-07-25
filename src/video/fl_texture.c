#include "video/fl_texture.h"
#include "core/fl_log.h"
#include "core/fl_system.h"
#include "../core/frame_buffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

FL_Texture* FL_LoadTexture(const char *path) {
    FL_Texture *temp = (FL_Texture*)malloc(sizeof(FL_Texture));
    if(!temp) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't allocate memory for sprite!\n");
        return NULL;
    }

    int n;
    unsigned char *data = stbi_load(path, &temp->width, &temp->height, &n, 0);
    if(!data) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't load %s file!\n", path);
        
        free(temp);
        return NULL;
    }

    const int size = temp->width * temp->height;
    temp->data = (uint32_t*)malloc(size * sizeof(uint32_t));

    // translate raw image data to faster RGBA texture format
    // one pixel: 0xaabbggrr
    uint8_t *p = data;
    for(int i = 0; i < size; ++i) {
        if(n == 1) { // GRAY
            temp->data[i] = (uint32_t)(*p << 16 | *p << 8 | *p);
        } else if(n == 2) { // GRAY-ALPHA
            temp->data[i] = (uint32_t)(p[1] << 24 | *p << 16 | *p << 8 | *p);
        } else if(n == 3) { // RGB
            temp->data[i] = (uint32_t)(p[2] << 16 | p[1] << 8 | p[0]);
        } else if(n == 4) { // RGBA
            temp->data[i] = (uint32_t)(p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0]);
        }

        p += n;
    }

    stbi_image_free(data);
    
    return temp;
}

void FL_FreeTexture(FL_Texture *o) {
    if(!o) {
        FL_WriteLog(FL_LOG_WARNING, "[FL_FreeTexture] Cannot free NULL pointer\n");
        return;
    }

    free(o->data);
    free(o);
}

void FL_DrawTexture(const FL_Texture *s, int x, int y) {
    if(x + s->width < 0 || x >= FL_GetWindowWidth() || y + s->height < 0 || y >= FL_GetWindowHeight()) return;

    uint32_t *start_line = s->data;
    int w = s->width, h = s->height;

    if(x < 0) {
        start_line += -x;

        w += x;
        x = 0;
    }
    if(y < 0) {
        start_line += -y * s->width;

        h += y;
        y = 0;
    }

    if(w + x > FL_GetWindowWidth()) {
        w = FL_GetWindowWidth() - x;
    }
    if(h + y > FL_GetWindowHeight()) {
        h = FL_GetWindowHeight() - y;
    }
        
    uint32_t *frame = h_lines[y] + x;
    uint32_t *tex = NULL;

    int hz_count = 0;
    while(h--) {
        tex = start_line;
        hz_count = w;

        while(hz_count--) {
            if(*tex != 0xff00ff) *frame = *tex++;
            frame++;
        }
        
        frame += FL_GetWindowWidth() - w;
        start_line += s->width;
    }
}

void FL_DrawTextureScaled(const FL_Texture *s, int x, int y, int w, int h) {
    if(w <= 0 || h <= 0) return;
    if(x + w < 0 || x >= FL_GetWindowWidth() || y + h < 0 || y >= FL_GetWindowHeight()) return;

    int ren_w = w, ren_h = h;

    uint32_t *start_line = s->data;

    int w_counter = 0;
    int h_counter = 0;

    if(x < 0) {
        w_counter = -x * s->width;

        ren_w += x;
        x = 0;

        while(w_counter >= w) {
            start_line++;
            w_counter -= w;
        }
    }
    if(y < 0) {
        h_counter += -y * s->height;

        ren_h += y;
        y = 0; 

        while(h_counter >= h) {
            start_line += s->width;
            h_counter -= h;
        }
    }
    if(ren_w + x > FL_GetWindowWidth()) {
        ren_w = FL_GetWindowWidth() - x;
    }
    if(ren_h + y > FL_GetWindowHeight()) {
        ren_h = FL_GetWindowHeight() - y;
    }

    uint32_t *frame = h_lines[y] + x;
    uint32_t *tex = NULL;

    int hz_count = 0;
    int v_count = ren_h;
    while(v_count--) {
        tex = start_line;
        hz_count = ren_w;

        while(hz_count--) {
            uint32_t color = *tex;
            if(color != 0xff00ff) *frame++ = color;
            else frame++;

            w_counter += s->width;
            while(w_counter >= w) {
                tex++;
                w_counter -= w;
            }
        }

        frame += FL_GetWindowWidth() - ren_w;

        h_counter += s->height;
        while(h_counter >= h) {
            start_line += s->width;
            h_counter -= h;
        }
    }
}
