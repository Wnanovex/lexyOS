#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// Special key scancodes
#define KEY_UP          0x48
#define KEY_DOWN        0x50
#define KEY_LEFT        0x4B
#define KEY_RIGHT       0x4D
#define KEY_HOME        0x47
#define KEY_END         0x4F
#define KEY_DELETE      0x53
#define KEY_TAB         0x09
#define KEY_ENTER       0x0A
#define KEY_BACKSPACE   0x08
#define KEY_ESC         0x1B

// Modifier key scancodes
#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_RSHIFT 0x36
#define SCANCODE_LCTRL  0x1D
#define SCANCODE_LALT   0x38

// Key event types
typedef enum {
    KEY_EVENT_CHAR,      // Regular character
    KEY_EVENT_SPECIAL    // Special key (arrows, etc.)
} key_event_type_t;

// Key event structure
typedef struct {
    key_event_type_t type;
    char character;      // For KEY_EVENT_CHAR
    uint8_t scancode;    // For KEY_EVENT_SPECIAL
    uint8_t shift;       // Modifier states
    uint8_t ctrl;
    uint8_t alt;
} keyboard_event_t;

void keyboard_init(void);
void keyboard_handler(void);

// Polling functions (called by TTY)
int keyboard_poll_event(keyboard_event_t* event);
int keyboard_has_event(void);

#endif // KEYBOARD_H
