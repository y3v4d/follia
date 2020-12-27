#ifndef xf_system_h
#define xf_system_h

#include "xf_utils.h"
#include "xf_event.h"

#include <stdint.h>

boolean XF_Initialize(int width, int height); // only software renderer
void XF_Close();

boolean XF_WindowShouldClose();

boolean XF_GetEvent(XF_Event* event);

int XF_GetWindowWidth();
int XF_GetWindowHeight();

double XF_GetDeltaTime();

void XF_ClearScreen();
void XF_DrawPoint(int x, int y, uint32_t color);
void XF_DrawLine(int x1, int y1, int x2, int y2, uint32_t color); // to upgrade
void XF_DrawRect(int x, int y, int w, int h, uint32_t color, boolean outline); // to upgrade
void XF_Render();

#endif
