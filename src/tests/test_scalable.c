#include "test_scalable.h"

#include "../x11framework.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void test_scalable() {
    if(!XF_Initialize(640, 480))
        exit(-1);

    XF_Texture *test_texture = XF_LoadBMP("data/a_test.bmp");
    if(!test_texture) {
        XF_Close();
        exit(-1);
    }

    XF_FontBDF *knxt_font = XF_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt_font) {
        XF_FreeTexture(test_texture);
        XF_Close();
        exit(-1);
    }

    XF_Event event;

    int width = 128, height = 128;
    int w_velocity = 0, h_velocity = 0;
    
    float x = 0, y = 0;
    float x_velocity = 0, y_velocity = 0;

    char fps_text[16];
    fps_text[0] = 0;
    clock_t fps_timer = clock();

    char circle_data_text[64];
    circle_data_text[0] = 0;

    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {
            if(event.type == XF_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case XF_KEY_w: h_velocity = 1; break;
                    case XF_KEY_s: h_velocity = -1; break;
                    case XF_KEY_a: w_velocity = -1; break;
                    case XF_KEY_d: w_velocity = 1; break;
                    case XF_KEY_i: y_velocity = -0.1f; break;
                    case XF_KEY_k: y_velocity = 0.1f; break;
                    case XF_KEY_j: x_velocity = -0.1f; break;
                    case XF_KEY_l: x_velocity = 0.1f; break;

                    default: break;
                }
            } else if(event.type == XF_EVENT_KEY_RELEASED) {
                switch(event.key.code) {
                    case XF_KEY_w: case XF_KEY_s: h_velocity = 0; break;
                    case XF_KEY_a: case XF_KEY_d: w_velocity = 0; break;
                    case XF_KEY_i: case XF_KEY_k: y_velocity = 0; break;
                    case XF_KEY_j: case XF_KEY_l: x_velocity = 0; break;
                    default: break;
                }
            }
        }

        width += w_velocity;
        if(width < 1) width = 1;

        height += h_velocity;
        if(height < 1) height = 1;

        x += x_velocity;
        y += y_velocity;

        snprintf(circle_data_text, 64, "X: %f Y: %f\nW: %d H: %d", x, y, width, height);

        if((double)(clock() - fps_timer) / CLOCKS_PER_SEC >= 0.2) {
            snprintf(fps_text, 16, "FPS: %.2f", 1000.0 / XF_GetDeltaTime());
            fps_timer = clock();
        }

        XF_ClearScreen();
        XF_DrawTextureScaled(test_texture, x, y, width, height);
        XF_DrawText(0, 0, fps_text, 16, XF_GetWindowWidth(), knxt_font);
        XF_DrawText(0, 20, circle_data_text, 64, XF_GetWindowWidth(), knxt_font);
        XF_Render();
    }

    XF_FreeTexture(test_texture);
    XF_FreeFontBDF(knxt_font);

    XF_Close();
}
