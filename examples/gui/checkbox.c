#include "checkbox.h"

void draw_checkbox(const struct CheckBox *checkbox) {
    uint32_t color = (checkbox->checked ? 0xff777777 : 0xff666666);

    XF_DrawRect(checkbox->x, checkbox->y, checkbox->w, checkbox->h, color, false);
    XF_DrawRect(checkbox->x, checkbox->y, checkbox->w, checkbox->h, 0, true);

    if(checkbox->checked) {
        XF_DrawLine(checkbox->x + 1, checkbox->y + 1, checkbox->x + checkbox->w - 1, checkbox->y + checkbox->h - 1, 0);
        XF_DrawLine(checkbox->x + 1, checkbox->y + checkbox->h - 1, checkbox->x + checkbox->w - 1, checkbox->y + 1, 0);
    }
}

void process_checkbox(struct CheckBox *checkbox, XF_MouseEvent *event) {
    if(event->type == XF_EVENT_MOUSE_PRESSED) {
        if(event->button == XF_MOUSE_BUTTON_1 && 
           event->x >= checkbox->x && event->x <= checkbox->x + checkbox->w && event->y >= checkbox->y && event->y <= checkbox->y + checkbox->h) {
            checkbox->checked = !checkbox->checked;
        }
    }
}
