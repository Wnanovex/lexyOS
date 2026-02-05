#include <ui/tty/tty.h>
#include <ui/console.h>
#include <drivers/input/keyboard.h>
#include <drivers/video/framebuffer.h>
#include <ui/shell/shell.h>

// Global TTY array
tty_t ttys[MAX_TTYS];
static int current_tty = 0;

void tty_init(void) {
    for (int i = 0; i < MAX_TTYS; i++) {
        ttys[i].mode = TTY_MODE_SHELL;
        ttys[i].update_func = NULL;
        ttys[i].draw_func = NULL;
        ttys[i].char_input_func = NULL;
        ttys[i].special_input_func = NULL;
        ttys[i].initialized = 0;
        ttys[i].needs_redraw = 1;
        ttys[i].buffer_index = 0;
        ttys[i].cursor_position = 0;
        ttys[i].command_buffer[0] = '\0';
        
        // Screen buffer disabled for now
        ttys[i].screen_buffer = NULL;
        ttys[i].screen_buffer_size = 0;
        ttys[i].screen_width = 0;
        ttys[i].screen_height = 0;
        ttys[i].screen_pitch = 0;
        ttys[i].has_saved_screen = 0;
        ttys[i].saved_cursor_x = 0;
        ttys[i].saved_cursor_y = 0;
    }
    current_tty = 0;
}

void tty_register(int tty_num, tty_mode_t mode, 
                  tty_update_func_t update_func,
                  tty_draw_func_t draw_func,
                  tty_char_input_func_t char_input_func,
                  tty_special_input_func_t special_input_func) {
    if (tty_num < 0 || tty_num >= MAX_TTYS) return;
    
    ttys[tty_num].mode = mode;
    ttys[tty_num].update_func = update_func;
    ttys[tty_num].draw_func = draw_func;
    ttys[tty_num].char_input_func = char_input_func;
    ttys[tty_num].special_input_func = special_input_func;
    ttys[tty_num].initialized = 1;
    ttys[tty_num].needs_redraw = 1;
}

int tty_switch(int tty_num) {
    if (tty_num < 0 || tty_num >= MAX_TTYS || !ttys[tty_num].initialized) {
        return -1;
    }
    
    if (tty_num == current_tty) {
        return 0;
    }
    
    // Switch to new TTY
    current_tty = tty_num;
    
    // Clear screen
    console_clear();
    ttys[current_tty].needs_redraw = 1;
    
    // Set cursor visibility
    if (ttys[current_tty].mode == TTY_MODE_GAME) {
        console_show_cursor(0);
    } else {
        console_show_cursor(1);
    }
    
    // Show TTY info and restore command buffer
    if (ttys[current_tty].mode == TTY_MODE_SHELL) {
        console_set_color_preset(CONSOLE_COLOR_PRESET_MATRIX);
        console_write("\nTTY");
        console_putchar('0' + tty_num);
        console_write(" - Type 'help' for commands\n");
        console_write("Press Alt+F1-F6 to switch TTYs\n\n");
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
        
        shell_print_prompt();
        
        // Restore command buffer if typing
        if (ttys[current_tty].buffer_index > 0) {
            console_write(ttys[current_tty].command_buffer);
        }
    }
    
    return 0;
}

int tty_get_current(void) {
    return current_tty;
}

void tty_poll_input(void) {
    keyboard_event_t event;
    
    while (keyboard_poll_event(&event)) {
        // Handle Alt+Fn keys for TTY switching
        if (event.type == KEY_EVENT_SPECIAL && event.alt) {
            if (event.scancode >= 0x3B && event.scancode <= 0x40) {
                int tty_num = event.scancode - 0x3B;
                if (tty_num < MAX_TTYS && ttys[tty_num].initialized) {
                    tty_switch(tty_num);
                    continue;
                }
            }
        }
        
        // Pass event to current TTY
        tty_t* current = &ttys[current_tty];
        
        if (event.type == KEY_EVENT_CHAR) {
            if (current->char_input_func) {
                current->char_input_func(event.character);
            }
        } else if (event.type == KEY_EVENT_SPECIAL) {
            if (current->special_input_func) {
                current->special_input_func(event.scancode);
            }
        }
    }
}

static tty_t tty_backup[MAX_TTYS];

void tty_change_mode(tty_mode_t new_mode,
                     tty_update_func_t update_func,
                     tty_draw_func_t draw_func,
                     tty_char_input_func_t char_input_func,
                     tty_special_input_func_t special_input_func) {
    
    int tty_num = current_tty;
    
    tty_backup[tty_num] = ttys[tty_num];
    
    ttys[tty_num].mode = new_mode;
    ttys[tty_num].update_func = update_func;
    ttys[tty_num].draw_func = draw_func;
    ttys[tty_num].char_input_func = char_input_func;
    ttys[tty_num].special_input_func = special_input_func;
    ttys[tty_num].needs_redraw = 1;
    ttys[tty_num].initialized = 1;
    
    console_clear();
    
    if (new_mode == TTY_MODE_GAME) {
        console_show_cursor(0);
    }
}

void tty_restore_to_shell(void) {
    int tty_num = current_tty;
    
    ttys[tty_num].mode = TTY_MODE_SHELL;
    ttys[tty_num].update_func = tty_backup[tty_num].update_func;
    ttys[tty_num].draw_func = tty_backup[tty_num].draw_func;
    ttys[tty_num].char_input_func = tty_backup[tty_num].char_input_func;
    ttys[tty_num].special_input_func = tty_backup[tty_num].special_input_func;
    ttys[tty_num].needs_redraw = 1;
    ttys[tty_num].initialized = 1;
    
    for (int i = 0; i < 256; i++) {
        ttys[tty_num].command_buffer[i] = tty_backup[tty_num].command_buffer[i];
    }
    ttys[tty_num].buffer_index = tty_backup[tty_num].buffer_index;
    ttys[tty_num].cursor_position = tty_backup[tty_num].cursor_position;
    
    console_show_cursor(1);
    console_clear();
    
    shell_print_prompt();
    
    if (ttys[tty_num].buffer_index > 0) {
        console_write(ttys[tty_num].command_buffer);
    }
}
