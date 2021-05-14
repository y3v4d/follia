#include "core/xf_system.h"
#include "core/xf_log.h"
#include "core/xf_timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xproto.h>
// using shm functions instead of regular is faster (software rendering)
// UPDATE (25.04.2021)
// regular XPutImage function seems to be much faster and stable on modern systems (don't use shm on default)
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
// better keyboard support
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>

#define X_USE_SHM 0

short WINDOW_WIDTH = 100, WINDOW_HEIGHT = 100;

// linux specific
Display *x_display = NULL;
int x_screen = 0;
Window x_window;
GC x_gc;
Visual *x_visual = NULL;
XEvent x_event;

XImage *x_buffer = NULL;
uint32_t **h_lines = NULL;
XShmSegmentInfo shm_info;

int x_shm_completion; // determinate what's the type of shm completion event
XF_Bool shm_complete = true; // determinate if the shm finished its job

Atom wm_delete_window;

XF_Bool x_window_close = false; // determinate if window should be closed

int x_error_handler(Display *display, XErrorEvent *event) {
    char msg[256];

    XGetErrorText(display, event->error_code, msg, 256);

    XF_WriteLog(XF_LOG_ERROR, msg);

    return 0;
}

XF_Bool XF_Initialize(int width, int height) {
    if(!XF_InitializeLog()) {
        printf("[ERROR] No messages will be written to log file\n");
    }

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    XF_WriteLog(XF_LOG_INFO, "Initializing XF system...\n");

    x_display = XOpenDisplay(NULL);
    if(!x_display) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't open display\n");
        return false;
    }
    XF_WriteLog(XF_LOG_INFO, "Successfully opened display\n");

    char *x_display_name = XDisplayString(x_display);
    XF_WriteLog(XF_LOG_INFO, "Display %s X version %d.%d\n", x_display_name, XProtocolVersion(x_display), XProtocolRevision(x_display));

    XSetErrorHandler(x_error_handler);

    XF_WriteLog(XF_LOG_INFO, "Checking for XKB extension...\n");
    int xkb_major_version = XkbMajorVersion, xkb_minor_version = XkbMinorVersion;
    if(!XkbLibraryVersion(&xkb_major_version, &xkb_minor_version)) {
        XF_WriteLog(XF_LOG_ERROR, "XKB compiler version (%d.%d) doesn't match XKB runtime version (%d.%d)!\n",
                    XkbMajorVersion, XkbMinorVersion, xkb_major_version, xkb_minor_version);

        return false;
    } else XF_WriteLog(XF_LOG_INFO, "Found matching XKB %d.%d extension!\n", xkb_major_version, xkb_minor_version);

    XF_WriteLog(XF_LOG_INFO, "Checking server for XKB extension...\n");
    int xkb_opcode, xkb_event, xkb_error;
    if(!XkbQueryExtension(x_display, &xkb_opcode, &xkb_event, &xkb_error, &xkb_major_version, &xkb_minor_version)) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't find compatibile server XKB extension!\n");
        XCloseDisplay(x_display);

        return false;
    } else XF_WriteLog(XF_LOG_INFO, "Found matching server XKB extension!\n");

    x_screen = XDefaultScreen(x_display);
    x_visual = XDefaultVisual(x_display, x_screen);

    XSetWindowAttributes x_window_attr;
    x_window_attr.background_pixel = XBlackPixel(x_display, x_screen);
    x_window_attr.border_pixel = XWhitePixel(x_display, x_screen);
    x_window_attr.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask;

    XF_WriteLog(XF_LOG_INFO, "Creating window...\n");
    x_window = XCreateWindow(x_display,                               // display
                             XRootWindow(x_display, x_screen),          // parent window
                             0, 0,                                      // x, y coordinates
                             WINDOW_WIDTH, WINDOW_HEIGHT,               // width, height
                             0,                                         // border width
                             XDefaultDepth(x_display, x_screen),        // depth
                             InputOutput,                               // class
                             x_visual,                                  // visual
                             CWBackPixel | CWBorderPixel | CWEventMask, // attributes mask
                             &x_window_attr);                           // attributes

    // WM_CLASS setup
    XClassHint *class_hint = XAllocClassHint();
    if(class_hint) {
        class_hint->res_class = "C-app";
        class_hint->res_name = "x11framework";
        XSetClassHint(x_display, x_window, class_hint);

        XFree(class_hint);
    } else XF_WriteLog(XF_LOG_WARNING, "Couldn't allocate memory for WM_CLASS value");

    // WM_NAME setup
    char* wm_name_string = "X11Framework";
    XTextProperty wm_text_property;

    XStringListToTextProperty(&wm_name_string, 1, &wm_text_property);
    XSetWMName(x_display, x_window, &wm_text_property);

    XFree(wm_text_property.value);

    // WM_DELETE setup
    wm_delete_window = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x_display, x_window, &wm_delete_window, 1);

    XF_WriteLog(XF_LOG_INFO, "Successfully created window\n");

    XF_WriteLog(XF_LOG_INFO, "Creating graphics context...\n");
    XGCValues x_gc_values;
    x_gc = XCreateGC(x_display, x_window, 0, &x_gc_values);
    XF_WriteLog(XF_LOG_INFO, "Successfully created graphics context\n");

    if(X_USE_SHM) {
        if(!XShmQueryExtension(x_display)) {
            XF_WriteLog(XF_LOG_ERROR, "System doesn't support shared-memory extension!\n");
            return false;
        }

        x_buffer = XShmCreateImage(x_display, x_visual, XDefaultDepth(x_display, x_screen), ZPixmap, NULL, &shm_info, WINDOW_WIDTH, WINDOW_HEIGHT);
        if(!x_buffer) {
            XF_WriteLog(XF_LOG_ERROR, "Couldn't allocate memory for shared-memory buffer!\n");
            return false;
        }

        shm_info.shmid = shmget(IPC_PRIVATE, x_buffer->bytes_per_line * x_buffer->height, IPC_CREAT|0777);
        if(shm_info.shmid == -1) {
            XF_WriteLog(XF_LOG_ERROR, "Couldn't get new shared memory id!\n");
            return false;
        }

        shm_info.shmaddr = x_buffer->data = shmat(shm_info.shmid, 0, 0);
        if(shm_info.shmaddr == (void*)-1) {
            XF_WriteLog(XF_LOG_ERROR, "Couldn't attach shared memory segment to the calling process!\n");
            return false;
        }

        shm_info.readOnly = False;

        if(!XShmAttach(x_display, &shm_info)) {
            XF_WriteLog(XF_LOG_ERROR, "Couldn't attach server to shared-memory segment!\n");
            return false;
        }

        x_shm_completion = XShmGetEventBase(x_display) + ShmCompletion;
    } else {
        x_buffer = XCreateImage(x_display, 
                                x_visual, 
                                XDefaultDepth(x_display, x_screen), 
                                ZPixmap, 
                                0, 
                                (char*)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4), 
                                WINDOW_WIDTH, WINDOW_HEIGHT, 
                                32, 
                                0);

        if(!x_buffer) {
            XF_WriteLog(XF_LOG_ERROR, "Couldn't create XImage structure!\n");
            return false;
        }
    }

    // allocate memory for shortcut table of rows
    h_lines = (uint32_t**)malloc(WINDOW_HEIGHT * sizeof(uint32_t*));
    for(int i = 0; i < WINDOW_HEIGHT; ++i) {
        h_lines[i] = (uint32_t*)&x_buffer->data[i * WINDOW_WIDTH * 4];
    }

    XMapWindow(x_display, x_window);

    //XAutoRepeatOff(x_display);

    return true;
}

void XF_Close() {
    //XAutoRepeatOn(x_display);

    if(X_USE_SHM) {
        XShmDetach(x_display, &shm_info);
        XDestroyImage(x_buffer);
        if(shmdt(shm_info.shmaddr) == -1)
            XF_WriteLog(XF_LOG_ERROR, "Couldn't detach shared memory from the calling process!\n");

        XSync(x_display, True);

        if(shmctl(shm_info.shmid, IPC_RMID, 0) == -1)
            XF_WriteLog(XF_LOG_ERROR, "Couldn't mark segment to be destroyed!\n");
    } else XDestroyImage(x_buffer);

    free(h_lines);

    XFreeGC(x_display, x_gc);
    XDestroyWindow(x_display, x_window);
    XCloseDisplay(x_display);
}

int XF_GetWindowWidth() { return WINDOW_WIDTH; }
int XF_GetWindowHeight() { return WINDOW_HEIGHT; }

XF_Bool XF_WindowShouldClose() { return x_window_close; }

int recent_button = 0; // stores pressed button for the motion event (for some reason
                       // motion event doesn't know what button is pressed, always contains 0)

XF_Bool XF_GetEvent(XF_Event* pevent) {
    int pending = XPending(x_display);

    if(pending) {
        XNextEvent(x_display, &x_event);

        uint32_t sym = 0;
        int mask = 0;
        switch(x_event.type) {
            case KeyPress:
                // I don't know if there's a better way to handle CapsLock in X11 with my current key-obtaining method
                if((x_event.xkey.state & LockMask) == 2) {
                    mask = 1 - (x_event.xkey.state & ShiftMask);
                } else mask = x_event.xkey.state & ShiftMask;

                sym = XkbKeycodeToKeysym(x_display, x_event.xkey.keycode, 0, mask);

                pevent->type = XF_EVENT_KEY_PRESSED;
                pevent->key.code = sym;
                break;
            case KeyRelease:
                if((x_event.xkey.state & LockMask) == 2) {
                    mask = 1 - (x_event.xkey.state & ShiftMask);
                } else mask = x_event.xkey.state & ShiftMask;

                sym = XkbKeycodeToKeysym(x_display, x_event.xkey.keycode, 0, mask);

                pevent->type = XF_EVENT_KEY_RELEASED;
                pevent->key.code = sym;
                break;
            case ButtonPress:
                pevent->type = XF_EVENT_MOUSE_PRESSED;
                pevent->mouse.x = x_event.xbutton.x;
                pevent->mouse.y = x_event.xbutton.y;
                pevent->mouse.button = x_event.xbutton.button;
                recent_button = x_event.xbutton.button;
                break;
            case ButtonRelease:
                pevent->type = XF_EVENT_MOUSE_RELEASED;
                pevent->mouse.x = x_event.xbutton.x;
                pevent->mouse.y = x_event.xbutton.y;
                pevent->mouse.button = x_event.xbutton.button;
                recent_button = 0;
                break;
            case MotionNotify:
                pevent->type = XF_EVENT_MOUSE_MOVED;
                pevent->mouse.x = x_event.xbutton.x;
                pevent->mouse.y = x_event.xbutton.y;
                pevent->mouse.button = recent_button;
                break;
            case ClientMessage:
                if(x_event.xclient.data.l[0] == wm_delete_window)
                    x_window_close = true;
                break;
            default: if(X_USE_SHM && x_event.type == x_shm_completion) shm_complete = true; break;
        }
    }

    return (pending != 0);
}

uint8_t clear_color = 166;
void XF_SetClearColor(uint8_t base) {
    clear_color = base;
}

Bool check_for_shm_proc(Display* display, XEvent* event, XPointer arg) {
    return (event->type == x_shm_completion);
}

void XF_ClearScreen() {
    if(X_USE_SHM && !shm_complete) {
        while(!XCheckIfEvent(x_display, &x_event, check_for_shm_proc, NULL)) {}
        shm_complete = true;
    }

    memset(x_buffer->data, clear_color, WINDOW_WIDTH * WINDOW_HEIGHT * 4);
}

XF_Bool range_check(int x, int y) {
    return (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT);
}

void XF_DrawPoint(int x, int y, uint32_t color) {
    *(h_lines[y] + x) = color;
}

// only for x0 < x1 !!!
void _plot_line_low(int x0, int y0, int x1, int y1, uint32_t color) {
    const int dx = x1 - x0;
    int dy = y1 - y0;

    int yi = 1;

    // determinate if line is going up or down
    if(dy < 0) {
        yi = -1;
        dy = -dy;
    }

    int d = 2 * dy - dx; // initial value D = A + B / 2, where A = dy B = -dy
                         // to delete the need for division, just multiplicate by 2

    int cy = y0;

    for(int i = x0; i <= x1; ++i) {
        XF_DrawPoint(i, cy, color);

        if(d > 0) {
            cy += yi;
            d += 2 * (dy - dx);
        } else d += 2 * dy;
    }
}

// only for y0 < y1 !!!
void _plot_line_high(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 - x0;
    const int dy = y1 - y0;

    int xi = 1;

    if(dx < 0) {
        xi = -1;
        dx = -dx;
    }

    int d = 2 * dx - dy; // D = A + B / 2, where A = dx B = -dy

    int cx = x0;

    for(int i = y0; i <= y1; ++i) {
        XF_DrawPoint(cx, i, color);

        if(d > 0) {
            cx += xi;
            d += 2 * (dx - dy);
        } else d += 2 * dx;
    }
}

void XF_DrawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    // fast implementation for straight lines
    if(y0 == y1) {
        int sx = (x0 < x1 ? 1 : -1);
        while(x0 != x1) {
            XF_DrawPoint(x0, y0, color);
            x0 += sx;
        }

        return;
    }
    if(x0 == x1) {
        int sy = (y0 < y1 ? 1 : -1);
        while(y0 != y1) {
            XF_DrawPoint(x0, y0, color);
            y0 += sy;
        }

        return;
    }

    // using bresenham algorithm
    if(abs(x1 - x0) > abs(y1 - y0)) {
        if(x0 < x1) _plot_line_low(x0, y0, x1, y1, color);
        else _plot_line_low(x1, y1, x0, y0, color);
    } else {
        if(y0 < y1) _plot_line_high(x0, y0, x1, y1, color);
        else _plot_line_high(x1, y1, x0, y0, color);
    }
}

void XF_DrawRect(int x, int y, int w, int h, uint32_t color, XF_Bool outline) {
    if(x >= WINDOW_WIDTH || y >= WINDOW_HEIGHT || x + w <= 0 || y + h <= 0) return;

    if(x < 0) {
        w += x;
        x = 0;
    }
    if(y < 0) {
        h += y;
        y = 0;
    }

    if(y + h >= WINDOW_HEIGHT) {
        h -= (y + h) - WINDOW_HEIGHT;
    }
    if(x + w >= WINDOW_WIDTH) {
        w -= (x + w) - WINDOW_WIDTH;
    }

    if(!outline) {
        uint32_t *s = h_lines[y] + x;

        int hz_count = 0;

        while(h--) {
            hz_count = w;

            while(hz_count--) {
                *s++ = color;
            }

            s += WINDOW_WIDTH - w;
        }
    } else {
        XF_DrawLine(x, y, x + w - 1, y, color);
        XF_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
        XF_DrawLine(x, y, x, y + h - 1, color);
        XF_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    }
}

void draw_point_in_range(int x, int y, uint32_t color) {
    if(range_check(x, y)) XF_DrawPoint(x, y, color);
}

void _draw_sorted_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color, XF_Bool outline) {
    int dx01 = x1 - x0;
    int dx02 = x2 - x0;

    int dy01 = -abs(y1 - y0);
    int sy01 = (y0 < y1 ? 1 : -1);
    int dy02 = -abs(y2 - y0);
    int sy02 = (y0 < y2 ? 1 : -1);

    int err01 = dx01 + dy01;
    int err02 = dx02 + dy02;

    int x01 = x0;
    int x02 = x0;

    int y01 = y0;
    int y02 = y0;

    while(true) {
        draw_point_in_range(x02, y02, color);

        if(x02 == x2 && y02 == y2) break;

        if(x01 == x1 && y01 == y1) {
            dx01 = x2 - x1;
            dy01 = -abs(y2 - y1);

            sy01 = (y1 < y2 ? 1 : -1);
            err01 = dx01 + dy01;
        }

        int e2 = err02 * 2;
        if(e2 <= dx02) {
            err02 += dx02;
            y02 += sy02;
        }
        if(e2 >= dy02) {
            err02 += dy02;
            ++x02;

            while(x02 != x01) {
                int ee2 = err01 * 2;
                if(ee2 >= dy01) {
                    err01 += dy01;
                    ++x01;
                }
                if(ee2 <= dx01) {
                    err01 += dx01;
                    y01 += sy01;
                }

                draw_point_in_range(x01, y01, color);
            }

            XF_DrawLine(x01, y01, x02, y02, color);
        }
    }
}

void XF_DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color, XF_Bool outline) {
    // determinate far left point
    if(x0 > x1 || x0 > x2) {
        if(x1 < x2) {
            if(x0 < x2) _draw_sorted_triangle(x1, y1, x0, y0, x2, y2, color, outline);
            else _draw_sorted_triangle(x1, y1, x2, y2, x0, y0, color, outline);
        }
        else {
            if(x0 < x1) _draw_sorted_triangle(x2, y2, x0, y0, x1, y2, color, outline);
            else _draw_sorted_triangle(x2, y2, x1, y1, x0, y1, color, outline);
        }
    } else {
        if(x1 < x2) _draw_sorted_triangle(x0, y0, x1, y1, x2, y2, color, outline);
        else _draw_sorted_triangle(x0, y0, x2, y2, x1, y1, color, outline);
    }
}

void XF_DrawCircle(int x, int y, int r, uint32_t color, XF_Bool fill) {
    // using bresenham circle algorithm (int)
    int x_change = 1 - 2 * r; // 1 - 2 * xn
    int y_change = 1;         // 2 * yn + 1
    int radius_error = 0;     // xn^2 + yn^2 - r^2

    int px = r;
    int py = 0;

    while (px >= py) {
        // plot
        draw_point_in_range(x + px, y - py, color);
        draw_point_in_range(x + py, y - px, color);
        draw_point_in_range(x - py, y - px, color);
        draw_point_in_range(x - px, y - py, color);
        draw_point_in_range(x - px, y + py, color);
        draw_point_in_range(x - py, y + px, color);
        draw_point_in_range(x + py, y + px, color);
        draw_point_in_range(x + px, y + py, color);

        if(fill) {
            // fill
            for(int i = x - px + 1; i < x + px; ++i) {
                draw_point_in_range(i, y - py, color);
                draw_point_in_range(i, y + py, color);
            }

            for(int i = x - py + 1; i < x + py; ++i) {
                draw_point_in_range(i, y - px, color);
                draw_point_in_range(i, y + px, color);
            }
        }

        ++py;
        radius_error += y_change;
        y_change += 2;

        // determinate if x needs to be changed, eq: 2 * [xn^2 + yn^2 - r^2 + (2 * yn + 1)] + (1 - 2 * xn) > 0
        if(2 * radius_error + x_change > 0) {
            --px;
            radius_error += x_change;

            x_change += 2;
        }
    }
}

XF_Timer dt_messure;

double XF_GetDeltaTime() {
    return dt_messure.delta;
}

void XF_Render() {
    if(X_USE_SHM) {
        XShmPutImage(x_display, x_window, x_gc, x_buffer, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, True);
        shm_complete = false;
    } else {
        XPutImage(x_display, x_window, x_gc, x_buffer, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    XF_StopTimer(&dt_messure);
    XF_StartTimer(&dt_messure);
}
