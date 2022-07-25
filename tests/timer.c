#include "follia.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    if(!FL_Initialize(640, 480))
        return -1;

    FL_SetTitle("Follia - Timer");

    FL_Timer timer;
    FL_StartTimer(&timer);

    FL_FontBDF* bitocra_39 = FL_LoadFontBDF("data/fonts/bitocra-39.bdf");
    if(!bitocra_39) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't load bitocra font!\n");
        
        FL_Close();
        return -1;
    }

    char fps_text[64];

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {}

        FL_StopTimer(&timer);
        if(timer.delta >= 500) {
            snprintf(fps_text, 64, "MS: %.4f\nFPS: %.4f", FL_GetDeltaTime(), 1000.0 / FL_GetDeltaTime());
            FL_StartTimer(&timer);
        }

        FL_ClearScreen();
        FL_DrawText(10, 10, fps_text, 64, FL_GetWindowWidth(), bitocra_39);
        FL_Render();
    }

    FL_FreeFontBDF(bitocra_39);

    FL_Close();
    return 0;
}
