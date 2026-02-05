#include <drivers/input/keyboard.h>
#include <ui/console.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEY_BUFFER_SIZE 32

// Port I/O
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

// US QWERTY scan code to ASCII (set 1)
static const char scancode_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, // Ctrl
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, // Left Shift
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, // Right Shift
    '*',
    0, // Alt
    ' ', // Space
    0, // Caps Lock
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // F1-F10
    0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Shifted characters
static const char scancode_shifted[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    0, '*', 0, ' '
};

// Keyboard state
static uint8_t shift_pressed = 0;
static uint8_t ctrl_pressed = 0;
static uint8_t alt_pressed = 0;
static uint8_t extended_key = 0;

// Key event buffer (circular queue)
static keyboard_event_t key_buffer[KEY_BUFFER_SIZE];
static volatile uint32_t buffer_head = 0;
static volatile uint32_t buffer_tail = 0;

void keyboard_init(void) {
    buffer_head = 0;
    buffer_tail = 0;
    shift_pressed = 0;
    ctrl_pressed = 0;
    alt_pressed = 0;
    extended_key = 0;
    
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN); 
    console_write("[KEYBOARD] Driver initialized (polling mode)\\n");
}

// Called by IRQ handler - just processes and buffers the key
void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Handle extended keys (0xE0 prefix for arrow keys, etc.)
    if (scancode == 0xE0) {
        extended_key = 1;
        return;
    }
    
    // Key release (bit 7 set)
    if (scancode & 0x80) {
        scancode &= 0x7F;
        
        // Track modifier key releases
        if (scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT) {
            shift_pressed = 0;
        } else if (scancode == SCANCODE_LCTRL) {
            ctrl_pressed = 0;
        } else if (scancode == SCANCODE_LALT) {
            alt_pressed = 0;
        }
        
        extended_key = 0;
        return;
    }
    
    // Key press - track modifier keys
    if (scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT) {
        shift_pressed = 1;
        extended_key = 0;
        return;
    } else if (scancode == SCANCODE_LCTRL) {
        ctrl_pressed = 1;
        extended_key = 0;
        return;
    } else if (scancode == SCANCODE_LALT) {
        alt_pressed = 1;
        extended_key = 0;
        return;
    }
    
    // Create key event
    keyboard_event_t event;
    
    // Check for Alt+Fn (F1-F6 for TTY switching)
    if (alt_pressed && scancode >= 0x3B && scancode <= 0x40) {
        event.type = KEY_EVENT_SPECIAL;
        event.scancode = scancode;
        event.shift = shift_pressed;
        event.ctrl = ctrl_pressed;
        event.alt = alt_pressed;
        event.character = 0;
        extended_key = 0;
        
        // Add to buffer
        uint32_t next_head = (buffer_head + 1) % KEY_BUFFER_SIZE;
        if (next_head != buffer_tail) {
            key_buffer[buffer_head] = event;
            buffer_head = next_head;
        }
        return;
    }
    
    // Regular keys
    event.type = extended_key ? KEY_EVENT_SPECIAL : KEY_EVENT_CHAR;
    event.scancode = scancode;
    event.shift = shift_pressed;
    event.ctrl = ctrl_pressed;
    event.alt = alt_pressed;
    
    if (event.type == KEY_EVENT_CHAR) {
        // Get character
        if (shift_pressed && scancode < 58) {
            event.character = scancode_shifted[scancode];
        } else if (scancode < 128) {
            event.character = scancode_ascii[scancode];
        } else {
            event.character = 0;
        }
        
        // Handle Ctrl combinations
        if (ctrl_pressed && event.character >= 'a' && event.character <= 'z') {
            event.character = event.character - 'a' + 1;
        }
    } else {
        event.character = 0;
    }
    
    extended_key = 0;
    
    // Add to buffer if not full
    uint32_t next_head = (buffer_head + 1) % KEY_BUFFER_SIZE;
    if (next_head != buffer_tail) {
        key_buffer[buffer_head] = event;
        buffer_head = next_head;
    }
}

// Poll for next key event (called by TTY) 
int keyboard_poll_event(keyboard_event_t* event) {
    if (!event)
        return 0;
    
    // Critical section - disable interrupts while accessing buffer
    __asm__ volatile("cli");
    
    if (buffer_tail == buffer_head) {
        __asm__ volatile("sti");
        return 0;
    }
    
    *event = key_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % KEY_BUFFER_SIZE;
    
    __asm__ volatile("sti");
    return 1;
}

// Check if any keys are available
int keyboard_has_event(void) {
    int ret;
    __asm__ volatile("cli");
    ret = (buffer_tail != buffer_head);
    __asm__ volatile("sti");
    return ret;
}

