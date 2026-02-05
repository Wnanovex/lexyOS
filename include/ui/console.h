#pragma once
#include <stdint.h>
#include <stddef.h>
#include <drivers/video/framebuffer.h>

typedef enum {
    CONSOLE_COLOR_PRESET_CLASSIC,
    CONSOLE_COLOR_PRESET_MATRIX,
    CONSOLE_COLOR_PRESET_AMBER,
    CONSOLE_COLOR_PRESET_CYAN,
    CONSOLE_COLOR_PRESET_PAPER,
    CONSOLE_COLOR_PRESET_BLUE_SCREEN,
    CONSOLE_COLOR_PRESET_HACKER,
    CONSOLE_COLOR_PRESET_SUNSET,
    CONSOLE_COLOR_PRESET_RED,
} console_color_preset_t;

void console_init(struct framebuffer* fb);

void console_clear(void);
void console_putchar(char c);
void console_write(const char* str);

void console_set_fg_color(uint8_t r, uint8_t g, uint8_t b);
void console_set_bg_color(uint8_t r, uint8_t g, uint8_t b);
void console_set_colors(uint8_t fg_r, uint8_t fg_g, uint8_t fg_b, 
                       uint8_t bg_r, uint8_t bg_g, uint8_t bg_b);
void console_set_color_preset(console_color_preset_t preset);

void console_set_scale(uint32_t scale);


void console_write_dec(uint32_t num);
void console_write_hex64(uint64_t num);
void console_write_hex32(uint32_t num);


// Shell support functions
void console_backspace(void);
void console_show_cursor(int show);
void console_get_cursor_pos(uint32_t* x, uint32_t* y);
void console_set_cursor_pos(uint32_t x, uint32_t y);

// NEW: Advanced cursor control
void console_move_cursor_left(void);
void console_move_cursor_right(void);
void console_save_cursor_pos(void);
void console_restore_cursor_pos(void);
void console_delete_char_at_cursor(void);
void console_insert_char_at_cursor(char c);
