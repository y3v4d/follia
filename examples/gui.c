#include "follia.h"

enum PrimitiveType { PRIMITIVE_LINE = 0, PRIMITIVE_RECT = 1 };

struct PrimitiveSelector {
    float x, y;
    float w, h;

    FL_Bool expanded;
    int current_option;

    FL_Bool recent_change;
};

struct CheckBox {
    float x, y;
    float w, h;

    FL_Bool checked;
};

struct Slider {
    float x, y;
    int width;

    float progress;
    FL_Bool focused;
};

void process_slider(struct Slider *slider, const FL_MouseEvent *mouse) {
    if(mouse->type == FL_EVENT_MOUSE_PRESSED && mouse->button == FL_MOUSE_BUTTON_1 && !slider->focused) {
        int expected_x = slider->x + slider->progress * slider->width;

        if(mouse->x >= expected_x - 5 && mouse->x <= expected_x + 5 && mouse->y >= slider->y - 10 && mouse->y <= slider->y + 10) {
            slider->focused = true;
        }
    } else if(slider->focused) {
        if(mouse->type == FL_EVENT_MOUSE_MOVED && mouse->x >= slider->x && mouse->x <= slider->x + slider->width) {
            slider->progress = (mouse->x - slider->x) / slider->width;
        } else if(mouse->type == FL_EVENT_MOUSE_RELEASED && mouse->button == FL_MOUSE_BUTTON_1) {
            slider->focused = false;
        }
    }
}

void draw_slider(const struct Slider *slider) {
    uint32_t color = (slider->focused ? 0xff444444 : 0xff666666);

    FL_DrawLine(slider->x, slider->y, slider->x + slider->width - 1, slider->y, 0x00000000);
    FL_DrawRect(slider->x + slider->progress * slider->width - 5, slider->y - 10, 10, 20, color, false);
    FL_DrawRect(slider->x + slider->progress * slider->width - 5, slider->y - 10, 10, 20, 0, true);
}

FL_Bool check_collision(int rx, int ry, int rw, int rh, int px, int py) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

void initialize_selector(struct PrimitiveSelector *selector) {
    selector->expanded = false;
    selector->current_option = PRIMITIVE_RECT;

    selector->recent_change = false;
}

void process_selector(struct PrimitiveSelector *selector, const FL_MouseEvent *event) {
    selector->recent_change = false;

    if(event->type == FL_EVENT_MOUSE_PRESSED) {
        if(!selector->expanded && check_collision(selector->x, selector->y, selector->w, selector->h, event->x, event->y)) {
            selector->expanded = true;
        } else if(selector->expanded) {
            int choice = -1;
            const int OPTIONS = 2;

            for(int i = 1; i <= OPTIONS; ++i) {
                const int new_y = selector->y + selector->h * i;
                if(check_collision(selector->x, new_y, selector->w, selector->h, event->x, event->y)) choice = i - 1;
            }

            if(choice != -1) {
                if(choice != selector->current_option) selector->recent_change = true;
                selector->current_option = choice;
            }

            selector->expanded = false;
        }
    }
}

void _draw_option(const struct PrimitiveSelector *selector, int option, int place) {
    const int start_y = selector->y + place * selector->h;

    FL_DrawRect(selector->x, start_y, selector->w, selector->h, 0xff666666, false);
    FL_DrawRect(selector->x, start_y, selector->w, selector->h, 0, true);
     
    const int figure_width = (selector->h < selector->w ? selector->h * 0.6 : selector->w * 0.6);
    const int center_x = selector->x + selector->w / 2;
    const int center_y = start_y + selector->h / 2;

    switch(option) {
        case PRIMITIVE_LINE:
            FL_DrawLine(center_x - figure_width / 2, center_y - figure_width / 2, center_x + figure_width / 2, center_y + figure_width / 2, 0xff00ff00);
            break;
        case PRIMITIVE_RECT: 
            FL_DrawRect(center_x - figure_width / 2, center_y - figure_width / 2, figure_width, figure_width, 0xffff0000, false);  
            break;
        default: break;
    }
}

void draw_selector(const struct PrimitiveSelector *selector) {
    _draw_option(selector, selector->current_option, 0);

    if(selector->expanded) {
        for(int i = 0; i < 2; ++i) {
            _draw_option(selector, i, i + 1);
        }
    }
}

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

int main() {
    return 0;
}
