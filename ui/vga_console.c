#include <ui/console.h>
#include <ui/vga_console.h>

void vga_console_init(void) {
    terminal_initialize();
}

void vga_console_clear(void) {
    terminal_clear();    
}

void vga_console_setcolor(uint8_t color) {
    terminal_setcolor(color);
}

void vga_console_putchar(char c) {
    terminal_putchar(c);
}

void vga_console_writestring(const char* data) {
    terminal_writestring(data);
}

uint8_t vga_consol_entry_color(enum vga_color fg, enum vga_color bg) {
    return vga_entry_color(fg, bg);
}


