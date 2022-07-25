#ifndef checkbox_h
#define checkbox_h

#include "x11framework.h"

struct CheckBox {
    float x, y;
    float w, h;

    FL_Bool checked;
};

void process_checkbox(struct CheckBox *checkbox, FL_MouseEvent *event);
void draw_checkbox(const struct CheckBox *checkbox);

#endif
