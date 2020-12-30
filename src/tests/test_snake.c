#include "test_snake.h"

#include "../x11framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct _vector_s {
    int x, y;
} vector_t;

vector_t* create_vector(int x, int y) {
    vector_t *temp = (vector_t*)malloc(sizeof(vector_t));
    temp->x = x;
    temp->y = y;

    return temp;
}

const unsigned MAX_PARTS = 255;
const unsigned FIELD_SIZE = 32;
const unsigned FIELD_W = 20, FIELD_H = 15;

void test_snake() {
    srand(time(NULL));

    if(!XF_Initialize(640, 480))
        exit(-1);

    XF_FontBDF *knxt_font = XF_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt_font) {
        XF_Close();
        exit(-1);
    }
    
    vector_t *parts[MAX_PARTS];
    for(unsigned i = 0; i < MAX_PARTS; ++i) parts[i] = NULL;

    unsigned length = 3;
    for(int i = 0; i < length; ++i) parts[i] = create_vector(length - 1 - i, 0);
    
    vector_t apple = { rand() % FIELD_W, rand() % FIELD_H };
    uint8_t direction = 0; // 0 - R, 1 - L, 2 - U, 3 - D

    char fps_text[16];

    clock_t fps_start, fps_end;
    fps_start = fps_end = clock();

    clock_t prev_c = clock();

    XF_Event event;

    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {
            if(event.type == XF_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case XF_KEY_d: direction = 0; break;
                    case XF_KEY_a: direction = 1; break;
                    case XF_KEY_w: direction = 2; break;
                    case XF_KEY_s: direction = 3; break;
                    default: break; 
                }
            }
        }

        if((double)(clock() - prev_c) / CLOCKS_PER_SEC >= 0.15) {
            // pass the position from the top to the bottom
            for(unsigned i = length - 1; i > 0; --i) {
                parts[i]->x = parts[i - 1]->x;
                parts[i]->y = parts[i - 1]->y;
            }

            switch(direction) {
                case 0: parts[0]->x++; break;
                case 1: parts[0]->x--; break;
                case 2: parts[0]->y--; break;
                case 3: parts[0]->y++; break;
                default: break;
            }

            // teleport parts to the opposite site of the screen if they go past one
            for(unsigned i = 0; i < MAX_PARTS && parts[i]; ++i) {
                if(parts[i]->x >= (int)FIELD_W) parts[i]->x = 0;
                if(parts[i]->x <= -1) parts[i]->x = FIELD_W - 1;
                if(parts[i]->y >= (int)FIELD_H) parts[i]->y = 0;
                else if(parts[i]->y <= -1) parts[i]->y = FIELD_H - 1;
            }

            // check collision with apple
            if(parts[0]->x == apple.x && parts[0]->y == apple.y) {
                if(length < MAX_PARTS) {
                    parts[length] = create_vector(parts[length - 1]->x, parts[length - 1]->y);
                    length++;

                    apple.x = rand() % FIELD_W; apple.y = rand() % FIELD_H;
                }
            }


            prev_c = clock();
        }

        if(fps_end == fps_start || (double)(fps_end - fps_start) / CLOCKS_PER_SEC >= 0.2) {
            snprintf(fps_text, 16, "FPS: %.2f", 1000.0 / XF_GetDeltaTime());

            fps_start = fps_end;
        }
        fps_end = clock();

        XF_ClearScreen();
        XF_DrawRect(apple.x * FIELD_SIZE, apple.y * FIELD_SIZE, FIELD_SIZE, FIELD_SIZE, 0xff0000, false);
        XF_DrawRect(apple.x * FIELD_SIZE, apple.y * FIELD_SIZE, FIELD_SIZE, FIELD_SIZE, 0, true);
        
        for(unsigned i = 0; i < MAX_PARTS && parts[i]; ++i) {
            XF_DrawRect(parts[i]->x * FIELD_SIZE, parts[i]->y * FIELD_SIZE, FIELD_SIZE, FIELD_SIZE, 0x00ff00, false);
            XF_DrawRect(parts[i]->x * FIELD_SIZE, parts[i]->y * FIELD_SIZE, FIELD_SIZE, FIELD_SIZE, 0, true);
        }

        XF_DrawText(10, 10, fps_text, 16, XF_GetWindowWidth(), knxt_font);
        XF_Render();
    }

    // free memory allocated for snake parts
    for(unsigned i = 0; i < MAX_PARTS && parts[i]; ++i)
        free(parts[i]);

    XF_FreeFontBDF(knxt_font);
 
    XF_Close();
}
