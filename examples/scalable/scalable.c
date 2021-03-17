#include "x11framework.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main() {
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

    float radius = 64;
    float radius_speed = 0.05;
    
    float x = XF_GetWindowWidth() / 2 - radius, y = XF_GetWindowHeight() / 2 - radius;
    float x_velocity = 0.2, y_velocity = 0.1;

    char fps_text[16];
    fps_text[0] = 0;
    clock_t fps_timer = clock();

    char circle_data_text[64];
    circle_data_text[0] = 0;
    
    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {}

        x += x_velocity;
        y += y_velocity;

        radius += radius_speed;
        if(radius >= 80 || radius <= 40) radius_speed *= -1;
        
        if(x + radius > XF_GetWindowWidth()) {
            x = XF_GetWindowWidth() - radius;
            x_velocity *= -1;
        } else if(x - radius < 0) {
            x = radius;
            x_velocity *= -1;
        }

        if(y + radius > XF_GetWindowHeight()) {
            y = XF_GetWindowHeight() - radius;
            y_velocity *= -1;
        } else if(y - radius < 0) {
            y = radius;
            y_velocity *= -1;
        }

        snprintf(circle_data_text, 64, "X: %.2f Y: %.2f\nRadius: %.2f\n", x, y, radius);

        if((double)(clock() - fps_timer) >= 0.2 * CLOCKS_PER_SEC) {
            snprintf(fps_text, 16, "FPS: %.2f", 1000.0 / XF_GetDeltaTime());
            fps_timer = clock();
        }

        XF_ClearScreen();
        XF_DrawTextureScaled(test_texture, x - radius, y - radius, radius * 2, radius * 2);
        XF_DrawText(0, 0, fps_text, 16, XF_GetWindowWidth(), knxt_font);
        XF_DrawText(0, 20, circle_data_text, 64, XF_GetWindowWidth(), knxt_font);
        XF_Render();
    }

    XF_FreeTexture(test_texture);
    XF_FreeFontBDF(knxt_font);
    XF_Close();

    return 0;
}
