#include "follia.h"

int main() {
    if(!FL_Initialize(640, 480))
        return -1;

    FL_SetTitle("Follia - Texture");

    FL_Texture *texture_rgb = FL_LoadTexture("data/rgb_texture.bmp");
    if(!texture_rgb) {
        FL_Close();
        return -1;
    }

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {}

        FL_ClearScreen();
        uint32_t *p = texture_rgb->data;
        for(int y = 0; y < texture_rgb->height; ++y) {
            for(int x = 0; x < texture_rgb->width; ++x) {
                FL_DrawPoint(100 + x, 100 + y, 0xff0000);
                ++p;
            }
        }
        FL_DrawTexture(texture_rgb, 200, 100);
        FL_Render();
    }

    FL_FreeTexture(texture_rgb);
    FL_Close();
    return 0;
}
