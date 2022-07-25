#include "x11framework.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    if(!XF_Initialize(640, 480))
        exit(-1);

    XF_SetTitle("Follia - Text");

    XF_FontBDF* knxt_font = XF_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt_font) {
        XF_Close();
        exit(-1);
    }

    XF_Event event;

    const int TEXT_SIZE = 255;
    char text[TEXT_SIZE];
    text[0] = 0;
    int text_index = 0;

    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {
            if(event.type == XF_EVENT_KEY_PRESSED) {
                if(event.key.code == XF_KEY_BACKSPACE && text_index > 0) {
                    text[--text_index] = '_';
                    text[text_index + 1] = 0;
                } else if(text_index < TEXT_SIZE - 1) {
                    if(event.key.code == XF_KEY_ENTER) text[text_index] = '\n';
                    else text[text_index] = event.key.code;

                    text[++text_index] = '_';
                    text[text_index + 1] = 0;
                }
            }
        }

        XF_ClearScreen();
        XF_DrawText(0, 0, text, 255, XF_GetWindowWidth(), knxt_font);
        XF_Render();
    } 

    XF_FreeFontBDF(knxt_font);
    XF_Close();
    return 0;
}
