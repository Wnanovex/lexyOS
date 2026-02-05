#pragma once
#include <stdint.h>

#define FONT_W 8
#define FONT_H 8

/* font8x8_basic[ASCII][ROW] 
 * Each character is 8 bytes, one byte per row.
 * Bit 0 is leftmost pixel, bit 7 is rightmost pixel.
 */
extern const uint8_t font8x8_basic[128][8];

/* Braille patterns U+2800 to U+28FF */
extern const uint8_t font8x8_braille[256][8];

/* Box Drawing characters U+2500 to U+257F */
extern const uint8_t font8x8_box[128][8];
