#include "video/xf_font.h"
#include "core/xf_log.h"
#include "core/xf_system.h"
#include "core/xf_primitives.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

uint32_t text_color = 0x000000;

struct _XF_CharBDF {
    uint32_t id;
  
    // bounding box 
    uint32_t bbw, bbh;
    int32_t bbxoff, bbyoff;

    // dwidth (distance between origin to the next character origin
    uint32_t dwx, dwy;

    uint8_t *data;
};

uint8_t hex_to_int(uint8_t n) {
    uint8_t temp;
    
    if(n >= '0' && n <= '9')
        temp = n - '0';
    else if(n >= 'A' && n <= 'F')
        temp = n - 'A' + 10;
    else
        temp = 0;

    return temp;
}

XF_CharBDF* load_char(FILE *file) {
    if(!file) {
        XF_WriteLog(XF_LOG_ERROR, "Wrong file passed!");
        
        return NULL;
    }

    XF_CharBDF *temp = (XF_CharBDF*)malloc(sizeof(XF_CharBDF));
    if(!temp) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't allocate memory for character!");
        
        fclose(file);
        return NULL;
    }

    // search for the nearest character 
    char buffer[255];
    do {
        fgets(buffer, 255, file);
    } while(strncmp(buffer, "STARTCHAR", 9) != 0 && !feof(file));

    if(feof(file)) {
        XF_WriteLog(XF_LOG_ERROR, "Unexpected end of file!\n");

        free(temp);

        return NULL;
    }
    
    do {
        fgets(buffer, 255, file);

        if(!strncmp(buffer, "ENCODING", 8)) {
            sscanf(&buffer[9], "%d", &temp->id);
        } else if(!strncmp(buffer, "BBX", 3)) {
            sscanf(&buffer[4], "%d %d %d %d", &temp->bbw, &temp->bbh, &temp->bbxoff, &temp->bbyoff);
        } else if(!strncmp(buffer, "DWIDTH ", 7)) {
            sscanf(&buffer[7], "%d %d", &temp->dwx, &temp->dwy);
        }
    } while(strncmp(buffer, "BITMAP", 6) != 0);

    unsigned bytes_per_line = ceilf((float)temp->bbw / 8);
   
    temp->data = (uint8_t*)malloc(bytes_per_line * temp->bbh * sizeof(uint8_t));
    for(int i = 0; i < temp->bbh * bytes_per_line; ++i) {
        uint8_t helper = 0;

        fread(&temp->data[i], 1, 1, file);
        fread(&helper, 1, 1, file);
        temp->data[i] = hex_to_int(temp->data[i]); 
        helper = hex_to_int(helper);

        temp->data[i] = (temp->data[i] << 4) | helper;

        if((i + 1) % bytes_per_line == 0) fgetc(file); // discard unnecessary \n
    }

    return temp;
}

void XF_FreeFontBDF(XF_FontBDF *font) {
    if(!font) {
        XF_WriteLog(XF_LOG_WARNING, "Attempt to double-free memory! Be carefull!\n");
        return;
    }

    for(int i = 0; i < font->char_number; ++i) {
        if(font->chars[i]) {
            free(font->chars[i]->data);
            free(font->chars[i]);
        } else break;
    }

    if(font->chars) free(font->chars);
    free(font);
}

XF_FontBDF* XF_LoadFontBDF(const char *path) {
    FILE *file = fopen(path, "rb");
    if(!file) {
       XF_WriteLog(XF_LOG_ERROR, "Couldn't load %s font!\n", path);

       return NULL;
    }

    char buffer[255];

    fgets(buffer, 255, file);
    if(strncmp(buffer, "STARTFONT 2.1", 13) != 0) {
        XF_WriteLog(XF_LOG_ERROR, "Uncorrent %s font metadata! (expected 2.1 version)\n", path);
        
        fclose(file);
    }

    XF_FontBDF *temp = (XF_FontBDF*)malloc(sizeof(XF_FontBDF));
    if(!temp) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't allocate memory for font!\n");

        fclose(file);
        return NULL;
    }

    do {
        fgets(buffer, 255, file);
    } while(strncmp(buffer, "FONTBOUNDINGBOX ", 15) != 0 && !feof(file));

    sscanf(&buffer[15], "%d %d", &temp->fbbw, &temp->fbbh);

    // for now, just ignore any metadata included with the font
    do {
        fgets(buffer, 255, file);
    } while(strncmp(buffer, "CHARS ", 6) != 0 && !feof(file));

    if(feof(file)) {
        XF_WriteLog(XF_LOG_ERROR, "Unexpected end of the %s font!\n", path);

        free(temp);
        fclose(file);
    }

    sscanf(&buffer[6], "%d", &temp->char_number);

    temp->chars = (XF_CharBDF**)malloc(temp->char_number * sizeof(XF_CharBDF*));
    if(!temp->chars) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't allocate memory for font!\n");

        free(temp);
        fclose(file);

        return NULL;
    }

    for(int i = 0; i < temp->char_number; ++i) {
        temp->chars[i] = load_char(file);
        if(!temp->chars[i]) {
            XF_FreeFontBDF(temp);
            fclose(file);

            return NULL;
        } 
    }

    temp->start_char = temp->chars[0]->id;
    
    return temp;    
}

void render_char(int* x, int* y, const char character, XF_FontBDF *font, uint32_t color) {
    XF_CharBDF *picked = NULL;
    
    for(int i = 0; i < font->char_number; ++i) {
        if(font->chars[i]->id == character) {
            picked = font->chars[i];
            break;
        }
    }

    if(!picked) {
       XF_WriteLog(XF_LOG_WARNING, "Cannot render unknown character!\n");
       return;
    }

    int ax = 0, ay = 0;

    int byte_index = 0;
    for(int i = 0; i < picked->bbh; ++i) {
        for(int j = 0; j < picked->bbw; ++j) {
            if(j % 8 == 0 && j != 0) byte_index++; // properly handle more bytes per character than one

            int bit_shift = 7 - j % 8;

            if((picked->data[byte_index] >> bit_shift) & 1) {
                XF_DrawPoint(*x + picked->bbxoff + ax, *y - picked->bbyoff + ay, color);
            } 
            ax++;

            if(ax >= picked->bbw) {
                ax = 0;
                ay++;
            }
        }

        byte_index++;
    }

    *x += picked->dwx;
    *y += picked->dwy; 
}

void XF_SetTextColor(uint32_t color) {
    if(color <= 0xFFFFFF) text_color = color;
    else XF_WriteLog(XF_LOG_WARNING, "Cannot assign > 0xFFFFFF color to RGB storage!");
}

void XF_DrawText(int x, int y, const char *text, int size, int max_width, XF_FontBDF *font) {
    int start_x = x;

    int next_word = 0;
    for(int i = 0; i < size && text[i] != 0; ++i) {
        if(i == next_word && i < size - 1) { // check if word has to be moved to another line
            // search for the next word
            int s = i + 1;
            while(text[s] != 0 && text[s] != ' ' && s != size -1) { s++; }
            
            if(x + ((s - i) * font->fbbw) >= max_width) { // don't count space for render
                x = start_x;
                if(i > 0) y += font->fbbh; // don't move first line if it doesn't fit
            }

            next_word = (s == size - 1 ? -1 : s + 1); // don't count end of text as another word
        } else if(x + font->fbbw >= max_width) { // displace letters in word if they don't fit
            if(text[i] != ' ') {
                x = start_x;
                y += font->fbbh;
            } else continue; // just don't render space if it doesn't fit the line, instead of moving it to another line
        } else if(text[i] == '\n') { // make \n function
            x = start_x;
            y += font->fbbh;

            continue;
        }

        render_char(&x, &y, text[i], font, text_color); 
    } 
}
