#ifndef checkbox_h
#define checkbox_h

#include "x11framework.h"

struct CheckBox {
    float x, y;
    float w, h;

    XF_Bool checked;
};

void process_checkbox(struct CheckBox *checkbox, XF_MouseEvent *event);
void draw_checkbox(const struct CheckBox *checkbox);

#endif
