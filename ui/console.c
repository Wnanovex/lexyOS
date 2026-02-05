#include <ui/console.h>
#include <ui/fb_console.h>

void console_init(struct framebuffer* fb) {
    if (!fb) return; 
    fb_console_init(fb);
}

void console_clear(void) {
    fb_console_clear();
}

void console_putchar(char c) {
    fb_console_putchar(c);
}

void console_write(const char* str) {
    if (!str) return;  
    fb_console_write(str);
}

void console_write_dec(uint32_t num) {
    if (num == 0) {
        console_putchar('0');
        return;
    }
    
    char buffer[16];
    int i = 0;
    
    while (num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    
    for (int j = i - 1; j >= 0; j--) {
        console_putchar(buffer[j]);
    }
}

void console_write_hex64(uint64_t num) {
    const char* hex_digits = "0123456789ABCDEF";
    
    console_write("0x");
    
    for (int i = 15; i >= 0; i--) {
        uint8_t digit = (num >> (i * 4)) & 0xF;
        console_putchar(hex_digits[digit]);
    }
}

void console_write_hex32(uint32_t num) {
    const char* hex_digits = "0123456789ABCDEF";
    
    console_write("0x");
    
    for (int i = 7; i >= 0; i--) {
        uint8_t digit = (num >> (i * 4)) & 0xF;
        console_putchar(hex_digits[digit]);
    }
}


void console_set_fg_color(uint8_t r, uint8_t g, uint8_t b) {
    fb_console_set_fg_color(r, g, b);
}

void console_set_bg_color(uint8_t r, uint8_t g, uint8_t b) {
    fb_console_set_bg_color(r, g, b);
}

void console_set_colors(uint8_t fg_r, uint8_t fg_g, uint8_t fg_b, 
                       uint8_t bg_r, uint8_t bg_g, uint8_t bg_b) {
    fb_console_set_fg_color(fg_r, fg_g, fg_b);
    fb_console_set_bg_color(bg_r, bg_g, bg_b);
}

void console_set_color_preset(console_color_preset_t preset) {
    switch (preset) {
        case CONSOLE_COLOR_PRESET_CLASSIC:
            console_set_colors(255, 255, 255, 0, 0, 0);
            break;
        case CONSOLE_COLOR_PRESET_MATRIX:
            console_set_colors(0, 255, 0, 0, 0, 0);
            break;
        case CONSOLE_COLOR_PRESET_AMBER:
            console_set_colors(255, 191, 0, 0, 0, 0);
            break;
        case CONSOLE_COLOR_PRESET_CYAN:
            console_set_colors(0, 255, 255, 0, 0, 0);
            break;
        case CONSOLE_COLOR_PRESET_PAPER:
            console_set_colors(0, 0, 0, 255, 255, 255);
            break;
        case CONSOLE_COLOR_PRESET_BLUE_SCREEN:
            console_set_colors(255, 255, 255, 0, 0, 170);
            break;
        case CONSOLE_COLOR_PRESET_HACKER:
            console_set_colors(0, 255, 0, 0, 20, 0);
            break;
        case CONSOLE_COLOR_PRESET_SUNSET:
            console_set_colors(255, 140, 0, 75, 0, 130);
            break;
        case CONSOLE_COLOR_PRESET_RED:
            console_set_colors(255, 0, 0, 0, 0, 0);
            break;
    }
}

void console_set_scale(uint32_t scale) {
    fb_console_set_scale(scale);
}

void console_backspace(void) {
    fb_console_backspace();
}

void console_show_cursor(int show) {
    fb_console_show_cursor(show);
}

void console_get_cursor_pos(uint32_t* x, uint32_t* y) {
    fb_console_get_cursor_pos(x, y);
}

void console_set_cursor_pos(uint32_t x, uint32_t y) {
    fb_console_set_cursor_pos(x, y);
}

void console_move_cursor_left(void) {
    fb_console_move_cursor_left();
}

void console_move_cursor_right(void) {
    fb_console_move_cursor_right();
}

void console_save_cursor_pos(void) {
    fb_console_save_cursor_pos();
}

void console_restore_cursor_pos(void) {
    fb_console_restore_cursor_pos();
}

void console_delete_char_at_cursor(void) {
    fb_console_delete_char_at_cursor();
}

void console_insert_char_at_cursor(char c) {
    fb_console_insert_char_at_cursor(c);
}
