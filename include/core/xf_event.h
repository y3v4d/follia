#ifndef xf_event_h
#define xf_event_h

// Event types (should be in its own enum)
#define XF_EVENT_KEY_PREFIX     0x0100
#define XF_EVENT_KEY_PRESSED    0x0101
#define XF_EVENT_KEY_RELEASED   0x0102
#define XF_EVENT_MOUSE_PREFIX   0x0200
#define XF_EVENT_MOUSE_PRESSED  0x0201
#define XF_EVENT_MOUSE_RELEASED 0x0202
#define XF_EVENT_MOUSE_MOVED    0x0203

#include "xf_key.h"
#include "xf_mouse.h"

#include <stdint.h>

typedef struct _XF_KeyEvent {
    uint16_t type;
    uint32_t code;
} XF_KeyEvent;

typedef struct _XF_MouseEvent {
    uint16_t type;
    int x, y;

    uint8_t button;
} XF_MouseEvent;

typedef union _XF_Event {
    uint16_t type;

    XF_KeyEvent key;
    XF_MouseEvent mouse;
} XF_Event;

#endif
