#include "video/fl_font.h"
#include "core/fl_log.h"
#include "core/fl_system.h"
#include "core/fl_primitives.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

uint32_t text_color = 0x000000;

struct _FL_CharBDF {
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

FL_CharBDF* load_char(FILE *file) {
    if(!file) {
        FL_WriteLog(FL_LOG_ERROR, "Wrong file passed!");
        
        return NULL;
    }

    FL_CharBDF *temp = (FL_CharBDF*)malloc(sizeof(FL_CharBDF));
    if(!temp) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't allocate memory for character!");
        
        fclose(file);
        return NULL;
    }

    // search for the nearest character 
    char buffer[255];
    do {
        fgets(buffer, 255, file);
    } while(strncmp(buffer, "STARTCHAR", 9) != 0 && !feof(file));

    if(feof(file)) {
        FL_WriteLog(FL_LOG_ERROR, "Unexpected end of file!\n");

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

void FL_FreeFontBDF(FL_FontBDF *font) {
    if(!font) {
        FL_WriteLog(FL_LOG_WARNING, "Attempt to double-free memory! Be carefull!\n");
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

FL_FontBDF* FL_LoadFontBDF(const char *path) {
    FILE *file = fopen(path, "rb");
    if(!file) {
       FL_WriteLog(FL_LOG_ERROR, "Couldn't load %s font!\n", path);

       return NULL;
    }

    char buffer[255];

    fgets(buffer, 255, file);
    if(strncmp(buffer, "STARTFONT 2.1", 13) != 0) {
        FL_WriteLog(FL_LOG_ERROR, "Uncorrent %s font metadata! (expected 2.1 version)\n", path);
        
        fclose(file);
    }

    FL_FontBDF *temp = (FL_FontBDF*)malloc(sizeof(FL_FontBDF));
    if(!temp) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't allocate memory for font!\n");

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
        FL_WriteLog(FL_LOG_ERROR, "Unexpected end of the %s font!\n", path);

        free(temp);
        fclose(file);
    }

    sscanf(&buffer[6], "%d", &temp->char_number);

    temp->chars = (FL_CharBDF**)malloc(temp->char_number * sizeof(FL_CharBDF*));
    if(!temp->chars) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't allocate memory for font!\n");

        free(temp);
        fclose(file);

        return NULL;
    }

    for(int i = 0; i < temp->char_number; ++i) {
        temp->chars[i] = load_char(file);
        if(!temp->chars[i]) {
            FL_FreeFontBDF(temp);
            fclose(file);

            return NULL;
        } 
    }

    temp->start_char = temp->chars[0]->id;
    
    return temp;    
}

int str_to_int(const char *s) {
    int n = 0;

    while(*s >= '0' && *s <= '9') {
        n = n * 10 + (*s - '0');
        ++s;
    }

    return n;
}

FL_FontFNT* FL_LoadFontFNT(const char *path) {
    FL_FontFNT *temp = (FL_FontFNT*)malloc(sizeof(FL_FontFNT));
    if(!temp) {
        return NULL;
    }

    FILE *info = fopen(path, "rb");
    if(!info) {
        free(temp);
        return NULL;
    }

    const unsigned SIZE = 512;
    char buffer[SIZE];

    char filename[SIZE];
    int filename_size = 0;

    while(fgets(buffer, SIZE, info)) {
        char *found;
        if((found = strstr(buffer, "file")) != NULL) {
            const int first = 6;
            int last = first;

            for(; found[last] != '"'; ++last);
            filename_size = last - first;

            strncpy(filename, found + first, filename_size);
            filename[filename_size] = 0;

            printf("Filename: %s\n", filename);
        }

        if((found = strstr(buffer, "size")) != NULL) {
            temp->size = str_to_int(found + 5);
            printf("Size: %d\n", temp->size);
        }

        if((found = strstr(buffer, "chars count")) != NULL) {
            temp->count = str_to_int(found + 12);
            printf("Chars count: %d\n", temp->count);
            break;
        }
    }

    int last_slash = 0;
    for(int i = 0; path[i] != 0; ++i) {
        if(path[i] == '/') last_slash = i;
    }

    char texture_path[SIZE];
    strncpy(texture_path, path, last_slash + 1);
    strncpy(texture_path + last_slash + 1, filename, filename_size + 1);
    printf("Texture path: %s\n", texture_path);

    temp->texture = FL_LoadTexture(texture_path);
    if(!temp->texture) {
        free(temp);
        fclose(info);
        return NULL;
    }

    temp->chars = (FL_CharFNT*)malloc(temp->count * sizeof(FL_CharFNT));
    if(!temp->chars) {
        FL_FreeTexture(temp->texture);
        free(temp);
        fclose(info);

        return NULL;
    }

    for(int i = 0; i < temp->count; ++i) {
        FL_CharFNT *c = &temp->chars[i];
        char *found = NULL;

        fgets(buffer, SIZE, info);

        found = strstr(buffer, "id");
        c->id = str_to_int(found + 3);

        found = strstr(buffer, "x");
        c->x = str_to_int(found + 2);

        found = strstr(buffer, "y");
        c->y = str_to_int(found + 2);

        found = strstr(buffer, "width");
        c->w = str_to_int(found + 6);

        found = strstr(buffer, "height");
        c->h = str_to_int(found + 7);

        found = strstr(buffer, "xoffset");
        c->x_off = str_to_int(found + 8);

        found = strstr(buffer, "yoffset");
        c->y_off = str_to_int(found + 8);

        found = strstr(buffer, "xadvance");
        c->x_adv = str_to_int(found + 9);
    }

    fclose(info);

    return temp;
}

void FL_FreeFontFNT(FL_FontFNT *o) {
    if(!o) return;

    if(o->texture) FL_FreeTexture(o->texture);
    if(o->chars) free(o->chars);

    free(o);
}

void render_char(int* x, int* y, const char character, FL_FontBDF *font, uint32_t color) {
    FL_CharBDF *picked = NULL;
    
    for(int i = 0; i < font->char_number; ++i) {
        if(font->chars[i]->id == character) {
            picked = font->chars[i];
            break;
        }
    }

    if(!picked) {
       FL_WriteLog(FL_LOG_WARNING, "Cannot render unknown character!\n");
       return;
    }

    int ax = 0, ay = 0;

    int byte_index = 0;
    for(int i = 0; i < picked->bbh; ++i) {
        for(int j = 0; j < picked->bbw; ++j) {
            if(j % 8 == 0 && j != 0) byte_index++; // properly handle more bytes per character than one

            int bit_shift = 7 - j % 8;

            if((picked->data[byte_index] >> bit_shift) & 1) {
                FL_DrawPoint(*x + picked->bbxoff + ax, *y - picked->bbyoff + ay, color);
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

void FL_SetTextColor(uint32_t color) {
    if(color <= 0xFFFFFF) text_color = color;
    else FL_WriteLog(FL_LOG_WARNING, "Cannot assign > 0xFFFFFF color to RGB storage!");
}

void render_char_fnt(int* x, int* y, const char character, FL_FontFNT *font) {
    FL_CharFNT *picked = NULL;
    
    for(int i = 0; i < font->count; ++i) {
        if(font->chars[i].id == character) {
            picked = &font->chars[i];
            break;
        }
    }

    if(!picked) {
       FL_WriteLog(FL_LOG_WARNING, "Cannot render unknown character!\n");
       return;
    }

    FL_Texture *tex = font->texture;
    uint32_t *data = &tex->data[picked->y * tex->width + picked->x];

    int ax = 0, ay = 0;

    int byte_index = 0;
    for(int i = 0; i < picked->h; ++i) {
        for(int j = 0; j < picked->w; ++j) {
            if((*data >> 24)) FL_DrawPoint(*x + picked->x_off + j, *y + picked->y_off + i, *data);
            ++data;
        }

        data += (tex->width - picked->w);
    }

    *x += picked->x_adv;
}

void FL_DrawTextFNT(int x, int y, const char *text, int size, int max_width, FL_FontFNT *font) {
    int start_x = x;

    int next_word = 0;
    for(int i = 0; i < size && text[i] != 0; ++i) {
        if(i == next_word && i < size - 1) { // check if word has to be moved to another line
            // search for the next word
            int s = i + 1;
            while(text[s] != 0 && text[s] != ' ' && s != size -1) { s++; }
            
            if(x + ((s - i) * font->size) >= max_width) { // don't count space for render
                x = start_x;
                if(i > 0) y += font->size; // don't move first line if it doesn't fit
            }

            next_word = (s == size - 1 ? -1 : s + 1); // don't count end of text as another word
        } else if(x + font->size >= max_width) { // displace letters in word if they don't fit
            if(text[i] != ' ') {
                x = start_x;
                y += font->size;
            } else continue; // just don't render space if it doesn't fit the line, instead of moving it to another line
        } else if(text[i] == '\n') {
            x = start_x;
            y += font->size;

            continue;
        }

        render_char_fnt(&x, &y, text[i], font);
    }
}

void FL_DrawText(int x, int y, const char *text, int size, int max_width, FL_FontBDF *font) {
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
