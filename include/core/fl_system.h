#ifndef __FL_SYSTEM_H__
#define __FL_SYSTEM_H__

#include "fl_utils.h"
#include "fl_event.h"

#include <stdint.h>

/*
 * FL_Initiailize
 * Creates window, allocates screen buffer with shared memory and intializes all necessary systems.
 *
 * width, height - dimensions of the window
 */
FL_Bool FL_Initialize(int width, int height); // only software renderer
void FL_Close();

FL_Bool FL_WindowShouldClose();

/*
 * FL_GetEvent
 * Get newest event from event queue, process it and writes it to passed FL_Event structure.
 * Function DOES NOT create new FL_Event structure and it HAS TO BE PREVIOUSLY ALLOCATED.
 *
 * event - it will store information about next event (if there was one)
 * 
 * Returns true if there was event in queue, or false if wasn't
 */
FL_Bool FL_GetEvent(FL_Event* event);

/*
 * For now, system won't check if the window change the size.
 */
int FL_GetWindowWidth();
int FL_GetWindowHeight();

/*
 * FL_GetDeltaTime
 * For this to work, it's necessary to run ONE FL_Render function every frame.
 */
double FL_GetDeltaTime();

double FL_GetCoreTimer(uint8_t type);

void FL_SetFrameTime(double ms);

/*
 * FL_SetClearColor
 * For fast implementation, you are allowed to use 8-bit color (white, black, shades of grey)
 *
 * base - between 0 - 255
 */
void FL_SetClearColor(uint8_t base);

void FL_SetTitle(char *name);

/*
 * FL_ClearScreen
 * Clears the screen buffer with gray color (166, 166, 166).
 */
void FL_ClearScreen();
void FL_Render();

#endif
