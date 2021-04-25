#include "x11framework.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    if(!XF_Initialize(640, 480))
        return -1;

    XF_Timer delta_timer;
    XF_StartTimer(&delta_timer);

    XF_FontBDF* bitocra_39 = XF_LoadFontBDF("data/fonts/bitocra-39.bdf");
    if(!bitocra_39) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't load bitocra font!\n");
        
        XF_Close();
        return -1;
    }

    char fps_text[64];

    XF_Event event;
    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {}

        XF_StopTimer(&delta_timer);
        snprintf(fps_text, 64, "MS: %f\nFPS: %f", XF_GetDeltaTime(), 1000.0 / XF_GetDeltaTime());
        XF_StartTimer(&delta_timer);

        XF_ClearScreen();
        XF_DrawText(10, 10, fps_text, 64, XF_GetWindowWidth(), bitocra_39);
        XF_Render();
    }

    XF_FreeFontBDF(bitocra_39);

    XF_Close();
    return 0;
}
