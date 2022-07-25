#ifndef __FL_EVENT_H__
#define __FL_EVENT_H__

// Event types (should be in its own enum)
#define FL_EVENT_KEY_PREFIX     0x0100
#define FL_EVENT_KEY_PRESSED    0x0101
#define FL_EVENT_KEY_RELEASED   0x0102
#define FL_EVENT_MOUSE_PREFIX   0x0200
#define FL_EVENT_MOUSE_PRESSED  0x0201
#define FL_EVENT_MOUSE_RELEASED 0x0202
#define FL_EVENT_MOUSE_MOVED    0x0203

#include "fl_key.h"
#include "fl_mouse.h"

#include <stdint.h>

typedef struct {
    uint16_t type;
    uint32_t code;
} FL_KeyEvent;

typedef struct {
    uint16_t type;
    int x, y;

    uint8_t button;
} FL_MouseEvent;

typedef union {
    uint16_t type;

    FL_KeyEvent key;
    FL_MouseEvent mouse;
} FL_Event;

#endif
