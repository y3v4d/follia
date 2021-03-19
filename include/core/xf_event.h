#ifndef xf_event_h
#define xf_event_h

// Event types (should be in its own enum)
#define XF_EVENT_KEY_PRESSED 0x00
#define XF_EVENT_KEY_RELEASED 0x01
#define XF_EVENT_MOUSE_PRESSED 0x02
#define XF_EVENT_MOUSE_RELEASED 0x03
#define XF_EVENT_MOUSE_MOVED 0x04

#include "xf_key.h"
#include "xf_mouse.h"

#include <stdint.h>

typedef struct _XF_KeyEvent {
    uint32_t code;
} XF_KeyEvent;

typedef struct _XF_MouseEvent {
    int x, y;

    uint8_t button;
} XF_MouseEvent;

typedef struct _XF_Event {
    uint8_t type;

    union {
        XF_KeyEvent key;
        XF_MouseEvent mouse;
    };
} XF_Event;

#endif
