#include "core/fl_system.h"
#include "core/fl_log.h"
#include "core/fl_timer.h"
#include "frame_buffer.h"

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

#define X_USE_SHM 1

int WINDOW_WIDTH = 100, WINDOW_HEIGHT = 100;

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
FL_Bool shm_complete = true; // determinate if the shm finished its job

Atom wm_delete_window;

FL_Bool x_window_close = false; // determinate if window should be closed

FL_Timer core_timers[FL_TIMER_ALL] = {0};

int x_error_handler(Display *display, XErrorEvent *event) {
    char msg[256];

    XGetErrorText(display, event->error_code, msg, 256);

    FL_WriteLog(FL_LOG_ERROR, msg);

    return 0;
}

FL_Bool FL_Initialize(int width, int height) {
    if(!FL_InitializeLog()) {
        printf("[ERROR] No messages will be written to log file\n");
    }

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    FL_WriteLog(FL_LOG_INFO, "Initializing XF system...\n");

    x_display = XOpenDisplay(NULL);
    if(!x_display) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't open display\n");
        return false;
    }
    FL_WriteLog(FL_LOG_INFO, "Successfully opened display\n");

    char *x_display_name = XDisplayString(x_display);
    FL_WriteLog(FL_LOG_INFO, "Display %s X version %d.%d\n", x_display_name, XProtocolVersion(x_display), XProtocolRevision(x_display));

    XSetErrorHandler(x_error_handler);

    FL_WriteLog(FL_LOG_INFO, "Checking for XKB extension...\n");
    int xkb_major_version = XkbMajorVersion, xkb_minor_version = XkbMinorVersion;
    if(!XkbLibraryVersion(&xkb_major_version, &xkb_minor_version)) {
        FL_WriteLog(FL_LOG_ERROR, "XKB compiler version (%d.%d) doesn't match XKB runtime version (%d.%d)!\n",
                    XkbMajorVersion, XkbMinorVersion, xkb_major_version, xkb_minor_version);

        return false;
    } else FL_WriteLog(FL_LOG_INFO, "Found matching XKB %d.%d extension!\n", xkb_major_version, xkb_minor_version);

    FL_WriteLog(FL_LOG_INFO, "Checking server for XKB extension...\n");
    int xkb_opcode, xkb_event, xkb_error;
    if(!XkbQueryExtension(x_display, &xkb_opcode, &xkb_event, &xkb_error, &xkb_major_version, &xkb_minor_version)) {
        FL_WriteLog(FL_LOG_ERROR, "Couldn't find compatibile server XKB extension!\n");
        XCloseDisplay(x_display);

        return false;
    } else FL_WriteLog(FL_LOG_INFO, "Found matching server XKB extension!\n");

    x_screen = XDefaultScreen(x_display);
    x_visual = XDefaultVisual(x_display, x_screen);

    XSetWindowAttributes x_window_attr = {
        .background_pixel = XBlackPixel(x_display, x_screen),
        .border_pixel = XWhitePixel(x_display, x_screen),
        .event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask
    };

    FL_WriteLog(FL_LOG_INFO, "Creating window...\n");
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
    } else FL_WriteLog(FL_LOG_WARNING, "Couldn't allocate memory for WM_CLASS value");

    // WM_NAME setup
    FL_SetTitle("Follia");

    // WM_DELETE setup
    wm_delete_window = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x_display, x_window, &wm_delete_window, 1);

    FL_WriteLog(FL_LOG_INFO, "Successfully created window\n");

    FL_WriteLog(FL_LOG_INFO, "Creating graphics context...\n");
    XGCValues x_gc_values;
    x_gc = XCreateGC(x_display, x_window, 0, &x_gc_values);
    FL_WriteLog(FL_LOG_INFO, "Successfully created graphics context\n");

    if(X_USE_SHM) {
        if(!XShmQueryExtension(x_display)) {
            FL_WriteLog(FL_LOG_ERROR, "System doesn't support shared-memory extension!\n");
            return false;
        }

        x_buffer = XShmCreateImage(x_display, x_visual, XDefaultDepth(x_display, x_screen), ZPixmap, NULL, &shm_info, WINDOW_WIDTH, WINDOW_HEIGHT);
        if(!x_buffer) {
            FL_WriteLog(FL_LOG_ERROR, "Couldn't allocate memory for shared-memory buffer!\n");
            return false;
        }

        shm_info.shmid = shmget(IPC_PRIVATE, x_buffer->bytes_per_line * x_buffer->height, IPC_CREAT|0777);
        if(shm_info.shmid == -1) {
            FL_WriteLog(FL_LOG_ERROR, "Couldn't get new shared memory id!\n");
            return false;
        }

        shm_info.shmaddr = x_buffer->data = shmat(shm_info.shmid, 0, 0);
        if(shm_info.shmaddr == (void*)-1) {
            FL_WriteLog(FL_LOG_ERROR, "Couldn't attach shared memory segment to the calling process!\n");
            return false;
        }

        shm_info.readOnly = False;

        if(!XShmAttach(x_display, &shm_info)) {
            FL_WriteLog(FL_LOG_ERROR, "Couldn't attach server to shared-memory segment!\n");
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
            FL_WriteLog(FL_LOG_ERROR, "Couldn't create XImage structure!\n");
            return false;
        }
    }

    // allocate memory for shortcut table of rows
    h_lines = (uint32_t**)malloc(WINDOW_HEIGHT * sizeof(uint32_t*));
    for(int i = 0; i < WINDOW_HEIGHT; ++i) {
        h_lines[i] = (uint32_t*)&x_buffer->data[i * WINDOW_WIDTH * 4];
    }

    XMapWindow(x_display, x_window);
    XSync(x_display, false);

    //XAutoRepeatOff(x_display);

    return true;
}

void FL_Close() {
    //XAutoRepeatOn(x_display);

    if(X_USE_SHM) {
        XShmDetach(x_display, &shm_info);
        XDestroyImage(x_buffer);
        if(shmdt(shm_info.shmaddr) == -1)
            FL_WriteLog(FL_LOG_ERROR, "Couldn't detach shared memory from the calling process!\n");

        XSync(x_display, True);

        if(shmctl(shm_info.shmid, IPC_RMID, 0) == -1)
            FL_WriteLog(FL_LOG_ERROR, "Couldn't mark segment to be destroyed!\n");
    } else XDestroyImage(x_buffer);

    free(h_lines);

    XFreeGC(x_display, x_gc);
    XDestroyWindow(x_display, x_window);
    XCloseDisplay(x_display);
}

int FL_GetWindowWidth() { return WINDOW_WIDTH; }
int FL_GetWindowHeight() { return WINDOW_HEIGHT; }

FL_Bool FL_WindowShouldClose() { return x_window_close; }

void FL_SetTitle(char *name) {
    XTextProperty wm_text_property;

    XStringListToTextProperty(&name, 1, &wm_text_property);
    XSetWMName(x_display, x_window, &wm_text_property);

    XFree(wm_text_property.value);
}

int recent_button = 0; // stores pressed button for the motion event (for some reason
                       // motion event doesn't know what button is pressed, always contains 0)

FL_Bool FL_GetEvent(FL_Event* pevent) {
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

                pevent->type = FL_EVENT_KEY_PRESSED;
                pevent->key.code = sym;
                break;
            case KeyRelease:
                if((x_event.xkey.state & LockMask) == 2) {
                    mask = 1 - (x_event.xkey.state & ShiftMask);
                } else mask = x_event.xkey.state & ShiftMask;

                sym = XkbKeycodeToKeysym(x_display, x_event.xkey.keycode, 0, mask);

                pevent->type = FL_EVENT_KEY_RELEASED;
                pevent->key.code = sym;
                break;
            case ButtonPress:
                pevent->type = FL_EVENT_MOUSE_PRESSED;
                pevent->mouse.x = x_event.xbutton.x;
                pevent->mouse.y = x_event.xbutton.y;
                pevent->mouse.button = x_event.xbutton.button;
                recent_button = x_event.xbutton.button;
                break;
            case ButtonRelease:
                pevent->type = FL_EVENT_MOUSE_RELEASED;
                pevent->mouse.x = x_event.xbutton.x;
                pevent->mouse.y = x_event.xbutton.y;
                pevent->mouse.button = x_event.xbutton.button;
                recent_button = 0;
                break;
            case MotionNotify:
                pevent->type = FL_EVENT_MOUSE_MOVED;
                pevent->mouse.x = x_event.xbutton.x;
                pevent->mouse.y = x_event.xbutton.y;
                pevent->mouse.button = recent_button;
                break;
            case ClientMessage:
                if((unsigned long)x_event.xclient.data.l[0] == wm_delete_window)
                    x_window_close = true;
                break;
            default: if(X_USE_SHM && x_event.type == x_shm_completion) shm_complete = true; break;
        }
    }

    return (pending != 0);
}

uint8_t clear_color = 166;
void FL_SetClearColor(uint8_t base) {
    clear_color = base;
}

Bool check_for_shm_proc(Display* display, XEvent* event, XPointer arg) {
    return (event->type == x_shm_completion);
}

FL_Timer dt_messure;

double FL_GetDeltaTime() {
    return dt_messure.delta;
}

double FL_GetCoreTimer(uint8_t type) {
    if(type < 0 || type > FL_TIMER_ALL) return -1;

    if(type == FL_TIMER_ALL) {
        double total = 0;
        for(int i = 0; i < FL_TIMER_ALL; ++i) {
            total += core_timers[i].delta;
        }

        return total;
    } else return core_timers[type].delta;
}

long frame_time = 0;
void FL_SetFrameTime(double ms) {
    if(ms < 0 || ms >= 1000) return;

    frame_time = ms * 1000000;
}

void FL_ClearScreen() {
    FL_StartTimer(&core_timers[FL_TIMER_CLEAR_SCREEN]);
    if(X_USE_SHM && !shm_complete) {
        while(!XCheckIfEvent(x_display, &x_event, check_for_shm_proc, NULL)) {}
        shm_complete = true;
    }

    memset((void*)x_buffer->data, clear_color, WINDOW_WIDTH * WINDOW_HEIGHT * 4);

    FL_StopTimer(&core_timers[FL_TIMER_CLEAR_SCREEN]);
    FL_StartTimer(&core_timers[FL_TIMER_CLEAR_TO_RENDER]);
}

void FL_Render() {
    FL_StopTimer(&core_timers[FL_TIMER_CLEAR_TO_RENDER]);
    FL_StartTimer(&core_timers[FL_TIMER_RENDER]);

    if(X_USE_SHM) {
        XShmPutImage(x_display, x_window, XDefaultGC(x_display, x_screen), x_buffer, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, True);
        shm_complete = false;
    } else {
        XPutImage(x_display, x_window, x_gc, x_buffer, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    FL_StopTimer(&core_timers[FL_TIMER_RENDER]);
    FL_StopTimer(&dt_messure);
    
    // sleep through the rest of the frame if limit set
    struct timespec sleeper = { 0 };
    sleeper.tv_sec = 0;
    sleeper.tv_nsec = frame_time - (long)(dt_messure.delta * 1000000);
    nanosleep(&sleeper, NULL);

    FL_StopTimer(&dt_messure);
    FL_StartTimer(&dt_messure);
}
