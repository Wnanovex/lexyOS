#include <drivers/video/vga_text.h>

static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0;

// Port I/O
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

void terminal_enable_cursor(void) {
    outb(0x3D4, 0x0A);  // Cursor start register
    outb(0x3D5, 0x00);  // Start from line 0
    outb(0x3D4, 0x0B);  // Cursor end register
    outb(0x3D5, 0x0F);  // End at line 15
}

void terminal_update_cursor(void) {
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;
    
    outb(0x3D4, 0x0F);  // Low byte
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);  // High byte
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void terminal_clear(void) {
    terminal_row = 0;
    terminal_column = 0;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_BUFFER[index] = vga_entry(' ', terminal_color);
        }
    }

    terminal_update_cursor();
}


void terminal_initialize(void) {
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    terminal_clear();  

    terminal_enable_cursor();
    terminal_update_cursor();
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    VGA_BUFFER[index] = vga_entry(c, color);
}

void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[y * VGA_WIDTH + x] = VGA_BUFFER[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();              // terminal_row = 0
            terminal_row = VGA_HEIGHT - 1; 
        }
        terminal_update_cursor();
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);  
    if (++terminal_column == VGA_WIDTH) {  
        terminal_column = 0;  
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();             // terminal_row = 0
            terminal_row = VGA_HEIGHT - 1;  
        }
    }
    
    terminal_update_cursor();
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

void terminal_backspace(void) {
    if (terminal_column > 0) {
        terminal_column--;
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        VGA_BUFFER[index] = vga_entry(' ', terminal_color);
        terminal_update_cursor(); 
    }
}
