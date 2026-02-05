#pragma once
#include <drivers/video/framebuffer.h>
#include <stdint.h>

void fb_console_init(struct framebuffer* fb);
void fb_console_clear(void);
void fb_console_putchar(char c);
void fb_console_write(const char* str);
void fb_console_set_scale(uint32_t scale);
void fb_console_set_fg_color(uint8_t r, uint8_t g, uint8_t b);
void fb_console_set_bg_color(uint8_t r, uint8_t g, uint8_t b);

void fb_console_backspace(void);           // Handle backspace
void fb_console_scroll(void);              // Scroll screen up
void fb_console_show_cursor(int show);     // Show/hide cursor
void fb_console_get_cursor_pos(uint32_t* x, uint32_t* y);  // Get cursor position
void fb_console_set_cursor_pos(uint32_t x, uint32_t y);    // Set cursor position

void fb_console_move_cursor_left(void);
void fb_console_move_cursor_right(void);
void fb_console_save_cursor_pos(void);
void fb_console_restore_cursor_pos(void);
void fb_console_delete_char_at_cursor(void);
void fb_console_insert_char_at_cursor(char c);
