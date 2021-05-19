#ifndef _xf_primitives_h_
#define _xf_primitives_h_

#include <stdint.h>
#include "xf_utils.h"

/*
 * All "Draw" functions doesn't render things on screen immediately, it just writes on screen buffer.
 * To display modified screen buffer, you HAVE TO call XF_Render function.
 */
void XF_DrawPoint(int x, int y, uint32_t color);
void XF_DrawLine(int x0, int y0, int x1, int y1, uint32_t color);
void XF_DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color, XF_Bool fill);
void XF_DrawRect(int x, int y, int w, int h, uint32_t color, XF_Bool fill);
void XF_DrawCircle(int x, int y, int r, uint32_t color, XF_Bool fill);
void XF_DrawNoise();

#endif
