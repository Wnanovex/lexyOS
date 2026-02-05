#include <ui/fb_console.h>
#include <ui/font/font8x8.h>

static struct framebuffer* fb;
static uint32_t cx, cy;
static uint32_t font_scale = 1;

static uint8_t fg_r = 255, fg_g = 255, fg_b = 255;
static uint8_t bg_r = 0, bg_g = 0, bg_b = 0;

static int cursor_visible = 1;

static void draw_glyph(uint32_t x, uint32_t y, const uint8_t* g) {
    if (!fb || !g) return;  // Null pointer check
    
    for (uint32_t r = 0; r < FONT_H; r++) {
        for (uint32_t col = 0; col < FONT_W; col++) {
            int is_foreground = (g[r] & (1 << col));
            
            for (uint32_t sy = 0; sy < font_scale; sy++) {
                for (uint32_t sx = 0; sx < font_scale; sx++) {
                    uint32_t px = x + (col * font_scale) + sx;
                    uint32_t py = y + (r * font_scale) + sy;
                    
                    if (is_foreground)
                        fb_put_pixel(fb, px, py, fg_r, fg_g, fg_b);
                    else
                        fb_put_pixel(fb, px, py, bg_r, bg_g, bg_b);
                }
            }
        }
    }
}

static void draw_char(uint32_t x, uint32_t y, char c) {
    // Bounds check for font array access
    unsigned char uc = (unsigned char)c;
    if (uc > 127) {
        c = '?';  // Replace invalid characters with '?'
    }
    const uint8_t* g = font8x8_basic[uc];
    draw_glyph(x, y, g);
}

// Clear a character cell (draw background)
static void clear_char_at(uint32_t x, uint32_t y) {
    if (!fb) return;  // Null pointer check
    
    uint32_t scaled_font_w = FONT_W * font_scale;
    uint32_t scaled_font_h = FONT_H * font_scale;
    
    for (uint32_t py = 0; py < scaled_font_h; py++) {
        for (uint32_t px = 0; px < scaled_font_w; px++) {
            fb_put_pixel(fb, x + px, y + py, bg_r, bg_g, bg_b);
        }
    }
}

// Draw cursor (underscore)
static void draw_cursor(void) {
    if (!cursor_visible || !fb) return;
    
    uint32_t scaled_font_w = FONT_W * font_scale;
    uint32_t scaled_font_h = FONT_H * font_scale;
    
    // Draw underscore at bottom of character cell
    for (uint32_t px = 0; px < scaled_font_w; px++) {
        for (uint32_t py = scaled_font_h - font_scale; py < scaled_font_h; py++) {
            fb_put_pixel(fb, cx + px, cy + py, fg_r, fg_g, fg_b);
        }
    }
}

// Erase cursor
static void erase_cursor(void) {
    if (!fb) return;  // Null pointer check
    
    uint32_t scaled_font_w = FONT_W * font_scale;
    uint32_t scaled_font_h = FONT_H * font_scale;
    
    // Erase underscore
    for (uint32_t px = 0; px < scaled_font_w; px++) {
        for (uint32_t py = scaled_font_h - font_scale; py < scaled_font_h; py++) {
            fb_put_pixel(fb, cx + px, cy + py, bg_r, bg_g, bg_b);
        }
    }
}

void fb_console_init(struct framebuffer* framebuffer) {
    fb = framebuffer;
    cx = cy = 0;
    font_scale = 1;
    cursor_visible = 1;
    
    fg_r = 255; fg_g = 255; fg_b = 255;
    bg_r = 0;   bg_g = 0;   bg_b = 0;
}

void fb_console_clear(void) {
    if (!fb) return;  // Null pointer check
    fb_clear(fb, bg_r, bg_g, bg_b);
    cx = cy = 0;
}

void fb_console_set_scale(uint32_t scale) {
    if (scale >= 1 && scale <= 8) {
        font_scale = scale;
    }
}

void fb_console_set_fg_color(uint8_t r, uint8_t g, uint8_t b) {
    fg_r = r;
    fg_g = g;
    fg_b = b;
}

void fb_console_set_bg_color(uint8_t r, uint8_t g, uint8_t b) {
    bg_r = r;
    bg_g = g;
    bg_b = b;
}

void fb_console_scroll(void) {
    if (!fb) return;  // Null pointer check
    
    uint32_t scaled_font_h = FONT_H * font_scale;
    
    // Copy screen up by one line
    for (uint32_t y = scaled_font_h; y < fb->height; y++) {
        for (uint32_t x = 0; x < fb->width; x++) {
            // Read pixel from current line
            volatile uint32_t* src_row = 
                (volatile uint32_t*)((uintptr_t)fb->addr + y * fb->pitch);
            volatile uint32_t* dst_row = 
                (volatile uint32_t*)((uintptr_t)fb->addr + (y - scaled_font_h) * fb->pitch);
            
            dst_row[x] = src_row[x];
        }
    }
    
    // Clear last line
    uint32_t last_line_y = fb->height - scaled_font_h;
    for (uint32_t y = last_line_y; y < fb->height; y++) {
        for (uint32_t x = 0; x < fb->width; x++) {
            fb_put_pixel(fb, x, y, bg_r, bg_g, bg_b);
        }
    }
    
    // Move cursor to last line
    cy = last_line_y;
}

void fb_console_backspace(void) {
    if (!fb) return;  // Null pointer check
    
    uint32_t scaled_font_w = FONT_W * font_scale;
    
    if (cx >= scaled_font_w) {
        // Move cursor back
        erase_cursor();
        cx -= scaled_font_w;
        
        // Clear the character at current position
        clear_char_at(cx, cy);
        
        // Redraw cursor
        draw_cursor();
    }
}

void fb_console_show_cursor(int show) {
    if (show && !cursor_visible) {
        cursor_visible = 1;
        draw_cursor();
    } else if (!show && cursor_visible) {
        erase_cursor();
        cursor_visible = 0;
    }
}

void fb_console_get_cursor_pos(uint32_t* x, uint32_t* y) {
    if (x) *x = cx;
    if (y) *y = cy;
}

void fb_console_set_cursor_pos(uint32_t x, uint32_t y) {
    if (!fb) return;  // Null pointer check
    
    erase_cursor();
    cx = x;
    cy = y;
    draw_cursor();
}

void fb_console_putchar(char c) {
    if (!fb) return;  // Null pointer check
    
    uint32_t scaled_font_w = FONT_W * font_scale;
    uint32_t scaled_font_h = FONT_H * font_scale;
    
    // Erase cursor before drawing
    erase_cursor();
    
    // Handle newline
    if (c == '\n') {
        cx = 0;
        cy += scaled_font_h;
        
        // Scroll if needed
        if (cy + scaled_font_h > fb->height) {
            fb_console_scroll();
        }
        
        draw_cursor();
        return;
    }
    
    // Handle carriage return
    if (c == '\r') {
        cx = 0;
        draw_cursor();
        return;
    }
    
    // Skip other non-printable characters
    if (c < 32 || c > 126) {
        draw_cursor();
        return;
    }
    
    // Draw character
    draw_char(cx, cy, c);
    cx += scaled_font_w;

    // Wrap to next line if needed
    if (cx + scaled_font_w > fb->width) {
        cx = 0;
        cy += scaled_font_h;
        
        // Scroll if needed
        if (cy + scaled_font_h > fb->height) {
            fb_console_scroll();
        }
    }
    
    // Redraw cursor at new position
    draw_cursor();
}

// UTF-8 decoder
static uint32_t decode_utf8(const char** str) {
    if (!str || !*str) return 0;  // Null pointer check
    
    const unsigned char* s = (const unsigned char*)*str;
    uint32_t codepoint = 0;
    
    if (s[0] == 0) {
        return 0;
    }
    
    if (s[0] < 0x80) {
        codepoint = s[0];
        (*str) += 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        codepoint = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        (*str) += 2;
    } else if ((s[0] & 0xF0) == 0xE0) {
        codepoint = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        (*str) += 3;
    } else if ((s[0] & 0xF8) == 0xF0) {
        codepoint = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) | 
                    ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        (*str) += 4;
    } else {
        (*str) += 1;
        return 0xFFFD;
    }
    
    return codepoint;
}

void fb_console_write(const char* str) {
    if (!str || !fb) return;  // Null pointer check
    
    const char* p = str;
    uint32_t scaled_font_w = FONT_W * font_scale;
    uint32_t scaled_font_h = FONT_H * font_scale;
    
    erase_cursor();
    
    while (*p) {
        uint32_t codepoint = decode_utf8(&p);
        
        if (codepoint == 0) break;
        
        // Handle newline
        if (codepoint == '\n') {
            cx = 0;
            cy += scaled_font_h;
            if (cy + scaled_font_h > fb->height) {
                fb_console_scroll();
            }
            continue;
        }
        
        // Handle carriage return
        if (codepoint == '\r') {
            cx = 0;
            continue;
        }
        
        // Handle Box Drawing characters (U+2500 to U+257F)
        if (codepoint >= 0x2500 && codepoint <= 0x257F) {
            uint32_t box_index = codepoint - 0x2500;
            // Bounds check
            if (box_index < 128) {  // Assuming font8x8_box has 128 entries
                const uint8_t* g = font8x8_box[box_index];
                draw_glyph(cx, cy, g);
                
                cx += scaled_font_w;
                
                if (cx + scaled_font_w > fb->width) {
                    cx = 0;
                    cy += scaled_font_h;
                    if (cy + scaled_font_h > fb->height) {
                        fb_console_scroll();
                    }
                }
            }
        }
        // Handle Braille characters (U+2800 to U+28FF)
        else if (codepoint >= 0x2800 && codepoint <= 0x28FF) {
            uint32_t braille_index = codepoint - 0x2800;
            // Bounds check
            if (braille_index < 256) {  // Assuming font8x8_braille has 256 entries
                const uint8_t* g = font8x8_braille[braille_index];
                draw_glyph(cx, cy, g);
                
                cx += scaled_font_w;
                
                if (cx + scaled_font_w > fb->width) {
                    cx = 0;
                    cy += scaled_font_h;
                    if (cy + scaled_font_h > fb->height) {
                        fb_console_scroll();
                    }
                }
            }
        }
        // Handle ASCII characters
        else if (codepoint >= 32 && codepoint <= 126) {
            draw_char(cx, cy, (char)codepoint);
            cx += scaled_font_w;
            
            if (cx + scaled_font_w > fb->width) {
                cx = 0;
                cy += scaled_font_h;
                if (cy + scaled_font_h > fb->height) {
                    fb_console_scroll();
                }
            }
        }
    }
    
    draw_cursor();
}

static uint32_t saved_cx = 0;
static uint32_t saved_cy = 0;

void fb_console_move_cursor_left(void) {
    uint32_t scaled_font_w = FONT_W * font_scale;
    
    if (cx >= scaled_font_w) {
        erase_cursor();
        cx -= scaled_font_w;
        draw_cursor();
    }
}

void fb_console_move_cursor_right(void) {
    if (!fb) return;  // Null pointer check
    
    uint32_t scaled_font_w = FONT_W * font_scale;
    
    if (cx + scaled_font_w < fb->width) {
        erase_cursor();
        cx += scaled_font_w;
        draw_cursor();
    }
}

void fb_console_save_cursor_pos(void) {
    saved_cx = cx;
    saved_cy = cy;
}

void fb_console_restore_cursor_pos(void) {
    erase_cursor();
    cx = saved_cx;
    cy = saved_cy;
    draw_cursor();
}

void fb_console_delete_char_at_cursor(void) {
    clear_char_at(cx, cy);
}

void fb_console_insert_char_at_cursor(char c) {
    erase_cursor();
    draw_char(cx, cy, c);
    draw_cursor();
}
