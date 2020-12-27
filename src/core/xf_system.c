#include "xf_system.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xproto.h>
// using shm functions instead of regular is faster (software rendering)
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
// better keyboard support
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>

short WINDOW_WIDTH = 100, WINDOW_HEIGHT = 100;

// linux specific
Display *display = NULL;
int screen = 0;
Window window;
GC gc;
Visual *visual = NULL;
XEvent event;

XImage *x_buffer = NULL;
uint32_t **h_lines = NULL;
XShmSegmentInfo shm_info;

int x_shm_completion; // determinate what's the type of shm completion event
boolean shm_complete = false; // determinate if the shm finished its job

Atom wm_delete_window;

boolean window_close = false; // determinate if window should be closed

int x_error_handler(Display *display, XErrorEvent *event) {
    char msg[256];

    XGetErrorText(display, event->error_code, msg, 256);

    XF_WriteLog(XF_LOG_ERROR, msg);

    return 0;
}

boolean XF_Initialize(int width, int height) {
    if(!XF_InitializeLog()) {
        printf("[ERROR] No messages will be written to log file\n");
    }

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    XF_WriteLog(XF_LOG_INFO, "Initializing XF system...\n");

    display = XOpenDisplay(NULL);
    if(!display) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't open display\n"); 
        return false;
    }
    XF_WriteLog(XF_LOG_INFO, "Successfully opened display\n");
    char *display_name = XDisplayString(display);
    XF_WriteLog(XF_LOG_INFO, "Display %s X version %d.%d\n", display_name, XProtocolVersion(display), XProtocolRevision(display));

    XSetErrorHandler(x_error_handler);

    int xkb_major_version = XkbMajorVersion, xkb_minor_version = XkbMinorVersion;
    if(!XkbLibraryVersion(&xkb_major_version, &xkb_minor_version)) {
        XF_WriteLog(XF_LOG_ERROR, "Coulnd't find XKB extension!\n");

        return false;
    } else XF_WriteLog(XF_LOG_INFO, "Found version %d.%d of XKB extension\n", xkb_major_version, xkb_minor_version);
    
    int xkb_opcode, xkb_event, xkb_error;
    if(!XkbQueryExtension(display, &xkb_opcode, &xkb_event, &xkb_error, &xkb_major_version, &xkb_minor_version)) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't query XKB extension!\n");
        XCloseDisplay(display);

        return false;
    } else XF_WriteLog(XF_LOG_INFO, "Successfuly queried XKB extension\n");
    
    screen = XDefaultScreen(display);
    visual = XDefaultVisual(display, screen);

    XSetWindowAttributes window_attr;
    window_attr.background_pixel = XBlackPixel(display, screen);
    window_attr.border_pixel = XWhitePixel(display, screen);
    window_attr.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

    XF_WriteLog(XF_LOG_INFO, "Creating window...\n");
    window = XCreateWindow(display, // display
                           XRootWindow(display, screen), // parent window
                           0, 0, // x, y coordinates
                           WINDOW_WIDTH, WINDOW_HEIGHT, // width, height
                           0, // border width
                           XDefaultDepth(display, screen), // depth
                           InputOutput, // class
                           visual, // visual
                           CWBackPixel | CWBorderPixel | CWEventMask, // attributes mask
                           &window_attr); // attributes

    // WM_CLASS
    XClassHint *class_hint = XAllocClassHint();
    if(class_hint) {
        class_hint->res_class = "C-app";
        class_hint->res_name = "Follia";
        XSetClassHint(display, window, class_hint);

        XFree(class_hint);
    } else XF_WriteLog(XF_LOG_WARNING, "Couldn't allocate memory for WM_CLASS value");

    // WM_NAME
    char* wm_name_string = "Follia";
    XTextProperty wm_text_property;

    XStringListToTextProperty(&wm_name_string, 1, &wm_text_property);
    XSetWMName(display, window, &wm_text_property);

    XFree(wm_text_property.value);

    // WM_DELETE
    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    XF_WriteLog(XF_LOG_INFO, "Successfully created window\n");

    XF_WriteLog(XF_LOG_INFO, "Creating graphics context...\n");
    XGCValues gc_values;
    gc = XCreateGC(display, window, 0, &gc_values);
    XF_WriteLog(XF_LOG_INFO, "Successfully created graphics context\n");

    if(!XShmQueryExtension(display)) {
        XF_WriteLog(XF_LOG_ERROR, "System doesn't support shared-memory extension!\n");
        return false;
    }

    x_buffer = XShmCreateImage(display, visual, XDefaultDepth(display, screen), ZPixmap, NULL, &shm_info, WINDOW_WIDTH, WINDOW_HEIGHT);
    if(!x_buffer) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't allocate memory for shared-memory x_buffer!\n");
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

    if(!XShmAttach(display, &shm_info)) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't attach server to shared-memory segment!\n");
        return false;
    }

    x_shm_completion = XShmGetEventBase(display) + ShmCompletion;

    h_lines = (uint32_t**)malloc(WINDOW_HEIGHT * sizeof(uint32_t*));
    for(int i = 0; i < WINDOW_HEIGHT; ++i) {
        h_lines[i] = (uint32_t*)&x_buffer->data[i * WINDOW_WIDTH * 4];
    }

    XMapWindow(display, window);

    XAutoRepeatOff(display);

    return true;
}

void XF_Close() {
    XAutoRepeatOn(display);

    XShmDetach(display, &shm_info);
    XDestroyImage(x_buffer);
    if(shmdt(shm_info.shmaddr) == -1)
        XF_WriteLog(XF_LOG_ERROR, "Couldn't detach shared memory from the calling process!\n");

    XSync(display, True);
    
    if(shmctl(shm_info.shmid, IPC_RMID, 0) == -1)
        XF_WriteLog(XF_LOG_ERROR, "Couldn't mark segment to be destroyed!\n");

    free(h_lines);

    XFreeGC(display, gc);
    XDestroyWindow(display, window); 
    XCloseDisplay(display);
}

int XF_GetWindowWidth() { return WINDOW_WIDTH; }
int XF_GetWindowHeight() { return WINDOW_HEIGHT; }

boolean XF_WindowShouldClose() { return window_close; }

boolean XF_GetEvent(XF_Event* pevent) {
    int pending = XPending(display);

    if(pending) {
        XNextEvent(display, &event);

        uint8_t sym = 0;
        int mask = 0;
        switch(event.type) {
            case KeyPress:
                // I don't know if there's a better way to handle CapsLock in X11 with my current key-obtaining method
                if((event.xkey.state & LockMask) == 2) {
                    mask = 1 - (event.xkey.state & ShiftMask);
                } else mask = event.xkey.state & ShiftMask;

                sym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, mask);

                pevent->type = XF_EVENT_KEY_PRESSED;
                pevent->key.code = sym;
                break;
            case KeyRelease:
                if((event.xkey.state & LockMask) == 2) {
                    mask = 1 - (event.xkey.state & ShiftMask);
                } else mask = event.xkey.state & ShiftMask;
               
                sym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, mask);

                pevent->type = XF_EVENT_KEY_RELEASED;
                pevent->key.code = sym;
                break;
            case ClientMessage:
                if(event.xclient.data.l[0] == wm_delete_window)
                    window_close = true;
                break;
            default: if(event.type == x_shm_completion) shm_complete = true; break;
        }
    }

    return (pending != 0);
}

void XF_ClearScreen() {
    memset(x_buffer->data, 166, WINDOW_WIDTH * WINDOW_HEIGHT * 4);
}

boolean range_check(int x, int y) {
    return (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT);
}

void XF_DrawPoint(int x, int y, uint32_t color) {
    *(h_lines[y] + x) = color;
}

void XF_DrawLine(int x1, int y1, int x2, int y2, uint32_t color) {
    if(x1 == x2) {
        int step = (y1 < y2 ? 1 : -1);
        for(int i = y1; i != y2; i += step)
            XF_DrawPoint(x1, i, color);
    } else if(y1 == y2) {
        int step = (x1 < x2 ? 1 : -1);
        for(int i = x1; i != x2; i += step)
            XF_DrawPoint(i, y1, color);
    } else {
        float m = (float)(y1 - y2) / (x1 - x2);
        float c = y1 - m * x1;

        int step = (x1 < x2 ? 1 : -1);
        for(int i = x1; i != x2; i += step)
            XF_DrawPoint(i, (int)(m * i + c), color);  
    }
}

void XF_DrawRect(int x, int y, int w, int h, uint32_t color, boolean outline) {
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
        XF_DrawLine(x, y, x + w, y, color);
        XF_DrawLine(x, y + h - 1, x + w, y + h - 1, color);
        XF_DrawLine(x, y, x, y + h, color);
        XF_DrawLine(x + w - 1, y, x + w - 1, y + h, color);
    }
}

Bool check_for_shm_proc(Display* display, XEvent* event, XPointer arg) {
    return (event->type == x_shm_completion);
}

void XF_Render() {
#if 0 // end up don't using this (very slow)
    if(PIXEL_W == 2) {
        uint32_t *p = (uint32_t*)&x_screen[0];
        uint32_t *s[2];

        for(int i = 0; i < 2; ++i) {
            s[i] = (uint32_t*)&image->data[i * WINDOW_WIDTH * PIXEL_W * 4];
        }

        int y = WINDOW_HEIGHT;
        
        while(y--) {
            int x = WINDOW_WIDTH;

            while(x--) {
                *(s[0]++) = *p;
                *(s[1]++) = *p;
                *(s[0]++) = *p;
                *(s[1]++) = *p; 

                p++;
            }

            s[0] += WINDOW_WIDTH * PIXEL_W;
            s[1] += WINDOW_WIDTH * PIXEL_W;
        }
    } else if(PIXEL_W == 3) {
        uint32_t *p = (uint32_t*)&x_screen[0];
        uint32_t *s[3];

        for(int i = 0; i < 3; ++i) {
            s[i] = (uint32_t*)&image->data[i * WINDOW_WIDTH * PIXEL_W * 4];
        }

        int y = WINDOW_HEIGHT;

        while(y--) {
            int x = WINDOW_WIDTH;

            while(x--) {
                *(s[0]++) = *p;
                *(s[1]++) = *p;
                *(s[2]++) = *p;
                *(s[0]++) = *p;
                *(s[1]++) = *p;
                *(s[2]++) = *p;
                *(s[0]++) = *p;
                *(s[1]++) = *p;
                *(s[2]++) = *p;

                p++;
            }

            s[0] += WINDOW_WIDTH * PIXEL_W * 2;
            s[1] += WINDOW_WIDTH * PIXEL_W * 2;
            s[2] += WINDOW_WIDTH * PIXEL_W * 2;
        }
    }
#endif

    XShmPutImage(display, window, gc, x_buffer, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, True);

    while(!XCheckIfEvent(display, &event, check_for_shm_proc, NULL)) {}
    
    /*shm_complete = false;
    do {
        XF_ProcessEvents();
    } while(!shm_complete);*/
}
