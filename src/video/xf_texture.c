#include "video/xf_texture.h"
#include "core/xf_log.h"
#include "core/xf_system.h"
#include "../core/frame_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// convert 32-bit little endian number to cpu number
// gets pointer to beginning of the 4-size uint8_t array
uint32_t le32_to_cpu(uint8_t *n) {
    return ((uint32_t)n[0] | (uint32_t)(n[1] << 8) | (uint32_t)(n[2] << 16) | (uint32_t)(n[3] << 24));
}

// function loads only BMP files with compression BI_RGB24 and BGR format
XF_Texture* XF_LoadBMP(const char *path) {
    XF_Texture *temp = (XF_Texture*)malloc(sizeof(XF_Texture));
    if(!temp) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't allocate memory for sprite!\n");
        return NULL;
    }

    FILE *file = fopen(path, "rb");
    if(!file) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't load %s file!\n", path);
        
        free(temp);
        return NULL;
    }

    uint8_t buffer[40];
    
    fread(buffer, 1, 14, file); // read header
    if(buffer[0] != 'B' || buffer[1] != 'M') {
        XF_WriteLog(XF_LOG_ERROR, "Invalid file %s format!\n", path);

        free(temp);
        return NULL;
    }

    fread(buffer, 1, 40, file); // read info header
    if(feof(file)) {
        XF_WriteLog(XF_LOG_ERROR, "Invalid information header %s format!\n", path);

        free(temp);
        return NULL;
    }

    temp->width = le32_to_cpu(buffer + 4);
    temp->height = le32_to_cpu(buffer + 8);

    unsigned size = temp->width * temp->height;

    temp->data = (uint32_t*)malloc(size * sizeof(uint32_t));
    if(!temp->data) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't allocate memory for sprite data!\n");

        free(temp);
        return NULL;
    }

    /*
     * 32-bit data variable stores 4 bytes in right-to-left order, so
     * with the BGR format of .bmp files there's no need to flip the bytes
     * if data is read 3 bytes at once.
     *
     * uint32_t = 0xaabbccdd
     *              3 2 1 0
     *  B   G   R  |  B   G   R  | ...
     * [0] [1] [2]   [0] [1] [2]
     *
     * Final: 0x00RRGGBB (no need to flip the bytes)
     */
    for(unsigned i = 0; i < size; ++i) {
        fread(&temp->data[i], 3, 1, file);
    }

    for(unsigned y = 0; y < temp->height / 2; ++y) {
        for(unsigned x = 0; x < temp->width; ++x) {
            const int start_y = y * temp->width;

            uint32_t swap = temp->data[start_y + x];
            temp->data[start_y + x] = temp->data[size - start_y - temp->width + x];
            temp->data[size - start_y - temp->width + x] = swap;
        }
    }

    if(feof(file)) {
        XF_WriteLog(XF_LOG_ERROR, "Unexpected end of the %s file!\n");

        free(temp->data);
        free(temp);
        return NULL;
    }

    fclose(file);
    return temp;
}

void XF_FreeTexture(XF_Texture *o) {
    if(!o) {
        XF_WriteLog(XF_LOG_WARNING, "Cannot free NULL sprite pointer\n");
        return;
    }

    free(o->data);
    free(o);

    o = NULL;
}

void XF_DrawTexture(const XF_Texture *s, int x, int y) {
    if(x + s->width < 0 || x >= XF_GetWindowWidth() || y + s->height < 0 || y >= XF_GetWindowHeight()) return;

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

    if(w + x > XF_GetWindowWidth()) {
        w = XF_GetWindowWidth() - x;
    }
    if(h + y > XF_GetWindowHeight()) {
        h = XF_GetWindowHeight() - y;
    }
        
    uint32_t *frame = h_lines[y] + x;
    uint32_t *tex = NULL;

    int hz_count = 0;
    while(h--) {
        tex = start_line;
        hz_count = w;

        while(hz_count--) {
            uint32_t pixel = *tex++;

            if(pixel != 0xff00ff) *frame++ = pixel;
            else frame++;
        }
        
        frame += XF_GetWindowWidth() - w;
        start_line += s->width;
    }
}

void XF_DrawTextureScaled(const XF_Texture *s, int x, int y, int w, int h) {
    if(w <= 0 || h <= 0) return;
    if(x + w < 0 || x >= XF_GetWindowWidth() || y + h < 0 || y >= XF_GetWindowHeight()) return;

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
    if(ren_w + x > XF_GetWindowWidth()) {
        ren_w = XF_GetWindowWidth() - x;
    }
    if(ren_h + y > XF_GetWindowHeight()) {
        ren_h = XF_GetWindowHeight() - y;
    }

    uint32_t *frame = h_lines[y] + x;
    uint32_t *tex = NULL;;

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

        frame += XF_GetWindowWidth() - ren_w;

        h_counter += s->height;
        while(h_counter >= h) {
            start_line += s->width;
            h_counter -= h;
        }
    }
}
