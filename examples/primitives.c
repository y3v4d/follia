#include "follia.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct Rect {
    float x, y;
    int w, h;

    float vx, vy;

    uint32_t color;
};

struct Circle {
    float x, y;
    int r;

    uint32_t color;
};

int main() {
    srand(time(NULL));

    if(!FL_Initialize(640, 480))
        return -1;

    FL_SetTitle("Follia - Primitives");

    FL_Timer delta_timer;
    FL_StartTimer(&delta_timer);

    FL_FontBDF* knxt = FL_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't load knxt font!\n");
        
        FL_Close();
        return -1;
    }

    FL_FontBDF *bitocra_39 = FL_LoadFontBDF("data/fonts/bitocra-39.bdf");
    if(!bitocra_39) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't load bitocra-39 font!\n");
        FL_FreeFontBDF(knxt);

        FL_Close();
        return -1;
    }

    const int TOTAL_RECTS = 800;
    struct Rect rects[TOTAL_RECTS];

    for(int i = 0; i < TOTAL_RECTS; ++i) {
        rects[i].w = rects[i].h = (rand() % 56) + 8;
        rects[i].x = rand() % (FL_GetWindowWidth() - rects[i].w);
        rects[i].y = rand() % (FL_GetWindowHeight() - rects[i].h);

        rects[i].vx = (float)(rand() % 4 + 6) / 10;
        rects[i].vy = (float)(rand() % 4 + 6) / 10;

        rects[i].color = ((rand() % 255) << 16 | (rand() % 255) << 8 | (rand() % 255));
    }

    const int TOTAL_CIRCLES = 100;
    struct Circle* circles[TOTAL_CIRCLES];
    int n_circles = 0;

    for(int i = 0; i < TOTAL_CIRCLES; ++i) circles[i] = NULL;

    int current_test = 0;

    struct Rect rect = {
        .x = FL_GetWindowWidth() >> 1,
        .y = FL_GetWindowHeight() >> 1,
        .w = 16,
        .h = 8,
        .color = 0
    };

    const int fps_text_s = 64;
    char fps_text[fps_text_s];

    const int meassure_text_s = 64;
    char meassure_text[meassure_text_s];
    FL_Timer meassure_timer;

    const int info_text_s = 64;
    char info_text[64] = "X: - Y: - ";

    int mouse_x = 0, mouse_y = 0;

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {
            if(event.type == FL_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case '1': current_test = 0; break;
                    case '2': current_test = 1; break;
                    case '3': current_test = 2; break;
                    case '4': current_test = 3; break;
                    case '5': current_test = 4; break;
                    default: break;
                }

                if(current_test == 3) {
                    if(event.key.code == 'w') {
                        rect.w += 1;
                        rect.h += 1;
                    } else if(event.key.code == 's' && rect.w > 0) {
                        rect.w -= 1;
                        rect.h -= 1;
                    }
                }
            } else if(event.type == FL_EVENT_MOUSE_PRESSED) {
                if(current_test == 2) {
                    if(n_circles < TOTAL_CIRCLES) {
                        circles[n_circles] = (struct Circle*)malloc(sizeof(struct Circle));
                        circles[n_circles]->x = event.mouse.x;
                        circles[n_circles]->y = event.mouse.y;
                        circles[n_circles]->r = 8;

                        circles[n_circles]->color = 0xff0000;

                        ++n_circles;
                    }
                }
            } else if(event.type == FL_EVENT_MOUSE_MOVED) {
                mouse_x = event.mouse.x;
                mouse_y = event.mouse.y;
            }
        }

        if(current_test == 1) {
            for(int i = 0; i < TOTAL_RECTS; ++i) {
                rects[i].x += rects[i].vx * FL_GetDeltaTime();
                rects[i].y += rects[i].vy * FL_GetDeltaTime();

                if(rects[i].x < 0) {
                    rects[i].x = 0;
                    rects[i].vx *= -1;
                } else if(rects[i].x + rects[i].w >= FL_GetWindowWidth()) {
                    rects[i].x = FL_GetWindowWidth() - rects[i].w - 1;
                    rects[i].vx *= -1;
                } 

                if(rects[i].y < 0) {
                    rects[i].y = 0;
                    rects[i].vy *= -1;
                } else if(rects[i].y + rects[i].h >= FL_GetWindowHeight()) {
                    rects[i].y = FL_GetWindowHeight() - rects[i].h - 1;
                    rects[i].vy *= -1;
                }
            }
        } else if(current_test == 3) {
            snprintf(info_text, info_text_s, "X: %d Y: %d", mouse_x, mouse_y);
        }

        FL_StopTimer(&delta_timer);
        if(delta_timer.delta >= 800) {
            snprintf(fps_text, fps_text_s, "MS: %.2f\nFPS: %.2f", FL_GetDeltaTime(), 1000.0 / FL_GetDeltaTime());
            FL_StartTimer(&delta_timer);
        }

        FL_ClearScreen();
        if(current_test == 0) {
            FL_StartTimer(&meassure_timer);
            FL_DrawNoise();
            FL_StopTimer(&meassure_timer);
            snprintf(meassure_text, meassure_text_s, "MS: %f", meassure_timer.delta);
        } else if(current_test == 1) {
            for(int i = 0; i < TOTAL_RECTS; ++i)
                FL_DrawRect(rects[i].x, rects[i].y, rects[i].w, rects[i].h, rects[i].color, true);
        } else if(current_test == 2) {
            for(int i = 0; i < n_circles; ++i) {
                if(i > 0) FL_DrawLine(circles[i - 1]->x, circles[i - 1]->y, circles[i]->x, circles[i]->y, 0x00ff00);
                FL_DrawCircle(circles[i]->x, circles[i]->y, circles[i]->r, circles[i]->color, true);
            }
        } else if(current_test == 3) {
            FL_DrawRect(rect.x, rect.y, rect.w, rect.h, rect.color, false);
            FL_DrawTextBDF(10, 86, info_text, info_text_s, FL_GetWindowWidth(), bitocra_39);
            FL_DrawLine(mouse_x, 0, mouse_x, FL_GetWindowHeight(), 0xffff00);
            FL_DrawLine(0, mouse_y, FL_GetWindowWidth(), mouse_y, 0xffff00);
        }

        FL_DrawTextBDF(10, 10, fps_text, fps_text_s, FL_GetWindowWidth(), bitocra_39);
        FL_Render();
    }

    for(int i = 0; i < n_circles; ++i) free(circles[i]);

    FL_FreeFontBDF(bitocra_39);
    FL_FreeFontBDF(knxt);

    FL_Close();
    return 0;
}
