#ifndef xf_system_h
#define xf_system_h

#include "xf_utils.h"
#include "xf_log.h"
#include "xf_key.h"
#include "xf_font.h"
#include "xf_sprite.h"

#include <stdint.h>

boolean XF_Initialize(int width, int height); // only software renderer
void XF_Close();

boolean XF_WindowShouldClose();

// convert it to more SDL like style, because it's more clear
// then this (add events queue, to properly handle SHM event, not visible for user)
int XF_ProcessEvents();
boolean XF_isKeyPressed(uint8_t keycode);
boolean XF_isKeyReleased(uint8_t keycode);

int XF_getWindowWidth();
int XF_getWindowHeight();

double XF_getDeltaTime();

void XF_ClearScreen();
void XF_DrawPoint(int x, int y, uint32_t color);
void XF_DrawLine(int x1, int y1, int x2, int y2, uint32_t color);
void XF_DrawRect(int x, int y, int w, int h, uint32_t color, boolean outline);
void XF_Render();

#endif
