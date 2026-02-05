#include <drivers/video/framebuffer.h>
#include <multiboot/multiboot2.h>

typedef volatile uint32_t vuint32_t;

static bool fb_initialized = false;
static struct framebuffer* current_fb = NULL;

static inline uint32_t fb_make_color(
    struct framebuffer* fb,
    uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t red   = ((uint32_t)r * ((1u << fb->r_size) - 1) / 255) << fb->r_pos;
    uint32_t green = ((uint32_t)g * ((1u << fb->g_size) - 1) / 255) << fb->g_pos;
    uint32_t blue  = ((uint32_t)b * ((1u << fb->b_size) - 1) / 255) << fb->b_pos;
    return red | green | blue;
}

void fb_init(struct framebuffer* out, void* multiboot_info)
{
    if (!out || !multiboot_info)
        return;

    uint8_t* mb = (uint8_t*)multiboot_info;
    uint32_t total_size = *(uint32_t*)mb;

    struct multiboot_tag* tag = (struct multiboot_tag*)(mb + 8);
    uint8_t* end = mb + total_size;

    while ((uint8_t*)tag < end && tag->type != MULTIBOOT_TAG_TYPE_END) {

        if (tag->size < sizeof(struct multiboot_tag))
            break;

        if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
            struct multiboot_tag_framebuffer* fb_tag =
                (struct multiboot_tag_framebuffer*)tag;

            if (fb_tag->framebuffer_type != 1 || fb_tag->framebuffer_bpp != 32)
                break;

            out->addr   = (uintptr_t)fb_tag->framebuffer_addr;
            out->width  = fb_tag->framebuffer_width;
            out->height = fb_tag->framebuffer_height;
            out->pitch  = fb_tag->framebuffer_pitch;
            out->bpp    = fb_tag->framebuffer_bpp;

            out->r_pos  = fb_tag->red_field_position;
            out->r_size = fb_tag->red_mask_size;
            out->g_pos  = fb_tag->green_field_position;
            out->g_size = fb_tag->green_mask_size;
            out->b_pos  = fb_tag->blue_field_position;
            out->b_size = fb_tag->blue_mask_size;

            current_fb = out;
            fb_initialized = true;
            return;
        }

        uint32_t aligned = (tag->size + 7) & ~7u;
        tag = (struct multiboot_tag*)((uint8_t*)tag + aligned);
    }
}

void fb_put_pixel(struct framebuffer* fb,
                  uint32_t x, uint32_t y,
                  uint8_t r, uint8_t g, uint8_t b)
{
    if (!fb_initialized || !fb)
        return;

    if (x >= fb->width || y >= fb->height)
        return;

    vuint32_t* row =
        (vuint32_t*)(fb->addr + (uintptr_t)y * fb->pitch);

    row[x] = fb_make_color(fb, r, g, b);
}

void fb_clear(struct framebuffer* fb,
              uint8_t r, uint8_t g, uint8_t b)
{
    if (!fb_initialized || !fb)
        return;

    uint32_t color = fb_make_color(fb, r, g, b);
    vuint32_t* ptr = (vuint32_t*)fb->addr;

    uint32_t pixels = (fb->pitch / 4) * fb->height;
    for (uint32_t i = 0; i < pixels; i++)
        ptr[i] = color;
}

void fb_fill_rect(struct framebuffer* fb,
                  uint32_t x, uint32_t y,
                  uint32_t w, uint32_t h,
                  uint8_t r, uint8_t g, uint8_t b)
{
    if (!fb_initialized || !fb)
        return;

    uint32_t color = fb_make_color(fb, r, g, b);

    for (uint32_t dy = 0; dy < h && y + dy < fb->height; dy++) {
        vuint32_t* row =
            (vuint32_t*)(fb->addr + (uintptr_t)(y + dy) * fb->pitch);

        for (uint32_t dx = 0; dx < w && x + dx < fb->width; dx++)
            row[x + dx] = color;
    }
}

struct framebuffer* fb_get_current(void)
{
    return current_fb;
}

bool fb_is_initialized(void)
{
    return fb_initialized;
}

