#include "follia.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    if(!FL_Initialize(640, 480))
        exit(-1);

    FL_SetTitle("Follia - Mouse");

    FL_FontBDF *knxt_font = FL_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt_font) {
        FL_Close();
        exit(-1);
    }

    char mouse_info[128];
    snprintf(mouse_info, 128, "Mouse X: - Y: -");

    FL_Event event;

    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {
            if(event.type == FL_EVENT_MOUSE_MOVED || event.type == FL_EVENT_MOUSE_PRESSED || event.type == FL_EVENT_MOUSE_RELEASED) {
                char mtype[32];

                switch(event.type) {
                    case FL_EVENT_MOUSE_MOVED:
                        strncpy(mtype, "FL_EVENT_MOUSE_MOVED", 20);
                        mtype[20] = 0;
                        break;
                    case FL_EVENT_MOUSE_RELEASED:
                        strncpy(mtype, "FL_EVENT_MOUSE_RELEASED", 23);
                        mtype[23] = 0;
                        break;
                    case FL_EVENT_MOUSE_PRESSED:
                        strncpy(mtype, "FL_EVENT_MOUSE_PRESSED", 22);
                        mtype[22] = 0;
                        break;
                }

                snprintf(mouse_info, 128, "Mouse\nX: %d Y: %d\nButton %d\nType: %s", 
                    event.mouse.x, event.mouse.y, event.mouse.button, mtype);
            }
        }

        FL_ClearScreen();
        FL_DrawTextBDF(10, 10, mouse_info, 128, FL_GetWindowWidth(), knxt_font);
        FL_Render();
    }

    FL_FreeFontBDF(knxt_font);
    FL_Close();

    return 0;
}
