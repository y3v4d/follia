#ifndef slider_h
#define slider_h

#include "x11framework.h"

struct Slider {
    float x, y;
    int width;

    float progress;
    XF_Bool focused;
};

void process_slider(struct Slider* slider, const XF_MouseEvent* mouse);
void draw_slider(const struct Slider* slider);

#endif
