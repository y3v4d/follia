#ifndef xf_frame_buffer_h
#define xf_frame_buffer_h

#include <stdint.h>

extern uint32_t **h_lines;

inline void fast_pixel_plot(int x, int y, uint32_t color) {
    *(h_lines[y] + x) = color;
}

#endif
