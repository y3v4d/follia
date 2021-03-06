#include "video/xf_texture.h"
#include "core/xf_log.h"
#include "core/xf_system.h"

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

    for(unsigned i = 0; i < size / 2; ++i) {
        uint32_t swap = temp->data[i];
        temp->data[i] = temp->data[size - 1 - i];
        temp->data[size - 1 - i] = swap;
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
    int ac_w = s->width, ac_h = s->height;
    uint32_t *start_line = s->data;

    if(x < 0) {
        start_line += -x;

        ac_w += x;
        x = 0;
    }
    if(y < 0) {
        start_line += -y * s->height;

        ac_h += y;
        y = 0;
    }

    if(ac_w + x > XF_GetWindowWidth()) {
        ac_w -= (ac_w + x) - XF_GetWindowWidth();
    }
    if(ac_h + y > XF_GetWindowHeight()) {
        ac_h -= (ac_h + y) - XF_GetWindowHeight();
    }
        
    uint32_t *coord = start_line;

    for(int ay = 0; ay < ac_h; ++ay) {
        for(int ax = 0; ax < ac_w; ++ax) {
            if(*coord != 0xff00ff) XF_DrawPoint(x + ax, y + ay, *coord);

            coord++;
        }

        start_line += s->width;
        coord = start_line;
    }
}

/* TO OPTIMIZE! */
void XF_DrawTextureScaled(const XF_Texture *s, int x, int y, int w, int h) {
    if(w < 0 || h < 0) return;

    int ac_w = w, ac_h = h;
    uint32_t *line_start = s->data;
    
    float w_ratio = (float)s->width / w;
    float h_ratio = (float)s->height / h;
    
    float w_counter = 0;
    float h_counter = 0;

    float w_offset = 0;
    float h_offset = 0;
    
    if(x < 0) {
        w_offset = -x * w_ratio;
        line_start += (uint32_t)w_offset;

        w_counter = w_offset - (int) w_offset;

        ac_w += x;
        x = 0;
    }
    if(y < 0) {
        h_offset = -y * h_ratio;
        line_start += (uint32_t)((int)h_offset * s->height);

        h_counter = h_offset - (int) h_offset;

        ac_h += y;
        y = 0;
    }

    if(ac_w + x > XF_GetWindowWidth()) {
        ac_w -= (ac_w + x) - XF_GetWindowWidth();
    }
    if(ac_h + y > XF_GetWindowHeight()) {
        ac_h -= (ac_h + y) - XF_GetWindowHeight();
    }
    
    uint32_t *coord = line_start;

    for(int ay = 0; ay < ac_h; ++ay) {
        for(int ax = 0; ax < ac_w; ++ax) {
            if(*coord != 0xff00ff) XF_DrawPoint(x + ax, y + ay, *coord);

            w_counter += w_ratio;
            if(w_counter >= 1) {
                coord += (int)w_counter;
                w_counter -= (int)w_counter;
            }
        }

        h_counter += h_ratio;
        if(h_counter >= 1) {
            line_start += s->width * ((int)h_counter);
            h_counter -= (int)h_counter;
        }
        
        coord = line_start;

        w_counter = w_offset - (int) w_offset;
    }
}
