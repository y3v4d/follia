#ifndef slider_h
#define slider_h

#include "x11framework.h"

struct Slider {
    float x, y;
    int width;

    float progress;
    FL_Bool focused;
};

void process_slider(struct Slider *slider, const FL_MouseEvent *mouse);
void draw_slider(const struct Slider *slider);

#endif
