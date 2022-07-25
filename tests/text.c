#include "follia.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    if(!FL_Initialize(640, 480))
        exit(-1);

    FL_SetTitle("Follia - Text");

    FL_FontBDF* knxt_font = FL_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt_font) {
        FL_Close();
        exit(-1);
    }

    FL_Event event;

    const int TEXT_SIZE = 255;
    char text[TEXT_SIZE];
    text[0] = 0;
    int text_index = 0;

    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {
            if(event.type == FL_EVENT_KEY_PRESSED) {
                if(event.key.code == FL_KEY_BACKSPACE && text_index > 0) {
                    text[--text_index] = '_';
                    text[text_index + 1] = 0;
                } else if(text_index < TEXT_SIZE - 1) {
                    if(event.key.code == FL_KEY_ENTER) text[text_index] = '\n';
                    else text[text_index] = event.key.code;

                    text[++text_index] = '_';
                    text[text_index + 1] = 0;
                }
            }
        }

        FL_ClearScreen();
        FL_DrawText(0, 0, text, 255, FL_GetWindowWidth(), knxt_font);
        FL_Render();
    } 

    FL_FreeFontBDF(knxt_font);
    FL_Close();
    return 0;
}
