#ifndef __FL_PRIMITIVES_H__
#define __FL_PRIMITIVES_H__

#include <stdint.h>
#include "fl_utils.h"

/*
 * All "Draw" functions doesn't render things on screen immediately, it just writes on screen buffer.
 * To display modified screen buffer, you HAVE TO call FL_Render function.
 */
void FL_DrawPoint(int x, int y, uint32_t color);
void FL_DrawLine(int x0, int y0, int x1, int y1, uint32_t color);
void FL_DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color, FL_Bool fill);
void FL_DrawRect(int x, int y, int w, int h, uint32_t color, FL_Bool fill);
void FL_DrawCircle(int x, int y, int r, uint32_t color, FL_Bool fill);
void FL_DrawNoise();

#endif
