#include "checkbox.h"

void draw_checkbox(const struct CheckBox *checkbox) {
    uint32_t color = (checkbox->checked ? 0xff777777 : 0xff666666);

    FL_DrawRect(checkbox->x, checkbox->y, checkbox->w, checkbox->h, color, false);
    FL_DrawRect(checkbox->x, checkbox->y, checkbox->w, checkbox->h, 0, true);

    if(checkbox->checked) {
        FL_DrawLine(checkbox->x + 1, checkbox->y + 1, checkbox->x + checkbox->w - 1, checkbox->y + checkbox->h - 1, 0);
        FL_DrawLine(checkbox->x + 1, checkbox->y + checkbox->h - 1, checkbox->x + checkbox->w - 1, checkbox->y + 1, 0);
    }
}

void process_checkbox(struct CheckBox *checkbox, FL_MouseEvent *event) {
    if(event->type == FL_EVENT_MOUSE_PRESSED) {
        if(event->button == FL_MOUSE_BUTTON_1 && 
           event->x >= checkbox->x && event->x <= checkbox->x + checkbox->w && event->y >= checkbox->y && event->y <= checkbox->y + checkbox->h) {
            checkbox->checked = !checkbox->checked;
        }
    }
}
