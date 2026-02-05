#ifndef TTY_H
#define TTY_H

#include <stdint.h>
#include <stddef.h>

#define MAX_TTYS 8

typedef enum {
    TTY_MODE_SHELL,
    TTY_MODE_GAME,
    TTY_MODE_TEXT
} tty_mode_t;

typedef void (*tty_update_func_t)(void);
typedef void (*tty_draw_func_t)(void);
typedef void (*tty_char_input_func_t)(char c);
typedef void (*tty_special_input_func_t)(uint8_t scancode);

// TTY structure
typedef struct {
    tty_mode_t mode;
    
    // Function pointers
    tty_update_func_t update_func;
    tty_draw_func_t draw_func;
    tty_char_input_func_t char_input_func;
    tty_special_input_func_t special_input_func;
    
    // State
    int initialized;
    int needs_redraw;
    
    // For shell TTYs
    char command_buffer[256];
    uint16_t buffer_index;
    uint32_t cursor_position;
    
    // Screen persistence - using kmalloc
    void* screen_buffer;
    size_t screen_buffer_size;
    uint32_t screen_width;
    uint32_t screen_height;
    uint32_t screen_pitch;
    int has_saved_screen;
    
    // Saved cursor position
    uint32_t saved_cursor_x;
    uint32_t saved_cursor_y;
} tty_t;

// Global TTY array - defined in tty.c
extern tty_t ttys[MAX_TTYS];

// TTY functions
void tty_init(void);
void tty_register(int tty_num, tty_mode_t mode, 
                  tty_update_func_t update_func,
                  tty_draw_func_t draw_func,
                  tty_char_input_func_t char_input_func,
                  tty_special_input_func_t special_input_func);
int tty_switch(int tty_num);
int tty_get_current(void);
void tty_poll_input(void);

void tty_change_mode(tty_mode_t new_mode,
                     tty_update_func_t update_func,
                     tty_draw_func_t draw_func,
                     tty_char_input_func_t char_input_func,
                     tty_special_input_func_t special_input_func);

void tty_restore_to_shell(void);

#endif // TTY_H
