#include "slider.h"

void process_slider(struct Slider *slider, const XF_MouseEvent *mouse) {
    if(mouse->type == XF_EVENT_MOUSE_PRESSED && mouse->button == XF_MOUSE_BUTTON_1 && !slider->focused) {
        int expected_x = slider->x + slider->progress * slider->width;

        if(mouse->x >= expected_x - 5 && mouse->x <= expected_x + 5 && mouse->y >= slider->y - 10 && mouse->y <= slider->y + 10) {
            slider->focused = true;
        }
    } else if(slider->focused) {
        if(mouse->type == XF_EVENT_MOUSE_MOVED && mouse->x >= slider->x && mouse->x <= slider->x + slider->width) {
            slider->progress = (mouse->x - slider->x) / slider->width;
        } else if(mouse->type == XF_EVENT_MOUSE_RELEASED && mouse->button == XF_MOUSE_BUTTON_1) {
            slider->focused = false;
        }
    }
}

void draw_slider(const struct Slider *slider) {
    uint32_t color = (slider->focused ? 0xff444444 : 0xff666666);

    XF_DrawLine(slider->x, slider->y, slider->x + slider->width - 1, slider->y, 0x00000000);
    XF_DrawRect(slider->x + slider->progress * slider->width - 5, slider->y - 10, 10, 20, color, false);
    XF_DrawRect(slider->x + slider->progress * slider->width - 5, slider->y - 10, 10, 20, 0, true);
}
