#include "selector.h"

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
