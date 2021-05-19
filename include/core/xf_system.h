#ifndef xf_system_h
#define xf_system_h

#include "xf_utils.h"
#include "xf_event.h"

#include <stdint.h>

/*
 * XF_Initiailize
 * Creates window, allocates screen buffer with shared memory and intializes all necessary systems.
 *
 * width, height - dimensions of the window
 */
XF_Bool XF_Initialize(int width, int height); // only software renderer
void XF_Close();

XF_Bool XF_WindowShouldClose();

/*
 * XF_GetEvent
 * Get newest event from event queue, process it and writes it to passed XF_Event structure.
 * Function DOES NOT create new XF_Event structure and it HAS TO BE PREVIOUSLY ALLOCATED.
 *
 * event - it will store information about next event (if there was one)
 * 
 * Returns true if there was event in queue, or false if wasn't
 */
XF_Bool XF_GetEvent(XF_Event* event);

/*
 * For now, system won't check if the window change the size.
 */
int XF_GetWindowWidth();
int XF_GetWindowHeight();

/*
 * XF_GetDeltaTime
 * For this to work, it's necessary to run ONE XF_Render function every frame.
 */
double XF_GetDeltaTime();

/*
 * XF_SetClearColor
 * For fast implementation, you are allowed to use 8-bit color (white, black, shades of grey)
 *
 * base - between 0 - 255
 */
void XF_SetClearColor(uint8_t base);

/*
 * XF_ClearScreen
 * Clears the screen buffer with gray color (166, 166, 166).
 */
void XF_ClearScreen();
void XF_Render();

#endif
