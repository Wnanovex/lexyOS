#pragma once
#include <drivers/video/vga_text.h>

void vga_console_init(void);

void vga_console_clear(void);

void vga_console_setcolor(uint8_t color);

void vga_console_putchar(char c);

void vga_console_writestring(const char* data);

uint8_t vga_consol_entry_color(enum vga_color fg, enum vga_color bg);



