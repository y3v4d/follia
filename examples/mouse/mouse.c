#include "x11framework.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    if(!XF_Initialize(640, 480))
        exit(-1);

    XF_FontBDF *knxt_font = XF_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt_font) {
        XF_Close();
        exit(-1);
    }

    char mouse_info[128];
    snprintf(mouse_info, 128, "Mouse X: - Y: -");

    XF_Event event;

    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {
            if(event.type == XF_EVENT_MOUSE_MOVED || event.type == XF_EVENT_MOUSE_PRESSED || event.type == XF_EVENT_MOUSE_RELEASED) {
                char mtype[32];

                switch(event.type) {
                    case XF_EVENT_MOUSE_MOVED:
                        strncpy(mtype, "XF_EVENT_MOUSE_MOVED", 20);
                        mtype[20] = 0;
                        break;
                    case XF_EVENT_MOUSE_RELEASED:
                        strncpy(mtype, "XF_EVENT_MOUSE_RELEASED", 23);
                        mtype[23] = 0;
                        break;
                    case XF_EVENT_MOUSE_PRESSED:
                        strncpy(mtype, "XF_EVENT_MOUSE_PRESSED", 22);
                        mtype[22] = 0;
                        break;
                }

                snprintf(mouse_info, 128, "Mouse\nX: %d Y: %d\nButton %d\nType: %s", 
                    event.mouse.x, event.mouse.y, event.mouse.button, mtype);
            }
        }

        XF_ClearScreen();
        XF_DrawText(10, 10, mouse_info, 128, XF_GetWindowWidth(), knxt_font);
        XF_Render();
    }

    XF_FreeFontBDF(knxt_font);
    XF_Close();

    return 0;
}
