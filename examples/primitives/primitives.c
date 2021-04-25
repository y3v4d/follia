#include "x11framework.h"
#include <stdio.h>
#include <stdlib.h>

#define MAIN_SPEED 0.9

enum PrimitiveType { PRIMITIVE_POINT = 0, PRIMITIVE_LINE = 1, PRIMITIVE_RECT = 2 };
struct Primitive {
    float x, y;
    float w, h;
    float vx, vy;

    enum PrimitiveType type;
};

int main() {
    if(!XF_Initialize(640, 480))
        return -1;

    XF_Timer delta_timer;
    XF_StartTimer(&delta_timer);

    XF_FontBDF* knxt = XF_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't load knxt font!\n");
        
        XF_Close();
        return -1;
    }

    char fps_text[64];

    struct Primitive main = { 10, 10, 64, 64, 0, 0, PRIMITIVE_RECT };

    const int TOTAL_PRIMITIVES = 32;

    XF_Event event;
    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {
            if(event.type == XF_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case 'w': main.vy = -MAIN_SPEED; break;
                    case 's': main.vy = MAIN_SPEED; break;
                    case 'a': main.vx = -MAIN_SPEED; break;
                    case 'd': main.vx = MAIN_SPEED; break;
                    default: break;
                }
            } else if(event.type == XF_EVENT_KEY_RELEASED) {
                switch(event.key.code) {
                    case 'w': case 's': main.vy = 0; break;
                    case 'a': case 'd': main.vx = 0; break;
                    default: break;
                }
            }
        }

        main.x += main.vx * XF_GetDeltaTime();
        main.y += main.vy * XF_GetDeltaTime();

        XF_StopTimer(&delta_timer);
        if(delta_timer.delta >= 800) {
            snprintf(fps_text, 64, "MS: %f\nFPS: %f", XF_GetDeltaTime(), 1000.0 / XF_GetDeltaTime());
            XF_StartTimer(&delta_timer);
        }

        XF_ClearScreen();
        XF_DrawRect(main.x, main.y, main.w, main.h, 0x00ff0000, false); 
        XF_DrawText(10, 10, fps_text, 64, XF_GetWindowWidth(), knxt);
        XF_Render();
    }

    XF_FreeFontBDF(knxt);

    XF_Close();
    return 0;
}
