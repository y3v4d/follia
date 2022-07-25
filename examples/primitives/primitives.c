#include "x11framework.h"
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

    if(!XF_Initialize(640, 480))
        return -1;

    XF_SetTitle("Follia - Primitives");

    XF_Timer delta_timer;
    XF_StartTimer(&delta_timer);

    XF_FontBDF* knxt = XF_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't load knxt font!\n");
        
        XF_Close();
        return -1;
    }

    XF_FontBDF *bitocra_39 = XF_LoadFontBDF("data/fonts/bitocra-39.bdf");
    if(!bitocra_39) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't load bitocra-39 font!\n");
        XF_FreeFontBDF(knxt);

        XF_Close();
        return -1;
    }

    const int TOTAL_RECTS = 800;
    struct Rect rects[TOTAL_RECTS];

    for(int i = 0; i < TOTAL_RECTS; ++i) {
        rects[i].w = rects[i].h = (rand() % 56) + 8;
        rects[i].x = rand() % (XF_GetWindowWidth() - rects[i].w);
        rects[i].y = rand() % (XF_GetWindowHeight() - rects[i].h);

        rects[i].vx = (float)(rand() % 4 + 6) / 10;
        rects[i].vy = (float)(rand() % 4 + 6) / 10;

        rects[i].color = ((rand() % 255) << 16 | (rand() % 255) << 8 | (rand() % 255));
    }

    const int TOTAL_CIRCLES = 100;
    struct Circle* circles[TOTAL_CIRCLES];
    int n_circles = 0;

    for(int i = 0; i < TOTAL_CIRCLES; ++i) circles[i] = NULL;

    int current_test = 0;

    const int fps_text_s = 64;
    char fps_text[fps_text_s];

    const int meassure_text_s = 64;
    char meassure_text[meassure_text_s];
    XF_Timer meassure_timer;

    XF_Event event;
    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {
            if(event.type == XF_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case '1': current_test = 0; break;
                    case '2': current_test = 1; break;
                    case '3': current_test = 2; break;
                    case '4': current_test = 3; break;
                    case '5': current_test = 4; break;
                    default: break;
                }
            } else if(event.type == XF_EVENT_MOUSE_PRESSED) {
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
            }
        }

        if(current_test == 1) {
            for(int i = 0; i < TOTAL_RECTS; ++i) {
                rects[i].x += rects[i].vx * XF_GetDeltaTime();
                rects[i].y += rects[i].vy * XF_GetDeltaTime();

                if(rects[i].x < 0) {
                    rects[i].x = 0;
                    rects[i].vx *= -1;
                } else if(rects[i].x + rects[i].w >= XF_GetWindowWidth()) {
                    rects[i].x = XF_GetWindowWidth() - rects[i].w - 1;
                    rects[i].vx *= -1;
                } 

                if(rects[i].y < 0) {
                    rects[i].y = 0;
                    rects[i].vy *= -1;
                } else if(rects[i].y + rects[i].h >= XF_GetWindowHeight()) {
                    rects[i].y = XF_GetWindowHeight() - rects[i].h - 1;
                    rects[i].vy *= -1;
                }
            }
        }

        XF_StopTimer(&delta_timer);
        if(delta_timer.delta >= 800) {
            snprintf(fps_text, fps_text_s, "MS: %.2f\nFPS: %.2f", XF_GetDeltaTime(), 1000.0 / XF_GetDeltaTime());
            XF_StartTimer(&delta_timer);
        }

        XF_ClearScreen();
        if(current_test == 0) {
            XF_StartTimer(&meassure_timer);
            XF_DrawNoise();
            XF_StopTimer(&meassure_timer);
            snprintf(meassure_text, meassure_text_s, "MS: %f", meassure_timer.delta);
        } else if(current_test == 1) {
            for(int i = 0; i < TOTAL_RECTS; ++i)
                XF_DrawRect(rects[i].x, rects[i].y, rects[i].w, rects[i].h, rects[i].color, true);
        } else if(current_test == 2) {
            for(int i = 0; i < n_circles; ++i) {
                if(i > 0) XF_DrawLine(circles[i - 1]->x, circles[i - 1]->y, circles[i]->x, circles[i]->y, 0x00ff00);
                XF_DrawCircle(circles[i]->x, circles[i]->y, circles[i]->r, circles[i]->color, true);
            }
        }

        XF_DrawText(10, 10, fps_text, fps_text_s, XF_GetWindowWidth(), bitocra_39);
        XF_Render();
    }

    for(int i = 0; i < n_circles; ++i) free(circles[i]);

    XF_FreeFontBDF(bitocra_39);
    XF_FreeFontBDF(knxt);

    XF_Close();
    return 0;
}
