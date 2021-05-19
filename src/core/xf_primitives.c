#include "core/xf_primitives.h"
#include "frame_buffer.h"

#include <stdlib.h>

static inline void _draw_point_in_range(int x, int y, uint32_t color) {
    if(x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT) *(h_lines[y] + x) = color;
}

void XF_DrawPoint(int x, int y, uint32_t color) {
    *(h_lines[y] + x) = color;
}

// only for x0 < x1 !!!
void _plot_line_low(int x0, int y0, int x1, int y1, uint32_t color) {
    const int dx = x1 - x0;
    int dy = y1 - y0;

    int yi = 1;

    // determinate if line is going up or down
    if(dy < 0) {
        yi = -1;
        dy = -dy;
    }

    int d = 2 * dy - dx; // initial value D = A + B / 2, where A = dy B = -dy
                         // to delete the need for division, just multiplicate by 2

    int cy = y0;

    for(int i = x0; i <= x1; ++i) {
        _draw_point_in_range(i, cy, color);

        if(d > 0) {
            cy += yi;
            d += 2 * (dy - dx);
        } else d += 2 * dy;
    }
}

// only for y0 < y1 !!!
void _plot_line_high(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 - x0;
    const int dy = y1 - y0;

    int xi = 1;

    if(dx < 0) {
        xi = -1;
        dx = -dx;
    }

    int d = 2 * dx - dy; // D = A + B / 2, where A = dx B = -dy

    int cx = x0;

    for(int i = y0; i <= y1; ++i) {
        _draw_point_in_range(cx, i, color);

        if(d > 0) {
            cx += xi;
            d += 2 * (dx - dy);
        } else d += 2 * dx;
    }
}

void XF_DrawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    // fast implementation for straight lines
    if(y0 == y1) {
        int sx = (x0 < x1 ? 1 : -1);
        while(x0 != x1) {
            _draw_point_in_range(x0, y0, color);
            x0 += sx;
        }

        return;
    }
    if(x0 == x1) {
        int sy = (y0 < y1 ? 1 : -1);
        while(y0 != y1) {
            _draw_point_in_range(x0, y0, color);
            y0 += sy;
        }

        return;
    }

    // using bresenham algorithm
    if(abs(x1 - x0) > abs(y1 - y0)) {
        if(x0 < x1) _plot_line_low(x0, y0, x1, y1, color);
        else _plot_line_low(x1, y1, x0, y0, color);
    } else {
        if(y0 < y1) _plot_line_high(x0, y0, x1, y1, color);
        else _plot_line_high(x1, y1, x0, y0, color);
    }
}

void XF_DrawRect(int x, int y, int w, int h, uint32_t color, XF_Bool fill) {
    // RANGE CHECK
    if(x >= WINDOW_WIDTH || y >= WINDOW_HEIGHT || x + w <= 0 || y + h <= 0) return;

    if(x < 0) {
        w += x;
        x = 0;
    }
    if(y < 0) {
        h += y;
        y = 0;
    }

    if(y + h >= WINDOW_HEIGHT) {
        h -= (y + h) - WINDOW_HEIGHT;
    }
    if(x + w >= WINDOW_WIDTH) {
        w -= (x + w) - WINDOW_WIDTH;
    }

    if(fill) {
        uint32_t *s = h_lines[y] + x;

        int hz_count = 0;

        while(h--) {
            hz_count = w;

            while(hz_count--) {
                *s++ = color;
            }

            s += WINDOW_WIDTH - w;
        }
    } else {
        XF_DrawLine(x, y, x + w - 1, y, color);
        XF_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
        XF_DrawLine(x, y, x, y + h - 1, color);
        XF_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    }
}

// points are sorted from far left point to far right
void _draw_sorted_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color, XF_Bool fill) {
    // RANGE CHECK
    if(x2 < 0 || x0 >= WINDOW_WIDTH || y0 < 0 || y1 < 0 || y2 < 0 || y0 >= WINDOW_HEIGHT || y1 >= WINDOW_HEIGHT || y2 >= WINDOW_HEIGHT)
        return;

    // bresenham algorithm, for one thread
    int dx_0 = x1 - x0; // v0 -> v1
    int dx_1 = x2 - x0; // v0 -> v2

    int dy_0 = -abs(y1 - y0);
    int dy_1 = -abs(y2 - y0);

    int sy_0 = (y0 < y1 ? 1 : -1);
    int sy_1 = (y0 < y2 ? 1 : -1);

    int err_0 = dx_0 + dy_0;
    int err_1 = dx_1 + dy_1;

    int x = x0;

    int y_0 = y0;
    int y_1 = y0;

    while(true) {
        _draw_point_in_range(x, y_1, color);

        if(x == x2 && y_1 == y2) break;

        int e2_1 = err_1 * 2;
        if(e2_1 <= dx_1) { // e_xy + e_y < 0
            err_1 += dx_1;
            y_1 += sy_1;
        }
        if(e2_1 >= dy_1) { // e_xy + e_x > 0
            err_1 += dy_1;

            while(true) {
                if(x == x2 && y_0 == y2) break;

                int e2_0 = err_0 * 2;
                if(e2_0 <= dx_0) {
                    err_0 += dx_0;
                    y_0 += sy_0;
                }
                if(e2_0 >= dy_0) {
                    err_0 += dy_0;

                    ++x;
                    if(x == x1 && y_0 == y1) { // change to v1 -> v2
                        dx_0 = x2 - x1;
                        dy_0 = -abs(y2 - y1);

                        sy_0 = (y1 < y2 ? 1 : -1);

                        err_0 = dx_0 + dy_0;
                    } else {
                        _draw_point_in_range(x, y_0, color);
                        break;
                    }
                }

                _draw_point_in_range(x, y_0, color);
            }

            if(fill) XF_DrawLine(x, y_0, x, y_1, color);
        }
    }
}

void XF_DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color, XF_Bool fill) {
    // determinate far left point
    if(x0 > x1 || x0 > x2) {
        if(x1 < x2) {
            if(x0 < x2) _draw_sorted_triangle(x1, y1, x0, y0, x2, y2, color, fill);
            else _draw_sorted_triangle(x1, y1, x2, y2, x0, y0, color, fill);
        }
        else {
            if(x0 < x1) _draw_sorted_triangle(x2, y2, x0, y0, x1, y2, color, fill);
            else _draw_sorted_triangle(x2, y2, x1, y1, x0, y1, color, fill);
        }
    } else {
        if(x1 < x2) _draw_sorted_triangle(x0, y0, x1, y1, x2, y2, color, fill);
        else _draw_sorted_triangle(x0, y0, x2, y2, x1, y1, color, fill);
    }
}

void XF_DrawCircle(int x, int y, int r, uint32_t color, XF_Bool fill) {
    // RANGE CHECK
    if(x + r < 0 || x - r >= WINDOW_WIDTH || y + r < 0 || y - r >= WINDOW_HEIGHT) return;

    // using bresenham circle algorithm (int)
    int x_change = 1 - 2 * r; // 1 - 2 * xn
    int y_change = 1;         // 2 * yn + 1
    int radius_error = 0;     // xn^2 + yn^2 - r^2

    int px = r;
    int py = 0;

    while (px >= py) {
        // plot
        _draw_point_in_range(x + px, y - py, color);
        _draw_point_in_range(x + py, y - px, color);
        _draw_point_in_range(x - py, y - px, color);
        _draw_point_in_range(x - px, y - py, color);
        _draw_point_in_range(x - px, y + py, color);
        _draw_point_in_range(x - py, y + px, color);
        _draw_point_in_range(x + py, y + px, color);
        _draw_point_in_range(x + px, y + py, color);

        if(fill) {
            // fill
            for(int i = x - px + 1; i < x + px; ++i) {
                _draw_point_in_range(i, y - py, color);
                _draw_point_in_range(i, y + py, color);
            }

            for(int i = x - py + 1; i < x + py; ++i) {
                _draw_point_in_range(i, y - px, color);
                _draw_point_in_range(i, y + px, color);
            }
        }

        ++py;
        radius_error += y_change;
        y_change += 2;

        // determinate if x needs to be changed, eq: 2 * [xn^2 + yn^2 - r^2 + (2 * yn + 1)] + (1 - 2 * xn) > 0
        if(2 * radius_error + x_change > 0) {
            --px;
            radius_error += x_change;

            x_change += 2;
        }
    }
}

void XF_DrawNoise() {
    uint32_t *s = h_lines[0];
    int range = WINDOW_WIDTH * WINDOW_HEIGHT;

    while(range--) {
        *s++ = (rand() % 255 << 16 | rand() % 255 << 8 | rand() % 255);
    }
}
