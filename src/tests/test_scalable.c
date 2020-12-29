#include "test_scalable.h"

#include "../x11framework.h"
#include <stdlib.h>

void test_scalable() {
    if(!XF_Initialize(640, 480))
        exit(-1);

    XF_Texture *test_texture = XF_LoadBMP("data/a_test.bmp");
    if(!test_texture) {
        XF_Close();
        exit(-1);
    }

    XF_Event event;

    int width = 64, height = 64;
    int w_velocity = 0, h_velocity = 0;

    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {
            if(event.type == XF_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case XF_KEY_w: h_velocity = 1; break;
                    case XF_KEY_s: h_velocity = -1; break;
                    case XF_KEY_a: w_velocity = -1; break;
                    case XF_KEY_d: w_velocity = 1; break;
                    default: break;
                }
            } else if(event.type == XF_EVENT_KEY_RELEASED) {
                switch(event.key.code) {
                    case XF_KEY_w: case XF_KEY_s: h_velocity = 0; break;
                    case XF_KEY_a: case XF_KEY_d: w_velocity = 0; break;
                    default: break;
                }
            }
        }

        width += w_velocity;
        height += h_velocity;

        XF_ClearScreen();
        XF_DrawTextureScaled(test_texture, 0, 0, width, height);
        XF_Render();
    }

    XF_FreeTexture(test_texture);

    XF_Close();
}
