#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct framebuffer {
    uintptr_t addr;
    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch;
    uint8_t   bpp;

    uint8_t r_pos;
    uint8_t r_size;
    uint8_t g_pos;
    uint8_t g_size;
    uint8_t b_pos;
    uint8_t b_size;
};

void fb_init(struct framebuffer* out, void* multiboot_info);

void fb_put_pixel(struct framebuffer* fb,
                  uint32_t x, uint32_t y,
                  uint8_t r, uint8_t g, uint8_t b);

void fb_clear(struct framebuffer* fb,
              uint8_t r, uint8_t g, uint8_t b);

void fb_fill_rect(struct framebuffer* fb,
                  uint32_t x, uint32_t y,
                  uint32_t w, uint32_t h,
                  uint8_t r, uint8_t g, uint8_t b);

struct framebuffer* fb_get_current(void);
bool fb_is_initialized(void);

