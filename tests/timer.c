#include "core/fl_system.h"
#include "core/fl_timer.h"
#include "follia.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_STRING 512
#define AVERAGE_COUNT 16

double average_array(double *a, int p) {
    double total = 0;
    for(int i = 0; i < p; ++i) {
        total += a[i];
    }

    return total / p;
}

int main() {
    int averages_counter = 0;
    double averages[FL_TIMER_ALL + 1][AVERAGE_COUNT];

    if(!FL_Initialize(640, 480))
        return -1;

    FL_SetTitle("Follia - Timer");
    FL_SetFrameTime(16.6);

    FL_FontBDF* font = FL_LoadFontBDF("data/fonts/knxt.bdf");
    if(!font) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't load bitocra font!\n");
        
        FL_Close();
        return -1;
    }

    char info_text[MAX_STRING];

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {}

        averages[FL_TIMER_CLEAR_SCREEN][averages_counter] = FL_GetCoreTimer(FL_TIMER_CLEAR_SCREEN);
        averages[FL_TIMER_RENDER][averages_counter] = FL_GetCoreTimer(FL_TIMER_RENDER);
        averages[FL_TIMER_CLEAR_TO_RENDER][averages_counter] = FL_GetCoreTimer(FL_TIMER_CLEAR_TO_RENDER);
        averages[FL_TIMER_ALL][averages_counter] = FL_GetCoreTimer(FL_TIMER_ALL);
        ++averages_counter;

        if(averages_counter >= AVERAGE_COUNT) averages_counter = 0;

        snprintf(
            info_text, MAX_STRING, 
            "Clear: %f\nRender: %f\nClear to render: %f\nAll: %f\nDelta: %f\nFPS: %f",
            average_array(averages[FL_TIMER_CLEAR_SCREEN], AVERAGE_COUNT),
            average_array(averages[FL_TIMER_RENDER], AVERAGE_COUNT),
            average_array(averages[FL_TIMER_CLEAR_TO_RENDER], AVERAGE_COUNT),
            average_array(averages[FL_TIMER_ALL], AVERAGE_COUNT),
            FL_GetDeltaTime(),
            1000.0 / FL_GetDeltaTime()
        );
            
        FL_ClearScreen();
        FL_DrawNoise();
        FL_DrawRect(0, 0, 280, 150, 0xffffff, true);
        FL_DrawTextBDF(8, 8, info_text, MAX_STRING, FL_GetWindowWidth() - 16, font);
        FL_Render();
    }

    FL_FreeFontBDF(font);

    FL_Close();
    return 0;
}
