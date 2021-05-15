#include "x11framework.h"
#include "slider.h"
#include "checkbox.h"
#include "selector.h"
#include <stdio.h>
#include <stdlib.h>

#define MAIN_SPEED 0.9

#define GRAVITY_SPEED 0.003
#define GRAVITY_CAP 100

struct Primitive {
    float x, y;
    float w, h;
    float vx, vy;

    uint32_t color;

    enum PrimitiveType type;
};

float absf(float n) {
    return (n >= 0 ? n : -n);
}

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

    struct Primitive main = { 10, 10, 64, 64, 0, 0, 0xffff0000, PRIMITIVE_RECT };
    struct Slider test_slider = { 100, 100, 100, 0.5f, false };

    struct CheckBox test_checkbox = { 200, 200, 24, 24, false };

    struct PrimitiveSelector test_selector = { 300, 300, 50, 30 };
    initialize_selector(&test_selector);

    XF_Texture* instruction_texture = XF_LoadBMP("data/instruction.bmp");
    if(!instruction_texture) {
        XF_FreeFontBDF(knxt);
        XF_Close();

        return -1;
    }

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
            } else if((event.type & XF_EVENT_MOUSE_PREFIX) == XF_EVENT_MOUSE_PREFIX) {
                process_slider(&test_slider, &event.mouse);
                process_checkbox(&test_checkbox, &event.mouse);
                process_selector(&test_selector, &event.mouse);
            }
        }

        main.x = (640 - main.w) * test_slider.progress;

        main.x += main.vx * XF_GetDeltaTime();
        main.y += main.vy * XF_GetDeltaTime();

        if(test_checkbox.checked) main.color = 0xff0000ff;
        else main.color = 0xffff0000;

        if(test_selector.recent_change) main.type = test_selector.current_option;

        XF_StopTimer(&delta_timer);
        if(delta_timer.delta >= 800) {
            snprintf(fps_text, 64, "MS: %f\nFPS: %f", XF_GetDeltaTime(), 1000.0 / XF_GetDeltaTime());
            XF_StartTimer(&delta_timer);
        }

        XF_ClearScreen();

        if(main.type == PRIMITIVE_RECT) XF_DrawRect(main.x, main.y, main.w, main.h, main.color, false); 
        else if(main.type == PRIMITIVE_LINE) XF_DrawLine(main.x, main.y, main.x + main.w, main.y + main.h, main.color);
        XF_DrawCircle(main.x, main.y + 100, 25, main.color, true);
        XF_DrawLine(0, 0, 200, 0, 0xffff0000);

        draw_slider(&test_slider);
        draw_checkbox(&test_checkbox);
        draw_selector(&test_selector);

        XF_DrawText(10, 10, fps_text, 64, XF_GetWindowWidth(), knxt);
        XF_DrawTexture(instruction_texture, XF_GetWindowWidth() - instruction_texture->width, XF_GetWindowHeight() - instruction_texture->height);
        XF_DrawTriangle(200, 300, 400, 300, 400, 200, 0xffff0000, true);
        XF_Render();
    }

    XF_FreeTexture(instruction_texture);
    XF_FreeFontBDF(knxt);

    XF_Close();
    return 0;
}
