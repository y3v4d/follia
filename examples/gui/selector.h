#ifndef selector_h
#define selector_h

#include "x11framework.h"

enum PrimitiveType { PRIMITIVE_LINE = 0, PRIMITIVE_RECT = 1 };

struct PrimitiveSelector {
    float x, y;
    float w, h;

    XF_Bool expanded;
    int current_option;

    XF_Bool recent_change;
};

void initialize_selector(struct PrimitiveSelector *selector);
void process_selector(struct PrimitiveSelector *selector, const XF_MouseEvent *event);
void draw_selector(const struct PrimitiveSelector *selector);

#endif
